/*
 * partb.c
 *
 * Created: 2/4/2022 12:30:08 PM
 *  Author: cfair
 */ 
/*
#include <avr/io.h>

#define F_CPU 16000000
#include <avr/interrupt.h>

void Initialize(){
	// digital pins B2 output
	DDRB |= (1<<DDB2);
	// D7 input & pull-up
	DDRD &= ~(1<<DDD7);
	PORTD |= (1<<PORTD7);
}


int main (void)
{
	Initialize();
	while(1){
		if( !(PIND & (1<<PORTD7)) )
		PORTB |= (1 << PINB1);
		else
		PORTB &= ~(1 << PORTB1);
	}
}
*/