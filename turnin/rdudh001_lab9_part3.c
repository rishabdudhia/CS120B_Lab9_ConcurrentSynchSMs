/*	Author: Rishab Dudhia (rdudh001)
 *  Partner(s) Name: 
 *	Lab Section: 022
 *	Assignment: Lab #9  Exercise #3
 *	Exercise Description: [optional - include for your own benefit]
 *	One LED at a time: B0, B1, B2, repeat (300ms); Blink B3 on and off (1000ms); when A2 button pressed speaker connected to B4 plays on for 2ms then off for 2ms; combine in last sm
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0; // TimerISR() sets to 1

unsigned long _avr_timer_M = 1; //start count from here, down to 0. default 1 ms
unsigned long _avr_timer_cntcurr = 0; //current internal count of 1ms ticks

void TimerOn () {
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B; //bit3 = 0: CTC mode (clear timer on compare)
		       //bit2bit1bit0 = 011: pre-scaler / 64
		       //00001011: 0x0B
		       //so, 8 MHz clock or 8,000,000 / 64 = 125,000 ticks/s
		       //thus, TCNT1 register will count at 125,000 ticks/s
	//AVR output compare register OCR1A
	OCR1A = 125; //timer interrupt will be generated when TCNT1 == OCR1A
	             //we want a 1 ms tick. 0.001s * 125,000 ticks/s = 125
		     //so when TCNT1 register equals 125,
		     //1 ms has passed. thus, we compare 125.

	//AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 10000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0 = 000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

//In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect){
	//CPU automatically calls when TCNT1 == OCR1 (every 1ms per TimerOn settings)
	_avr_timer_cntcurr--; // count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

//Set TimerISR() to tick every M ms
void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

static unsigned char threeLEDs;
enum TL_States {smstart1, run} tl_state;

void ThreeLEDsSM () {
	switch (tl_state) {
		case smstart1:
			threeLEDs = 0x01;
			tl_state = run;
			break;
		case run:
			if (threeLEDs == 0x04)
				threeLEDs = 0x01;
			else
				threeLEDs = threeLEDs << 1;
			tl_state = run;
			break;
	}

	switch (tl_state) {
		case smstart1:
		case run:
			break;
	}
}

static unsigned char blinkingLED;
enum BL_States {smstart2, go} bl_state;

void BlinkingLEDSM () {
	switch (bl_state) {
		case smstart2:
			blinkingLED = 0x08;
			bl_state = go;
			break;
		case go:
			if (blinkingLED == 0x08)
				blinkingLED = 0x00;
			else
				blinkingLED = 0x08;
			bl_state = go;
			break;
	}

	switch (bl_state) {
		case smstart2:
		case go:
			break;
	}
}

unsigned char sound = 0x00;
enum S_States {smstart3, wait, on1, on2, off1, off2} s_state;

void SoundSM() {
	unsigned char input = ~PINA & 0x04;//FIXME negation
	
	switch (s_state) {
		case smstart3:
			s_state = wait;
			sound = 0x00;
			break;
		case wait:
			if (input == 0x00)
				s_state = wait;
			else
				s_state = on1;
			break;
		case on1:
			if (input == 0x00)
                                s_state = wait;
                        else
                                s_state = on2;
                        break;
		case on2:
			if (input == 0x00)
                                s_state = wait;
                        else
                                s_state = off1;
                        break;
		case off1:
			if (input == 0x00)
                                s_state = wait;
                        else
                                s_state = off2;
                        break;
		case off2:
			if (input == 0x00)
                                s_state = wait;
                        else
                                s_state = on1;
                        break;
	}

	switch (s_state) {
		case smstart3:
			break;
		case wait:
		case off1:
		case off2:
			sound = 0x00;
			break;
		case on1:
		case on2:
			sound = 0x10;
			break;
	}
}


void CombineLEDsSM() {
	unsigned char temp = sound | blinkingLED | threeLEDs;
	PORTB = temp;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    tl_state = smstart1;
    bl_state = smstart2;
    s_state = smstart3;
    sound = 0x00;
    static unsigned long tl_elapsed = 300;
    static unsigned long bl_elapsed = 1000;
    static unsigned long s_elapsed = 2;
    //static unsigned long ct_elapsed = 1;
    TimerSet(1);
    TimerOn();
    /* Insert your solution below */
    while (1) {
	if (tl_elapsed >= 300) {
		ThreeLEDsSM();
		tl_elapsed = 0;
	}
	if (bl_elapsed >= 1000) {
		BlinkingLEDSM();
		bl_elapsed = 0;
	}
	if (s_elapsed >= 2) {
		SoundSM();
		s_elapsed = 0;
	}
	//if (ct_elapsed >= 1) {
		CombineLEDsSM();
		//ct_elapsed = 0;
	//}
	while (!TimerFlag) {}
	TimerFlag = 0;
	tl_elapsed += 1;
	bl_elapsed += 1;
	s_elapsed += 1;
	//ct_elapsed += 1;
    }
    return 1;
}
