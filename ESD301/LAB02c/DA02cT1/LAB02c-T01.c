/*
 * Task01C_Part2.c
 *
 * Created: 3/4/2020 2:09:19 AM
 * Author : Owner
 */ 

#define cycles_412500us 6446
#define cycles_337500us 5274
#define cycles_2s 31251

#include <avr/io.h>

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
			delay(cycles_2s);
			PORTB |= (0x01<<PORTB2);	// Turns off the LED.
		}
		
		// Generates the waveform.
		PORTB |= (0x01<<PORTB3);		// Turns PORTB3 high for 55% duty cycle.
		delay(cycles_412500us);
		PORTB = ~(0x01<<PORTB3);		// Turns PORTB3 low for 45% of the period.
		delay(cycles_337500us);
	}
}

// Runs delay given an amount of cycles (unsigned int .
void delay(unsigned int cycles) {
	// Sets up timer 0.
	TCCR0A = 0x00;						// Sets timer0 to normal mode.
	TCCR0B = 0x05;						// Sets timer0 prescaler to 1024.
	
	// Keeps delaying until the amount of cycles needed is less than 255.
	while (cycles >= 255) {
		cycles -= 255;
		TCNT0 = 0x00;
		while (1)
			if (TCNT0 == 255)
				break;
	}

	// Delays the last few cycles.
	TCNT0 = 0x00;
	while (1)
		if (TCNT0 == cycles)
			break;
}

