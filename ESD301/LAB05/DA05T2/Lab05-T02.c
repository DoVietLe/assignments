#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

// Values for the seven-segment (common cathode configuration).
const uint8_t sevenSegmentCode[] = {
	0x03,
	0x9F,
	0x25,
	0x0D,
	0x99,
	0x49,
	0x41,
	0x1F,
	0x01,
	0x09
};

// SPI functions.
void initializeSPI() {
	// Initializes ports.
	DDRD |= (1<<PIND4);				// Sets latch clock pin as output.
	DDRB |= (1<<PINB3)|(1<<PINB5);	// Sets MOSI pin as output.
	PORTD &= ~(1<<PIND4);			// Sets clock output to zero.
	
	//Sets SPI settings.
	SPCR0 = 0b01110000;				// Enables SPI mode as master.
}

void clkLatch() {
	PORTD |= (1<<PIND4);
	_delay_ms(1);
	PORTD &= ~(1<<PIND4);
}

void sendData(uint8_t data) {
	SPDR0 = data;					// Sends data.
	while ( !(SPSR0&(1<<SPIF)) );	// Waits until the data is sent.
	SPSR0 |= (1<<SPIF);				// Clears the flag.
	//SPCR0 |= ();
}

void displayDigits(uint8_t digits[]) {
	// For all digits in the array:
	for (uint8_t i = 0; i < 4; i++) {
		sendData(sevenSegmentCode[digits[i]]);	// Send the seven-segment value corresponding to the digit.
		sendData(1<<(4+i));						// Send the 1-hot value corresponding to which 8-segment.
		clkLatch();								// Clock in the value.
	}
}

// Temperature functions.
void enableADC() {
	// Sets PINC4 as input.
	DDRC &= ~(1<<4);
	
	// Sets up ADC settings.
	ADMUX = 0xC4;					// Reads from ADC4. Right justified. Internal 1.1V reference.
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

uint16_t analogToTemp(uint16_t t, char unit) {
	// Takes analog converted number and converts it into temperature.
	float convertedTemp = t*110.0/1023.0;
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

void getDigits(uint16_t value, uint8_t arr[]) {
	for (uint8_t i = 0; i < 4; i++) {
		arr[i] = ( (uint8_t)(value/pow(10, i)) ) % 10;
	}
}

int main(void) {
	// Variables.
	uint16_t analogValue, temp;			// Stores the analog value and the temp.
	uint8_t digits[4];					// Stores the digits.
	
	// Sets all initial values.
	initializeSPI();
	enableADC();
	
	while (1) {
		analogValue = analogRead();
		temp = analogToTemp(analogValue, 'c');
		getDigits(temp, digits);
		displayDigits(digits);
	}
}

