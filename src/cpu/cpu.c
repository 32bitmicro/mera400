//  Copyright (c) 2012-2018 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#define _XOPEN_SOURCE 600
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include <emawp.h>

#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"
#include "cpu/interrupts.h"
#include "cpu/clock.h"
#ifdef HAVE_SOUND
#include "cpu/buzzer.h"
#endif
#include "io/io.h"

#include "em400.h"
#include "cfg.h"
#include "utils/utils.h"
#include "log.h"
#include "log_crk.h"
#include "ectl/brk.h"

#include "ectl.h" // for global constants

static int cpu_state = ECTL_STATE_OFF;
uint16_t regs[8];
uint16_t rIC;
uint16_t rKB;
int rALARM;
uint16_t rMOD;
int rMODc;
uint16_t rIR;
unsigned RM, Q, BS, NB;

int P;
uint32_t N;
int cpu_mod_active;

struct timespec cpu_timer;
unsigned long ips_counter;
static int speed_real;
static int throttle_granularity;
static float cpu_delay_factor;
pthread_t idler_th;
struct timespec idle_timer;

int cpu_mod_present;
int cpu_user_io_illegal;
static int nomem_stop;

#ifdef HAVE_SOUND
static int sound_enabled;
#endif

struct awp *awp;

// opcode table (instruction decoder decision table)
struct iset_opcode *cpu_op_tab[0x10000];

pthread_mutex_t cpu_wake_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cpu_wake_cond = PTHREAD_COND_INITIALIZER;

