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


#include "config.h"
#include <util/setbaud.h>
#include <avr/io.h>
#include <stdio.h>

#define CONVERT_CRLF 1

int uart_getchar(FILE *stream);
int uart_putchar(char c, FILE *stream);

FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

void uart_init(void) {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

#if USE_2X
	UCSR0A |= _BV(U2X0);
#else
	UCSR0A &= ~_BV(U2X0);
#endif
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
}

int uart_putchar(char c, FILE *stream) {
#ifdef CONVERT_CRLF
	if( c == '\n' ) {
		uart_putchar('\r', stream);
	}
#endif
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return (int)c;
}

int uart_getchar(FILE *stream) {
	unsigned char c;
	loop_until_bit_is_set(UCSR0A, RXC0);
	c = UDR0;
#ifdef CONVERT_CRLF
	if( c == '\r' ) {
		c = '\n';
	}
#endif
	return (int)c;
}
