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
#include "bt_stack_callback.h"
#include "bt_config.h"
#include "timeout.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

#ifdef BT_TWS_SUPPORT
#include "bt_tws_api.h"
#endif
#define MAX_PHONE_NUMBER_LENGTH		20

typedef uint32_t BT_CHECK_EVENT_LIST;
#define BT_EVENT_NONE					0x00
#define BT_EVENT_AVRCP_DISCONNECT		0x01//bit0:��A2DP�Ͽ���,����AVRCP��ʱ�Ͽ�(5s)
#define BT_EVENT_L2CAP_LINK_DISCONNECT	0x02//bit1:��L2CAP�����쳣ʱ�Ͽ�����Ҫ��������������������ʾ�������ӳɹ����Զ�����
#define BT_EVENT_AVRCP_CONNECT			0x04//bit2:������A2DP��,��������AVRCP����(50ms)
extern BT_CHECK_EVENT_LIST btCheckEventList;
extern uint32_t btEventListB0Count;
extern uint32_t btEventListB1Count;
extern uint32_t btEventListB1State;//�쳣ǰ����״̬�ı��棬ȷ���ڻ������Ƿ���Ҫ���Ͳ�������
extern uint32_t btEventListB2Count;
extern uint32_t btEventListCount;

typedef uint8_t (*Name_Confirm_Callback_t)(uint8_t name[6]);
typedef uint8_t (*Other_Confirm_Callback_t)(void);
// �����Ҫ����ĳЩ�ֻ����������������ǰ��ӣ�bt_stack_server��ʼ��ʱ�����ú���ָ����ײ�ȥ���ж�����
// ������о������ں���BtConnectDecide��ʵ��
extern uint8_t Name_confirm_Callback_Set(Name_Confirm_Callback_t callback);
// CFG_TWS_ONLY_IN_BT_MODE�򿪺���Կ��ٲ忨�п��ܵ��´ӻ��������������лص�ʵ�ֹ���
// ������о������ں���BtConnectConfirm��ʵ��
extern uint8_t Other_confirm_Callback_Set(Other_Confirm_Callback_t callback);

#ifndef BT_TWS_SUPPORT
//������Ȩ���ӵĵײ㺯���ӿ�
typedef void (*SEC_Confirm_Callback_t)(uint32_t val);

extern void SEC_Confirm_Callback_Set(SEC_Confirm_Callback_t callback);


// ʹ�òο�:
// 1. �򿪼�Ȩ���ܣ�������BtStackServiceEntrance()
// BtNumericalDisplayFuncEnable(1)
// 2. ����callback��������BtStackServiceEntrance()
//SEC_Confirm_Callback_Set(BtNumericalVerify);
// 3. ʵ�ֻص�����BtNumericalVerify�����¾��Ǵ�ӡ��Զ���豸�������ļ�Ȩ����

//����Զ���豸�ļ�Ȩ����
extern void BtNumericalAccecpt(void);
//�ܾ�Զ���豸�ļ�Ȩ����
extern void BtNumericalReject(void);
#endif

#ifdef BT_TWS_SUPPORT
#define MAX_FLASH_DDB_DEVICE_NUM	9
#endif
typedef enum
{
	BT_STACK_STATE_NONE,
	BT_STACK_STATE_INITAILIZING,
	BT_STACK_STATE_READY	
} BT_STACK_STATE;

//bt reset state
typedef enum
{
	BT_RST_STATE_NONE,
	BT_RST_STATE_START,
	BT_RST_STATE_WAITING,
	BT_RST_STATE_FINISHED,
} BT_RST_STATE;

