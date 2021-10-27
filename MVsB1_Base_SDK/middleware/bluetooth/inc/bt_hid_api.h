/**
 *******************************************************************************
 * @file    bt_hid_api.h
 * @author  Halley
 * @version V1.0.1
 * @date    27-Apr-2016
 * @brief   Hid related api
 *******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */


/**
 * @addtogroup Bluetooth
 * @{
 * @defgroup bt_hid_api bt_hid_api.h
 * @{
 */

#ifndef __BT_HID_API_H__
#define __BT_HID_API_H__

typedef enum{
	BT_STACK_EVENT_HID_NONE = 0,
	
	/**
	 *@biref
	 *	HID channel connected. Only if control and interrupt
	 *	channel are both connected.
	 *	The union params.bd_addr is valid
	 */
	BT_STACK_EVENT_HID_CONNECTED,

	/**
	 *@biref
	 *	HID channel disconnected. Only if control and interrupt
	 *	channel are both disconnected.
	 *	The union params.bd_addr is valid
	 */	
	BT_STACK_EVENT_HID_DISCONNECTED,

	/**
	 *@biref
	 *	HID channel receives data. 
	 *	The params.hidData->channel indicate which channel (control
	 *	or interrupt channel) has received the data
	 *	The union params.hidData->data indicate the data
	 *	The union params.hidData->dataLen indicate the length of
	 *	received data.And the paramsLen is equal to dataLen.
	 */ 
	BT_STACK_EVENT_HID_DATA_RECEIVED,

	/**
	 *@biref
	 *	HID channel has sent the data. 
	 *	The union params.hidData is valid and the data in hidData
	 *	can be released or reused in this callback event.
	 */ 
	BT_STACK_EVENT_HID_DATA_SENT
}BT_HID_CALLBACK_EVENT;


/**
 * SDP_ATTRIBUTE structure
 *
 *     Defines an attribute's ID and value.
 */
typedef struct _SDPAttribute		SDPAttribute;


/**
 * SDP_ATTRIBUTE structure
 *
 *     Defines an attribute's ID and value.
 */
struct _SDPAttribute
{
	/* Attribute ID. */
	uint16_t			id;

	/* Length of the value buffer */
	uint16_t			len;

	/* An array of bytes that contains the value
	* of the attribute. 
	*/
	const uint8_t		*value;

	/*Internal use, should be set 0*/
	uint16_t			flags;
};


/*
 * A set of SDP attributes of HID profiles.
 * Used for SDP records
 */
typedef struct _HidSdpRecord		HidSdpRecord;


struct _HidSdpRecord
{

	/* Total number of attributes in "attribs". */
	uint8_t			num;

	/* Pointer to an array of attributes.
	 *
	 * Store the attributes in ascending order by attribute ID, and
	 * do not add two attributes with the same ID.
	 */
	SDPAttribute	*attribs;

	/* The service class fields of the class of device.
	 * The device class portion is ignored.
	 */
	uint32_t				classOfDevice; 
};

typedef uint8_t			HidChannelType;

/** Hid control channel */
#define HidCtrlChannel			1

/** Hid interrupt channel*/
#define HidIntrChannel			2


typedef struct _HidData		HidData;

struct _HidData
{
	HidChannelType	channel;
	uint8_t 		*data;
	uint16_t		dataLen;
};


typedef struct _BT_HID_CALLBACK_PARAMS
{
	uint16_t					paramsLen;
	bool						status;
	uint16_t					errorCode;

	union
	{
		uint8_t					*bd_addr;

		HidData					hidData;
	}params;
}BT_HID_CALLBACK_PARAMS;

typedef void (*BTHidCallbackFunc)(BT_HID_CALLBACK_EVENT event, BT_HID_CALLBACK_PARAMS * param);

typedef struct _HidAppFeatures
{
	BTHidCallbackFunc		hidAppCallback;
}HidAppFeatures;

void BtHidCallback(BT_HID_CALLBACK_EVENT event, BT_HID_CALLBACK_PARAMS * param);


bool HidInitWithSdp(HidSdpRecord *hidSdpRecord);

void HidDeinit(void);

bool HidConnect(uint8_t * addr);

bool HidDisconnect(void);

bool HidRegisterSdpRecord(HidSdpRecord *hidSdpRecord);

bool HidSendData(HidData * hidData);

bool HidAppInit(BTHidCallbackFunc callback);


//app
void BtHidUserKey(uint8_t cmd);

//user command 
//Note:该命令不能在蓝牙协议栈任务中调用
void HidCmd_VolUp(void);
void HidCmd_VolDn(void);
void HidCmd_Next(void);
void HidCmd_Prev(void);
void HidCmd_Mute(void);
void HidCmd_Stop(void);
void HidCmd_Camera(void);//自拍

#endif /*__BT_HID_API_H__*/
/**
 * @}
 * @}
 */
  

