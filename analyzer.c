/*
 * AVR Logic Analyzer
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
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include <stdio.h>
#include <util/delay.h>

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

#define LED_DDR DDRD
#define LED_PORT PORTD
#define LED_P PD2
#define LED_INIT sbi(LED_DDR, LED_P)
#define LED_ON sbi(LED_PORT, LED_P)
#define LED_OFF cbi(LED_PORT, LED_P)
#define LED_FLIP LED_PORT ^= _BV(LED_P)

#define BTN_DDR DDRD
#define BTN_PORT PORTD
#define BTN_PIN PIND
#define BTN_PCINT_vect PCINT2_vect
#define BTN_PCINT PCINT19
#define BTN_PCMSK PCMSK2
#define BTN_PCIE PCIE2
#define BTN_P PD3

#define OVERHEAD 3 // timer clock ticks

#define INPUT_DDR DDRC
#define INPUT_PORT PORTC
#define INPUT_PIN PINC
#define INPUT_NUM 2
const uint8_t inputs[] = {PC0, PC1};
uint8_t inputs_cur[INPUT_NUM];
uint8_t inputs_start[INPUT_NUM];

volatile uint8_t state = 0; // 0 = waiting, 1 = running, 2 = send data, 3 = stop

#define SAMPLE_INPUT(arg) (INPUT_PIN & _BV(inputs[arg]) ? 1 : 0)

volatile uint16_t tmx = 0; // high word
#define tm TCNT1 // low word
uint32_t tm_cur;

#define DATALEN 1500
uint8_t data[DATALEN];
uint16_t dptr = 0;

ISR(TIMER1_OVF_vect) {
	tmx++;
}

void initialize(void) {
	state = 0;
	for(uint8_t i = 0; i < INPUT_NUM; ++i) {
		cbi(INPUT_DDR, inputs[i]); // set as input
		sbi(INPUT_PORT, inputs[i]); // enable pull-up
	}
	_delay_ms(1);
	for(uint8_t i = 0; i < INPUT_NUM; ++i) {
		inputs_cur[i] = SAMPLE_INPUT(i);
		inputs_start[i] = inputs_cur[i];
	}
}

inline void reset_timer() {
	tm = OVERHEAD;
	tmx = 0;
}

inline void start(void) {
	//reset_timer();
	tm_cur = 0;
	state = 1;
}

inline uint8_t sample_inputs(void) {
	uint8_t val;
	for(uint8_t i = 0; i < INPUT_NUM; ++i) {
		val = SAMPLE_INPUT(i);
		if( val != inputs_cur[i] ) {
			tm_cur = ((uint32_t)tmx << 16) | tm;
			reset_timer();
			inputs_cur[i] = val;
			return i | (val << 3);
		}
	}
	return 0xff; // no change
}

inline uint8_t write_event(uint8_t inchange) {
	uint8_t tmlen;
	if( tm_cur > 0xffffff ) {
		tmlen = 3;
	} else if( tm_cur > 0xffff ) {
		tmlen = 2;
	} else if( tm_cur > 0xff ) {
		tmlen = 1;
	} else {
		tmlen = 0;
	}
	if( dptr + tmlen + 2 >= DATALEN ) {
		//printf("full at %d\n", dptr);
		state = 2;
		return 0;
	}
	data[dptr++] = inchange | (tmlen << 4);
	for( uint8_t i = 0; i <= tmlen; i++ ) {
		data[dptr++] = tm_cur >> (i * 8);
	}
	//printf("w %hhx %hhx %lu %u\n", inchange | (tmlen << 4), tmlen, tm_cur, dptr);

	return 1;
}

void send_data(void) {
	uint16_t dptr2 = 0;
	uint8_t tmp, inp, val, tmlen;
	uint32_t t;

	puts("begin");

	for( uint8_t i = 0; i < INPUT_NUM; ++i ) {
		printf("-: %hhu = %hhu\n", i, inputs_start[i]);
	}

	while( dptr2 < dptr ) {
		tmp = data[dptr2++];
		tmlen = tmp >> 4;
		val = (tmp >> 3) & 1;
		inp = tmp & 0x07;
		t = 0;
		for( uint8_t i = 0; i <= tmlen; ++i ) {
			t |= ((uint32_t)data[dptr2++]) << (i * 8);
		}
		printf("%lu: %hhu = %hhu\n", t, inp, val, dptr2);
	}
	puts("end\n");
}

ISR(BTN_PCINT_vect) {
	if( BTN_PIN & _BV(BTN_P) && state >= 1 ) {
		state = 2;
	}
}

int main(void) {
	uint8_t tmp;

	// init 16-bit Timer1, clk/8 prescaler
	sbi(TCCR1B, CS11);
	sbi(TIMSK1, TOIE1);

	// configure pin change interrupt for button
	BTN_DDR &= ~_BV(BTN_P);
	BTN_PORT |= _BV(BTN_P);
	BTN_PCMSK |= _BV(BTN_PCINT);
	PCICR |= _BV(BTN_PCIE);

	uart_init();
	stdout = &uart_io;
	stdin = &uart_io;

	puts("hello");

	sei();

	LED_INIT;
	LED_ON;

	initialize();
	while(1) {
		if( state == 0 || state == 1 ) {
			// waiting for trigger or running
			if( (tmp = sample_inputs()) != 0xff ) {
				// input changed
				if( state == 0 ) {
					start();
					LED_OFF;
				}
				write_event(tmp);
			}
		} else if( state == 2 ) {
			// finished running, send data
			LED_ON;
			send_data();
			state = 3; // stop
		}
	}
	return 1;
}
