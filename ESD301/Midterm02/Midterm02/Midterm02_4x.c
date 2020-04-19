#include <avr/interrupt.h>
#include <avr/io.h>

#define UBBR_VALUE 103
#define MAX_PWM 256
#define VARIATION 10

volatile uint16_t overflows[4];
volatile uint16_t capturedOverflows[4];
volatile uint16_t capturedVal[4];

volatile uint32_t periodTimerRising[2];			// Stores the period in timer counts between
volatile uint32_t periodTimerFalling[2];		// Stores the period in timer counts between
volatile uint32_t periods[80];					// Stores period for 20 periods.
volatile uint8_t arrayPos = 0;
volatile uint32_t averagePeriod = 0;			// Calculates average period.
volatile uint64_t rpm = 0;
volatile uint32_t prevPeriod = 1;
volatile uint8_t cw = 0;
volatile char myIntString[20];

void enableDirection() {
	DDRD |= (1<<PIND4)|(1<<PIND5);
	setCounterClockwise();
}

void setCounterClockwise() {
	cw = 0;
	PORTD |= (1<<PIND4);
	PORTD &= ~(1<<PIND5);
}

void setClockwise() {
	cw = 1;
	PORTD |= (1<<PIND5);
	PORTD &= ~(1<<PIND4);
}

void setPWM() {
	// Sets up port.
	DDRD |= (1<<PIND6);				// Uses PIND6 as output for OC0A.
	
	// Adjusts PWM settings.
	TCCR0A = 0b10000011;			// Use Fast PWM on OC0A in non-inverting mode.
	TCCR0B = 0b00000010;			// Use Fast PWM with 8 prescaler.
}

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

void setDutyAndCalculateSpeed(float duty) {
	// Calculates the cycles for the duty cycle.
	uint16_t cycles = (duty/100)*MAX_PWM;
	
	// Assigns the cycles if it's not the current cycle.
	if (cycles != OCR0A) {
		OCR0A = cycles;
		
		// Resets everything to calculate 20 period cycle.
		arrayPos = 0;								// Resets array pos to restart calculation.
		averagePeriod = 0;							// Resets period.
		while (arrayPos != 39 && OCR0A >= 35);		// Waits until 20 periods are recorded.

		// Calculates the average period.
		for (int i = 0; i < 40; i++)
			averagePeriod += periods[i]/40.0;
		
		// Calculates RPM.
		rpm = (16000000*60)/(8*averagePeriod*120);
		
		sendSpeed(rpm);
	}
}

// Enables the UART and it's transmitter.
void enableTransmit() {
	// Sets port.
	DDRD |= 0x02;
	
	// Sets baud rate.
	UBRR0 = UBBR_VALUE;
	
	// Sets UART settings.
	UCSR0C = 0b00000110;			// to async mode with no parity
	// 2 stop bits, and a frame of 8 bits.
	UCSR0B = 0b00001000;			// Enables transmit line.
}

// Disables the transmitter on UART.
void disableTransmit() {
	UCSR0B &= ~(1<<TXEN0);
}

// Sends one byte of data out the TX line.
void sendByte(uint8_t b) {
	if (UCSR0B & (1<<TXEN0)) {				// Checks whether transmit is enabled.
		while (!(UCSR0A & (1<<UDRE0)));		// Waits until the UDR0 register is ready.
		UDR0 = b;							// Sends the byte.
	}
}

void sendString(char string[]) {
	int i = 0;
	while (string[i] != '\0')
	sendByte(string[i++]);
}

void sendSpeed(uint64_t data) {
	// Sends period through USART.
	snprintf(myIntString, 20, "RPM: %u", data);
	sendString(myIntString);
	sendByte('\n');
	sendByte('\r');
}

void setPCI() {
	// Sets port direction.
	DDRD &= ~(1<<PIND2);			// Sets INT0 pin as input.
	PORTD |= (1<<PIND2);			// Enables pull-up resistor.
	
	// Assigns settings.
	EIMSK |= (1<<INT0);				// Enables INT0 interrupt.
	EICRA |= (1<<ISC01);			// Trigger interrupt on falling edge.
}

