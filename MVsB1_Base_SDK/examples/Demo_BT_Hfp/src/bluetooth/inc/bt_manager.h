/**
 **************************************************************************************
 * @file    bt_manager.c
 * @brief   management of all bluetooth event and apis
 *
 * @author  Halley
 * @version V1.1.0
 *
 * $Created: 2016-07-18 16:24:11$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef _BT_MANAGER_H_
#define _BT_MANAGER_H_

#include <string.h>
#include "timeout.h"
#include "bt_stack_callback.h"
#include "bt_config.h"

#define MAX_PHONE_NUMBER_LENGTH		20

typedef enum
{
	BT_STACK_STATE_NONE,
	BT_STACK_STATE_INITAILIZING,
	BT_STACK_STATE_READY	
} BT_STACK_STATE;


//scan page state 
typedef enum
{
	BT_SCAN_PAGE_STATE_NONE,
	BT_SCAN_PAGE_STATE_CLOSING,
	BT_SCAN_PAGE_STATE_DISCONNECTING,
	BT_SCAN_PAGE_STATE_DISABLE,
	BT_SCAN_PAGE_STATE_OPENING,
	BT_SCAN_PAGE_STATE_ENABLE
} BT_SCAN_PAGE_STATE;

/**
* Bt device connectable mode. 
*/
typedef enum
{
	BT_DEVICE_CONNECTION_MODE_NONE,				/* cann't be discovered and connected*/
	BT_DEVICE_CONNECTION_MODE_DISCOVERIBLE, 	/* can be discovered*/
	BT_DEVICE_CONNECTION_MODE_CONNECTABLE,		/* can be connected*/
	BT_DEVICE_CONNECTION_MODE_ALL,				/* can be discovered and connected*/
} BT_DEVICE_CONNECTION_MODE;

typedef enum 
{
	BT_HFP_STATE_NONE,
	BT_HFP_STATE_CONNECTING,
	BT_HFP_STATE_CONNECTED,
	BT_HFP_STATE_INCOMING,
	BT_HFP_STATE_OUTGOING,
	BT_HFP_STATE_ACTIVE,
	BT_HFP_STATE_3WAY_INCOMING_CALL,		//1CALL ACTIVE, 1CALL INCOMING
	BT_HFP_STATE_3WAY_OUTGOING_CALL,		//1CALL ACTIVE, 1CALL OUTGOING
	BT_HFP_STATE_3WAY_ATCTIVE_CALL			//2CALL ACTIVE
} BT_HFP_STATE;

typedef enum
{
	BT_A2DP_STATE_NONE,
	BT_A2DP_STATE_CONNECTING,
	BT_A2DP_STATE_CONNECTED,
	BT_A2DP_STATE_STREAMING,
} BT_A2DP_STATE;

typedef enum
{
	BT_AVRCP_STATE_NONE,
	BT_AVRCP_STATE_CONNECTING,
	BT_AVRCP_STATE_CONNECTED,	
} BT_AVRCP_STATE;

typedef enum
{
	BT_SPP_STATE_NONE,
	BT_SPP_STATE_CONNECTING,
	BT_SPP_STATE_CONNECTED,
} BT_SPP_STATE;

typedef uint8_t TIMER_FLAG;
#define TIMER_UNUSED					0x00
#define TIMER_USED						0x01
#define TIMER_STARTED					0x02


typedef struct _BT_TIMER
{
	TIMER_FLAG	timerFlag;
	TIMER		timerHandle;
}BT_TIMER;

//上电时加载FLASH的配置参数
typedef struct _BT_CONFIGURATION_PARAMS
{
	//BT参量表中排序
	uint8_t			bt_LocalDeviceAddr[BT_ADDR_SIZE];
	uint8_t			ble_LocalDeviceAddr[BT_ADDR_SIZE];
	uint8_t			bt_LocalDeviceName[BT_NAME_SIZE];
	uint8_t			ble_LocalDeviceName[BT_NAME_SIZE];	//预留:现BLE name是通过BLE工具生成的广播包中包含

	uint8_t			bt_ConfigHeader[4];
	
	uint8_t			bt_trimValue;
	uint8_t			bt_TxPowerValue;
	
	uint32_t		bt_SupportProfile;
	uint8_t			bt_WorkMode;
	uint8_t			bt_twsFunc;
	uint8_t			bt_remoteDevNum;
	uint8_t			bt_simplePairingFunc;
	uint8_t			bt_pinCode[4];
	
	uint8_t			bt_reconFunc;
	uint8_t			bt_reconDevNum;
	uint8_t			bt_reconCount;
	uint8_t			bt_reconIntervalTime;
	uint8_t			bt_reconTimeout;
	uint8_t			bt_accessMode;

	//inquiry scan
	uint16_t		bt_InquiryScanInterval;
	uint16_t		bt_InquiryScanWindow;

	//page scan
	uint16_t		bt_PageScanInterval;
	uint16_t		bt_PageScanWindow;

	uint8_t			bt_avrcpVolSync;
}BT_CONFIGURATION_PARAMS;

