/*
 * GccApplication1.c
 *
 * Created: 3/13/2020 5:21:23 PM
 * Author : do177
 */ 

#include <avr/interrupt.h>
#include <avr/io.h>

#define UBBR_VALUE 103
#define CYCLES_100ms 6249

volatile char cTempString[10];
volatile char fTempString[10];
volatile int passes = 0;

void setTimerAndInterrupts() {
	// Set interrupt settings.
	OCR1A = CYCLES_100ms;
	TIMSK1 = 0b00000010;		// Enables interrupt on compare match A.
	TIFR1 |= 0x02;				// Clears the interrupt flag.
	sei();
	
	// Sets timer settings.
	TCCR1B = 0b00001100;		// Sets timer1 to be in CTC mode with 256 pre-scaler.
	TCCR1A = 0b00000000;	
	TCNT1 = 0;
}

// Enables the UART and it's transmitter.
void enableTransmit() {
	// Sets port.
	DDRD = 0x02;
	
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
	if (UCSR0B & (1<<TXEN0)) {					// Checks whether transmit is enabled.
		while (!(UCSR0A & (1<<UDRE0)));		// Waits until the UDR0 register is ready.
		UDR0 = b;								// Sends the byte.
	}
}

void sendString(char string[]) {
	int i = 0;
	while (string[i] != '\0')
		sendByte(string[i++]);
}

void enableADC() {
	// Sets PINC0 as input.
	DDRC &= ~1;		
	
	// Sets up ADC settings.
	ADMUX = 0xC0;					// Reads from ADC0. Right justified. Internal 1.1V reference.
	ADCSRB = 0x00;					// Free running mode.
	ADCSRA = 0b10000111;			// Enables ADC with 128 pre-scaler.
}

void disableADC() {
	ADCSRA &= 0x7F;					// Disables the ADEN bit.
}

uint16_t analogRead() {
	ADCSRA |= (1<<6);				// Enables ADSC to start AD conversion.
	while (!(ADCSRA&(1<<4)));		// Waits until ADIF is set indicating ADC is done.
	ADCSRA |= (1<<4);				// Clears ADIF flag.
	return ADC;						// Returns value.
}

float analogToTemp(uint16_t t, char unit) {
	// Takes analog converted number and converts it into temperature.
	//return t;
	float convertedTemp = t*110/1023;
	switch (unit) {
		case 'C': 
		case 'c':
			return convertedTemp;			// Returns temperature converted to fahrenheit.
		case 'F':
		case 'f':
		default:
			return convertedTemp*1.8 + 32;	// Returns temperature converted to celsius.
	}
}

int main(void) {
	// Enables resources. 
	enableTransmit();
	enableADC();
	setTimerAndInterrupts();
	while (1)
		// When five 100ms passes occurs, send the data.
		if (passes >= 5) {
			passes = 0;
			// Reads in data and converts it to celsius and temperature.
			uint16_t temp = analogRead();
			float cTemp = analogToTemp(temp, 'C');
			float fTemp = analogToTemp(temp, 'F');
			// Converts the floats into strings.
			snprintf(cTempString, 10, "%f", cTemp);
			snprintf(fTempString, 10, "%f", fTemp);
			// Sends the data serially.
			sendString("Temperature		Celsius: ");
			sendByte(cTempString);
			sendString("	Fahrenheit: ");
			sendString(fTempString);
			sendByte('\r');
			sendByte('\n');
		}
}

// Increments the amount of 100ms that passes.
ISR(TIMER1_COMPA_vect) {
	TIFR1 |= 0x02;					// Clears the interrupt flag.
	passes++;
}