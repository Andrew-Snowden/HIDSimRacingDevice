/*
 * USB_Module.c
 *
 * Created: 3/14/2022 6:19:34 PM
 *  Author: andre
 */ 

#include "USB_Module.h"
#include "USB_Definitions.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "UART_Module.h"

static void USB_PLL_Init();
static void USB_PLL_Disable();
static void USB_PLL_Enable();
static void USB_Endpoint_Configure();

static USBDescriptor* usb_descriptor;
static volatile uint8_t config_state = 0;
volatile uint8_t buttons = 0;
volatile uint8_t axes[4] = {0, 0, 0, 0};

ISR(USB_GEN_vect)
{
	if (UDINT & (1 << EORSTI))	//End of reset
	{
		USB_Endpoint_Configure();
		UDINT &= ~(1 << EORSTI);
		
		UERST = 1;	//Reset endpoint
		UERST = 0;
		
		UEIENX |= (1 << RXSTPE);	//Enable receive setup packet interrupt
	}
	if (UDINT & (1 << SOFI) && config_state != 0)	//Start-of-frame packet received
	{
		UDINT &= ~(1 << SOFI);
	}
}

ISR(USB_COM_vect)
{
	uint8_t current_endpoint = 0;
	//See which endpoint
	for (int i = 0; i < 2; i++)
	{
		if (UEINT & (1 << i))
		{
			if (i == 0)
			{
				UART_Put_String("EP 0\n");
				UENUM = 0;
			}
			if (i == 1)
			{
				UENUM = 1;
				UART_Put_String("EP 1!\n");
			}
			current_endpoint = i;
			break;
		}
	}
	

	if (UEINTX & (1 << RXSTPI) && current_endpoint == 0) //Setup Packet Received
	{		
		USBRequestType bmRequestType = UEDATX;
		USBRequest bRequest = UEDATX;
		uint16_t wValue = UEDATX;
		wValue |= (UEDATX << 8);
		uint16_t wIndex = UEDATX;
		wIndex |= (UEDATX << 8);
		uint16_t wLength = UEDATX;
		wLength |= (UEDATX << 8);
		
		UEINTX &= ~( (1 << RXSTPI) | (1 << TXINI) );
		
		USBRequestDirection direction = (bmRequestType & 0x80) ? REQUEST_D2H : REQUEST_H2D;
		USBRequestType type = ((bmRequestType & 0x60) >> 5);
		USBRequestRecipient recipient = (bmRequestType & 0x1F);
		
		UART_Put_Num(wLength);
		
		switch (bRequest)
		{
			case GET_STATUS:
			{
				UART_Put_String("GET_STATUS\n");
			}
			break;
			
			case CLEAR_FEATURE:
			{
				UART_Put_String("CLEAR_FEATURE\n");
			}
			break;
			
			case SET_FEATURE:
			{
				UART_Put_String("SET_FEATURE\n");
			}
			break;
			
			case SET_ADDRESS:
			{
				uint8_t address = wValue & 0xFF;
				
				while (!(UEINTX & (1 << TXINI)));
				UEINTX &= ~(1 << TXINI);
				while (!(UEINTX & (1 << TXINI)));
				
				UDADDR = wValue | (1 << ADDEN);
					
				UART_Put_Char('A');
			}
			break;
			
			case GET_DESCRIPTOR:
			{		
				if (wValue == 0x0100) //Device Descriptor
				{
					while (!(UEINTX & (1 << TXINI)));
					
					UEDATX = usb_descriptor->device_descriptor.bLength;
					UEDATX = usb_descriptor->device_descriptor.bDescriptorType;
					UEDATX = usb_descriptor->device_descriptor.bcdUSB & 0x00FF;
					UEDATX = usb_descriptor->device_descriptor.bcdUSB >> 8;
					UEDATX = usb_descriptor->device_descriptor.bDeviceClass;
					UEDATX = usb_descriptor->device_descriptor.bDeviceSubClass;
					UEDATX = usb_descriptor->device_descriptor.bDeviceProtocol;
					UEDATX = usb_descriptor->device_descriptor.bMaxPacketSize0;
					UEDATX = usb_descriptor->device_descriptor.idVendor & 0x00FF;
					UEDATX = usb_descriptor->device_descriptor.idVendor >> 8;
					UEDATX = usb_descriptor->device_descriptor.idProduct & 0x00FF;
					UEDATX = usb_descriptor->device_descriptor.idProduct >> 8;
					UEDATX = usb_descriptor->device_descriptor.bcdDevice & 0x00FF;
					UEDATX = usb_descriptor->device_descriptor.bcdDevice >> 8;
					UEDATX = usb_descriptor->device_descriptor.iManufacturer;
					UEDATX = usb_descriptor->device_descriptor.iProduct;
					UEDATX = usb_descriptor->device_descriptor.iSerialNumber;
					UEDATX = usb_descriptor->device_descriptor.bNumConfigurations;
					
					UEINTX &= ~(1 << TXINI);	//Send packet			
					
					while (!(UEINTX & (1 << RXOUTI)));
					UEINTX &= ~(1 << RXOUTI);
					UART_Put_Char('V');
				}
				else if (wValue == 0x0200) //Configuration Descriptor
				{
					while (!(UEINTX & (1 << TXINI)));
					
					
					if (wLength == 9)
					{
						UEDATX = usb_descriptor->config_descriptor.bLength;
						UEDATX = usb_descriptor->config_descriptor.bDescriptorType;
						UEDATX = usb_descriptor->config_descriptor.wTotalLength & 0xFF;
						UEDATX = usb_descriptor->config_descriptor.wTotalLength >> 8;
						UEDATX = usb_descriptor->config_descriptor.bNumInterfaces;
						UEDATX = usb_descriptor->config_descriptor.bConfigurationValue;
						UEDATX = usb_descriptor->config_descriptor.iConfiguration;
						UEDATX = usb_descriptor->config_descriptor.bmAttributes;
						UEDATX = usb_descriptor->config_descriptor.MaxPower;
					}					
					else
					{
						//Populate Config Descriptor
						UEDATX = usb_descriptor->config_descriptor.bLength;
						UEDATX = usb_descriptor->config_descriptor.bDescriptorType;
						UEDATX = usb_descriptor->config_descriptor.wTotalLength & 0xFF;
						UEDATX = usb_descriptor->config_descriptor.wTotalLength >> 8;
						UEDATX = usb_descriptor->config_descriptor.bNumInterfaces;
						UEDATX = usb_descriptor->config_descriptor.bConfigurationValue;
						UEDATX = usb_descriptor->config_descriptor.iConfiguration;
						UEDATX = usb_descriptor->config_descriptor.bmAttributes;	
						UEDATX = usb_descriptor->config_descriptor.MaxPower;	
					
						//Populate Interface Descriptor
						UEDATX = usb_descriptor->interface_descriptor.bLength;
						UEDATX = usb_descriptor->interface_descriptor.bDescriptorType;
						UEDATX = usb_descriptor->interface_descriptor.bInterfaceNumber;
						UEDATX = usb_descriptor->interface_descriptor.bAlternateSetting;
						UEDATX = usb_descriptor->interface_descriptor.bNumEndpoints;
						UEDATX = usb_descriptor->interface_descriptor.bInterfaceClass;
						UEDATX = usb_descriptor->interface_descriptor.bInterfaceSubClass;
						UEDATX = usb_descriptor->interface_descriptor.bInterfaceProtocol;
						UEDATX = usb_descriptor->interface_descriptor.iInterface;
			
						//Populate HID Descriptor
						UEDATX = usb_descriptor->hid_descriptors[0].bLength;
						UEDATX = usb_descriptor->hid_descriptors[0].bDescriptorType;
						UEDATX = usb_descriptor->hid_descriptors[0].bcdHID & 0xFF;
						UEDATX = usb_descriptor->hid_descriptors[0].bcdHID >> 8;
						UEDATX = usb_descriptor->hid_descriptors[0].bCountryCode;
						UEDATX = usb_descriptor->hid_descriptors[0].bNumDescriptors;
						UEDATX = usb_descriptor->hid_descriptors[0].bDescriptorTypeReport;
						UEDATX = usb_descriptor->hid_descriptors[0].wDescriptorLength & 0xFF;
						UEDATX = usb_descriptor->hid_descriptors[0].wDescriptorLength >> 8;

						//Populate Endpoint Descriptor
						UEDATX = usb_descriptor->endpoint_descriptors[0].bLength;
						UEDATX = usb_descriptor->endpoint_descriptors[0].bDescriptorType;
						UEDATX = usb_descriptor->endpoint_descriptors[0].bEndpointAddress;
						UEDATX = usb_descriptor->endpoint_descriptors[0].bmAttributes;
						UEDATX = usb_descriptor->endpoint_descriptors[0].wMaxPacketSize & 0xFF;
						UEDATX = usb_descriptor->endpoint_descriptors[0].wMaxPacketSize >> 8;
						UEDATX = usb_descriptor->endpoint_descriptors[0].bInterval;
					}
					
					UEINTX &= ~(1 << TXINI);	//Send packet 					
					
					while (!(UEINTX & (1 << RXOUTI)));
					UEINTX &= ~(1 << RXOUTI);
					UART_Put_Char('C');
				}
				else if (wValue == 0x2100) //HID Descriptor
				{
					UART_Put_String("HID\n");
				}
				else if (wValue == 0x2200) //HID Report
				{
					while (!(UEINTX & (1 << TXINI)));
					
					for (int i = 0; i < usb_descriptor->report_length; i++)
					{
						UEDATX = usb_descriptor->hid_report[i];
					}
					
					UEINTX &= ~(1 << TXINI);
					
					while (!(UEINTX & (1 << RXOUTI)));
					UEINTX &= ~(1 << RXOUTI);
					
					UART_Put_String("Report\n");
				}
				else
				{
					while (!(UEINTX & (1 << TXINI)));
					UECONX |= (1 << STALLRQ) | (1 << EPEN);  // Enable the endpoint and stall, the
															// descriptor does not exist
					
					UEINTX &= ~( (1 << RXOUTI) | (1 << TXINI) );	
					UART_Put_Char('S');		
							
					UART_Put_String("bmRequestType: ");
					UART_Put_Hex(bmRequestType);
					UART_Put_Char('\n');
					
					UART_Put_String("bRequest: ");
					UART_Put_Hex(bRequest);
					UART_Put_Char('\n');
					
					UART_Put_String("wValue: ");
					UART_Put_Hex(wValue);
					UART_Put_Char('\n');
					
					UART_Put_String("wIndex: ");
					UART_Put_Hex(wIndex);
					UART_Put_Char('\n');
					
					UART_Put_String("wLength: ");
					UART_Put_Hex(wLength);
					UART_Put_Char('\n');
				}
			}
			break;
			
			case SET_DESCRIPTOR:
			{
				UART_Put_String("SET_DESCRIPTOR\n");
			}
			break;
			
			case GET_CONFIGURATION:
			{
				while (!(UEINTX & (1 << TXINI)));
				
				UEDATX = config_state;
				
				UEINTX &= ~(1 << TXINI);
				
				while (!(UEINTX & (1 << RXOUTI)));
				UEINTX &= ~(1 << RXOUTI);
					
				UART_Put_String("GET_CONFIGURATION\n");
			}
			break;
			
			case SET_CONFIGURATION:
			{
				config_state = wValue & 0xFF;
				while (!(UEINTX & (1 << TXINI)));
				UEINTX &= ~(1 << TXINI);
				
				while (!(UEINTX & (1 << TXINI)));
				
				//Select EP 1
				UENUM = 1;
				
				//Enable endpoint (EPEN = 1)
				UECONX |= (1 << EPEN);
				
				//Configure endpoint
				UECFG0X = 0xC1;		//Interrupt EP, IN
				
				UECFG1X &= ~((1 << EPSIZE0) | (1 << EPSIZE1) | (1 << EPSIZE2) | (3 << EPBK0));	//8-byte, Allocate
				UECFG1X |= (1 << ALLOC);
				
				/*UEINTX = 0;
				UERST = 0x1E;          // Reset all of the endpoints
				UERST = 0;*/
				
				UEIENX |= (1 << TXINE);
				
				//Test configuration (CFGOK == 1)
				while (!(UESTA0X & (1 << CFGOK)));		//Enters if config is bad
				{
					//UART_Put_String("Bad Config\n");
				}
				
				UART_Put_String("SET_CONFIGURATION: ");
				UART_Put_Num(config_state);
				UART_Put_Char('\n');
			}
			break;
			
			case GET_INTERFACE:
			{
				while (!(UEINTX & (1 << TXINI)));
				if (wLength == 0)
				{
					UEINTX &= ~(1 << TXINI);
				}
				else
				{
					UEDATX = 0x00;
					
					UEINTX &= ~(1 << TXINI);
					
					while (!(UEINTX & (1 << RXOUTI)));
				}
				
				
				UEINTX &= ~(1 << RXOUTI);
				UART_Put_String("GET_INTERFACE\n");
			}
			break;
			
			case SET_INTERFACE:
			{
				UART_Put_String("SET_INTERFACE\n");
			}
			break;
			
			case SYNCH_FRAME:
			{
				UART_Put_String("SYNCH_FRAME\n");
			}
			break;
			
			default:
			{
				UART_Put_String("Default\n");
			}
			break;
		}
		
		UART_Put_Char('\n');
	}
	else if (UEINTX & (1 << TXINI) && current_endpoint == 1)
	{
		UEINTX &= ~(1 << TXINI);
		
		UEDATX = buttons;
	
		
		for (int i = 0; i < 3; i++)
		{
			UEDATX = axes[i];
		}
		
		UEDATX = 0;
		
		UEINTX &= ~(1 << FIFOCON);
		
	}
}

