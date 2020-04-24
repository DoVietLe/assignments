/*
 * Assignment03aTask01.c
 *
 * Created: 3/9/2020 11:58:12 PM
 * Author : Owner
 */ 

#define F_CPU 16000000
#define BAUD 9600
#define UBBR_VALUE 103

#include <avr/delay.h>
#include <avr/io.h>

void enableRandomTimer() {
	// Enables timer in normal mode with a pre-scaler of 1024.
	TCCR0A = 0x00;
	TCCR0B = 0x05;
}

uint8_t getInt() {
	uint8_t num = TCNT0;
	num ^= (num<<(num%8));
	return num;
}

float getNumber() {
	float num = getInt();
	num = sqrt(num);
	return num;
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

int main(void)
{						
	// Declares our strings.					
	char myString[] = "I love CPE301";
	char myIntString[10];
	char myFloatString[10];

	enableTransmit();							// Enables TX0.
	enableRandomTimer();

    while (1) 
    {
		// Sends the string.
		sendString(myString);
		sendByte(' ');

		// Sends the int.
		uint8_t myInt = getInt();
		snprintf(myIntString, 10, "%d", myInt);
		sendString(myIntString);
		sendByte(' ');
		
		// Sends the float.
		float myFloat = getNumber();
		snprintf(myFloatString, 10, "%f", myFloat);
		sendString(myFloatString);
		sendByte(' ');
		
		sendByte('\r');
		sendByte('\n');								// Endline.
		
		_delay_ms(1000);	
    }
}

