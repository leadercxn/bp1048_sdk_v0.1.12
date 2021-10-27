////////////////////////////////////////////////////////////////////////////////////////////
//					Mountain View Silicon Tech. Inc.
//	Copyright 2012, Mountain View Silicon Tech. Inc., ShangHai, China
//					All rights reserved.
//
//  bt stack related API
//  filename: bt_stack_api.h
//
////////////////////////////////////////////////////////////////////////////////////////////

/**
* @addtogroup Bluetooth
* @{
* @defgroup bt_stack_api bt_stack_api.h
* @{
*/

#ifndef __BT_STACK_API_H__
#define __BT_STACK_API_H__

#include "type.h"
#include "bt_config.h"
#include "bt_stack_api.h"
#include "bt_stack_callback.h"

#include "bt_common_api.h"
#include "bt_a2dp_api.h"
#include "bt_avrcp_api.h"
#include "bt_hfp_api.h"
#include "bt_spp_api.h"
#include "bt_pbap_api.h"
#include "bt_hid_api.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif
#ifdef BT_TWS_SUPPORT
#include "bt_tws_api.h"
#endif
#include "bt_mfi_api.h"

typedef void (*BTStackCallbackFunc)(BT_STACK_CALLBACK_EVENT event, BT_STACK_CALLBACK_PARAMS * param);

typedef uint32_t (*BTHfScoCallbackFunc)(uint8_t* data, uint32_t len);

/* support profiles */

#define BT_PROFILE_SUPPORTED_A2DP				0x0001

#define BT_PROFILE_SUPPORTED_AVRCP				0x0002

#define BT_PROFILE_SUPPORTED_HFP				0x0004

#define BT_PROFILE_SUPPORTED_MFI				0x0008

#define BT_PROFILE_SUPPORTED_SPP				0x0010

#define BT_PROFILE_SUPPORTED_OBEX				0x0020

#define BT_PROFILE_SUPPORTED_HID				0x0040

#define BT_PROFILE_SUPPORTED_PBAP				0x0080

#define BT_PROFILE_SUPPORTED_TWS				0x8000

/**
* Stack Debug Info mask
*/
typedef uint32_t			BtStackDbgInfo;

#define BT_STATCK_DEBUG_INFO_COMMON				(1 << 0)
#define BT_STATCK_DEBUG_INFO_HFP				(1 << 1)
#define BT_STATCK_DEBUG_INFO_A2DP				(1 << 2)
#define BT_STATCK_DEBUG_INFO_AVRCP				(1 << 3)
#define BT_STATCK_DEBUG_INFO_SPP				(1 << 4)
#define BT_STATCK_DEBUG_INFO_MFI				(1 << 5)
#define BT_STATCK_DEBUG_INFO_HID				(1 << 6)

/**
 * Stack Debug infomations output
 */
void SetBtStackDebugInfo(BtStackDbgInfo mask);

/** 
 * Max length of local bluetooth device name
 * DON'T modify this value!!!
 */
typedef struct _BtStackParams
{
	uint8_t				*localDevName;
	
	uint32_t			supportProfiles;
	BTStackCallbackFunc	callback;
	BTHfScoCallbackFunc scoCallback;

	uint8_t				btSimplePairing;
	uint8_t				btPinCode[16];//max 16
	uint8_t				btPinCodeLen;
#ifdef BT_TWS_SUPPORT
	/*TWS releated features*/
	TwsAppFeatures		twsFeatures;
#endif	
	/*HFP releated features*/
	HfpAppFeatures		hfpFeatures;
	
	/*A2DP releated features*/
	A2dpAppFeatures		a2dpFeatures;

	/*AVRCP releated features*/
	AvrcpAppFeatures	avrcpFeatures;
	
	SppAppFeatures		SppAppFeatures;
	
	HidAppFeatures		hidFeatures;

	MfiAppFeatures		mfiFeatures;
}BtStackParams;


