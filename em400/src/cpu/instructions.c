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

#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/interrupts.h"
#include "cpu/alu.h"
#include "cpu/memory.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"
#include "io/io.h"

#include "em400.h"
#include "cfg.h"
#include "utils.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#endif
#include "debugger/log.h"

// -----------------------------------------------------------------------
// ---- 20 - 36 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_lw()
{
	Rw(IR_A, N);
}

// -----------------------------------------------------------------------
void op_tw()
{
	Rw(IR_A, MEMNB(N));
}

// -----------------------------------------------------------------------
void op_ls()
{
	Rw(IR_A, (R(IR_A) & ~R(7)) | ((uint16_t)N & R(7)));
}

// -----------------------------------------------------------------------
void op_ri()
{
	MEMw(R(IR_A), N);
	Rinc(IR_A);
}

// -----------------------------------------------------------------------
void op_rw()
{
	MEMw(N, R(IR_A));
}

// -----------------------------------------------------------------------
void op_pw()
{
	MEMNBw(N, R(IR_A));
}

// -----------------------------------------------------------------------
void op_rj()
{
	Rw(IR_A, R(R_IC));
	Rw(R_IC, N);
}

// -----------------------------------------------------------------------
void op_is()
{
	if ((MEMNB(N) & R(IR_A)) == R(IR_A)) {
		Rw(R_P, 1);
	} else {
		MEMNBw(N, (MEMNB(N) | R(IR_A)));
	}
}

// -----------------------------------------------------------------------
void op_bb()
{
	if ((R(IR_A) & (uint16_t) N) == (uint16_t) N) {
		Rw(R_P, 1);
	}
}

// -----------------------------------------------------------------------
void op_bm()
{
	if ((MEMNB(N) & R(IR_A)) == R(IR_A)) {
		Rw(R_P, 1);
	}
}

// -----------------------------------------------------------------------
void op_bs()
{
	if ((R(IR_A) & R(7)) == ((uint16_t) N & R(7))) {
		Rw(R_P, 1);
	}
}

// -----------------------------------------------------------------------
void op_bc()
{
	if ((R(IR_A) & (uint16_t) N) != (uint16_t) N) {
		Rw(R_P, 1);
	}
}

// -----------------------------------------------------------------------
void op_bn()
{
	if ((R(IR_A) & (uint16_t) N) == 0) {
		Rw(R_P, 1);
	}
}

// -----------------------------------------------------------------------
void op_ou()
{
	int io_result = io_dispatch(IO_OU, N, regs+IR_A);
	Rw(R_IC, MEM(R(R_IC) + io_result));
}

// -----------------------------------------------------------------------
void op_in()
{
	int io_result = io_dispatch(IO_IN, N, regs+IR_A);
	Rw(R_IC, MEM(R(R_IC) + io_result));
}

