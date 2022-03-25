/*
 * USB_Definitions.h
 *
 * Created: 3/20/2022 12:50:38 PM
 *  Author: andre
 */ 


#ifndef USB_DEFINITIONS_H_
#define USB_DEFINITIONS_H_

#include <stdint.h>

typedef struct 
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
} DeviceDescriptor;

typedef struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t MaxPower;
} ConfigurationDescriptor;

typedef struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} InterfaceDescriptor;

typedef struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} EndpointDescriptor;

typedef struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	uint8_t bDescriptorTypeReport;
	uint8_t wDescriptorLength;
} HIDDescriptor;

typedef struct 
{
	DeviceDescriptor device_descriptor;
	ConfigurationDescriptor config_descriptor;
	InterfaceDescriptor interface_descriptor;
	EndpointDescriptor endpoint_descriptors[2];
	HIDDescriptor hid_descriptors[2];
	uint8_t hid_report[53];
	uint16_t report_length;
} USBDescriptor;

typedef enum
{
	GET_STATUS = 0x0,
	CLEAR_FEATURE = 0x1,
	SET_FEATURE = 0x3,
	SET_ADDRESS = 0x5,
	GET_DESCRIPTOR = 0x6,
	SET_DESCRIPTOR = 0x7,
	GET_CONFIGURATION = 0x8,
	SET_CONFIGURATION = 0x9,
	GET_INTERFACE = 0x0A,
	SET_INTERFACE = 0x11,
	SYNCH_FRAME = 0x12
} USBRequest;

typedef enum
{
	REQUEST_H2D = 0,
	REQUEST_D2H = 1
} USBRequestDirection;

typedef enum
{
	REQUEST_STANDARD = 0,
	REQUEST_CLASS = 1,
	REQUEST_VENDOR = 2
} USBRequestType;

typedef enum
{
	REQUEST_DEVICE = 0,
	REQUEST_INTERFACE = 1,
	REQUEST_ENDPOINT = 2,
	REQUEST_OTHER = 3
} USBRequestRecipient;


#endif /* USB_DEFINITIONS_H_ */