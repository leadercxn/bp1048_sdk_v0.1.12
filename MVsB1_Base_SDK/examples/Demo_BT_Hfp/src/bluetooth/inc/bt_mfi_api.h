/**
 *******************************************************************************
 * @file    bt_mfi_api.h
 * @author  Halley
 * @version V1.0.1
 * @date    27-Apr-2016
 * @brief   MFi related api
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
* @defgroup bt_mfi_api bt_mfi_api.h
* @{
*/


#ifndef __BT_MFI_H__
#define __BT_MFI_H__

typedef enum{
	BT_STACK_EVENT_MFI_NONE = 0,
	/**
	 * @brief
	 * This event will be received when MFi channel connected
	 * the union params.bd_addr is vaild
	 */
	BT_STACK_EVENT_MFI_CONNECTED,

	/**
	 * @brief
	 * This event will be received when MFi channel disconnected
	 */
	BT_STACK_EVENT_MFI_DISCONNECTED,

	/**
	 * @brief
	 * This event will be received when data arrived in MFi channel
	 * the union params.mfiReceivedData is vaild and paramsLen indicate
	 * the length of mfiReceivedData
	 */
	BT_STACK_EVENT_MFI_DATA_RECEIVED,

	/**
	 * @brief
	 * This event will be received when data has been sent to RF controller
	 * the union params.mfiSentData is vaild and paramsLen indicate
	 * the length of data sent.
	 *
	 * @note
	 * The memory alloced by the data can be release here.
	 */
	BT_STACK_EVENT_MFI_DATA_SENT
}BT_MFI_CALLBACK_EVENT;

typedef struct _BT_MFI_CALLBACK_PARAMS
{
	uint16_t					paramsLen;
	bool						status;
	uint16_t					errorCode;

	union
	{
		uint8_t					*bd_addr;

		uint8_t					*mfiReceivedData;
		uint8_t					*mfiSentData;
	}params;
}BT_MFI_CALLBACK_PARAMS;

typedef void (*BTMfiCallbackFunc)(BT_MFI_CALLBACK_EVENT event, BT_MFI_CALLBACK_PARAMS * param);

typedef struct _MfiAppFeatures
{
	BTMfiCallbackFunc		mfiAppCallback;
}MfiAppFeatures;



/**
 * @brief
 *	Connect a remote iOS device with MFi channel
 *
 * @param
 *	addr - the address of remote device
 *
 * @return
 *	True for this command has been sent to bluetooth stack successful
 *
 * @note
 *	It will receive the event BT_STACK_EVENT_MFI_CONNECTED when 
 *	it connected to remote iOS device.
 */

bool MFiConnect(uint8_t* addr);


/**
 * @brief
 *	Disconnect the MFi channel
 *
 * @param
 *	NONE
 *
 * @return
 *	True for this command has been sent to bluetooth stack successful
 *
 * @note
 *	It will receive the event BT_STACK_EVENT_MFI_DISCONNECTED if the
 *	channel is disconnected.
 */

bool MFiDisconnect(void);

/**
 * @brief
 *	Send data to the remote device via MFi channel
 *
 * @param
 *	data -	the data to be sent. And the pointer of the data will
 *			be received in BT_STACK_EVENT_MFI_DATA_SENT. So the 
 *			memory of the data should be hold until the event
 *			BT_STACK_EVENT_MFI_DATA_SENT received. So the varible
 *			data should be created by "malloc" method or  "global"
 *			varibles
 *
 *	dataLen - the length of the data.
 *
 * @return
 *	True for this command has been sent to bluetooth stack successful.
 *
 * @note
 *	It will receive the event BT_STACK_EVENT_MFI_DATA_SENT when the data
 *	has been sent to RF controller successful. If the data is created by
 *	"malloc" method, the "release" can be called in that event.
 */
bool MFiSendData(uint8_t * data, uint16_t dataLen);

#endif /*__BT_MFI_H__*/

/**
 * @}
 * @}
 */
 

