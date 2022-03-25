/*
 * UART_Module.h
 *
 * Created: 3/16/2022 12:45:38 AM
 *  Author: andre
 */ 


#ifndef UART_MODULE_H_
#define UART_MODULE_H_

#include <stdint.h>

void UART_Init();

void UART_Put_Char(uint8_t character);

void UART_Put_String(uint8_t * string);

void UART_Put_Num(int32_t number);

void UART_Put_Hex(int32_t number);

#endif /* UART_MODULE_H_ */