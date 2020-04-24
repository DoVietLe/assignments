#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define DDR_SHIFT DDRB
#define PORT_SHIFT PORTB
#define DDR_CLK DDRD
#define PORT_CLK PORTD
#define PIN_SCLK 7
#define PIN_LCLK 4

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

// 'Custom Serial Transfer Protocol' functions.
void initializePort() {
	// Initializes the direction.
	DDR_SHIFT = 0x01;							// Sets lower most bit as output.
	DDR_CLK = (1<<PIN_SCLK)|(1<<PIN_LCLK);		// Sets clock pin as output.
	
	// Initializes output.
	PORT_SHIFT = 0x00;
	PORT_CLK = 0x00;
}

void clkShift() {
	// Sets clock input to shift register as high.
	PORT_CLK |= (1<<PIN_SCLK);
	//_delay_ms(1);
	PORT_CLK &= ~(1<<PIN_SCLK);
}

void clkLatch() {
	PORT_CLK |= (1<<PIN_LCLK);
	//_delay_ms(1);
	PORT_CLK &= ~(1<<PIN_LCLK);
}

void sendData(uint8_t data) {
	// Places data into the port.
	PORT_SHIFT = data;
	
	// Repeatedly clocks the value into the shift register,
	// then shifts the next value into the position.
	for (uint8_t i = 0; i < 8; i++) {
		clkShift();
		PORT_SHIFT >>= 1;
	}
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
	initializePort();
	enableADC();
	
    while (1) {
		analogValue = analogRead();
		temp = analogToTemp(analogValue, 'c');
		getDigits(temp, digits);
		displayDigits(digits);
    }
}

