/*
 * GccApplication1.c
 *
 * Created: 2/11/2022 10:02:02 AM
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
#include <string.h>

volatile int ovrflw = 0;
volatile int falling = 1;
volatile int rising_edge = 0;
volatile int falling_edge = 0;
volatile int diff = 0;
volatile long period = 0;
volatile int addspace = 0;
volatile int curr = 0;
int reset = 0;
int light1 = 0;
int light2 = 0;
int inp[4] = {2, 2, 2, 2};
char String[50];

void Initialize()
{
	cli(); //Disable global interrupts
	//set output pins B1-5
	DDRB |= (1<<DDB1);
	DDRB |= (1<<DDB2);
	DDRB |= (1<<DDB3);
	DDRB |= (1<<DDB4);

	DDRB &= ~(1<<DDB0); //Set Pin 8 (PB0) to input
	PORTB |= (1<<PORTB0); //Enable Pin 8 (PB0) pull-up resistor

	//Enable Normal for ICR1 register (WGM10 - 0, WGM11 - 0, WGM12 - 0, WGM13 - 0)
	TCCR1A &= ~(1<<WGM10);
	TCCR1A &= ~(1<<WGM11);
	TCCR1B &= ~(1<<WGM12);
	TCCR1B &= ~(1<<WGM13);
	
	//Enable clock at 16MHz with (clkio/256 prescaling) ****
	TCCR1B |= (1<<CS12); //1
	TCCR1B &= ~(1<<CS11); //0
	TCCR1B |= (1<<CS10); //1
	
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

char Classify(int code[]){
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
					return 's';
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
					return 'l';
				}
				else if (code[3]==2){
					return 'r';
				}
			}
			else if (code[2]==1){
				if (code[3]==0){
					return 'p';
				}
				else if (code[3]==1){
					return 'j';
				}
				else if (code[3]==2){
					return 'w';
				}
			}
			else{
				return 'a';}
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
					return 'd';
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
				else if (code[3]==1){
				return 'q';
				}
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
	return 'a';
}

int isDash(long period, int overflow){
	// Morse Math:
	// Dot = 480000/1024 = 30000
	// Dash = 3200000/1024 = 200000 -> 2 overflows
	// Space = 400000/1024 -> 6 overflows
	
	if(period > 4500 && period<=18000){
		return 1; //dash
		light1 = 1;
	}
	else if(period >=18000){ //determine correct classification measure (overflow or period)
		return 2; //space
		light1 = 1;
		light2 = 1;
	}
	else{
		return 0; //dot
		light2 = 1;
	}
	_delay_ms(20);
}

ISR(TIMER1_CAPT_vect)
{
	TIFR1 |= (1<<ICF1);
	//Check if falling edge was found
	if (falling == 1)
	{
		//sprintf(String, "%c", 'a');
		//UART_putstring(String);
		falling_edge = ICR1; //Store variable
		period = falling_edge - rising_edge; //16-bit timer counts to 65536
		
		inp[curr] = isDash(period, ovrflw);
		curr++; //encode as dot or dash
		
		/*
		sprintf(String, "%ld", period);
		UART_putstring(String);
		//sprintf(String, "%c", '|');
		//UART_putstring(String);
		//sprintf(String, "%d", ovrflw);
		//UART_putstring(String);
		sprintf(String, "%c", '~');
		UART_putstring(String);
		*/
		
		
		if (period > 28000) // on space
		{
			addspace = 1;
		}
		
		/*
		sprintf(String, "%d", inp[0]);
		UART_putstring(String);
		sprintf(String, "%d", inp[1]);
		UART_putstring(String);
		sprintf(String, "%d", inp[2]);
		UART_putstring(String);
		sprintf(String, "%d", inp[3]);
		UART_putstring(String);
		sprintf(String, "%c", '|');
		UART_putstring(String); 
		*/
		
		period = 0;
		falling = 0;
		
		rising_edge = 0;
	}
	else if (falling == 0) //Check if rising edge was found
	{
		ovrflw = 0;
		period=0;
		//sprintf(String, "%c", 'b');
		//UART_putstring(String);
		rising_edge = ICR1; //Store variable
		//diff = falling_edge - rising_edge;
		//period = (ovrflw*65536) + diff;
		
		if(curr > 3){
			addspace = 1;
			curr = 0;
			reset = 1;
		}
		
		falling = 1;
		_delay_ms(20);
	}
	//Reset overflow counter
	
	//Toggle between rising edge and falling edge
	TCCR1B ^= (1<<ICES1);
}

//Overflow interrupt to keep track of amount of times the Timer1 overflowed
ISR(TIMER1_OVF_vect)
{
	ovrflw++;
}



int main(void)
{
	UART_init(BAUD_PRESCALER);
	
	Initialize();
	while(1)
	{ //consider X-ing all of this
		//classify four successive presses into a len 4 array. end fill when space encountered.
		if(addspace==1){
			//sprintf(String, "%c", 'X');
			//UART_putstring(String);
			char c = Classify(inp); //classify as a character
			sprintf(String, "%c", c);
			UART_putstring(String); //CONVERT CHAR to STR
			reset = 1;
			addspace = 0;
			curr = 0;
			//period = 0;
			_delay_ms(10);
		}
		
		if(reset == 1){
			for(int i = 0; i<4;i++){
				inp[i] = 2;
			}
			reset = 0;
			period = 0;
		}
		
		if(light1 == 1){
			PORTB |= (1<<PORTB3); //led flash
			PORTB &= ~(1<<PORTB4);
			light1 = 0;
		}
		
		if(light2 == 1){
			PORTB |= (1<<PORTB4); //led flash
			PORTB &= ~(1<<PORTB3);
			light2 = 0;
		}
	}
}
