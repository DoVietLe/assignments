/*
 * Assignment_2a_C.c
 *
 * Created: 2/23/2020 10:16:39 PM
 * Author : Owner
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRB = (0x01<<PORTB3);
	
    while (1) 
    {
		PORTB = (0x01<<PORTB3);
		_delay_ms(412.5);
		PORTB = 0x00;
		_delay_ms(337.5);
    }
}

