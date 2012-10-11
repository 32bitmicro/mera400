//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdio.h>
#include <arpa/inet.h>
#include "em400_debuger.h"
#include "mjc400.h"
#include "em400_mem.h"
#include "mjc400_regs.h"
#include "mjc400_iset.h"
#include "mjc400_instr.h"
#include "mjc400_timer.h"

// -----------------------------------------------------------------------
void mjc400_reset()
{
	IC = 0;
	IR = 0;
	SR = 0;
	for (int i=0 ; i<16 ; i++) {
		R[i] = 0;
	}
	RZ = 0;
	P = 0;
	MOD = 0;
	MODcnt = 0;
	KB = 0;
}

// -----------------------------------------------------------------------
int16_t mjc400_get_eff_arg()
{
	uint32_t N;

	// argument is in next word
	if (IR_C == 0) {
		N = MEM(IC);
		IC++;
	// argument is in field C
	} else {
		N = R[IR_C];
	}

	// B-modification
	N += R[IR_B];

	// PRE-modification
	N += MOD;
	
	// if D is set, N is an address in current memory block
	if (IR_D == 1) {
		N = MEM((uint16_t) N);
	}

	// store 17th bit for byte addressing
	ZC17 = (N & 0b10000000000000000) >> 16;

	return (int16_t) N;
}

// -----------------------------------------------------------------------
void mjc400_step()
{
	// do not branch by default
	P = 0;

	// fetch instruction into IR
	// (additional argument is fetched by the instruction, if necessary)
	IR = MEM(IC);
	IC++;

	// execute instruction
	int op_res;
	op_res = mjc400_iset[IR_OP].op_fun();

	switch (op_res) {
		// normal instruction
		case OP_OK:
			MOD = 0;
			MODcnt = 0;
			break;
		// pre-modification
		case OP_MD:
			break;
		// illegal instruction
		case OP_ILLEGAL:
			MOD = MODcnt = 0;
			if (P!=0) {
				P = 0;
			} else {
				RZ_6sb;
			}
			break;
	}

	IC += P;
}

// vim: tabstop=4
