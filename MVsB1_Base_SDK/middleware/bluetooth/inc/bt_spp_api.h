
/*
* bt_spp_api.h
*/

/**
* @addtogroup Bluetooth
* @{
* @defgroup bt_spp_api bt_spp_api.h
* @{
*/

#ifndef __BT_SPP_API_H__
#define __BT_SPP_API_H__

typedef enum{
	BT_STACK_EVENT_SPP_NONE = 0,
	BT_STACK_EVENT_SPP_CONNECTED,
	BT_STACK_EVENT_SPP_DISCONNECTED,
	BT_STACK_EVENT_SPP_DATA_RECEIVED,
	BT_STACK_EVENT_SPP_DATA_SENT,
}BT_SPP_CALLBACK_EVENT;

typedef struct _BT_SPP_CALLBACK_PARAMS
{
	uint16_t			paramsLen;
	bool				status;
	uint16_t			errorCode;

	union
	{
		uint8_t					*bd_addr;
		
		uint8_t					*sppReceivedData;
		uint8_t					*sppSentData;
	}params;
}BT_SPP_CALLBACK_PARAMS;

typedef void (*BTSppCallbackFunc)(BT_SPP_CALLBACK_EVENT event, BT_SPP_CALLBACK_PARAMS * param);
typedef void (*BTSppHookFunc)(void);

void BtSppCallback(BT_SPP_CALLBACK_EVENT event, BT_SPP_CALLBACK_PARAMS * param);
void BtSppHookFunc(void);

typedef struct _SppAppFeatures
{
	BTSppCallbackFunc		sppAppCallback;
	BTSppHookFunc			sppAppHook;
}SppAppFeatures;

/**
 * @brief  connect the spp profile
 * @param  addr - the remote device address
 * @return 0 - SUCESS
 * @Note
 *		
 */
bool SppConnect(uint8_t * addr);

/**
 * @brief  disconnect the spp profile
 * @param  NONE
 * @return 0 - SUCESS
 * @Note
 *		
 */
bool SppDisconnect(void);

/**
 * @brief  spp send data
 * @param  data
 *			dataLen
 * @return 0 - SUCESS
 * @Note   spp发送最大长度不超过350Bytes
 *		
 */
bool SppSendData(uint8_t * data, uint16_t dataLen);

/**
 * @brief  send data via spp channel is ready?
 * @param  NONE
 * @return 1=ready, 0=not ready
 * @Note
 *		
 */
bool SppIsReady(void);

/**
 * @brief  spp profile initial
 * @param  callback
 * @return 1=success
 * @Note
 *		
 */
bool SppAppInit(BTSppCallbackFunc callback);
#endif


