/*
 * Task03C.c
 *
 * Created: 3/6/2020 2:00:25 AM
 * Author : Owner
 */ 

#define cycles_412500us 6445
#define cycles_337500us 5273
#define cycles_2s 31250

#include <avr/interrupt.h>
#include <avr/io.h>

volatile unsigned int passes = 0;

int main(void)
{
	// Sets PORB3 as an output.
	DDRB = (0x01<<PORTB2 | 0x01<<PORTB3);
	PORTB |= (0x01<<PORTB2);		// Makes sure LED is off.
	
	// Sets timer overflow interrupt.
	TIFR0 = 0x02;					// Clears the OCF0A flag.
	TIMSK0 = 0x02;					// Enabled OCIE0A (compare A interrupt).
	OCR0A = 128;					// Sets compare A to 128.
	sei();							// Enables global interrupt.
	
	// Sets timer.
	TCCR0A = 0b00000010;			// Sets timer 0 to CTC mode.
	TCCR0B = 0b00000101;			// Sets pre-scaler to 1024.
	TCNT0 = 0x00;					// Starts the counter at 0.
	
	// Generates the wave.
	while (1) {
		// Waits for PINC3 to go low (the button is active high).
		if ((PINC & (0x01<<PINC3)) == 0x00) {
			PORTB &= ~(0x01<<PORTB2);	// Turns on the LED by allowing current to sink.
			delay(cycles_2s);
			PORTB |= (0x01<<PORTB2);	// Turns off the LED.
		}
		
		PORTB |= (0x01<<PORTB3);	// Sets PORTB3 to high.
		delay(cycles_412500us);		// Waits the 55% duty cycle.
		PORTB &= ~(0x01<<PORTB3);	// Clears PORTB3.
		delay(cycles_337500us);		// Waits the remaining 45% of the period.
	}
}

// Creates a delay given a certain amount of cycles.
void delay(unsigned int cycles) {
	// Calculates passes and remaining cycles needed.
	unsigned int rPasses = cycles/129;
	unsigned int rCycles = cycles%129;
	
	// Resets timer.
	TCNT0 = 0x00;
	passes = 0;
	
	// Do nothing until the passes reaches the required passes.
	while (passes < rPasses) {
	}
	
	// Sets the timer to overflow after remaining cycles needed passes.
	TCNT0 = (128-rCycles);
	passes = 0;
	
	// Waits for the final overflow.
	while (passes < 1) {
	}
}

ISR(TIMER0_COMPA_vect) {
	TIFR0 = 0x02;					// Clears the interrupt flag.
	passes++;
}


