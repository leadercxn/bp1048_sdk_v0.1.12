//////////////////////////////////////////////////////////////////
//
// bt_stack_callback.h
//

/**
* @addtogroup Bluetooth
* @{
* @defgroup bt_stack_callback bt_stack_callback.h
* @{
*/

#ifndef __BT_STACK_CALLBACK_H__
#define __BT_STACK_CALLBACK_H__

#include "bt_common_api.h"


typedef enum 
{
	BT_STACK_EVENT_NONE = 0,

	/**
	 *@brief
	 * The stack is initialized
	 *
	 *@note
	 * No params
	 */
	BT_STACK_EVENT_COMMON_STACK_INITIALIZED,
	
	/**
	 *@brief
	 * The stack memory is out of size
	 *
	 *@note
	 * No params
	 */
	BT_STACK_EVENT_COMMON_STACK_OUT_OF_MEM_ERR,
		
	/**
	 *@brief
	 * The stack memory free size
	 *
	 *@note
	 * No params
	 */
	BT_STACK_EVENT_COMMON_STACK_FREE_MEM_SIZE,

	/**
	 *@brief
	 * The stack has been uninitialized
	 *
	 *@note
	 * No params
	 */
	BT_STACK_EVENT_COMMON_STACK_UNINITIALIZED,

	/**
	 *@brief
	 * A remote device is found.
	 *
	 *@note
	 * params.inquiryResult is vaild
	 */
	BT_STACK_EVENT_COMMON_INQUIRY_RESULT,

	/**
	 *@brief
	 * The inquiry process is completed.
	 *
	 *@note
	 * No params
	 */
	BT_STACK_EVENT_COMMON_INQUIRY_COMPLETE,

	/**
	 *@brief
	 * The inquiry process is cancelled.
	 *
	 *@note
	 * No params
	 */
	BT_STACK_EVENT_COMMON_INQUIRY_CANCELED,

	/**
	 *@brief
	 * Indicate the mode of the link changed.
	 *
	 *@note
	 * params.modeChange is vaild
	 */
	BT_STACK_EVENT_COMMON_MODE_CHANGE,

	/**
	 *@brief
	 * Indicate the name of a remote device
	 *
	 *@note
	 * params.remDevName is vaild
	 */
	BT_STACK_EVENT_COMMON_GET_REMDEV_NAME,

	/**
	 *@brief
	 * Indicate that an accessibility change is complete.
	 *
	 *@note
	 * params.accessMode is vaild.
	 */
	BT_STACK_EVENT_COMMON_ACCESS_MODE,

	/**
	 *@brief
	 * Handles a L2CAP packet reassembly error by disconnecting the faulty link.
	 *
	 *@note
	 * No params
	 */
	BT_STACK_EVENT_COMMON_CONNECTION_ABORTED,

	/**
	 *@brief
	 * page time out(hf) or src not ready(acl waiting for timeout).
	 *
	 *@note
	 * No params
	 */
	BT_STACK_EVENT_COMMON_PAGE_TIME_OUT,
		
	/**
	 *@brief
	 * add new record information.
	 *
	 *@note
	 * No params
	 */
	BT_STACK_EVENT_TRUST_LIST_ADD_NEW_RECORD,
	
	/**
	 *@brief
	 * delete new record information.
	 *
	 *@note
	 * No params
	 */
	BT_STACK_EVENT_TRUST_LIST_DELETE_RECORD,
		
	/**
	 *@brief
	 * baseband lost connection. need to reconnect
	 *
	 *@note
	 * No params
	 */
	BT_STACK_EVENT_COMMON_BB_LOST_CONNECTION,
}BT_STACK_CALLBACK_EVENT;

typedef struct _BT_STACK_MEM_PARAMS
{
	uint32_t 					stackFreeMemSize;
	uint32_t 					stackMallocMemSize;
}BT_STACK_MEM_PARAMS;

typedef struct _BT_STACK_CALLBACK_PARAMS
{
	uint16_t					paramsLen;
	bool						status;
	uint16_t					errorCode;

	union
	{
		BT_STACK_MEM_PARAMS		stackMemParams;
		uint8_t					*bd_addr;
		InquriyResultParam		inquiryResult;
		ModeChange				modeChange;
		RequestRemNameParam		remDevName;
		BtAccessMode			accessMode;
	}params;
}BT_STACK_CALLBACK_PARAMS;

void BtStackCallback(BT_STACK_CALLBACK_EVENT event, BT_STACK_CALLBACK_PARAMS * param);

#endif //__BT_STACK_CALLBACK_H__
