/*
 * Timers.c
 *
 * Created: 3/15/2022 10:28:03 PM
 *  Author: andre
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

static void Timer0_Init();
static void Timer1_Init();
static void Timer2_Init();

void Timers_Init()
{
	Timer0_Init();
	Timer1_Init();
	Timer2_Init();
}

static void Timer0_Init()
{
	cli();
	
	TCCR0A |= (2 << WGM00);		//clear timer on compare
	TCCR0B |= (5 << CS00);		//Use /1024 prescaler
	OCR0A = 252;				//Set compare value: 252. 62 output compares is 500ms
	TIFR1 |= (1 << OCF1A);
	TCNT1 = 0;
	TIMSK0 |= (1 << OCIE0A);	//Enable interrupt
	
	sei();
}

static void Timer1_Init()
{
	
}

static void Timer2_Init()
{
	
}

static volatile uint8_t count_value = 0;

ISR(TIMER0_COMPA_vect)
{
	count_value++;

	if (count_value == 62)
	{
		PINB |= (1 << PINB5);
		count_value = 0;
	}
	
}