// -----------------------------------------------------------------------
// ---- 37 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_37()
{
	iset_37[EXT_OP_37(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
void op_37_ad()
{
	alu_add32(1, 2, MEM(N), MEM(N+1), 1);
}

// -----------------------------------------------------------------------
void op_37_sd()
{
	alu_add32(1, 2, MEM(N), MEM(N+1), -1);
}

// -----------------------------------------------------------------------
void op_37_mw()
{
	alu_mul32(1, 2, MEM(N));
}

// -----------------------------------------------------------------------
void op_37_dw()
{
	alu_div32(1, 2, MEM(N));
}

// -----------------------------------------------------------------------
void op_37_af()
{
	// TODO: floats
}

// -----------------------------------------------------------------------
void op_37_sf()
{
	// TODO: floats
}

// -----------------------------------------------------------------------
void op_37_mf()
{
	// TODO: floats
}

// -----------------------------------------------------------------------
void op_37_df()
{
	// TODO: floats
}

// -----------------------------------------------------------------------
// ---- 40 - 57 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_aw()
{
	alu_add16(IR_A, N, 0);
}

// -----------------------------------------------------------------------
void op_ac()
{
	alu_add16(IR_A, N, Fget(FL_C));
}

// -----------------------------------------------------------------------
void op_sw()
{
	alu_add16(IR_A, -N, 0);
}

// -----------------------------------------------------------------------
void op_cw()
{
	alu_compare((int16_t) R(IR_A), N);
}

// -----------------------------------------------------------------------
void op_or()
{
	Rw(IR_A, R(IR_A) | (uint16_t) N);
	alu_set_flag_Z(R(IR_A), 16);
}

// -----------------------------------------------------------------------
void op_om()
{
	MEMNBw(N, MEMNB(N) | R(IR_A));
	alu_set_flag_Z(MEMNB(N), 16);
}

// -----------------------------------------------------------------------
void op_nr()
{
	Rw(IR_A, R(IR_A) & (uint16_t) N);
	alu_set_flag_Z(R(IR_A), 16);
}

// -----------------------------------------------------------------------
void op_nm()
{
	MEMNBw(N, MEMNB(N) & R(IR_A));
	alu_set_flag_Z(MEMNB(N), 16);
}

// -----------------------------------------------------------------------
void op_er()
{
	Rw(IR_A, R(IR_A) & ~(uint16_t) N);
	alu_set_flag_Z(R(IR_A), 16);
}

// -----------------------------------------------------------------------
void op_em()
{
	MEMNBw(N, MEMNB(N) & ~R(IR_A));
	alu_set_flag_Z(MEMNB(N), 16);
}

// -----------------------------------------------------------------------
void op_xr()
{
	Rw(IR_A, R(IR_A) ^ (uint16_t) N);
	alu_set_flag_Z(R(IR_A), 16);
}

// -----------------------------------------------------------------------
void op_xm()
{
	MEMNBw(N, MEMNB(N) ^ R(IR_A));
	alu_set_flag_Z(MEMNB(N), 16);
}

// -----------------------------------------------------------------------
void op_cl()
{
	alu_compare(R(IR_A), (uint16_t) N);
}

// -----------------------------------------------------------------------
void op_lb()
{
	Rw(IR_A, (nR(IR_A) & 0b1111111100000000) | MEMNBb(N));
}

// -----------------------------------------------------------------------
void op_rb()
{
	MEMNBwb(N, R(IR_A));
}

// -----------------------------------------------------------------------
void op_cb()
{
	alu_compare((uint8_t) R(IR_A), MEMNBb(N));
}

// -----------------------------------------------------------------------
// ---- 60 - 67 ----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_awt()
{
	alu_add16(IR_A, N, 0);
}

// -----------------------------------------------------------------------
void op_trb()
{
	Radd(IR_A, N);
	if (!R(IR_A)) {
		Rw(R_P, 1);
	}
}

// -----------------------------------------------------------------------
void op_irb()
{
	Rinc(IR_A);
	if (R(IR_A)) Radd(R_IC, N);
}

// -----------------------------------------------------------------------
void op_drb()
{
	Rdec(IR_A);
	if (R(IR_A)) Radd(R_IC, N);
}

// -----------------------------------------------------------------------
void op_cwt()
{
	alu_compare((int16_t) R(IR_A), N);
}

// -----------------------------------------------------------------------
void op_lwt()
{
	Rw(IR_A, N);
}

// -----------------------------------------------------------------------
void op_lws()
{
	Rw(IR_A, MEM(R(R_IC) + N));
}

// -----------------------------------------------------------------------
void op_rws()
{
	MEMw(R(R_IC) + N, R(IR_A));
}

// -----------------------------------------------------------------------
// ---- 70 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_70()
{
	iset_70[EXT_OP_70(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
void op_70_ujs()
{
	Radd(R_IC, N);
}

// -----------------------------------------------------------------------
void op_70_jls()
{
	if (Fget(FL_L)) Radd(R_IC, N);
}

// -----------------------------------------------------------------------
void op_70_jes()
{
	if (Fget(FL_E)) Radd(R_IC, N);
}

// -----------------------------------------------------------------------
void op_70_jgs()
{
	if (Fget(FL_G)) Radd(R_IC, N);
}

// -----------------------------------------------------------------------
void op_70_jvs()
{
	if (Fget(FL_V)) {
		Radd(R_IC, N);
		Fclr(FL_V);
	}
}

// -----------------------------------------------------------------------
void op_70_jxs()
{
	if (Fget(FL_X)) Radd(R_IC, N);
}

// -----------------------------------------------------------------------
void op_70_jys()
{
	if (Fget(FL_Y)) Radd(R_IC, N);
}

// -----------------------------------------------------------------------
void op_70_jcs()
{
	if (Fget(FL_C)) Radd(R_IC, N);
}

// -----------------------------------------------------------------------
// ---- 71 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_71()
{
	iset_71[EXT_OP_71(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
void op_71_blc()
{
	if (((R(0) >> 8) & IR_b) != IR_b) {
		Rw(R_P, 1);
	}
}

// -----------------------------------------------------------------------
void op_71_exl()
{
	uint16_t SP = nMEMB(0, 97);
	nMEMBw(0, SP, R(R_IC));
	nMEMBw(0, SP+1, R(0));
	nMEMBw(0, SP+2, R(R_SR));
	nMEMBw(0, SP+3, IR_b);
	Rw(R_IC, nMEMB(0, 96));
	reg_write(0, 0, 1, 1);
	nMEMBw(0, 97, SP+4);
	SR_RM9cb;
	int_update_rp();
	SR_Qcb;
}

// -----------------------------------------------------------------------
void op_71_brc()
{
	if ((R(0) & IR_b) != IR_b) {
		Rw(R_P, 1);
	}
}

// -----------------------------------------------------------------------
void op_71_nrf()
{
	// TODO: floats
}

// -----------------------------------------------------------------------
// ---- 72 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_72()
{
	iset_72[EXT_OP_72(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
void op_72_ric()
{
	Rw(IR_A, R(R_IC));
}

// -----------------------------------------------------------------------
void op_72_zlb()
{
	Rw(IR_A, nR(IR_A) & 0b0000000011111111);
}

// -----------------------------------------------------------------------
void op_72_sxu()
{
	if (R(IR_A) & 0b1000000000000000) Fset(FL_X);
	else Fclr(FL_X);
	alu_set_flag_Z(R(IR_A), 16);
}

// -----------------------------------------------------------------------
void op_72_nga()
{
	alu_negate(IR_A, 1);
}

// -----------------------------------------------------------------------
void op_72_slz()
{
	if (R(IR_A) & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	Rw(IR_A, R(IR_A)<<1);
}

// -----------------------------------------------------------------------
void op_72_sly()
{
	uint16_t ir_a = nR(IR_A);
	Rw(IR_A, (R(IR_A)<<1) | Fget(FL_Y));
	if (ir_a & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
}

// -----------------------------------------------------------------------
void op_72_slx()
{
	if (R(IR_A) & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	Rw(IR_A, (R(IR_A)<<1) | Fget(FL_X));
}

// -----------------------------------------------------------------------
void op_72_sry()
{
	uint16_t ir_a = nR(IR_A);
	Rw(IR_A, (R(IR_A)>>1) | Fget(FL_Y)<<15);
	if (ir_a & 1) Fset(FL_Y);
	else Fclr(FL_Y);
}

// -----------------------------------------------------------------------
void op_72_ngl()
{
	alu_negate(IR_A, 0);
}

// -----------------------------------------------------------------------
void op_72_rpc()
{
	Rw(IR_A, R(0));
}

// -----------------------------------------------------------------------
void op_72_shc()
{
	if (!IR_t) return;

	uint16_t falling = (R(IR_A) & ((1<<IR_t)-1)) << (16-IR_t);
	
	Rw(IR_A, (R(IR_A) >> IR_t) | falling);
}

// -----------------------------------------------------------------------
void op_72_rky()
{
	// TODO: does it work that way?
	Rw(IR_A, R(R_KB));
}

// -----------------------------------------------------------------------
void op_72_zrb()
{
	Rw(IR_A, nR(IR_A) & 0b1111111100000000);
}

// -----------------------------------------------------------------------
void op_72_sxl()
{
	if (R(IR_A) & 1) Fset(FL_X);
	else Fclr(FL_X);
	alu_set_flag_Z(R(IR_A), 16);
}

// -----------------------------------------------------------------------
void op_72_ngc()
{
	alu_negate(IR_A, Fget(FL_C));
}

// -----------------------------------------------------------------------
void op_72_svz()
{
	uint16_t ir_a = nR(IR_A);
	Rw(IR_A, R(IR_A)<<1);
	if (ir_a & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	alu_set_flag_V(ir_a, ir_a, R(IR_A), 16);
}

// -----------------------------------------------------------------------
void op_72_svy()
{
	uint16_t ir_a = nR(IR_A);
	Rw(IR_A, R(IR_A)<<1 | Fget(FL_Y));
	if (ir_a & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	alu_set_flag_V(ir_a, ir_a, R(IR_A), 16);
}

// -----------------------------------------------------------------------
void op_72_svx()
{
	uint16_t ir_a = nR(IR_A);
	Rw(IR_A, R(IR_A)<<1 | Fget(FL_X));
	if (ir_a & 0b1000000000000000) Fset(FL_Y);
	else Fclr(FL_Y);
	alu_set_flag_V(ir_a, ir_a, R(IR_A), 16);
}

// -----------------------------------------------------------------------
void op_72_srx()
{
	if (R(IR_A) & 1) Fset(FL_Y);
	else Fclr(FL_Y);
	Rw(IR_A, (R(IR_A)>>1) | Fget(FL_X)<<15);
}

// -----------------------------------------------------------------------
void op_72_srz()
{
	if (R(IR_A) & 1) Fset(FL_Y);
	else Fclr(FL_Y);
	Rw(IR_A, (R(IR_A)>>1) & 0b0111111111111111);
}

// -----------------------------------------------------------------------
void op_72_lpc()
{
	reg_write(0, R(IR_A), 1, 1);
}

// -----------------------------------------------------------------------
// ---- 73 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_73()
{
	iset_73[EXT_OP_73(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
void op_73_hlt()
{
	// handle hlt 077 as "exit emulation" if user wants to
	if (em400_cfg.exit_on_hlt) {
		if (N == 077) {
			em400_quit = 1;
			return;
		}
	}

	// otherwise, wait for interrupt
	LOG(D_CPU, 1, "HALT");
	pthread_mutex_lock(&int_mutex_rp);
	while (!RP) {
		pthread_cond_wait(&int_cond_rp, &int_mutex_rp);
	}
	pthread_mutex_unlock(&int_mutex_rp);
}

// -----------------------------------------------------------------------
void op_73_mcl()
{
	Rw(R_SR, 0);
	int_clear_all();
	reg_write(0, 0, 1, 1);
	mem_remove_maps();
	io_reset();
}

// -----------------------------------------------------------------------
void op_73_cit()
{
	int_clear(INT_SOFT_U);
	int_clear(INT_SOFT_L);
}

// -----------------------------------------------------------------------
void op_73_sil()
{
	int_set(INT_SOFT_L);
}

// -----------------------------------------------------------------------
void op_73_siu()
{
	int_set(INT_SOFT_U);
}

// -----------------------------------------------------------------------
void op_73_sit()
{
	int_set(INT_SOFT_U);
	int_set(INT_SOFT_L);
}

// -----------------------------------------------------------------------
void op_73_giu()
{
	// TODO: 2-cpu configuration
}

// -----------------------------------------------------------------------
void op_73_gil()
{
	// TODO: 2-cpu configuration
}

// -----------------------------------------------------------------------
void op_73_lip()
{
	uint16_t SP = nMEMB(0, 97);
	Rw(R_IC, nMEMB(0, SP-4));
	reg_write(0, nMEMB(0, SP-3), 1, 1);
	Rw(R_SR, nMEMB(0, SP-2));
	int_update_rp();
	nMEMBw(0, 97, SP-4);
#ifdef WITH_DEBUGGER
	dbg_touch_pop(&touch_int);
#endif
}

// -----------------------------------------------------------------------
void op_73_sint()
{
	int_set(INT_EXTRA);
}

// -----------------------------------------------------------------------
void op_73_sind()
{
	int_clear(INT_EXTRA);
}

// -----------------------------------------------------------------------
// ---- 74 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_74()
{
	iset_74[EXT_OP_74(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
void op_74_uj()
{
	Rw(R_IC, N);
}

// -----------------------------------------------------------------------
void op_74_jl()
{
	if (Fget(FL_L)) Rw(R_IC, N);
}

// -----------------------------------------------------------------------
void op_74_je()
{
	if (Fget(FL_E)) Rw(R_IC, N);
}

// -----------------------------------------------------------------------
void op_74_jg()
{
	if (Fget(FL_G)) Rw(R_IC, N);
}

// -----------------------------------------------------------------------
void op_74_jz()
{
	if (Fget(FL_Z)) Rw(R_IC, N);
}

// -----------------------------------------------------------------------
void op_74_jm()
{
	if (Fget(FL_M)) Rw(R_IC, N);
}

// -----------------------------------------------------------------------
void op_74_jn()
{
	if (!Fget(FL_E)) Rw(R_IC, N);
}

// -----------------------------------------------------------------------
void op_74_lj()
{
	MEMw(N, R(R_IC));
	Rw(R_IC, N+1);
}

// -----------------------------------------------------------------------
// ---- 75 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_75()
{
	iset_75[EXT_OP_75(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
void op_75_ld()
{
	Rw(1, MEM(N));
	Rw(2, MEM(N+1));
}

// -----------------------------------------------------------------------
void op_75_lf()
{
	Rw(1, MEM(N));
	Rw(2, MEM(N+1));
	Rw(3, MEM(N+2));
}

// -----------------------------------------------------------------------
void op_75_la()
{
	Rw(1, MEM(N));
	Rw(2, MEM(N+1));
	Rw(3, MEM(N+2));
	Rw(4, MEM(N+3));
	Rw(5, MEM(N+4));
	Rw(6, MEM(N+5));
	Rw(7, MEM(N+6));
}

// -----------------------------------------------------------------------
void op_75_ll()
{
	Rw(5, MEM(N));
	Rw(6, MEM(N+1));
	Rw(7, MEM(N+2));
}

// -----------------------------------------------------------------------
void op_75_td()
{
	Rw(1, MEMNB(N));
	Rw(2, MEMNB(N+1));
}

// -----------------------------------------------------------------------
void op_75_tf()
{
	Rw(1, MEMNB(N));
	Rw(2, MEMNB(N+1));
	Rw(3, MEMNB(N+2));
}

// -----------------------------------------------------------------------
void op_75_ta()
{
	Rw(1, MEMNB(N));
	Rw(2, MEMNB(N+1));
	Rw(3, MEMNB(N+2));
	Rw(4, MEMNB(N+3));
	Rw(5, MEMNB(N+4));
	Rw(6, MEMNB(N+5));
	Rw(7, MEMNB(N+6));
}

// -----------------------------------------------------------------------
void op_75_tl()
{
	Rw(5, MEMNB(N));
	Rw(6, MEMNB(N+1));
	Rw(7, MEMNB(N+2));
}

// -----------------------------------------------------------------------
// ---- 76 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_76()
{
	iset_76[EXT_OP_76(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
void op_76_rd()
{
	MEMw(N, R(1));
	MEMw(N+1, R(2));
}

// -----------------------------------------------------------------------
void op_76_rf()
{
	MEMw(N, R(1));
	MEMw(N+1, R(2));
	MEMw(N+2, R(3));
}

// -----------------------------------------------------------------------
void op_76_ra()
{
	MEMw(N, R(1));
	MEMw(N+1, R(2));
	MEMw(N+2, R(3));
	MEMw(N+3, R(4));
	MEMw(N+4, R(5));
	MEMw(N+5, R(6));
	MEMw(N+6, R(7));
}

// -----------------------------------------------------------------------
void op_76_rl()
{
	MEMw(N, R(5));
	MEMw(N+1, R(6));
	MEMw(N+2, R(7));
}

// -----------------------------------------------------------------------
void op_76_pd()
{
	MEMNBw(N, R(1));
	MEMNBw(N+1, R(2));
}

// -----------------------------------------------------------------------
void op_76_pf()
{
	MEMNBw(N, R(1));
	MEMNBw(N+1, R(2));
	MEMNBw(N+2, R(3));
}

// -----------------------------------------------------------------------
void op_76_pa()
{
	MEMNBw(N, R(1));
	MEMNBw(N+1, R(2));
	MEMNBw(N+2, R(3));
	MEMNBw(N+3, R(4));
	MEMNBw(N+4, R(5));
	MEMNBw(N+5, R(6));
	MEMNBw(N+6, R(7));
}

// -----------------------------------------------------------------------
void op_76_pl()
{
	MEMNBw(N, R(5));
	MEMNBw(N+1, R(6));
	MEMNBw(N+2, R(7));
}

// -----------------------------------------------------------------------
// ---- 77 ---------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void op_77()
{
	iset_77[EXT_OP_77(nR(R_IR))].op_fun();
}

// -----------------------------------------------------------------------
void op_77_mb()
{
	SR_SET_QNB(MEM(N));
}

// -----------------------------------------------------------------------
void op_77_im()
{
	SR_SET_MASK(MEM(N));
	int_update_rp();
}

// -----------------------------------------------------------------------
void op_77_ki()
{
	MEMw(N, int_get_nchan());
}

// -----------------------------------------------------------------------
void op_77_fi()
{
	int_put_nchan(MEM(N));
}

// -----------------------------------------------------------------------
void op_77_sp()
{
	Rw(R_IC, MEMNB(N));
	reg_write(0, MEMNB(N+1), 1, 1);
	Rw(R_SR, MEMNB(N+2));
	int_update_rp();
}

// -----------------------------------------------------------------------
void op_77_md()
{
	Rw(R_MOD, N);
	Rinc(R_MODc);
}

// -----------------------------------------------------------------------
void op_77_rz()
{
	MEMw(N, 0);
}

// -----------------------------------------------------------------------
void op_77_ib()
{
	MEMw(N, MEM(N)+1);
	if (!MEM(N)) {
		Rw(R_P, 1);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
