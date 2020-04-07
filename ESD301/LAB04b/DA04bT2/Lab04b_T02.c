#include <avr/io.h>

#define cycles_20ms 4999
#define cycles_1ms 249
#define cycles_2ms 499

volatile uint16_t currentCycles = (cycles_2ms+cycles_1ms)/2;

void enableADC() {
	// Sets PINC0 as input.
	DDRC &= ~(1<<PINC0);
	
	// Sets up ADC settings.
	ADMUX = 0x40;					// Reads from ADC0. Right justified. AVCC reference.
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

void enablePWM() {
	DDRB |= (1<<PINB2);				// Uses PINB2 as output for OC1B.
	OCR1A = cycles_20ms;			// Sets the frequency of the PWM.
	TCCR1A = 0b00100011;			// Use Fast PWM on OC1B in non-inverting mode.
	TCCR1B = 0b00011011;			// Use Fast PWM with 64 prescaler.
}

void setDuty(uint16_t cycles) {
	OCR1B = cycles;
}

int main(void)
{
	// Enable settings.
	enableADC();
	setDuty(currentCycles);
	enablePWM();
   
	while (1) {
		// Calculates cycles based on analog signal. (min is cycles_1ms, max is cycles_2ms).
		uint16_t cycles = cycles_1ms + (analogRead()/1023)*(cycles_2ms - cycles_1ms);
		// Changes cycle if a change is detected.
		if (cycles <= currentCycles - 1  || cycles >= currentCycles + 1) {
			currentCycles = cycles;
			setDuty(currentCycles);
		}
    }
}

