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

#ifndef DRV_CHAN_H
#define DRV_CHAN_H

#include <inttypes.h>

// things common for 'classic' mem and char channels

enum chan_cmds_e {
	// channel OU
	CHAN_CMD_EXISTS		= 0b000000,
	CHAN_CMD_MASK_PN	= 0b000010,
	CHAN_CMD_MASK_NPN	= 0b000100,
	CHAN_CMD_ASSIGN		= 0b000110,
	// channel IN
	CHAN_CMD_INTSPEC	= 0b000010,
	CHAN_CMD_ALLOC		= 0b000110,
	CHAN_CMD_STATUS		= 0b000100,
};

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
