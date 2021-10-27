/**
 *****************************************************************************
 * @file     standard request.h
 * @author   Orson
 * @version  V1.0.0
 * @date     24-June-2013
 * @brief    standard module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#ifndef __STANDARD_REQUEST_H__
#define	__STANDARD_REQUEST_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
	
#include "type.h"


//USB types, the second of three bRequestType fields
#define USB_TYPE_MASK			(0x03 << 5)
#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

//USB recipients, the third of three bRequestType fields
#define USB_RECIP_MASK			0x1F
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03


//Standard requests, for the bRequest field of a SETUP packet.
//These are qualified by the bRequestType field, so that for example
//TYPE_CLASS or TYPE_VENDOR specific feature flags could be retrieved
//by a GET_STATUS request.
#define USB_REQ_GET_STATUS				0
#define USB_REQ_CLEAR_FEATURE			1
#define USB_REQ_SET_FEATURE				3
#define USB_REQ_SET_ADDRESS				5
#define USB_REQ_GET_DESCRIPTOR			6
#define USB_REQ_SET_DESCRIPTOR			7
#define USB_REQ_GET_CONFIGURATION		8
#define USB_REQ_SET_CONFIGURATION		9
#define USB_REQ_GET_INTERFACE			10
#define USB_REQ_SET_INTERFACE			11
#define USB_REQ_SYNCH_FRAME				12

//USB feature flags are written using USB_REQ_{CLEAR,SET}_FEATURE,
//and are read as a bit array returned by USB_REQ_GET_STATUS.
//(So there are at most sixteen features of each type.)
#define USB_DEVICE_SELF_POWERED			0	//(read only) from GET_STATUS request
#define USB_DEVICE_REMOTE_WAKEUP		1	//dev may initiate wakeup
#define USB_DEVICE_TEST_MODE			2	//(high speed only)
#define USB_DEVICE_B_HNP_ENABLE			3	//dev may initiate HNP
#define USB_DEVICE_A_HNP_SUPPORT		4	//RH port supports HNP 
#define USB_DEVICE_A_ALT_HNP_SUPPORT	5	//other RH port does
#define USB_DEVICE_DEBUG_MODE			6	//(special devices only)

#define USB_ENDPOINT_HALT				0	//IN/OUT will STALL



//STANDARD DESCRIPTORS ... as returned by GET_DESCRIPTOR,
//or (rarely) accepted by SET_DESCRIPTOR.
//Note that all multi-byte values here are encoded in little endian
//byte order "on the wire".  But when exposed through Linux-USB APIs,
//they've been converted to cpu byte order.
#define USB_DT_DEVICE					1
#define USB_DT_CONFIG					2
#define USB_DT_STRING					3
#define USB_DT_INTERFACE				4
#define USB_DT_ENDPOINT					5
#define USB_DT_DEVICE_QUALIFIER			6
#define USB_DT_OTHER_SPEED_CONFIG		7
#define USB_DT_INTERFACE_POWER			8

//these are from a minor usb 2.0 revision (ECN)
#define USB_DT_OTG						9
#define USB_DT_DEBUG					10
#define USB_DT_INTERFACE_ASSOCIATION	11

//conventional codes for class-specific descriptors
#define USB_DT_CS_DEVICE		0x21
#define USB_DT_CS_CONFIG		0x22
#define USB_DT_CS_STRING		0x23
#define USB_DT_CS_INTERFACE		0x24
#define USB_DT_CS_ENDPOINT		0x25

#define USB_DT_DEVICE_SIZE		18

//Device and/or Interface Class codes as found in bDeviceClass or bInterfaceClass
//and defined by www.usb.org documents
#define USB_CLASS_PER_INTERFACE		0	//for DeviceClass
#define USB_CLASS_AUDIO				1
#define USB_CLASS_COMM				2
#define USB_CLASS_HID				3
#define USB_CLASS_PHYSICAL			5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER			7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB				9
#define USB_CLASS_CDC_DATA			10
#define USB_CLASS_CSCID				11	//chip+ smart card
#define USB_CLASS_CONTENT_SEC		13	//content security
#define USB_CLASS_VIDEO				14
#define USB_CLASS_APP_SPEC			0xFE
#define USB_CLASS_VENDOR_SPEC		0xFF
#define USB_CLASS_WIRELESS          0xE0


#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_ENDPOINT_AUDIO_SIZE	9	//Audio extension


//设备描述符
typedef struct _USB_DEVICE_DESCRIPTOR {
	uint8_t		bLength;
	uint8_t		bDescriptorType;
	uint16_t	bcdUSB;
	uint8_t		bDeviceClass;
	uint8_t		bDeviceSubClass;
	uint8_t		bDeviceProtocol;
	uint8_t		bMaxPacketSize0;
	uint16_t	idVendor;
	uint16_t	idProduct;
	uint16_t	bcdDevice;
	uint8_t		iManufacturer;
	uint8_t		iProduct;
	uint8_t		iSerialNumber;
	uint8_t		bNumConfigurations;
}USB_DEVICE_DESCRIPTOR, *PUSB_DEVICE_DESCRIPTOR;


//配置描述符
typedef struct _USB_CONFIGURATION_DESCRIPTOR {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t MaxPower;
} USB_CONFIGURATION_DESCRIPTOR, *PUSB_CONFIGURATION_DESCRIPTOR;


/**
 * USB_DT_INTERFACE: Interface descriptor
 */
typedef struct _USB_INTERFACE_DESCRIPTOR
{
	uint8_t  	bLength;
	uint8_t  	bDescriptorType;
	uint8_t  	bInterfaceNumber;
	uint8_t  	bAlternateSetting;
	uint8_t  	bNumEndpoints;
	uint8_t  	bInterfaceClass;
	uint8_t  	bInterfaceSubClass;
	uint8_t  	bInterfaceProtocol;
	uint8_t  	iInterface;
} USB_INTERFACE_DESCRIPTOR, *PUSB_INTERFACE_DESCRIPTOR;



typedef struct _USB_FUN
{
	uint8_t Length;
	uint8_t *pData;
}USB_FUN;


#define		MAX_USB_SN_LEN			50
#define 	MAX_DESCRIPTOR_LEN		64	//512
#define 	MAXINTERFACE			6
#define		MAX_USB_STRING_LEN		50
typedef struct _OTG_HOST_INFO
{
	uint8_t DeviceAddress;
	USB_DEVICE_DESCRIPTOR DeviceDesCriptor;//18 BYTE
	USB_CONFIGURATION_DESCRIPTOR ConfigDesCriptor;//9 BYTE
	uint8_t DesCriptorBuffer[MAX_DESCRIPTOR_LEN];//512 BYTE
	
	USB_FUN UsbInterface[MAXINTERFACE];//解析后的接口描述符数据指针
	uint8_t UsbInterfaceNum;//接口有效数
	
	uint8_t SN[MAX_USB_STRING_LEN];	//Serial number, end with "\0\0"
	uint8_t	LenSN;	//length of SN
}OTG_HOST_INFO;

extern OTG_HOST_INFO OtgHostInfo;
extern uint8_t gMaxPacketSize0;

extern uint8_t OTG_HostInit(void);
bool OTG_HostEnumDevice(void);
#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
