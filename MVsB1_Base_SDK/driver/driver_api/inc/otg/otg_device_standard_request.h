

#ifndef __OTG_DEVICE_STANDARD_H__
#define	__OTG_DEVICE_STANDARD_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include "type.h"

#define USB_DT_DEVICE					1
#define USB_DT_CONFIG					2
#define USB_DT_STRING					3
#define USB_DT_INTERFACE				4
#define USB_DT_ENDPOINT					5
#define USB_DT_DEVICE_QUALIFIER			6
#define USB_HID_REPORT					0x22

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


// Max packet size. Fixed, user should not modify.
#define	DEVICE_FS_CONTROL_MPS		64
#define	DEVICE_FS_INT_IN_MPS		64
#define	DEVICE_FS_BULK_IN_MPS		64
#define	DEVICE_FS_BULK_OUT_MPS		64
#define	DEVICE_FS_ISO_IN_MPS		192
#define	DEVICE_FS_ISO_OUT_MPS		192

// Endpoint number. Fixed, user should not modify.
#define	DEVICE_CONTROL_EP			0x00
#define	DEVICE_INT_IN_EP			0x81
#define	DEVICE_BULK_IN_EP			0x82
#define	DEVICE_BULK_OUT_EP			0x03
#define	DEVICE_ISO_IN_EP			0x84
#define	DEVICE_ISO_OUT_EP			0x05

#define MSC_INTERFACE_NUM			0
#define AUDIO_ATL_INTERFACE_NUM		1
#define AUDIO_SRM_OUT_INTERFACE_NUM	2
#define AUDIO_SRM_IN_INTERFACE_NUM	3
#define HID_CTL_INTERFACE_NUM		4
#define HID_DATA_INTERFACE_NUM		5

#define USB_VID				0x8888
#define USB_PID_BASE		0x1717//具体PID叠加下列功能值作为Offset
#define AUDIO_ONLY			0
#define MIC_ONLY			1
#define AUDIO_MIC			2
#define READER				3
#define AUDIO_READER		4
#define MIC_READER			5
#define AUDIO_MIC_READER	6
#define HID					7
#define USBPID(x)			(USB_PID_BASE + x)

//用于在线调音
#define HID_DATA_FUN_EN	1
//MIC上传的声道数
#define MIC_CH	2

void OTG_DeviceModeSel(uint8_t Mode,uint16_t UsbVid,uint16_t UsbPid);
void OTG_DeviceRequestProcess(void);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__OTG_DEVICE_STANDARD_H__

