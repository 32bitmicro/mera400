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

#ifndef OPS_H
#define OPS_H

enum _machine_type_e {
	MERA400 = 0,
	K202 = 1
};

struct kw_t {
	char *mnemo[2];
	int type;
	int opcode;
};

struct var_t {
	int value;
	char *name;
};

extern struct var_t extracodes[];
extern struct kw_t ops[];
extern struct kw_t pragmas[];
extern int mnemo_sel;

struct kw_t * get_op(int set, char *name);
struct kw_t * get_pragma(int set, char *name);
struct kw_t * get_kw(struct kw_t *dict, int set, char *name);
struct var_t * get_pvar(struct var_t *d, char *name);

#endif

// vim: tabstop=4