void enableCapture() {
	// Sets port directions.
	DDRB &= ~(1<<PINB0);			// Sets PINB0 (ICP1) as input.
	DDRE &= ~(1<<PINE2);			// Sets PINE2 (ICP3) as input.
	
	// Sets up timer 1.
	TCCR1A = 0b00000000;			// Sets timer to normal mode.
	TCCR1B = 0b01000001;			// Enables input capture on rising edge. Prescaler as 8.
	TIMSK1 |= (1<<ICIE1)|(1<<TOIE1);// Enables capture interrupt and overflow interrupt.
	
	// Sets up timer 3.
	TCCR3A = 0b00000000;			// Sets timer to normal mode.
	TCCR3B = 0b01000001;			// Enables input capture on rising edge. Prescaler as 8.
	TIMSK3 |= (1<<ICIE3)|(1<<TOIE3);// Enables capture interrupt and overflow interrupt.
}

int main(void)
{
	// Variables.
	float dutyCycle = 0;			// Stores the duty cycles.
	
	// Enables settings.
	setPWM();
	enableTransmit();
	enableADC();
	enableDirection();
	enableCapture();
	setPCI();
	sei();
	
	// Initializes direction.
	setCounterClockwise();
	
	while (1)
	{
		// Gets duty cycle based on potentiometer.
		dutyCycle = 100*(analogRead()/1023);		// Calculates ratio based on potentiometer value.
		setDutyAndCalculateSpeed(dutyCycle);		// Set the new duty cycle to the PWM.
	}
}

ISR(TIMER1_CAPT_vect) {
	if (TCCR1B & (1<<ICES1)) {
		capturedVal[0] = ICR1;				// Saves the spare value when a capture occurs.
		TCNT1 = 0;							// Resets timer.
		capturedOverflows[0] = overflows[0];// Stores the amount of overflows.
		overflows[0] = 0;					// Resets the overflow counter.
		TCCR1B &= ~(1<<ICES1);				// Switch to capture falling edge.
		
		// Stores period.
		periods[arrayPos] = (uint32_t)capturedVal[0] + (uint32_t)capturedVal[1] + 0x10000L*capturedOverflows[0];
	} else {
		capturedVal[1] = ICR1;				// Saves the spare value when a capture occurs.
		TCNT1 = 0;							// Resets timer.
		capturedOverflows[1] = overflows[1];// Stores the amount of overflows.
		overflows[1] = 0;					// Resets the overflow counter.
		TCCR1B |= (1<<ICES1);				// Switch to capture rising edge.
		
		// Stores period.
		periods[arrayPos] = (uint32_t)capturedVal[0] + (uint32_t)capturedVal[1] + 0x10000L*capturedOverflows[1];
	}
	
	// Increments array pos.
	arrayPos = (arrayPos + 1) % 80;
	
	TIFR1 |= (1<<ICF1);						// Clears interrupt flag.
}

ISR(TIMER3_CAPT_vect) {
	if (TCCR3B & (1<<ICES3)) {
		capturedVal[2] = ICR3;				// Saves the spare value when a capture occurs.
		TCNT3 = 0;							// Resets timer.
		capturedOverflows[2] = overflows[2];// Stores the amount of overflows.
		overflows[2] = 0;					// Resets the overflow counter.
		TCCR3B &= ~(1<<ICES3);				// Switch to capture falling edge.
		
		// Stores period.
		periods[arrayPos] = (uint32_t)capturedVal[2] + (uint32_t)capturedVal[3] + 0x10000L*capturedOverflows[2];
	} else {
		capturedVal[3] = ICR3;				// Saves the spare value when a capture occurs.
		TCNT3 = 0;							// Resets timer.
		capturedOverflows[3] = overflows[3];// Stores the amount of overflows.
		overflows[3] = 0;					// Resets the overflow counter.
		TCCR3B |= (1<<ICES3);				// Switch to capture rising edge.
		
		// Stores period.
		periods[arrayPos] = (uint32_t)capturedVal[2] + (uint32_t)capturedVal[3] + 0x10000L*capturedOverflows[3];
	}
	
	// Increments array pos.
	arrayPos = (arrayPos + 1) % 80;
	
	TIFR1 |= (1<<ICF1);						// Clears interrupt flag.
}

ISR(TIMER1_OVF_vect) {
	overflows[0]++;					// Increments the overflows.
	overflows[1]++;
	TIFR1 |= (1<<TOV1);				// Clears interrupt flag.
}

ISR(TIMER3_OVF_vect) {
	overflows[2]++;					// Increments the overflows.
	overflows[3]++;
	TIFR3 |= (1<<TOV3);				// Clears interrupt flag.
}

ISR(INT0_vect) {
	// Reverses the direction based on the current direction.
	if (cw) {
		setCounterClockwise();
	} else {
		setClockwise();
	}
	
	EIFR |= (1<<INTF0);				// Clears interrupt flag.
}


