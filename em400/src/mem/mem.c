//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "mem/mem_elwro.h"
#include "mem/mem_mega.h"
#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "mem/mem.h"
#include "cpu/interrupts.h"
#include "io/io.h"

#include "em400.h"
#include "cfg.h"
#include "errors.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#endif
#include "debugger/log.h"

pthread_spinlock_t mem_spin;

struct mem_slot_t mem_map[MEM_MAX_NB][MEM_MAX_AB];	// final (as seen by emulation) logical->physical segment mapping

#define mem_ptr(nb, addr) (mem_map[nb][(addr) >> 12].seg ? mem_map[nb][(addr) >> 12].seg + ((addr) & 0b0000111111111111) : NULL)

// -----------------------------------------------------------------------
void mem_update_map()
{
	int nb, ab;

	for (nb=0 ; nb<MEM_MAX_NB ; nb++) {
		for (ab=0 ; ab<MEM_MAX_AB ; ab++) {
			pthread_spin_lock(&mem_spin);
			mem_elwro_seg_set(nb, ab, &mem_map[nb][ab]);
			if (!mem_map[nb][ab].seg) {
				mem_mega_seg_set(nb, ab, &mem_map[nb][ab]);
			}
			pthread_spin_unlock(&mem_spin);
		}
	}
}

