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

#ifndef DRV_DRIVERS_H
#define DRV_DRIVERS_H

#include "io.h"

enum drv_chan_type {
	CHAN_NONE = 0,
	CHAN_CHAR,
	CHAN_MEM,
	CHAN_PI,
	CHAN_MULTIX,
	CHAN_PLIX
};

enum drv_unit_type {
	UNIT_NONE = 0,
	UNIT_9425,
	UNIT_WINCHESTER,
	UNIT_TERM_NET,
	UNIT_TERM_SERIAL
};

struct drv_chan_t {
	int type;
	int (*f_init)(struct chan_t *ch);
	void (*f_shutdown)(struct chan_t *ch);
	void (*f_reset)(struct chan_t *ch);
	int (*f_cmd)(struct chan_t *ch, int dir, int unit, int cmd, int r);
};

struct drv_unit_t {
	int type;
	int chan_type;
	int (*f_init)(struct unit_t *u, int cfgc, char **cfgv);
	void (*f_shutdown)(struct unit_t *u);
	void (*f_reset)(struct unit_t *u);
	int (*f_cmd)(struct unit_t *u, int dir, int cmd, int r);
};

extern struct drv_chan_t drv_chan[];
extern struct drv_unit_t drv_unit[];

#endif

// vim: tabstop=4