// -----------------------------------------------------------------------
static void cpu_idle_in_wait()
{
	pthread_mutex_lock(&cpu_wake_mutex);
	while ((cpu_state == ECTL_STATE_WAIT) && !atom_load_acquire(&RP)) {
		LOG(L_CPU, "idling in state WAIT");
		pthread_cond_wait(&cpu_wake_cond, &cpu_wake_mutex);
	}
	cpu_state &= ~ECTL_STATE_WAIT;
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
static int cpu_idle_in_stop()
{
	pthread_mutex_lock(&cpu_wake_mutex);
	while ((cpu_state & (ECTL_STATE_STOP|ECTL_STATE_OFF|ECTL_STATE_CLO|ECTL_STATE_CLM|ECTL_STATE_CYCLE)) == ECTL_STATE_STOP) {
		LOG(L_CPU, "idling in state STOP");
		pthread_cond_wait(&cpu_wake_cond, &cpu_wake_mutex);
	}
	int ret = cpu_state;
	pthread_mutex_unlock(&cpu_wake_mutex);
	return ret;
}

// -----------------------------------------------------------------------
void cpu_trigger_state(int state)
{
	pthread_mutex_lock(&cpu_wake_mutex);
	cpu_state |= state;
	pthread_cond_broadcast(&cpu_wake_cond);
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
void cpu_clear_state(int state)
{
	pthread_mutex_lock(&cpu_wake_mutex);
	cpu_state &= ~state;
	pthread_cond_broadcast(&cpu_wake_cond);
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
int cpu_state_get()
{
	return atom_load_acquire(&cpu_state);
}

// -----------------------------------------------------------------------
void cpu_trigger_cycle()
{
	pthread_mutex_lock(&cpu_wake_mutex);
	if (cpu_state & ECTL_STATE_STOP) {
		cpu_state |= ECTL_STATE_CYCLE;
		pthread_cond_broadcast(&cpu_wake_cond);
	}
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
void cpu_mem_fail(int nb)
{
	int_set(INT_NO_MEM);
	if ((nb == 0) && nomem_stop) {
		rALARM = 1;
		cpu_trigger_state(ECTL_STATE_STOP);
	}
}

// -----------------------------------------------------------------------
int cpu_mem_get(int nb, uint16_t addr, uint16_t *data)
{
	if (!mem_get(nb, addr, data)) {
		cpu_mem_fail(nb);
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int cpu_mem_put(int nb, uint16_t addr, uint16_t data)
{
	if (!mem_put(nb, addr, data)) {
		cpu_mem_fail(nb);
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int cpu_mem_mget(int nb, uint16_t saddr, uint16_t *dest, int count)
{
	int words;
	if ((words = mem_mget(nb, saddr, dest, count)) != count) {
		cpu_mem_fail(nb);
	}
	return words;
}

// -----------------------------------------------------------------------
int cpu_mem_mput(int nb, uint16_t saddr, uint16_t *src, int count)
{
	int words;
	if ((words = mem_mput(nb, saddr, src, count)) != count) {
		cpu_mem_fail(nb);
	}
	return words;
}

// -----------------------------------------------------------------------
int cpu_mem_get_byte(int nb, uint32_t addr, uint8_t *data)
{
	int shift;
	uint16_t orig = 0;

	if (!cpu_mod_active || (Q & BS)) addr &= 0xffff;

	shift = 8 * (~addr & 1);
	addr >>= 1;

	if (!cpu_mem_get(nb, addr, &orig)) return 0;
	*data = orig >> shift;

	return 1;
}

// -----------------------------------------------------------------------
int cpu_mem_put_byte(int nb, uint32_t addr, uint8_t data)
{
	int shift;
	uint16_t orig = 0;

	if (!cpu_mod_active || (Q & BS)) addr &= 0xffff;

	shift = 8 * (~addr & 1);
	addr >>= 1;

	if (!cpu_mem_get(nb, addr, &orig)) return 0;
	orig = (orig & (0b1111111100000000 >> shift)) | (((uint16_t) data) << shift);
	if (!cpu_mem_put(nb, addr, orig)) return 0;

	return 1;
}

// -----------------------------------------------------------------------
#ifdef HAVE_SOUND
static void * idler_thread(void *ptr)
{
	int state;

	while (1) {
		pthread_mutex_lock(&cpu_wake_mutex);
		while ((cpu_state & (ECTL_STATE_STOP|ECTL_STATE_WAIT)) == 0) {
			pthread_cond_wait(&cpu_wake_cond, &cpu_wake_mutex);
		}
		pthread_mutex_unlock(&cpu_wake_mutex);

		do {
			idle_timer.tv_nsec += 4000;
			if (idle_timer.tv_nsec > 1000000000) {
				idle_timer.tv_nsec -= 1000000000;
				idle_timer.tv_sec++;
			}
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &idle_timer, NULL);

			state = atom_load_acquire(&cpu_state);
			if ((state & (ECTL_STATE_STOP|ECTL_STATE_WAIT))) {
				buzzer_update(-1, 4000);
			}
		} while ((state & (ECTL_STATE_STOP|ECTL_STATE_WAIT)));
	}

	pthread_exit(NULL);
}
#endif

// -----------------------------------------------------------------------
int cpu_init(struct cfg_em400 *cfg)
{
	int res;

	if (cfg->cpu_awp) {
		awp = awp_init(regs+0, regs+1, regs+2, regs+3);
		if (!awp) return LOGERR("Failed to initialize AWP.");
	}

	rKB = cfg->keys;

	cpu_mod_present = cfg->cpu_mod;
	cpu_user_io_illegal = cfg->cpu_user_io_illegal;
	nomem_stop = cfg->cpu_stop_on_nomem;
	speed_real = cfg->speed_real;
	throttle_granularity = cfg->throttle_granularity;
	cpu_delay_factor = 1.0f/cfg->cpu_speed_factor;

	res = iset_build(cpu_op_tab, cpu_user_io_illegal);
	if (res != E_OK) {
		return LOGERR("Failed to build CPU instruction table.");
	}

	int_update_mask(0);

	// this is checked only at power-on
	if (mem_mega_boot()) {
		LOG(L_CPU, "Bootstrap from MEGA PROM is enabled.");
		rIC = 0xf000;
	} else {
		rIC = 0;
	}

	cpu_mod_off();

#ifdef HAVE_SOUND
	sound_enabled = cfg->sound_enabled;
	if (sound_enabled) {
		if (buzzer_init(cfg) != E_OK) {
			return LOGERR("Failed to initialize buzzer.");
		}
		if (pthread_create(&idler_th, NULL, idler_thread, NULL)) {
			return LOGERR("Failed to spawn CPU idler thread.");
		}
		pthread_setname_np(idler_th, "idler");
	}
#endif

	return E_OK;
}

// -----------------------------------------------------------------------
void cpu_shutdown()
{
	awp_destroy(awp);
}

// -----------------------------------------------------------------------
int cpu_mod_on()
{
	cpu_mod_active = 1;
	clock_set_int(INT_EXTRA);

	return E_OK;
}

// -----------------------------------------------------------------------
int cpu_mod_off()
{
	cpu_mod_active = 0;
	clock_set_int(INT_CLOCK);

	return E_OK;
}

// -----------------------------------------------------------------------
static void cpu_clear(int scope)
{
	cpu_clear_state(scope | ECTL_STATE_WAIT);

	// I/O reset should return when we're sure that I/O won't change CPU state (backlogged interrupts, memory writes, ...)
	io_reset();
	mem_reset();
	cpu_mod_off();

	regs[0] = 0;
	SR_write(0);

	int_update_mask(RM);
	int_clear_all();

	if (scope & ECTL_STATE_CLO) {
		rALARM = 0;
		rMOD = 0;
		rMODc = 0;
		cpu_trigger_state(ECTL_STATE_STOP);
	}

	// call even if logging is disabled - user may enable it later
	// and we still want to know if we're running a known OS
	log_check_os();
	log_reset_process();
	log_intlevel_reset();
	log_syscall_reset();
}

// -----------------------------------------------------------------------
int cpu_ctx_switch(uint16_t arg, uint16_t ic, uint16_t int_mask)
{
	uint16_t sp;

	if (!cpu_mem_get(0, 97, &sp)) return 0;

	LOG(L_CPU, "Store current process ctx [IC: 0x%04x, R0: 0x%04x, SR: 0x%04x, 0x%04x] @ 0x%04x, set new IC: 0x%04x", rIC, regs[0], SR_read(), arg, sp, ic);


	if (!cpu_mem_put(0, sp, rIC)) return 0;
	if (!cpu_mem_put(0, sp+1, regs[0])) return 0;
	if (!cpu_mem_put(0, sp+2, SR_read())) return 0;
	if (!cpu_mem_put(0, sp+3, arg)) return 0;
	if (!cpu_mem_put(0, 97, sp+4)) return 0;

	regs[0] = 0;
	rIC = ic;
	Q = 0;
	RM &= int_mask;

	int_update_mask(RM);

	return 1;
}

// -----------------------------------------------------------------------
void cpu_ctx_restore()
{
	uint16_t sr;
	uint16_t sp;

	if (!cpu_mem_get(0, 97, &sp)) return;
	if (!cpu_mem_get(0, sp-4, &rIC)) return;
	if (!cpu_mem_get(0, sp-3, regs)) return;
	if (!cpu_mem_get(0, sp-2, &sr)) return;
	SR_write(sr);
	int_update_mask(RM);
	if (!cpu_mem_put(0, 97, sp-4)) return;

	LOG(L_CPU, "Loaded process ctx @ 0x%04x: [IC: 0x%04x, R0: 0x%04x, SR: 0x%04x]", sp-4, rIC, regs[0], sr);

	return;
}

// -----------------------------------------------------------------------
static int cpu_do_cycle()
{
	struct iset_opcode *op;
	uint16_t data;
	char opcode[32];
	int instruction_time = 0;

	if (LOG_WANTS(L_CPU)) {
		log_store_cycle_state(SR_read(), rIC);
	}

	// fetch instruction
	if (!cpu_mem_get(QNB, rIC, &rIR)) {
		LOGCPU(L_CPU, "        no mem, instruction fetch");
		goto ineffective;
	}
	op = cpu_op_tab[rIR];
	rIC++;

	// check instruction effectivness
	if (P || ((regs[0] & op->jmp_nef_mask) != op->jmp_nef_result)) {
		if (LOG_WANTS(L_CPU)) {
			log_log_dasm(0, 0, "skip: ");
		}
		if ((op->flags & OP_FL_ARG_NORM) && !IR_C) rIC++;
		goto ineffective;
	} else if (op->flags & OP_FL_ILLEGAL) {
		int2binf(opcode, "... ... . ... ... ...", rIR, 16);
		LOGCPU(L_CPU, "    illegal: %s (0x%04x)", opcode, rIR);
		int_set(INT_ILLEGAL_INSTRUCTION);
		goto ineffective;
	} else if (Q && (op->flags & OP_FL_USR_ILLEGAL)) {
		if (LOG_WANTS(L_CPU)) {
			log_log_dasm(0, 0, "illegal: ");
		}
		int_set(INT_ILLEGAL_INSTRUCTION);
		goto ineffective;
	}

	// prepare argument
	if ((op->flags & OP_FL_ARG_NORM)) {
		if (IR_C) {
			N = regs[IR_C] + rMOD;
		} else {
			if (!cpu_mem_get(QNB, rIC, &data)) {
				LOGCPU(L_CPU, "    no mem, long arg fetch @ %i:0x%04x", QNB, (uint16_t) rIC);
				goto ineffective;
			} else {
				N = data + rMOD;
				rIC++;
			}
			if (speed_real) instruction_time += TIME_MEM;
		}
		if (IR_B) {
			N = (uint16_t) N + regs[IR_B];
			if (speed_real) instruction_time += TIME_BMOD;
		}
		if (IR_D) {
			if (!cpu_mem_get(QNB, N, &data)) {
				LOGCPU(L_CPU, "    no mem, indirect arg fetch @ %i:0x%04x", QNB, (uint16_t) N);
				goto ineffective;
			} else {
				N = data;
			}
			if (speed_real) instruction_time += TIME_DMOD;
		}
	} else if ((op->flags & OP_FL_ARG_SHORT)) {
		N = (uint16_t) IR_T + (uint16_t) rMOD;
	}

	if (rMODc) {
		if (speed_real) instruction_time += TIME_PREMOD;
	}

	if (LOG_WANTS(L_CPU)) {
		log_log_dasm((op->flags & (OP_FL_ARG_NORM | OP_FL_ARG_SHORT)), N, "");
	}

	if (speed_real) {
		instruction_time += op->time;
		if (op->fun == op_72_shc) instruction_time += IR_t * TIME_SHIFT;
	}

	// execute instruction
	op->fun();

	// clear mod if instruction wasn't md
	if (op->fun != op_77_md) {
		rMODc = rMOD = 0;
	}
	return instruction_time;

ineffective:
	if (speed_real) instruction_time += TIME_MEM + TIME_INEFFECTIVE;
	P = 0;
	rMOD = 0;
	rMODc = 0;
	return instruction_time;
}

// -----------------------------------------------------------------------
void cpu_loop()
{
	pthread_mutex_lock(&cpu_wake_mutex);
	cpu_state = ECTL_STATE_STOP;
	pthread_mutex_unlock(&cpu_wake_mutex);
	cpu_trigger_state(0);

	int cpu_time;
	int cpu_time_cumulative = 0;

	clock_gettime(CLOCK_MONOTONIC, &cpu_timer);

	while (1) {
		int state = atom_load_acquire(&cpu_state);

		// CPU running
		if (state == 0) {
cycle:
			// interrupt
			if (atom_load_acquire(&RP) && !P && !rMODc) {
				int_serve();
			// CPU cycle
			} else {
				cpu_time = cpu_do_cycle();
				ips_counter++;

				if (speed_real) {
					cpu_time *= cpu_delay_factor;
#ifdef HAVE_SOUND
					if (sound_enabled) {
						buzzer_update(rIR, cpu_time);
					}
#endif
					cpu_time_cumulative += cpu_time;
					if ((ips_counter % throttle_granularity) == 0) {
						cpu_timer.tv_nsec += cpu_time_cumulative;
						if (cpu_timer.tv_nsec > 1000000000) {
							cpu_timer.tv_nsec -= 1000000000;
							cpu_timer.tv_sec++;
						}
						cpu_time_cumulative = 0;
						clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &cpu_timer, NULL);
					}
				}

				// breakpoint hit?
				if (ectl_brk_check()) {
					cpu_trigger_state(ECTL_STATE_STOP | ECTL_STATE_BRK);
				}
			}

		// quit emulation
		} else if ((state & ECTL_STATE_OFF)) {
			break;

		// CPU reset
		} else if (state & (ECTL_STATE_CLM | ECTL_STATE_CLO)) {
			cpu_clear(state & (ECTL_STATE_CLM | ECTL_STATE_CLO));

		// CPU stopped
		} else if ((state & ECTL_STATE_STOP)) {
			idle_timer.tv_sec = cpu_timer.tv_sec;
			idle_timer.tv_nsec = cpu_timer.tv_nsec;
			if ((cpu_idle_in_stop() & ECTL_STATE_CYCLE)) {
				// CPU cycle triggered while in stop
				cpu_clear_state(ECTL_STATE_CYCLE);
				cpu_timer.tv_sec = idle_timer.tv_sec;
				cpu_timer.tv_nsec = idle_timer.tv_nsec;
				goto cycle;
			} else {
				cpu_timer.tv_sec = idle_timer.tv_sec;
				cpu_timer.tv_nsec = idle_timer.tv_nsec;
			}

		// CPU waiting for an interrupt
		} else if ((state & ECTL_STATE_WAIT)) {
			idle_timer.tv_sec = cpu_timer.tv_sec;
			idle_timer.tv_nsec = cpu_timer.tv_nsec;
			cpu_idle_in_wait();
			cpu_timer.tv_sec = idle_timer.tv_sec;
			cpu_timer.tv_nsec = idle_timer.tv_nsec;
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