void USB_Power_On(USBDescriptor *descriptor)
{
	usb_descriptor = descriptor;
	
	cli();
	UENUM = 0;
	// disable USB general interrupts
	USBCON &= 0b11111110;
	// disable all USB device interrupts
	UDIEN &= 0b10000010;
	// disable USB endpoint interrupts
	UEIENX &= 0b00100000;
	
	UENUM = 1;
	// disable USB general interrupts
	USBCON &= 0b11111110;
	// disable all USB device interrupts
	UDIEN &= 0b10000010;
	// disable USB endpoint interrupts
	UEIENX &= 0b00100000;
	sei();
	
	//Turn on USB Pad regulator
	UHWCON |= (1 << UVREGE);
	
	//Enable PLL
	USB_PLL_Init();
	
	UART_Put_String("Before EN\n");
	//Enable USB interface
	USBCON |= (1 << USBE) | (1 << OTGPADE);
	USBCON &= ~(1 << FRZCLK);
	UART_Put_String("After EN\n");	
	
	//Full-Speed mode
	UDCON &= ~(1 << LSM);
	
	//Endpoint configure (although configuration will be lost after hosts issues device reset...)
	//USB_Endpoint_Configure();
	
	for (uint8_t i = 2; i <= 6; i++) {
		UENUM = (UENUM & 0xF8) | i;   // select endpoint i
		UECONX &= ~(1 << EPEN);
	}
	
	//Wait for VBUS connection (should be connected considering device is on and not self-powered...)
	while (!(USBSTA & (1 << VBUS))); UART_Put_String("VBUS Good\n");
	
	//Attach device (Clear DETACH)
	UDCON &= ~(1 << DETACH);
	
	UDIEN |= (1<<EORSTE);	//Enable end-of-reset interrupt
	
}

