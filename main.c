/*
 * USBProject.c
 *
 * Created: 3/12/2022 4:13:26 PM
 * Author : andre
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include "USB_Module.h"
#include "USB_Definitions.h"
#include "Tasks.h"
#include "Timers.h"
#include "UART_Module.h"
#include "Encoder_Module.h"

#define ADC4 0x4
#define ADC5 0x5
#define ADC6 0x6

void GPIO_Init();
void USB_Descriptor_Init(USBDescriptor *descriptor);
void ADC_Init();
void Buttons_Update();
void ADC_Read();
void ADC_Select(uint8_t selection);

int main(void)
{
	USBDescriptor usb_descriptor;
	
	Timers_Init();
	GPIO_Init();
	UART_Init();
	Encoder_Init();
	ADC_Init();
	USB_Descriptor_Init(&usb_descriptor);
	
	USB_Power_On(&usb_descriptor);	
	
    while (1) 
    {
		Buttons_Update();
		ADC_Read();
	}
}

void ADC_Init()
{
	ADMUX |= (1 << ADLAR) | (1 << REFS1);	//Left-adjust
	ADC_Select(ADC6);
	
	ADCSRA |= (7 << ADPS0);
	ADCSRA |= (1 << ADEN);
	
	DIDR0 |= (1 << ADC4D) | (1 << ADC5D) | (1 << ADC6D);
}

void ADC_Select(uint8_t selection)
{
	ADMUX = (ADMUX & 0xE0) | (selection & 0x1F);
}

void ADC_Read()
{
	ADC_Select(ADC4);
	ADCSRA |= (1 << ADSC);
	while(!(ADCSRA & (1 << ADIF)));
	axes[0] = ADCH;
	ADCSRA |= (1 << ADIF);
	
	ADC_Select(ADC5);
	ADCSRA |= (1 << ADSC);
	while(!(ADCSRA & (1 << ADIF)));
	axes[1] = ADCH;
	ADCSRA |= (1 << ADIF);
	
	ADC_Select(ADC6);
	ADCSRA |= (1 << ADSC);
	while(!(ADCSRA & (1 << ADIF)));
	axes[2] = ADCH;
	ADCSRA |= (1 << ADIF);
}

void Buttons_Update()
{
	if (PINB & (1 << PINB1))
	{
		buttons |= (1 << 0);
	}
	else
	{
		buttons &= ~(1 << 0);
	}
	
	if (PINB & (1 << PINB3))
	{
		buttons |= (1 << 1);
	}
	else
	{
		buttons &= ~(1 << 1);
	}
	
	if (PINB & (1 << PINB2))
	{
		buttons |= (1 << 2);
	}
	else
	{
		buttons &= ~(1 << 2);
	}
	
	if (PINB & (1 << PINB6))
	{
		buttons |= (1 << 3);
	}
	else
	{
		buttons &= ~(1 << 3);
	}
	
	if (PINB & (1 << PINB4))
	{
		buttons |= (1 << 4);
	}
	else
	{
		buttons &= ~(1 << 4);
	}
	
	if (PINE & (1 << PINE6))
	{
		buttons |= (1 << 5);
	}
	else
	{
		buttons &= ~(1 << 5);
	}
}

void USB_Descriptor_Init(USBDescriptor *descriptor)
{
	//Configure Device Descriptor
	DeviceDescriptor *device = &(descriptor->device_descriptor);
	device->bLength = 18;
	device->bDescriptorType = 0x01;
	device->bcdUSB = 0x0110;
	device->bDeviceClass = 0x00;
	device->bDeviceSubClass = 0x00;
	device->bDeviceProtocol = 0x00;
	device->bMaxPacketSize0 = 64;
	device->idVendor = 0x03ED;
	device->idProduct = 0x2FF4;
	device->bcdDevice = 0x0100;
	device->iManufacturer = 0x00;
	device->iProduct = 0x00;
	device->iSerialNumber = 0x00;
	device->bNumConfigurations = 1;
	
	//Configure Configuration Descriptor
	ConfigurationDescriptor *config = &(descriptor->config_descriptor);
	config->bLength = 0x09;
	config->bDescriptorType = 0x02;
	config->wTotalLength = 0x22;
	config->bNumInterfaces = 0x01;
	config->bConfigurationValue = 0x01;
	config->iConfiguration = 0x00;
	config->bmAttributes = 0x80;
	config->MaxPower = 0x32;
	
	//Configure Interface Descriptor
	InterfaceDescriptor *interface = &(descriptor->interface_descriptor);
	interface->bLength = 0x09;
	interface->bDescriptorType = 0x04;
	interface->bInterfaceNumber = 0x00;
	interface->bAlternateSetting = 0x00;
	interface->bNumEndpoints = 0x01;
	interface->bInterfaceClass = 0x03;
	interface->bInterfaceSubClass = 0x00;
	interface->bInterfaceProtocol = 0x00;
	interface->iInterface = 0x00;
	
	//Configure Endpoint Descriptor
	EndpointDescriptor *endpoint = &(descriptor->endpoint_descriptors[0]);
	endpoint->bLength = 0x07;
	endpoint->bDescriptorType = 0x05;
	endpoint->bEndpointAddress = 0x81;
	endpoint->bmAttributes = 0x03;
	endpoint->wMaxPacketSize = 0x8;
	endpoint->bInterval = 0xA;
	
	//Configure HID Descriptor
	HIDDescriptor *hid = &(descriptor->hid_descriptors[0]);
	hid->bLength = 0x09;
	hid->bDescriptorType = 0x21;
	hid->bcdHID = 0x111;
	hid->bCountryCode = 0x00;
	hid->bNumDescriptors = 0x01;
	hid->bDescriptorTypeReport = 0x22;
	hid->wDescriptorLength = 0x35; //53 bytes
	
	//Configure Report Descriptor
	uint8_t const gamepad_report[53] = {
		0x05, 0x01,	//Generic Desktop
		0x09, 0x05, //Game Pad
		0xA1, 0x01,	//Collection(App)
		0xA1, 0x00,		//Collection (Physical)
		0x05, 0x09,			//Usage Page (Button)
		0x19, 0x01,			//Usage Min (Button 1)
		0x29, 0x06,			//Usage Max (Button 6)
		0x15, 0x00,			//Logical Min (0)
		0x25, 0x01,			//Logical Max (1)
		0x95, 0x06,			//Report Count (6)
		0x75, 0x01,			//Report Size (1)
		0x81, 0x02,			//Input (Variable)
		0x95, 0x01,			//Report Count (1)
		0x75, 0x02,			//Report Size (2)
		0x81, 0x01,			//Input (Cnst)
		0x05, 0x01,			//Usage Page (Generic)
		0x09, 0x30,			//Usage (x)
		0x09, 0x31,			//Usage (y)
		0x09, 0x32,			//Usage (z)
		0x09, 0x33,			//Usage (Rx)
		0x15, 0x00,			//Logical Min (0)
		0x26, 0xFF, 0x00,	//Logical Max (255)
		0x75, 0x08,			//Report Size (8)
		0x95, 0x04,			//Report Count (4)
		0x81, 0x82,			//Input (Variable)
		0xC0,			//End Collection
		0xC0		//End Collection
	};
	
	for (int i = 0; i < 53; i++)
	{
		descriptor->hid_report[i] = gamepad_report[i];
	}
	
	descriptor->report_length = 53;
}

void GPIO_Init()
{
	PORTB &= ~(1 << PORTB5);
	DDRB |= (1 << DDB5);

	DDRB &= ~(1 << DDB2);
	
	//Turn off RX LED (Because it's annoying and bright)
	PORTB &= ~(1 << PORTB0);
	DDRB &= ~(1 << DDB0);
	
	
	//Button1 - PB1
	PORTB &= ~(1 << PORTB1);
	DDRB &= ~(1 << DDB1);
	
	//Button2 - PB3
	PORTB &= ~(1 << PORTB3);
	DDRB &= ~(1 << DDB3);
	
	//Button3 - PB2
	PORTB &= ~(1 << PORTB2);
	DDRB &= ~(1 << DDB2);
	
	//Button4 - PB6
	PORTB &= ~(1 << PORTB6);
	DDRB &= ~(1 << DDB6);
	
	//Button5 - PB4
	PORTB &= ~(1 << PORTB4);
	DDRB &= ~(1 << DDB4);
	
	//Button6 - PE6
	PORTE &= ~(1 << PORTE6);
	DDRE &= ~(1 << DDE6);
	
	
}

