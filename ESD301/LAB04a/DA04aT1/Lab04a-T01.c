#include <avr/io.h>
#include <avr/interrupt.h>

#define MAX_PWM 255

volatile uint8_t status = 0x00;		// Keeps track of whether the motor is on.

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

void setDuty(float duty) {
	OCR0A = (duty/100)*MAX_PWM;
}

void setPinInterrupts() {
	DDRC &= ~(1<<PINC1);			// Sets PINC1 as input.
	PCICR = (1<<PCIE1);				// Enables pin change interrupt on PCINT[14:8].
	PCMSK1 = (1<<PCINT9);			// Enables pin change interrupt on PCINT9 (PINC1).
	sei();							// Enables global interrupts.
}

int main(void) {
	// Stores the duty cycle.
	int dutyCycle = 0;
	
	// Sets up everything.
    enableADC();
	setPinInterrupts();
	
	// Constantly updates the duty cycle.
    while (1) {
		// Gets duty cycle based on potentiometer.
		dutyCycle = 95*(analogRead()/1023);		// Max duty cycle is 95%, so calculate ratio of 95% potentiometer is on.
		setDuty(dutyCycle);						// Set the new duty cycle to the PWM.
    }	
}

ISR(PCINT1_vect) {
	// Makes sure a rising edge occurred on button press.
	if (PINC & (1<<PINC1)) {
		status = ~status;
		if (status) {
			DDRD |= (1<<PIND6);				// Uses PIND6 as output for OC0A.
			TCCR0A = 0b10000011;			// Use Fast PWM on OC0A in non-inverting mode.
			TCCR0B = 0b00000010;			// Use Fast PWM with 1024 prescaler.
		} else {
			DDRD &= ~(1<<PIND6);			// Turns PIND6 into an input pin.
			TCCR0A = 0x00;					// Turns off PWM.
			TCCR0B = 0x00;					// Turns off clock.
		}
	}
	PCIFR = (1<<PCIF1);						// Clears interrupt flag.
}