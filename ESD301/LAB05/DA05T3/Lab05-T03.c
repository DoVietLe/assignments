#define F_CPU 16000000UL

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "ds18b20.c"

#define UBBR_VALUE 103

char myIntString[20];

// USART functions.
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

int main(void) {
	// Variables.
	uint16_t temp;			// Stores the analog value and the temp.
	
	// Sets all initial values.
	enableTransmit();
	sei();					
	
	while (1) {
		temp = ds18b20_gettemp();
			
		snprintf(myIntString, 20, "%u\n", temp);
		sendString(myIntString);
		
		_delay_ms(500);
	}
}