/**
 * @brief  bt stack run init
 * @param  stackParams
 * @return 0 - init success
 *			other - init error
 * @Note
 *		
 */
int32_t BTStackRunInit(BtStackParams *stackParams);

/**
 * @brief  bt stack run
 * @param  NONE
 * @return NONE
 * @Note
 *		
 */
void  BTStackRun(void);

/**
 * @brief  bt stack run uninit
 * @param  NONE
 * @return NONE
 * @Note
 *		
 */
void  BTStackRunUninit(void);

/**
 * @brief  has bt data to process
 * @param  NONE
 * @return TRUE/FALSE
 * @Note
 *		
 */
bool HasBtDataToProccess(void);

/**
 * @brief  get current bt library version
 * @param  NONE
 * @return the pointer of string
 * @Note
 *		
 */
const unsigned char * GetLibVersionBt(void);


/**
 * @brief  get local bt address
 * @param  bt_addr
 * @return TRUE/FALSE
 * @Note
 *		
 */
bool GetLocalBtAddr(uint8_t * bt_addr);

/**
 * @brief  get local bt name
 * @param  bt_name
 *			bt_name_len
 * @return TRUE/FALSE
 * @Note
 *		
 */
bool GetLocalBtName(uint8_t * bt_name, uint8_t * bt_name_len);


/*
*
*/
typedef uint8_t (*BTStackRecvHookFunc) (uint8_t* data, uint32_t data_len);
void BTStackSetReceivedDataHook(BTStackRecvHookFunc HookFunc);


typedef void (*BTStackSendHookFunc)(void);
void BTStackSetSendDataHook(BTStackSendHookFunc HookFunc);

/**
 * @brief  bt stack memory alloc
 * @param  btStackSize
 * @return TRUE/FALSE
 * @Note
 *		
 */
uint32_t BTStackMemAlloc(uint32_t btStackSize, void* stackBuf, uint8_t IsOsSys);

/**
 * @brief  bt stack memory free
 * @param  NONE
 * @return 0 - SUCESS
 * @Note
 *		
 */
int32_t BTStackMemFree(void);

/**
 * @brief
 * 		Write Inquiry Scan Params: Interval and Window
 * 
 * @param
 *		Interval: 0x0012~0x1000 (default: 0x1000); only even values are valid 
 *		Window: 0x0011~Inquiry Scan Interval(Max:0x1000) (default: 0x0012);
 *		time = N*0.625ms
 *	
 * @note
 *		在bt_common_app.c里面调用
 */
void WriteInquiryScanActivity(uint16_t newInterval, uint16_t newWindow);

/**
 * @brief
 * 		Write Page Scan Params: Interval and Window
 * 
 * @param
 *		Interval: 0x0012~0x1000 (default: 0x1000); only even values are valid 
 *		Window: 0x0011~Inquiry Scan Interval(Max:0x1000) (default: 0x0012);
 *		time = N*0.625ms
 *	
 * @note
 */
void WritePageScanActivity(uint16_t newInterval, uint16_t newWindow);

/**
 * @brief
 * 		page time out value
 * 
 * @param
 *		TimeOutValueInMs: 0x0001~0xffff (default: 0x2000); only even values are valid 
 *		time = N*0.625ms
 *	
 * @note
 */
void BTStatckSetPageTimeOutValue(uint16_t TimeOutValueInMs);

/**
 * @brief
 * 		get bt disconnect reason
 * 
 * @param
 *		NULL 
 *		
 * @return
 *		reason: 
 *		1=remote device terminate connection
 *		2=BP10 terminate connection 
 *		3=remote device link loss
 *	
 * @note
 */
#define BT_DISCON_REASON_ERROR			0
#define BT_DISCON_REASON_REMOTE_DIS		1
#define BT_DISCON_REASON_LOCAL_DIS		2
#define BT_DISCON_REASON_LINKLOSS		3
uint32_t GetBtDisconReason(void);

#endif