// -----------------------------------------------------------------------
int mem_init()
{
	int res;

	eprint("Initializing memory (Elwro: %d modules, MEGA: %d modules)\n", em400_cfg.mem_elwro, em400_cfg.mem_mega);

	pthread_spin_init(&mem_spin, 0);

	if (em400_cfg.mem_elwro+em400_cfg.mem_mega > MEM_MAX_MODULES+1) {
		return E_MEM;
	}

	res = mem_elwro_init(em400_cfg.mem_elwro, em400_cfg.mem_os);
	if (res != E_OK) {
		return res;
	}

	res = mem_mega_init(em400_cfg.mem_mega, em400_cfg.mem_mega_prom);
	if (res != E_OK) {
		return res;
	}

	mem_update_map();

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_shutdown()
{
	eprint("Shutdown memory\n");

	mem_mega_shutdown();
	mem_elwro_shutdown();
}

// -----------------------------------------------------------------------
int mem_cmd(uint16_t n, uint16_t r)
{
	int res;
	int nb		= (r & 0b0000000000001111);
	int ab		= (r & 0b1111000000000000) >> 12;
	int mp		= (n & 0b0000000000011110) >> 1;
	int seg		= (n & 0b0000000111100000) >> 5;
	int flags	= (n & 0b1111111000000000) >> 9;

	// if MEGA is present and MEM_MEGA_ALLOC is set => command for MEGA
	if ((em400_cfg.mem_mega > 0) && ((flags & MEM_MEGA_ALLOC))) {
		res = mem_mega_cmd(nb, ab, mp, seg, flags);
	// Elwro otherwise (but mask segment number to 3 bits)
	} else {
		res = mem_elwro_cmd(nb, ab, mp, seg & 0b0111);
	}
	if (res == IO_OK) {
		mem_update_map();
	}
	return res;
}

// -----------------------------------------------------------------------
void mem_reset()
{
	pthread_spin_lock(&mem_spin);
	mem_elwro_reset();
	mem_mega_reset();
	pthread_spin_unlock(&mem_spin);
	mem_update_map();
}

// -----------------------------------------------------------------------
int mem_get(int nb, uint16_t addr, uint16_t *data)
{
	uint16_t *ptr;

	pthread_spin_lock(&mem_spin);
	ptr = mem_ptr(nb, addr);
	if (ptr) {
		*data = *ptr;
		pthread_spin_unlock(&mem_spin);
	} else {
		pthread_spin_unlock(&mem_spin);
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_put(int nb, uint16_t addr, uint16_t data)
{
	uint16_t *ptr;

	pthread_spin_lock(&mem_spin);
	ptr = mem_ptr(nb, addr);
	if (ptr) {
		if (!mem_mega_prom || (mem_map[nb][addr>>12].seg != mem_mega_prom)) {
			*ptr = data;
		}
		pthread_spin_unlock(&mem_spin);
	} else {
		pthread_spin_unlock(&mem_spin);
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_mget(int nb, uint16_t saddr, uint16_t *dest, int count)
{
	int i;
	uint16_t *ptr;

	pthread_spin_lock(&mem_spin);
	for (i=0 ; i<count ; i++) {
		ptr = mem_ptr(nb, (uint16_t) (saddr+i));
		if (ptr) {
			*(dest+i) = *ptr;
		} else {
			pthread_spin_unlock(&mem_spin);
			return 0;
		}
	}
	pthread_spin_unlock(&mem_spin);
	return 1;
}

// -----------------------------------------------------------------------
int mem_mput(int nb, uint16_t saddr, uint16_t *src, int count)
{
	int i;
	uint16_t *ptr;

	pthread_spin_lock(&mem_spin);
	for (i=0 ; i<count ; i++) {
		ptr = mem_ptr(nb, (uint16_t) (saddr+i));
		if (ptr) {
			if (!mem_mega_prom || (mem_map[nb][(saddr+i)>>12].seg != mem_mega_prom)) {
				*ptr = *(src+i);
			}
		} else {
			pthread_spin_unlock(&mem_spin);
			return 0;
		}
	}
	pthread_spin_unlock(&mem_spin);
	return 1;
}

// -----------------------------------------------------------------------
int mem_cpu_get(int nb, uint16_t addr, uint16_t *data)
{
	if (!mem_get(nb, addr, data)) {
		int_set(INT_NO_MEM);
		if (nb == 0) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
			return 0;
		}
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_cpu_put(int nb, uint16_t addr, uint16_t data)
{
	if (!mem_put(nb, addr, data)) {
		int_set(INT_NO_MEM);
		if (nb == 0) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
			return 0;
		}
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_cpu_mget(int nb, uint16_t saddr, uint16_t *dest, int count)
{
	if (!mem_mget(nb, saddr, dest, count)) {
		int_set(INT_NO_MEM);
		if (nb == 0) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
			return 0;
		}
	}
	return 1;
}


// -----------------------------------------------------------------------
int mem_cpu_mput(int nb, uint16_t saddr, uint16_t *src, int count)
{
	if (!mem_mput(nb, saddr, src, count)) {
		int_set(INT_NO_MEM);
		if (nb == 0) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
			return 0;
		}
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_get_byte(int nb, uint16_t addr, uint8_t *data)
{
	int shift;
	uint16_t addr17;
	uint16_t orig = 0;

	shift = 8 * (~addr & 1);

	if (cpu_mod) {
		addr17 = (addr >> 1) | (regs[R_ZC17] << 15);
		regs[R_ZC17] = 0;
	} else {
		addr17 = addr >> 1;
	}

	if (!mem_cpu_get(nb, addr17, &orig)) return 0;
	*data = orig >> shift;

	return 1;
}

// -----------------------------------------------------------------------
int mem_put_byte(int nb, uint16_t addr, uint8_t data)
{
	int shift;
	uint16_t addr17;
	uint16_t orig = 0;

	shift = 8 * (~addr & 1);

	if (cpu_mod) {
		addr17 = (addr >> 1) | (regs[R_ZC17] << 15);
		regs[R_ZC17] = 0;
	} else {
		addr17 = addr >> 1;
	}

	if (!mem_cpu_get(nb, addr17, &orig)) return 0;
	orig = (orig & (0b1111111100000000 >> shift)) | (((uint16_t) data) << shift);
	if (!mem_cpu_put(nb, addr17, orig)) return 0;

	return 1;
}

// -----------------------------------------------------------------------
void mem_clear()
{
	pthread_spin_lock(&mem_spin);
	mem_elwro_clear();
	mem_mega_clear();
	pthread_spin_unlock(&mem_spin);
}

// -----------------------------------------------------------------------
int mem_seg_load(FILE *f, uint16_t *ptr)
{
	int res;

	if (!ptr) {
		return E_MEM_BLOCK_TOO_SMALL;
	}

	res = fread(ptr, sizeof(uint16_t), MEM_SEGMENT_SIZE, f);
	if (ferror(f)) {
		return E_FILE_OPERATION;
	}

	// we swap bytes from big-endian to host-endianness at load time
	pthread_spin_lock(&mem_spin);
	for (int i=0 ; i<res ; i++) {
		*(ptr+i) = ntohs(*(ptr+i));
	}
	pthread_spin_unlock(&mem_spin);
	
	return res;
}
// -----------------------------------------------------------------------
int mem_load(const char* fname, int nb, int start_ab, int len)
{
	int ret = E_OK;
	int loaded = 0;
	int res;
	uint16_t *ptr;
	uint16_t seg = start_ab;

	FILE *f = fopen(fname, "rb");
	if (f == NULL) {
		return E_FILE_OPEN;
	}

	LOG(L_MEM, 1, "Loading memory image: %s -> %d:%d", fname, nb, start_ab);

	do {
		// get pointer to segment in a block
		pthread_spin_lock(&mem_spin);
		ptr = mem_map[nb][seg].seg;
		if (!ptr) {
			ret = E_MEM_BLOCK_TOO_SMALL;
			pthread_spin_unlock(&mem_spin);
			break;
		}
		pthread_spin_unlock(&mem_spin);

		// read chunk of data
		res = mem_seg_load(f, ptr);
		if (res <= 0) {
			break;
		}

		loaded += res;
		seg++;
	} while ((res == MEM_SEGMENT_SIZE) && ((loaded < len) || (len <= 0)));

	fclose(f);

	if (loaded > 0) {
		return loaded;
	} else {
		return ret;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