typedef struct _BT_DB_RECORD
{
	uint8_t			bdAddr[BT_ADDR_SIZE];
	bool			trusted;
	uint8_t			linkKey[16];
	uint8_t			keyType;
	uint8_t			pinLen;
}BT_DB_RECORD;

typedef struct _BT_LINK_DEVICE_INFO
{
	BT_DB_RECORD	device;
	uint8_t			UsedFlag;
}BT_LINK_DEVICE_INFO;


typedef struct _BT_MANAGER_ST
{

	uint8_t					btDevAddr[BT_ADDR_SIZE];
	uint8_t					btDevName[BT_NAME_SIZE];
	BT_LINK_DEVICE_INFO		btLinkDeviceInfo[8];
	BT_STACK_STATE			stackState;
	BT_DEVICE_CONNECTION_MODE	deviceConMode;
	
	uint8_t					remoteAddr[BT_ADDR_SIZE];
	uint8_t					btDdbLastAddr[BT_ADDR_SIZE];

	uint8_t 				btLinkState; //0=disconnect; 1=connected
	uint8_t					btLastAddrUpgradeIgnored; //1:用于测试盒校准频偏后,不更新最后一次连接设备信息

#ifdef BT_RECONNECTION_FUNC
	uint8_t					btReconnectTryCount;
	uint8_t					btReconnectIntervalTime;
	BT_TIMER				btReconnectTimer;
	bool					BtPowerOnFlag;
	bool					btReconnectedFlag;
#endif

	uint16_t				btConnectedProfile;

	uint8_t					btCurConnectFlag;
	BT_SCAN_PAGE_STATE		btScanPageState;

	
	//hfp
	BT_HFP_STATE			hfpState;
	uint8_t					remoteHfpAddr[BT_ADDR_SIZE];
	bool					scoConnected;
	uint8_t					phoneNumber[MAX_PHONE_NUMBER_LENGTH];
	bool					callWaitingFlag;
	uint8_t					batteryLevel;
	uint8_t					signalLevel;
	bool					roamFlag;
	bool					voiceRecognition;
	uint8_t					volGain;
	uint8_t					hfpScoCodecType;
	uint8_t					hfpVoiceState;		//1:非通话,其他应用使用了通话链路传输音频 0:正常通话

	//a2dp
	BT_A2DP_STATE			a2dpState;
	uint8_t					remoteA2dpAddr[BT_ADDR_SIZE];

	//avrcp
	BT_AVRCP_STATE			avrcpState;
	uint8_t					remoteAvrcpAddr[BT_ADDR_SIZE];
	bool					avrcpConnectStart;
	BT_TIMER				avrcpPlayStatusTimer;	
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
	uint8_t					avrcpSyncEnable;
	uint8_t					avrcpSyncVol; //默认值0xff(未同步), 正常值范围(0-32)
#endif

	//spp
	BT_SPP_STATE			sppState;
	uint8_t					remoteSppAddr[BT_ADDR_SIZE];
	
	
} BT_MANAGER_ST;

extern BT_MANAGER_ST		btManager;

/**
 * @brief  Clear Bt Manager Register
 * @param  NONE
 * @return NONE
 * @Note
 *
 */
void ClearBtManagerReg(void);

/**
 * @brief  Get Bt Manager
 * @param  NONE
 * @return ptr is returned
 * @Note
 *
 */
BT_MANAGER_ST * GetBtManager(void);

/**
 * @brief  Set Bt Stack State
 * @param  state: The state of bt stack.
 * @return NONE
 * @Note
 *
 */
void SetBtStackState(BT_STACK_STATE state);

/**
 * @brief  Get Bt Stack State
 * @param  NONE
 * @return The state of bt stack.
 * @Note
 *
 */