void USB_Power_Off()
{
	//Detach USB
	UDCON |= (1 << DETACH);
	
	//Disable USB
	USBCON &= ~(1 << USBE);
	
	//Disable PLL
	USB_PLL_Disable();
	
	//Disable USB Pad
	UHWCON &= ~(1 << UVREGE);
}

void USB_Suspend()
{
	//Clear Suspend bit
	
	//Freeze USB clock
	
	//Disable PLL
	
	//Ensure interrupts to exit sleep mode
	
	//Make the MCU enter sleep mode
}

void USB_Resume()
{
	//Enable PLL
	
	//Wait PLL Lock
	
	//Unfreeze USB clock
	
	//Clear resume information
}

static void USB_Endpoint_Configure()
{
	//Configure Endpoint 0 (default control)------------------------------------
	
	//Select endpoint (UENUM.EPNUM = 0)
	UENUM = (UENUM & 0xF8) | 0;
	
	//Enable endpoint (EPEN = 1)
	UECONX |= (1 << EPEN);
	
	//Configure endpoint
	UECFG0X = 0;		//Control Endpoint, OUT
	
	UECFG1X |= (1 << EPSIZE0) | (1 << EPSIZE1);	//64-byte, Allocate
	UECFG1X &= ~(1 << EPSIZE2);
	
	UECFG1X |= (1 << ALLOC);
	
	UEINTX = 0;
	
	//Test configuration (CFGOK == 1)
	while (!(UESTA0X & (1 << CFGOK)));		//Enters if config is bad
	{
		//UART_Put_String("Bad Config\n");
	}

	//UART_Put_String("Good Config 0\n");
	


	

	//UART_Put_String("Good Config 1\n");
	
	
}

static void USB_PLL_Init()
{
	//PINDIV = /2
	PLLCSR |= (1 << PINDIV) | (1 << PLLE);
	
	//PINMUX = 0
	//PLLFRQ &= ~(1 << PINMUX);
	
	//PLLUSB = 1
	//PLLFRQ |= (1 << PLLUSB);
	
	//PDIV = 96MHz
	//PLLFRQ |= 0xA;
	
	
	USB_PLL_Enable();
}

static void USB_PLL_Disable()
{
	//Disable PLL
	PLLCSR &= ~(1 << PLLE);
}

static void USB_PLL_Enable()
{
	//Enable PLL
	//PLLCSR |= (1 << PLLE);
	
	//Wait for lock
	while(!(PLLCSR & (1 << PLOCK)));
}