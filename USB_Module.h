/*
 * USB_Module.h
 *
 * Created: 3/14/2022 6:19:23 PM
 *  Author: andre
 */ 


#ifndef USB_MODULE_H_
#define USB_MODULE_H_

#include "USB_Definitions.h"

void USB_Power_On(USBDescriptor *descriptor);
void USB_Power_Off();
void USB_Suspend();
void USB_Resume();

void USB_Register_Config();		//Will allow struct passed in with device/interface/endpoint config

extern volatile uint8_t buttons;
extern volatile uint8_t axes[4];

#endif /* USB_MODULE_H_ */