/*
 * lab3.c
 *
 * Created: 3/1/2022 9:51: AM
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

char String[25];
volatile int distance = 0;
volatile int beg = 0;
volatile int end = 0;
volatile int rise = 1;
volatile int frequency = 0;
volatile int count = 0;
volatile int discrete = 0;

void Initialize()
{
	
	
	//GPIO PIN SETUP
	//Buzzer Pin
	DDRD|=(1<<DDD5); //PD5 output
	PORTD|=(1<<PORTD5); //D5 high

	cli();

	//Ultrasound sensor
	//Echo
	DDRB &= ~(1<<DDB0);//PB8 input pin
	//Trig
	DDRD |= (1<<DDD7); //PD7 output
	//Button Pin
	DDRB &= ~(1<<DDB4);//PB4 input pin
	PORTD &= ~(1<<PORTB4); //B4 LOW

	//Interrupt
	PCICR |= (1<<PCIE0);
	PCMSK0 |= (1<<PCINT4);
	
	//10us pulse req
	PORTD |= (1<<PORTD7); 
	_delay_us(10);
	PORTD &= ~(1<<PORTD7); 
	
	//TIMER0 
	//clock + prescale
	TCCR0B |= (1<<CS00); //1
	TCCR0B |= (1<<CS01); //1
	TCCR0B &= ~(1<<CS02); //0

	TCCR0A&=~(1<<WGM01);
	TCCR0A |= (1<<WGM00);
	TCCR0B |= (1<<WGM02);

	TCCR0A |= (1<<COM0B1); //1
	TCCR0A &= ~(1<<COM0B0); //0
	OCR0A = 30;
	OCR0B = 15;

	TIMSK0 |= (1<<OCIE0A);
	TIMSK0 |= (1<<OCIE0B);
	
	//TIMER1 ADJUSTMENTS
	//clock + prescale
	TCCR1B &= ~(1<<CS10); //0
	TCCR1B |= (1<<CS11); //1
	TCCR1B &= ~(1<<CS12); //0
	
	//Normal mode for ICR1 register
	TCCR1A &= ~(1<<WGM10);
	TCCR1A &= ~(1<<WGM11);
	TCCR1B &= ~(1<<WGM12);
	TCCR1B &= ~(1<<WGM13);
	
	TIMSK1 |= (1<<ICIE1); //Enable input capture interrupt
	TCCR1B |= (1<<ICES1); //Look for rising edge
	TIFR1 |= (1<<ICF1); //Clear interrupt flag
	
	//overflow interrupt
	TIMSK1 |= (1<<TOIE1);
	
	//From part E (ADC + photo sensor)
	ADMUX &= ~(1<<MUX0);
	ADMUX &= ~(1<<MUX1);
	ADMUX &= ~(1<<MUX2);
	ADMUX &= ~(1<<MUX3);
	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1);
	
	ADCSRA |= (1<<ADPS0);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS2);
	ADCSRA |= (1<<ADATE);
	ADCSRA |= (1<<ADEN);
	ADCSRA |= (1<<ADSC);

	ADCSRB &= ~(1<<ADTS0);
	ADCSRB &= ~(1<<ADTS1);
	ADCSRB &= ~(1<<ADTS2);
	
	DIDR0 |= (1<<ADC0D);
	
	sei(); 
}

ISR(PCINT0_vect)
{

	if (PINB & (1<<PINB4))
	{
		if (discrete == 0) //discrete mode
		{
			sprintf(String,"\n discrete mode");
			UART_output(String);
			_delay_ms(1000);
			discrete = 1;
		}
		else //continuous mode
		{
			sprintf(String,"\n continuous mode");
			UART_output(String);
			_delay_ms(1000);
			discrete = 0;
		}
	}
}

//Timer1 interrupt, sends signal to ultrasonic sensor
ISR(TIMER1_OVF_vect)
{
	count++;
	if (count == 3)
	{
		PORTD |= (1<<PORTD7); //Toggle Pin 7 (D7) HIGH
		_delay_us(10);
		PORTD &= ~(1<<PORTD7); //Toggle Pin 7 (D7) LOW
		//Reset variables
		beg = 0;
		end = 0;
		count = 0;
	}
}

//Ultrasonic sensor distance measure with input capture interrupt
ISR(TIMER1_CAPT_vect)
{
	if (rise == 1)
	{
		beg = ICR1;
		TCCR1B &= ~(1<<ICES1);
		rise = 0;
	}
	else if (rise == 0)
	{
		end = ICR1;

		distance = ((end-beg)*0.034/2)*0.5;
		
		if (discrete == 1)
		{
			//Discrete setup for frequency
			if (distance >= 1 && distance <= 4)
			{
				frequency = 60;
			}
			else if (distance >= 5 && distance <= 10)
			{
				frequency = 64;
			}
			else if (distance >= 11 && distance <= 16)
			{
				frequency = 71;
			}
			else if (distance >= 17 && distance <= 24)
			{
				frequency = 80;
			}
			else if (distance >= 25 && distance <= 28)
			{
				frequency = 90;
			}
			else if (distance >= 29 && distance <= 38)
			{
				frequency = 95;
			}
			else if (distance >= 39 && distance <= 48)
			{
				frequency = 107;
			}
			else if (distance >=49)
			{
				frequency = 120;
			}
		}
		else
		{
			//Continuous equation for frequency
			frequency = 1.25*distance + 58.95;
		}
		
		TCCR1B |= (1<<ICES1);
		rise = 1;
	}
}

//Set OCR0A with frequency
ISR(TIMER0_COMPA_vect)
{
	OCR0A = frequency;
}

//Output Compare B Interrupt to set the value for OCR0B using the frequency and ADC values
ISR(TIMER0_COMPB_vect)
{
	if (ADC >= 0 && ADC < 100)
	{
		OCR0B = .05*frequency;
	}
	else if(ADC >= 100 && ADC < 200)
	{
		OCR0B = .10*frequency;
	}
	else if(ADC >= 200 && ADC < 300)
	{
		OCR0B = .15*frequency;
	}
	else if(ADC >= 300 && ADC < 400)
	{
		OCR0B = .20*frequency;
	}
	else if(ADC >= 400 && ADC < 500)
	{
		OCR0B = .25*frequency;
	}
	else if(ADC >= 500 && ADC < 600)
	{
		OCR0B = .30*frequency;
	}
	else if(ADC >= 600 && ADC < 700)
	{
		OCR0B = .35*frequency;
	}
	else if(ADC >= 700 && ADC < 800)
	{
		OCR0B = .40*frequency;
	}
	else if(ADC >= 800 && ADC < 900)
	{
		OCR0B = .45*frequency;
	}
	else if(ADC >= 900 && ADC < 1000)
	{
		OCR0B = .50*frequency;
	}
}

int main(void)
{
	Initialize();
	UART_init(BAUD_PRESCALER);
	
	while(1)
	{
			if (ADC >= 2 && ADC <= 103)
			{
				sprintf(String,"ADC: %u, Duty Cycle: 5%%\n", ADC);
				sprintf(String,"Distance: %u", distance);
				UART_output(String);
			}
			else if(ADC >= 104 && ADC <= 205)
			{
				sprintf(String,"ADC Value: %u, Duty Cycle: 10%%\n", ADC);
				sprintf(String,"Distance: %u", distance);
				UART_output(String);
			}
			else if(ADC >= 206 && ADC <= 307)
			{
				sprintf(String,"ADC Value: %u, Duty Cycle: 15%%\n", ADC);
				sprintf(String,"Distance: %u", distance);
				UART_output(String);
			}
			else if(ADC >= 308 && ADC <= 409)
			{
				sprintf(String,"ADC Value: %u, Duty Cycle: 20%%\n", ADC);
				sprintf(String,"Distance: %u", distance);
				UART_output(String);
			}
			else if(ADC >= 410 && ADC <= 511)
			{
				sprintf(String,"ADC Value: %u, Duty Cycle: 25%%\n", ADC);
				sprintf(String,"Distance: %u", distance);
				UART_output(String);
			}
			else if(ADC >= 512 && ADC <= 613)
			{
				sprintf(String,"ADC Value: %u, Duty Cycle: 30%%\n", ADC);
				sprintf(String,"Distance: %u", distance);
				UART_output(String);
			}
			else if(ADC >= 614 && ADC <= 715)
			{
				sprintf(String,"ADC Value: %u, Duty Cycle: 35%%\n", ADC);
				sprintf(String,"Distance: %u", distance);
				UART_output(String);
			}
			else if(ADC >= 716 && ADC <= 817)
			{
				sprintf(String,"ADC Value: %u, Duty Cycle: 40%%\n", ADC);
				sprintf(String,"Distance: %u", distance);
				UART_output(String);
			}
			else if(ADC >= 818 && ADC <= 919)
			{
				sprintf(String,"ADC Value: %u, Duty Cycle: 45%%\n", ADC);
				sprintf(String,"Distance: %u", distance);
				UART_output(String);
			}
			else if(ADC >= 920)
			{
				sprintf(String,"ADC Value: %u, Duty Cycle: 50%%\n", ADC);
				sprintf(String,"Distance: %u", distance);
				UART_output(String);
			}
			


		}
	}
