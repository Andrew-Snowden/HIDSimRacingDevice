/*
 * Encoder_Module.c
 *
 * Created: 3/16/2022 8:21:08 PM
 *  Author: andre
 */ 

#include "Encoder_Module.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

static volatile int32_t rotary_encoder_position = 0;

int32_t Encoder_Get_Position()
{
	return rotary_encoder_position;
}

void Encoder_Init()
{
	rotary_encoder_position = 0;
	
	//PORTD0: no pull-up, input (A Phase)
	PORTD &= ~(1 << PORTD0);
	DDRD &= ~(1 << DDD0);
	
	//PORTD1: no pull-up, input (B Phase)
	PORTD &= ~(1 << PORTD1);
	DDRD &= ~(1 << DDD1);
	
	//EICRA.ISC0 = 3 (Positive edge triggered)
	EICRA |= (1 << ISC00);
	
	//INT0 = 1 (Enable external interrupt 0)
	EIMSK |= (1 << INT0);
}

ISR(INT0_vect)
{
	//PORTB |= (1 << PORTB5);
	if ( (!(PIND & (1 << PIND0)) && (PIND & (1 << PIND1))) || ((PIND & (1 << PIND0)) && !(PIND & (1 << PIND1))))	//Forward
	{
		rotary_encoder_position++;
	}
	else		//Backward
	{
		rotary_encoder_position--;
	}
	
}