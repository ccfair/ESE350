/*
 * ESE350 Lab2.c
 *
 * Created: 2/4/2022 10:32:51 AM
 * Author : cfair
 */ 

#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include "uart.h"

volatile int ovrflw = 0;
volatile int falling = 1;
volatile int ovrvar1 = 0;
volatile int ovrvar2 = 0;
volatile int diff = 0;
volatile int period = 0;
volatile int addspace = 0;
char morse[4];
int inp[4] = {2, 2, 2, 2};

void Initialize()
{
	cli(); //Disable global interrupts
	//set output pins B1-5
	DDRB = 0b01111100;

	DDRB &= ~(1<<DDB0); //Set Pin 8 (PB0) to input
	PORTB |= (1<<PORTB0); //Enable Pin 8 (PB0) pull-up resistor

	//Enable Normal for ICR1 register (WGM10 - 0, WGM11 - 0, WGM12 - 0, WGM13 - 0)
	TCCR1A &= ~(1<<WGM10);
	TCCR1A &= ~(1<<WGM11);
	TCCR1B &= ~(1<<WGM12);
	TCCR1B &= ~(1<<WGM13);
	
	//Enable clock at 16MHz with (clkio/8 prescaling)
	TCCR1B &= ~(1<<CS10); //0
	TCCR1B |= (1<<CS11); //1
	TCCR1B &= ~(1<<CS12); //0
	
	//Enable noise canceler for better button sensing
	TCCR1B |= (1<<ICNC1);
	
	TCCR1B &= ~(1<<ICES1); //Look for falling edge
	TIFR1 |= (1<<ICF1); //Clear interrupt flag
	TIMSK1 |= (1<<ICIE1); //Enable input capture interrupt
	
	//Enable overflow interrupt
	TIMSK1 |= (1<<TOIE1);
	TIFR1 |= (1<<TOV1);
	
	sei(); //Enable global interrupts
}

ISR(TIMER1_CAPT_vect)
{
	//Check if falling edge was found
	if (falling == 1)
	{
		ovrvar1 = ICR1; //Store variable
		falling = 0;
		if ((ovrvar1 - ovrvar2) > 6250)
		{
			addspace = 1;
		}
	}
	else if (falling == 0) //Check if rising edge was found
	{
		ovrvar2 = ICR1; //Store variable
		diff = ovrvar2 - ovrvar1;
		period = (ovrflw*65536) + diff;
		falling = 1;
	}
	//Reset overflow counter
	ovrflw = 0;
	
	//Toggle between rising edge and falling edge
	TCCR1B ^= (1<<ICES1);
}

//Overflow interrupt to keep track of amount of times the Timer1 overflowed
ISR(TIMER1_OVF_vect)
{
	ovrflw++;
}

char Classify(int code[4]){
	// input array has elements = 0 if dot, 1 if dash, 2 if space
	if(code[0] == 0){
		if (code[1]==0){
			if (code[2]==0){
				if (code[3]==0){
					return 'h';
				}
				else if (code[3]==1){
					return 'v';
				}
				else if (code[3]==2){
					return 's'
				}
			}
			else if (code[2]==1){
				if (code[3]== 0){
					return 'f';
				}
				else if (code[3]==2){
					return 'u';
				}
			}
			else if (code[2]==2){
				return 'i';
			}
		}
		else if (code[1]==1){
			if (code[2]==0){
				if (code[3]==0){
					return 'l;'
				}
				else if (code[3]==2){
					return 'r';
				}
			}
			else if (code[2]==1){
				if (code[3]==0){
					return 'p;'
				}
				else if (code[3]==1){
					return 'j';
				}
				else if (code[3]==2){
					return 'w';
				}
			}
		}
		else if (code[1]==2){
			return 'e';
		}
	}
	else if(code[0] == 1){
		if (code[1]==0){
			if (code[2]==0){
				if (code[3]==0){
					return 'b';
				}
				else if (code[3]==1){
					return 'x';
				}
				else if (code[3]==2){
					return 'd'
				}
			}
			else if (code[2]==1){
				if (code[3]== 0){
					return 'c';
				}
				else if (code[3]==1)
					return 'y';
				else if (code[3]==2){
					return 'k';
				}
			}
			else if (code[2]==2){
				return 'n';
			}
		}
		else if (code[1]==1){
			if (code[2]==0){
				if (code[3]==0){
					return 'z';
				}
				else if (code[3]==1)
					return 'q';
				else if (code[3]==2){
					return 'g';
				}
			}
			else if (code[2]==1){
				return 'o';
			}
			else if (code[2]==2)
				return 'm';
		}
		else if (code[1]==2){
			return 't';
		}
	}
}

int main(void)
{
	UART_init(BAUD_PRESCALER);
	
	Initialize();
	while(1)
	{
		
	}
