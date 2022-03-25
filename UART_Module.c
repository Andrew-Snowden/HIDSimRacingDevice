/*
 * UART_Module.c
 *
 * Created: 3/16/2022 12:45:59 AM
 *  Author: andre
 */ 

#include "UART_Module.h"

#include <avr/io.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

void UART_Init()
{	
	//UBRRn = 25 (Baud Rate = 38400)
	UBRR1L = 25;
	
	//TXEN = 1 (Enable TX)
	UCSR1B |= (1 << TXEN1);
	
	//UMSELn = 0 (Asynchronous)
	UCSR1C &= ~(3 << UMSEL10);
	
	//U2Xn = 1 (Double-Speed. Baud-rate = 76800)
	UCSR1A |= (1 << U2X1);
	
	//UCSZn = (8 data bits)
	UCSR1C |= (3 << UCSZ10);
	
	//UPMn = 0 (no parity)
	UCSR1C &= ~(3 << UPM10);
	
	//USBSn = (1 stop bit)
	UCSR1C &= ~(1 << USBS1);
}

void UART_Put_Char(uint8_t character)
{
	while (!(UCSR1A & (1 << UDRE1)));
	
	UDR1 = character;
	
	if (character == '\n')
	{
		while (!(UCSR1A & (1 << UDRE1)));
		
		UDR1 = '\r';
	}
}

void UART_Put_String(uint8_t * string)
{
	for (uint16_t i = 0; i < strlen(string); i++)
	{
		UART_Put_Char(string[i]);
		if (string[i] == '\n')
		{
			UART_Put_Char('\r');
		}
	}
}

void UART_Put_Num(int32_t number)
{
	char result[10] = {0};
	itoa(number, result, 10);
	result[9] = '\0';
	
	UART_Put_String(result);
}

void UART_Put_Hex(int32_t number)
{
	char result[10] = {0};
	itoa(number, result, 16);
	result[9] = '\0';
	
	UART_Put_String(result);
}