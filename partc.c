/*
 * partc.c
 *
 * Created: 2/4/2022 12:19:39 PM
 *  Author: cfair
 */ 
/*
#include <avr/io.h>

#define F_CPU 16000000
#include <util/delay.h>

void Initialize(){
	cli();
	// digital pins B1 output
	DDRB |= (1<<DDB1);
	// D7 input & pull-up
	DDRD &= ~(1<<DDD7)
	PORTD |= (1<<PORTD7)
	PCICR |= (1<<PCIE2);
	PCMSK2 |= (1<<PCINT21);
	sei();
}

ISR(PCINT2_vect){
	PORTB ^= (1<<PORTB1)
}

int main (void)
{
	Initialize();
	while(1);
}
*/