#include <avr/interrupt.h>
#include <avr/io.h>

volatile uint16_t currentCycle = 374;
volatile uint8_t currentSequence = 0;
volatile uint8_t sequences[] = {
	0b00000001, 
	0b00000010, 
	0b00000100, 
	0b00001000, 
};
/*
volatile uint8_t sequences[] = {
	0b00000001, 
	0b00000011, 
	0b00000010, 
	0b00000110, 
	0b00000100, 
	0b00001100, 
	0b00001000, 
	0b00001001
};
*/

void setTimer() {	
	// Sets time parameters.
	TCCR1A |= 0b00000000;			// Sets timer to CTC mode.
	TCCR1B |= 0b00001100;			// Sets pre-scaler to 256.
	TCNT1 = 0;
	
	// Sets interrupts.
	TIMSK1 |= (1<<OCIE1A);			// Enables interrupt on compare A match.
	TIFR1 |= (1<<OCF1A);			// Clears interrupt flag.
	sei();							// Enables global interrupt.
}

void setDelay(int cycles) {
	// Updates the delay.
	OCR1A = cycles;					
}

/*
void delay() {
	TCNT1 = 1;						// Sets TCNT1 to 1.
	while (TCNT1 != 0);				// Waits until TCNT1 is 0 (OCR1A value reached).
}
*/

void enableADC() {
	// Sets PINC0 as input.
	DDRC &= ~(1<<PINC0);
	
	// Sets up ADC settings.
	ADMUX = 0x40;					// Reads from ADC0. Right justified. Internal 1.1V reference.
	ADCSRB = 0x00;					// Free running mode.
	ADCSRA = 0b10000111;			// Enables ADC with 128 pre-scaler.
}

void disableADC() {
	ADCSRA &= 0x7F;					// Disables the ADEN bit.
}

float analogRead() {
	ADCSRA |= (1<<6);				// Enables ADSC to start AD conversion.
	while (!(ADCSRA&(1<<4)));		// Waits until ADIF is set indicating ADC is done.
	ADCSRA |= (1<<4);				// Clears ADIF flag.
	return ADC;						// Returns value.
}

uint16_t getCycles() {
	// Calculates cycles. Minimum delay is 100ms and maximum is 1s.
	uint16_t cycles = 374 + (3749-374)*analogRead()/1023;
	return cycles;
}

void setPorts() {
	DDRB = 0x0F;					// Enables PINB[3:0] to control the motor.
	PORTB = sequences[currentSequence];
}

int main(void)
{
	setPorts();
    enableADC();
	setDelay(currentCycle);
	setTimer();
	
    while (1) {
		// Calculates cycles based on ADC value.
		uint16_t cycles = getCycles();
		// If cycles is different than the current cycle, update the delay.
		if (cycles <= currentCycle - 15 || cycles >= currentCycle + 15) {
			currentCycle = cycles;
			setDelay(currentCycle);
		}
    }
}

// Occurs when CTC value is reached.
ISR(TIMER1_COMPA_vect) {
	// Updates the current sequence.
	currentSequence = (currentSequence + 1) % 4;
	// Sets the pins to the current sequence based on the table.
	PORTB = sequences[currentSequence];
	// Clears the interrupt flag.
	TIFR1 |= (1<<OCF1A);	
}