BT_STACK_STATE GetBtStackState(void);

/**
 * @brief  Set Bt Device Connection Mode
 * @param  mode: connection mode
 * @return TRUE: success
 * @Note
 *
 */
bool SetBtDeviceConnState(BT_DEVICE_CONNECTION_MODE mode);

/**
 * @brief  Get Bt Device Connection Mode
 * @param  NONE
 * @return connection mode
 * @Note
 *
 */
BT_DEVICE_CONNECTION_MODE GetBtDeviceConnMode(void);


/**
* Bt device connected profile. 
*/
#define BT_CONNECTED_HFP_FLAG			0x0001
#define BT_CONNECTED_A2DP_FLAG			0x0002
#define BT_CONNECTED_AVRCP_FLAG			0x0004
#define BT_CONNECTED_SPP_FLAG			0x0008
#define BT_CONNECTED_HID_FLAG			0x0010
#define BT_CONNECTED_MFI_FLAG			0x0020

void SetBtConnectedProfile(uint16_t connectedProfile);
void SetBtDisconnectProfile(uint16_t disconnectProfile);
uint8_t GetNumOfBtConnectedProfile(void);

void SetBtCurConnectFlag(uint8_t state);
uint8_t GetBtCurConnectFlag(void);


/*****************************************************************************
* bt reconnect function
******************************************************************************/
void BtReconnectDevice(void);
uint32_t BtReconnectProfile(void);
void BtStartReconnectProfile(void);
void BtStartReconnectDevice(uint8_t mode);
void BtReconnectCB(void);
void CheckBtReconnectTimer(void);


/*HFP*/
/**
 * @brief  connect the hfp profile 
 * @param  addr - the remote address
 * @return NONE
 * @Note
 *		
 */
void BtHfpConnect(uint8_t * addr);

/**
 * @brief  disconnect the hfp profile
 * @param  NONE
 * @return NONE
 * @Note
 *		
 */
void BtHfpDisconnect(void);

/**
 * @brief  get the hfp profile state
 * @param  NONE
 * @return state
 * @Note
 *		
 */
BT_HFP_STATE GetHfpState(void);

int16_t GetHfpConnectedAddr(uint8_t * addr);

/**
 * @brief  get the sco connect flag
 * @param  NONE
 * @return TRUE - connected
 * @Note
 *		
 */
bool GetScoConnectFlag(void);


/*A2DP*/
/**
 * @brief  get the a2dp profile state
 * @param  NONE
 * @return state
 * @Note
 *		
 */
BT_A2DP_STATE GetA2dpState(void);
int16_t GetA2dpConnectedAddr(uint8_t * addr);

/**
 * @brief  connect the a2dp profile 
 * @param  addr - the remote address
 * @return NONE
 * @Note
 *		
 */
void BtA2dpConnect(uint8_t *addr);

/*AVRCP*/
/**
 * @brief  get the avrcp profile state
 * @param  NONE
 * @return state
 * @Note
 *		
 */
BT_AVRCP_STATE GetAvrcpState(void);
int16_t GetAvrcpConnectedAddr(uint8_t * addr);

/**
 * @brief  connect the avrcp profile 
 * @param  addr - the remote address
 * @return NONE
 * @Note
 *		
 */
void BtAvrcpConnect(uint8_t *addr);


/**
 * @brief  timer start - get bt audio play status
 * @param  NONE
 * @return NONE
 * @Note
 *		
 */
void TimerStart_BtPlayStatus(void);

/**
 * @brief  timer stop - get bt audio play status
 * @param  NONE
 * @return NONE
 * @Note
 *		
 */
void TimerStop_BtPlayStatus(void);

/**
 * @brief  check timer start flag - get bt audio play status
 * @param  NONE
 * @return TRUE/FALSE
 * @Note
 *		
 */
uint8_t CheckTimerStart_BtPlayStatus(void);


/*SPP*/
BT_SPP_STATE GetSppState(void);
int16_t GetSppConnectedAddr(uint8_t * addr);
void BtSppConnect(uint8_t *addr);

/*pbap*/
void BtConnectPbap(void);
void BtDisconnectPbap(void);
void GetMobilePhoneBook(void);
void GetSim1CardPhoneBook(void);
void GetSim2CardPhoneBook(void);
void BtStopReconnect(void);

//蓝牙连接断开处理
void BtLinkConnect(void);
void BtLinkDisconnect(void);

#endif

