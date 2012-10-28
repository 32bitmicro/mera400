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

#ifndef DEBUG_EVAL_H
#define DEBUG_EVAL_H

#include <stdlib.h>
#include <inttypes.h>

enum _node_type {
	N_OPER = 1,
	N_VAL = 2,
	N_REG = 3
};

struct node_t {
	int type;
	int16_t val;
	struct node_t *n1;
	struct node_t *n2;
};

struct node_t * n_val(int16_t v);
struct node_t * n_reg(int r);
struct node_t * n_oper(int oper, struct node_t *n1, struct node_t *n2);
uint16_t n_eval(struct node_t * n);

#endif

// vim: tabstop=4