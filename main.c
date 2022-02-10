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
#include "uart.h" //where to put this

volatile int ovrflw = 0;
volatile int falling = 1;
volatile int rising_edge = 0;
volatile int falling_edge = 0;
volatile int diff = 0;
volatile long period = 0;
volatile int addspace = 0;
volatile int curr = 0;
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
	
	//Enable clock at 16MHz with (clkio/256 prescaling) ****
	TCCR1B |= (1<<CS10); //1
	TCCR1B &= ~(1<<CS11); //0
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
	TIFR1 |= (1<<ICF1);
	//Check if falling edge was found
	if (falling == 1)
	{
		falling_edge = ICR1; //Store variable
		period = (ovrflw*65536) + rising_edge - falling_edge; //16-bit timer counts to 65536
		if (ovrflw > 6) // on space
		{
			char c = Classify(inp); //classify as a character
			UART_putstring(c); //CONVERT CHAR to STR
			// possibly reset strings?
			curr = 0;
		}
		falling = 0;
		ovrflw = 0;
	}
	else if (falling == 0) //Check if rising edge was found
	{
		rising_edge = ICR1; //Store variable
		diff = falling_edge - rising_edge;
		period = (ovrflw*65536) + diff;
		
		inp[curr] = isDash(period, ovrflw); //encode as dot or dash
		curr++;
		
		if(curr > 3)
			curr = 0; 
		
		falling = 1;
		ovrflw = 0;
	}
	//Reset overflow counter
	
	//Toggle between rising edge and falling edge
	TCCR1B ^= (1<<ICES1);
}

int isDash(long period, int overflow){
	// Morse Math:
	// Dot = 480000/8 = 30000
	// Dash = 3200000/8 = 200000 -> 2 overflows
	// Space = 400000 -> 6 overflows
	
	if(overflow > 1 && overflow <6){
		return 1; //dash
		PORTB = 0b10000000; //led flash
	}
	else if(overflow > 6){ //determine correct classification measure (overflow or period)
		return 2; //space
		PORTB = 0b00100000; //led flash
	}
	else{
		return 0; //dot
		PORTB = 0b01000000; //led flash
	}
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
	{ //consider X-ing all of this
		//classify four successive presses into a len 4 array. end fill when space encountered.
		DECODE:for(int i=0; i<4; i++){
			//when interrupt is triggered, record period and overflow
			inp[i] = isDash(period, ovrflw);
			if(addspace==1){
				period = 0;
				ovrflw = 0;
				goto DECODE;
			}
		}
		//classify array with Classify() fn
		str outputChar = (str)Classify(inp);

		//spit out letters to Serial
		UART_putstring(outputChar);
		_delay_ms(300);
	}
}
