/*
 * Midterm_Project.c
 *
 * Created: 3/15/2020 3:22:19 PM
 * Author : do177
 */ 

#include <avr/interrupt.h>
#include <avr/io.h>

#define CYCLES_1s 15624			// Amount of cycles for 1s delay @16MHz, 1024 pre-scaler.
#define UBBR_VALUE 103

// Lists the functionalities.
const char functionalities[] = "h - Help screen \n" 
								"t - Display temperature in centrigrade \n"
								"T - Display temperature in fahrenheit \n"
								"o - Turn on PB5 LED \n"
								"O - Turn off PB5 LED \n"
								"s - Send a string to the terminal \n"
								"i - Send an integer to the terminal \n\n";
const char myString[] = "CPE 301 Midterm.\n";
// Keeps track of the delay between blinks of LED on PB2. Initialized to 1s.
volatile int delayAmount = 0;
volatile int delayMax = 1;
// Stores the temperature as a string.
volatile char temperatureString[10];
volatile char integer[10];

void enableRandomTimer() {
	// Enables timer in normal mode with a pre-scaler of 1024.
	TCCR0A = 0x00;
	TCCR0B = 0x05;
}

uint8_t getInt() {
	uint8_t num = TCNT0;
	num ^= (num<<(num%8));
	return num % 10;
}

void initializeBlinkingLED() {
	// Sets PB2 as output.
	DDRB |= (1<<2);
	PORTB &= ~(1<<2);
	
	// Set interrupt settings.
	OCR1A = CYCLES_1s;
	TIMSK1 = 0b00000010;		// Enables interrupt on compare match A.
	TIFR1 |= 0x02;				// Clears the interrupt flag.
	sei();
	
	// Sets timer settings.
	TCCR1B = 0b00001101;		// Sets timer1 to be in CTC mode with 1024 pre-scaler.
	TCCR1A = 0b00000000;
	TCNT1 = 0;
}

// Enables ADC.
void enableADC() {
	// Sets PINC4 as input.
	DDRC &= ~(1<<4);
	
	// Sets up ADC settings.
	ADMUX = 0xC4;					// Reads from ADC4. Right justified. Internal 1.1V reference.
	ADCSRB = 0x00;					// Free running mode.
	ADCSRA = 0b10000111;			// Enables ADC with 128 pre-scaler.
}

// Disables ADC.
void disableADC() {
	ADCSRA &= 0x7F;					// Disables the ADEN bit.
}

// Converts an analog to digital value on PC0.
uint16_t analogRead() {
	ADCSRA |= (1<<6);				// Enables ADSC to start AD conversion.
	while (!(ADCSRA&(1<<4)));		// Waits until ADIF is set indicating ADC is done.
	ADCSRA |= (1<<4);				// Clears ADIF flag.
	return ADC;						// Returns value.
}

// Uses a formula to convert a digital read value to temperature (formula works for 1.1V reference voltage).
float analogToTemp(uint16_t t, char unit) {
	// Takes analog converted number and converts it into temperature.
	//return t;
	float convertedTemp = t*110/1023;
	switch (unit) {
		case 't':
		return convertedTemp;			// Returns temperature in Celsius.
		case 'T':
		default:
		return convertedTemp*1.8 + 32;	// Returns temperature converted to Fahrenheit.
	}
}

uint8_t asciiToInt8(uint8_t n) {
	if (n >= 48 && n <= 57) {
		sendByte();
		return n - 48;
	}
	return 1;
}

// Sets up UART.
void setUART() {
	// Sets port.
	DDRD |= 0x02;
	
	// Sets baud rate.
	UBRR0 = UBBR_VALUE;
	
	// Sets UART settings.
	UCSR0C |= 0b00000110;			// to async mode with no parity
}

// Enables transmitter on PORTD1.
void enableTransmit() {
	// Sets port.
	DDRD |= 0x02;
	UCSR0B |= (1<<RXEN0);			// Enables transmit line.
}

// Disables the transmitter on UART.
void disableTransmit() {
	UCSR0B &= ~(1<<TXEN0);
}

// Enables receiver on PORTD0.
void enableReceive() {
	// Sets port.
	DDRD &= ~1;
	UCSR0B |= (1<<RXEN0);			// Enables receive line.
}

// Sends one byte of data out the TX line.
void sendByte(uint8_t b) {
	if (UCSR0B & (1<<TXEN0)) {					// Checks whether transmit is enabled.
		while (!(UCSR0A & (1<<UDRE0)));			// Waits until the UDR0 register is ready.
		UDR0 = b;								// Sends the byte.
	}
}

// Sends every ASCII byte in a string.
void sendString(char string[]) {
	int i = 0;
	while (string[i] != '\0')
		sendByte(string[i++]);
}

char getByte() {
	while (!(UCSR0A & (1<<7)));	// Waits until the data is fully received.
	return UDR0;
}

int main(void)
{
	// Sets everything up.
	enableRandomTimer();
    enableADC();
	setUART();
	enableTransmit();
	enableReceive();
	initializeBlinkingLED();
	
	DDRB |= (1<<5);				// Enables PB5 and PB2 for the LEDs.
	PORTB &= ~(1<<5);
	
	sendString(functionalities);
	
    while (1)  {
		// Polls for a serial data on the RX line.
		char instr = getByte();
		
		// When data is sent, perform something based on the instructions
		// if the instruction is defined. Otherwise, poll again.
		if (instr == 'h') {
			sendString(functionalities);
		} else if (instr == 't' || instr == 'T') {
			uint16_t temp = analogRead();		// Reads in temperature and converts.
			float convertedTemp = analogToTemp(temp, instr);
			snprintf(temperatureString, 10, "%f\n", convertedTemp);
			sendString(temperatureString);
			sendByte('\n');
		} else if (instr == 'o') {
			PORTB &= ~(1<<5);					// Turns on LED.
		} else if (instr == 'O') {
			PORTB |= (1<<5);					// Turns off LED.
		} else if (instr == 's') {
			sendString(myString);
		} else if (instr == 'i') {
			uint8_t num = getInt();				// Grabs a random integer.
			snprintf(integer, 10, "%d\n", num);
			sendString(integer);				// Sends the integer to the terminal.
			sendByte('\n');
			delayMax = num;						// Replaces delay with new integer.
		}
    }
}

ISR(TIMER1_COMPA_vect) {
	TIFR1 |= 0x02;				// Clears interrupt flag.
	delayAmount = (delayAmount + 1) % delayMax;
	if (delayAmount == 0)
		PORTB ^= (1<<2);		// Toggles PB2.
}