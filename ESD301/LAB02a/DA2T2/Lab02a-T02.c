/*
 * Assignment_2a_C.c
 *
 * Created: 2/23/2020 10:16:39 PM
 * Author : Owner
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRC = 0x00;			// Sets PORTC3 as input port.
	PORTC = (0x01<<PORTC3);	// Activates the pull up resistor on PORTC3.
	DDRB = (0x01<<PORTB2 | 0x01<<PORTB3);	// Sets PORTB2 as an output port.
	PORTB = (0x01<<PORTB2);	// Output 1 on PORTB2 (Turns off LED since
							// LED is connected to Vcc).
	
    while (1) 
    {
		// Waits for PINC3 to go low (the button is active high).
		if ((PINC & (0x01<<PINC3)) == 0x00) {
			PORTB &= ~(0x01<<PORTB2);	// Turns on the LED by allowing current to sink.
			_delay_ms(2000);
			PORTB |= (0x01<<PORTB2);	// Turns off the LED.
		}
		
		// Generates the waveform.
		PORTB |= (0x01<<PORTB3);		// Turns PORTB3 high for 55% duty cycle.
		_delay_ms(412.5);
		PORTB = ~(0x01<<PORTB3);		// Turns PORTB3 low for 45% of the period.
		_delay_ms(337.5);
    }
}

