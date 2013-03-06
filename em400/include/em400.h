//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef EM400_H
#define EM400_H

extern int em400_quit;
extern unsigned int ips_counter;
extern struct timeval ips_start;
extern struct timeval ips_end;

struct em400_opts_t {
    char *program_name;
	char *config_file;
	int exit_on_hlt;
#ifdef WITH_DEBUGGER
	int autotest;
	char *pre_expr;
    char *test_expr;
    int ui_simple;
#endif
} em400_opts;

#endif

// vim: tabstop=4
