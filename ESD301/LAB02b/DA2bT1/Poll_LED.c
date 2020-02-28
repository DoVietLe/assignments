/*
 * Assignment_2a_C.c
 *
 * Created: 2/23/2020 10:16:39 PM
 * Author : Owner
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>	// Includes the interrupt file.

int main(void)
{
	// Sets up the interrupt.
	EICRA = 0x02;			// Sets the interrupt to occur on falling edges.
	EIMSK = 0x01;			// Enables INT0 interrupt.
	sei();
	
	// Sets up the ports.
    DDRC = 0x00;			// Sets PORTC3 as input port.
	PORTC = (0x01<<PORTC3);	// Activates the pull up resistor on PORTC3.
	DDRB = (0x01<<PORTB2 | 0x01<<PORTB3);	// Sets PORTB2 as an output port.
	PORTB = (0x01<<PORTB2);	// Output 1 on PORTB2 (Turns off LED since
							// LED is connected to Vcc).
	
    while (1) 
    {
		// Generates the waveform.
		PORTB |= (0x01<<PORTB3);		// Turns PORTB3 high for 55% duty cycle.
		_delay_ms(412.5);
		PORTB = ~(0x01<<PORTB3);		// Turns PORTB3 low for 45% of the period.
		_delay_ms(337.5);
    }
}

ISR(INT0_vect) {
	PORTB &= ~(0x01<<PORTB2);
	_delay_ms(2000);
	PORTB |= (0x01<<PORTB2);
}