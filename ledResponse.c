/*
#include <avr/io.h>

#define F_CPU 16000000
#include <util/delay.h>

int waiting = 1;
int ledNum = 1;

void Initialize(){
	
	DDRB = 0b01111000; // Port initialization
	PORTB = 0b10000000; //pins enabling
	// D7 input & pull-up
	DDRD &= ~(1<<DDD7);
	PORTD |= (1<<PORTD7);
}


int main (void)
{
	Initialize();
	while(1){
		if(waiting && (PIND & (1<<PORTD7)) ){
			ledNum++;
			waiting = 0;
		} else if(!(PIND & (1<<DDD7))) waiting = 1;
		
		PORTB |= 1<<(DDB1 + (ledNum%4));
		PORTB &= ~(1<<(PORTB1 + ((ledNum-1)%4)));
	}
}
*/