//scan page state 
typedef enum
{
	BT_SCAN_PAGE_STATE_NONE,
	BT_SCAN_PAGE_STATE_CLOSING,
	BT_SCAN_PAGE_STATE_DISCONNECTING,
	BT_SCAN_PAGE_STATE_DISABLE,
	BT_SCAN_PAGE_STATE_OPENING,
	BT_SCAN_PAGE_STATE_ENABLE,
#ifndef BT_TWS_SUPPORT
	BT_SCAN_PAGE_STATE_SNIFF,	
#else
	BT_SCAN_PAGE_STATE_OPEN_WAITING,
	BT_SCAN_PAGE_STATE_EXIT_DEEPSLEEP_WAITING, //��sleep�˳��ȴ�
#endif
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

#ifdef BT_TWS_SUPPORT
typedef enum
{
	BT_TWS_STATE_NONE,
	BT_TWS_STATE_CONNECTING,
	BT_TWS_STATE_CONNECTED,
} BT_TWS_STATE;
#endif

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

typedef enum
{
	BT_MFI_STATE_NONE,
	BT_MFI_STATE_CONNECTING,
	BT_MFI_STATE_CONNECTED,
} BT_MFI_STATE;

typedef enum
{
	BT_OBEX_STATE_NONE,
	BT_OBEX_STATE_CONNECTING,
	BT_OBEX_STATE_CONNECTED,
} BT_OBEX_STATE;

typedef enum
{
	BT_PBAP_STATE_NONE,
	BT_PBAP_STATE_CONNECTING,
	BT_PBAP_STATE_CONNECTED,
} BT_PBAP_STATE;

typedef uint8_t TIMER_FLAG;
#define TIMER_UNUSED					0x00
#define TIMER_USED						0x01
#define TIMER_STARTED					0x02

#ifdef BT_TWS_SUPPORT
typedef uint8_t RECONNECT_TYPE;
#define RECONNECT_NONE					0x00
#define RECONNECT_DEVICE				0x01
#define RECONNECT_TWS					0x02
#endif

typedef struct _BT_TIMER
{
	TIMER_FLAG	timerFlag;
	TIMER		timerHandle;
}BT_TIMER;

#ifdef BT_SNIFF_ENABLE

typedef struct _BT_SNIFF_CTRL
{
	uint8_t bt_sniff_start       : 1;
	uint8_t bt_sleep_enter       : 1;
#ifndef BT_TWS_SUPPORT
	uint8_t bt_sleep_fastpower_f : 1;
#endif
}_BT_SNIFF_CTRL;

#endif //BT_SNIFF_ENABLE

//�ϵ�ʱ����FLASH�����ò���
typedef struct _BT_CONFIGURATION_PARAMS
{
	//BT������������
	uint8_t			bt_LocalDeviceAddr[BT_ADDR_SIZE];//flash mac˳��: NAP-UAP-LAP (�˱������ڱ���flash�Ĳ���,���������޸�)
	uint8_t			ble_LocalDeviceAddr[BT_ADDR_SIZE];
	uint8_t			bt_LocalDeviceName[BT_NAME_SIZE];
	uint8_t			ble_LocalDeviceName[BT_NAME_SIZE];	//Ԥ��:��BLE name��ͨ��BLE�������ɵĹ㲥���а���

	uint8_t			bt_ConfigHeader[4];
	
	uint8_t			bt_trimValue;
	uint8_t			bt_TxPowerValue;
	
	uint32_t		bt_SupportProfile;
	uint8_t			bt_WorkMode;
	uint8_t			bt_twsFunc;
	uint8_t			bt_remoteDevNum;
	uint8_t			bt_simplePairingFunc;
	uint8_t			bt_pinCode[17];//max len: 16
	uint8_t			bt_pinCodeLen;
	
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
#ifdef FLASH_SAVE_REMOTE_BT_NAME
	uint8_t			bdName[BT_NAME_SIZE];
#endif
}BT_DB_RECORD;

typedef struct _BT_LINK_DEVICE_INFO
{
	BT_DB_RECORD	device;
	uint8_t			UsedFlag;
#ifdef BT_TWS_SUPPORT
	uint8_t			tws_role;
	uint8_t			remote_profile;
#endif
}BT_LINK_DEVICE_INFO;

#ifdef BT_TWS_SUPPORT
typedef struct _BT_RECONNECT_INFO
{
	uint8_t					btReconnectType; //0=device, 1=tws
	uint8_t					btReconnectTryCount;
	uint8_t					btReconnectIntervalTime;
	BT_TIMER				btReconnectTimer;
	bool					BtPowerOnFlag;
	bool					btReconnectedFlag;
}BT_RECONNECT_INFO;
#endif

//a2dp stream type
#define  BT_A2DP_STREAM_TYPE_SBC	0 //defined
#define  BT_A2DP_STREAM_TYPE_AAC	1


typedef struct _BT_MANAGER_ST
{
	uint8_t 				ddbUpdate;	//��ʶremote device information�Ƿ���Ҫд�뵽FLASH
	uint8_t					btDevAddr[BT_ADDR_SIZE];//ϵͳӦ�õ�����mac˳��: LAP-UAP-NAP
//	uint8_t					btDevName[BT_NAME_SIZE];
#ifdef BT_TWS_SUPPORT
	BT_LINK_DEVICE_INFO		btLinkDeviceInfo[MAX_FLASH_DDB_DEVICE_NUM];//1(tws)+ 8(device)
#else
	BT_LINK_DEVICE_INFO		btLinkDeviceInfo[8];
#endif
	BT_STACK_STATE			stackState;
	BT_DEVICE_CONNECTION_MODE	deviceConMode;
	
	uint8_t					remoteAddr[BT_ADDR_SIZE];
	uint8_t					remoteName[BT_NAME_SIZE];
	uint8_t					remoteNameLen;
	uint8_t					btDdbLastAddr[BT_ADDR_SIZE];
	uint8_t					btDdbLastProfile;
	uint32_t				btDdbLastInfoOffset;
#ifdef BT_TWS_SUPPORT
	uint8_t					btTwsDeviceAddr[BT_ADDR_SIZE];
#endif
	uint8_t					btAccessModeEnable; //1=��ʼ�����Զ�����ɱ������ɱ�����״̬
	uint32_t				btConStateProtectCnt;//10s��ʱ,���ڱ�����ǰ������������״̬
	uint8_t 				btLinkState; //0=disconnect; 1=connected
	uint8_t					btLastAddrUpgradeIgnored; //1:���ڲ��Ժ�У׼Ƶƫ��,���������һ�������豸��Ϣ

#ifdef BT_RECONNECTION_FUNC
	//remote device
	uint8_t					btReconnectTryCount;
	uint8_t					btReconnectIntervalTime;
	BT_TIMER				btReconnectTimer;
	bool					BtPowerOnFlag;
	bool					btReconnectedFlag;
	uint8_t					btReconnectDelayCount;//�ϵ��������ʼ��,�ڽ���ģʽ���ٷ������
	
#ifdef BT_TWS_SUPPORT
	//tws device
	RECONNECT_TYPE			btReconnectType; //1=tws, 2=device
	uint8_t					btReconDelayFlag; //���ӳ�ͻ,��ʱ����
	RECONNECT_TYPE			btReconnectDeviceFlag;//�������豸��־ //1=tws, 2=device
	
	bool					TwsPowerOnFlag;
	uint8_t					btTwsReconnectTryCount;
	uint8_t					btTwsReconnectIntervalTime;
	BT_TIMER				btTwsReconnectTimer;
	uint8_t					btTwsReconnectList;
#endif //defined(BT_TWS_SUPPORT)	
#endif //BT_RECONNECTION_FUNC

	uint16_t				btConnectedProfile;

	uint8_t					btCurConnectFlag;
	BT_SCAN_PAGE_STATE		btScanPageState;

#ifdef BT_TWS_SUPPORT
//tws
	uint8_t					TwsFilterInfor[7];	//tws����������Ϣ
	BT_TWS_STATE			twsState;
	BT_TWS_ROLE				twsRole;
	uint8_t					twsFlag;//��TWS��Լ�¼
	uint32_t				twsSimplePairingCfg;
	uint8_t					twsEnterPairingFlag;//tws������ʼ��־ //flag:0=normal; 1=pairing
	uint8_t					twsMode;//0=active mode; 2=sniff mode
	uint8_t					twsSbSlaveDisable;//1=�ӻ��ر�TWS��������;
	uint8_t					twsStopConnect;	// 1 = ֹͣTWS�������
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
	uint8_t					twsSoundbarSlaveTestFlag; //1 = soundbar slave����У׼���Ա�־
#endif
#endif

	BT_RST_STATE			btRstState; //BT�ָ���������
	uint32_t				btRstWaitingCount;

	uint8_t					appleDeviceFlag;	//Note:�˱�־ͨ��HFP�������ж�;           1=apple device; 0=other device
	
	//hfp
	BT_HFP_STATE			hfpState;
	uint8_t					scoState;	// �����ֻ���������û��sco��·���������1��û��sco��ǿ�в��ű�������
	uint8_t					localringState;	// Ϊ1ʱ��ʾ�����ڲ��ű��������������У���ͨ�绰������δ��ͨ����»�ԭ��0
//	uint8_t					remoteHfpAddr[BT_ADDR_SIZE];
	bool					scoConnected;
	uint8_t					phoneNumber[MAX_PHONE_NUMBER_LENGTH];
	bool					callWaitingFlag;
	uint8_t					batteryLevel;
	uint8_t					signalLevel;
	bool					roamFlag;
	bool					voiceRecognition;
	uint8_t					volGain;
	uint8_t					hfpScoCodecType;
	uint8_t					hfpVoiceState;		//1:��ͨ��,����Ӧ��ʹ����ͨ����·������Ƶ 0:����ͨ��

	//a2dp
	BT_A2DP_STATE			a2dpState;
//	uint8_t					remoteA2dpAddr[BT_ADDR_SIZE];
	uint8_t					a2dpStreamType;//=sbc; =aac
	uint32_t				aacFrameNumber;//������յ���aac֡��,���ڹ���decoder

	//avrcp
	BT_AVRCP_STATE			avrcpState;
//	uint8_t					remoteAvrcpAddr[BT_ADDR_SIZE];
	bool					avrcpConnectStart;
	BT_TIMER				avrcpPlayStatusTimer;	
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
	uint8_t					avrcpSyncEnable;
	uint8_t					avrcpSyncVol; //Ĭ��ֵ0xff(δͬ��), ����ֵ��Χ(0-32)
#endif

#if (BT_SPP_SUPPORT == ENABLE)
	//spp
	BT_SPP_STATE			sppState;
	//uint8_t					remoteSppAddr[BT_ADDR_SIZE];
#endif
	
#if (BT_MFI_SUPPORT == ENABLE)
	//mfi
	BT_MFI_STATE			mfiState;
#endif

#if (BT_OBEX_SUPPORT == ENABLE)
	//obex
	BT_OBEX_STATE			obexState;
#endif

#if (BT_PBAP_SUPPORT == ENABLE)
	//pbap
	BT_PBAP_STATE			pbapState;
#endif

	uint32_t				btDutModeEnable;		//DUTģʽ
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
 * @brief  Set Bt HF Sco State
 * @param  state: The state of bt HF Sco.
 * @return NONE
 * @Note
 *
 */
void SetBtScoState(uint8_t state);

/**
 * @brief  Get Bt HF Sco State
 * @param  NONE
 * @return The state of bt HF Sco.
 * @Note
 *
 */
uint8_t GetBtScoState(void);

/**
 * @brief  Set Bt Device Connection Mode
 * @param  mode: connection mode
 * @return TRUE: success
 * @Note
 *
 */
bool SetBtDeviceConnState(BT_DEVICE_CONNECTION_MODE mode);

/**
 * @brief  Get Bt Device Connection State
 * @param  NONE
 * @return connection mode
 * @Note
 *
 */
BT_DEVICE_CONNECTION_MODE GetBtDeviceConnState(void);


/**
* Bt device connected profile. 
*/
#define BT_CONNECTED_A2DP_FLAG			0x0001
#define BT_CONNECTED_AVRCP_FLAG			0x0002
#define BT_CONNECTED_HFP_FLAG			0x0004
#define BT_CONNECTED_MFI_FLAG			0x0008
#define BT_CONNECTED_SPP_FLAG			0x0010
#define BT_CONNECTED_OPP_FLAG			0x0020
#define BT_CONNECTED_HID_FLAG			0x0040

void SetBtConnectedProfile(uint16_t connectedProfile);
void SetBtDisconnectProfile(uint16_t disconnectProfile);
uint16_t GetBtConnectedProfile(void);
uint8_t GetNumOfBtConnectedProfile(void);

void SetBtCurConnectFlag(uint8_t state);
uint8_t GetBtCurConnectFlag(void);
#ifdef BT_TWS_SUPPORT
void BtAccessModeUpdate(BtAccessMode accessMode);
#endif
uint8_t BtConnectDecide(uint8_t *Addr);
uint8_t BtConnectConfirm(void);
/*****************************************************************************
* bt reconnect function
******************************************************************************/
void BtReconnectDevice(void);
uint32_t BtReconnectProfile(void);
void BtStartReconnectProfile(void);
void BtStartReconnectDevice(uint8_t mode);
void BtReconnectCB(void);
void CheckBtReconnectTimer(void);

#ifdef BT_TWS_SUPPORT
void BtReconnectTws(void);
void BtStartReconnectTws(uint8_t tryCount, uint8_t interval);
void BtStopReconnectTws(void);
void BtReconnectTwsCB(void);
void CheckBtReconnectTwsTimer(void);
void BtReconnectTws_Slave(void);
#endif

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
 * @brief  set the hfp profile state
 * @param  state
 * @return NONE
 * @Note
 *		
 */
void SetHfpState(BT_HFP_STATE state);

/**
 * @brief  get the hfp profile state
 * @param  NONE
 * @return state
 * @Note
 *		
 */
BT_HFP_STATE GetHfpState(void);

/**
 * @brief  
 * @param  
 * @return 
 * @Note
 *		
 */
int16_t GetHfpConnectedAddr(uint8_t * addr);

/**
 * @brief  get the sco connect flag
 * @param  NONE
 * @return TRUE - connected
 * @Note
 *		
 */
bool GetScoConnectFlag(void);

/**
 * @brief  get callin phone number
 * @param  phone number
 * @return state
 * @Note
 *		
 */
int16_t GetBtCallInPhoneNumber(uint8_t * number);

/**
 * @brief  set the voice recognition flag
 * @param  flag
 * @return state
 * @Note
 *		
 */
int16_t SetBtHfpVoiceRecognition(bool flag);

/**
 * @brief  use the voice recognition funciton
 * @param  
 * @return 
 * @Note
 * 
 */
void OpenBtHfpVoiceRecognitionFunc(void);

/**
 * @brief  Get bt hfp voice recognition flag
 * @param  flag
 * @return state
 * @Note
 * 
 */
int16_t GetBtHfpVoiceRecognition(bool * flag);

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
 * @brief  avrcp profile state
 * @param  NULL
 * @return 0: avrcp disconnect
 *         1: avrcp is connected
 * @Note
 *		
 */
 bool IsAvrcpConnected(void);

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

//�������ӶϿ�����״̬����
void BtLinkStateConnect(void);
void BtLinkStateDisconnect(void);
bool GetBtLinkState(void);

//�ֶ�����/�Ͽ�����
void BtConnectCtrl(void);
void BtDisconnectCtrl(void);

#ifdef BT_SNIFF_ENABLE
//����sniff�Ĵ�������
void Bt_sniff_state_init(void);
//����deepsleep flag
void Bt_sniff_sleep_enter(void);
void Bt_sniff_sleep_exit(void);
uint8_t Bt_sniff_sleep_state_get(void);

//����sniff flag
void Bt_sniff_sniff_start(void);
void Bt_sniff_sniff_stop(void);
uint8_t Bt_sniff_sniff_start_state_get(void);

#ifndef BT_TWS_SUPPORT
//���������Ƿ�ʹ��fastpower������sniffʱDIS��changemodeʱEN
void Bt_sniff_fastpower_dis(void);
void Bt_sniff_fastpower_en(void);
uint8_t Bt_sniff_fastpower_get(void);
#endif
#endif//BT_SNIFF_ENABLE


BT_OBEX_STATE GetObexState(void);

//app
void BtSetAccessMode_NoDisc_NoCon(void);
void BtSetAccessMode_NoDisc_Con(void);
void BtSetAccessMode_Disc_Con(void);

#endif

