/*
 * UART library for AVR
 * Copyright (C) 2013  Michal Belica <devel@beli.sk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#ifndef _UART_H
#define _UART_H
#include <stdio.h>

#if !defined(F_CPU) || !defined(BAUD)
#	error F_CPU and BAUD have to be defined for baud rate calculation.
#endif

void uart_init(void);
char uart_putchar(char c, FILE *stream);
char uart_getchar(FILE *stream);
extern FILE uart_io;
#endif
