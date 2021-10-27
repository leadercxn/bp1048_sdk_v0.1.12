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
 

#include "bt_manager.h"

#include "type.h"
#include "debug.h"

#include "bt_app_func.h"
#include "bt_ddb_flash.h"
#include "bt_stack_api.h"
#include "app_message.h"

BT_MANAGER_ST		btManager;


extern void Set_rwip_sleep_enable(bool flag);

extern BT_CONFIGURATION_PARAMS		*btStackConfigParams;
uint8_t bt_powerup_flag = 1;

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtLinkConnect(void)
{
#if defined(CFG_APP_CONFIG)
	if((GetA2dpState() >= BT_A2DP_STATE_CONNECTED))
	{
		MessageContext		msgSend;
		msgSend.msgId		= MSG_BT_STATE_CONNECTED;
		MessageSend(GetMainMessageHandle(), &msgSend);
	}
#endif
	btManager.btLinkState = 1;
}

void BtLinkDisconnect(void)
{

	if( btManager.btLinkState
		&& (GetAvrcpState() == BT_AVRCP_STATE_NONE)
		&& (GetA2dpState() == BT_A2DP_STATE_NONE)
		&& (GetHfpState() == BT_HFP_STATE_NONE)
	)
	{
#if defined(CFG_APP_CONFIG)
		MessageContext		msgSend;
		msgSend.msgId		= MSG_BT_STATE_DISCONNECT;
		MessageSend(GetMainMessageHandle(), &msgSend);
#endif
		btManager.btLinkState = 0;

		//btManager.btLastAddrUpgradeIgnored = 0;
	}
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void SetBtCurConnectFlag(uint8_t state)
{
	btManager.btCurConnectFlag = state;
}

uint8_t GetBtCurConnectFlag(void)
{
	return btManager.btCurConnectFlag;
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtStackCallback(BT_STACK_CALLBACK_EVENT event, BT_STACK_CALLBACK_PARAMS * param)
{
	switch(event)
	{
		case BT_STACK_EVENT_COMMON_STACK_INITIALIZED:
		{
			BT_DBG("BT_STACK_EVENT_COMMON_STACK_INITIALIZED\n");
			SetBtDeviceConnMode(BT_DEVICE_CONNECTION_MODE_NONE);
			bt_powerup_flag = 1;
		}
		break;

		case BT_STACK_EVENT_COMMON_STACK_FREE_MEM_SIZE:
		{
			//BT_DBG("stack mem:%d, %d\n", param->params.stackMemParams.stackMallocMemSize, param->params.stackMemParams.stackFreeMemSize);
		}
		break;

		case BT_STACK_EVENT_COMMON_STACK_OUT_OF_MEM_ERR:
		{
			BT_DBG("!!!ERR: BT_STACK_EVENT_COMMON_STACK_FREE_MEM_SIZE\n");
		}
		break;

		case BT_STACK_EVENT_COMMON_INQUIRY_RESULT:
		{
			BT_DBG("Inquriy Result:\n");
			BT_DBG("\t%02x:%02x:%02x:%02x:%02x:%02x",
					(param->params.inquiryResult.addr)[0],
					(param->params.inquiryResult.addr)[1],
					(param->params.inquiryResult.addr)[2],
					(param->params.inquiryResult.addr)[3],
					(param->params.inquiryResult.addr)[4],
					(param->params.inquiryResult.addr)[5]);
			BT_DBG("\t %d dB", param->params.inquiryResult.rssi);
			BT_DBG("\t extLen = %d , ext = %s \n",param->params.inquiryResult.extRespLen, param->params.inquiryResult.extResp);
		}
		break;

		case BT_STACK_EVENT_COMMON_INQUIRY_COMPLETE:
		{
			BT_DBG("BT_STACK_EVENT_COMMON_INQUIRY_COMPLETE\n");
		}
		break;

		case BT_STACK_EVENT_COMMON_INQUIRY_CANCELED:
		{
			BT_DBG("BT_STACK_EVENT_COMMON_INQUIRY_CANCELED\n");
		}
		break;


		case BT_STACK_EVENT_COMMON_MODE_CHANGE:
			BT_DBG("mode - %d, addrs -  %02x:%02x:%02x:%02x:%02x:%02x\n",
					param->params.modeChange.mode,
					(param->params.modeChange.addr)[0],
					(param->params.modeChange.addr)[1],
					(param->params.modeChange.addr)[2],
					(param->params.modeChange.addr)[3],
					(param->params.modeChange.addr)[4],
					(param->params.modeChange.addr)[5]);
			if(param->params.modeChange.mode == BTLinkModeSniff)
			{
//				SwitchModeTo(APP_MODE_IDLE);
			}
		break;

		case BT_STACK_EVENT_COMMON_GET_REMDEV_NAME:
		{
			BT_DBG("Remote Device:\n");
			BT_DBG("\t%02x:%02x:%02x:%02x:%02x:%02x",
					(param->params.remDevName.addr)[0],
					(param->params.remDevName.addr)[1],
					(param->params.remDevName.addr)[2],
					(param->params.remDevName.addr)[3],
					(param->params.remDevName.addr)[4],
					(param->params.remDevName.addr)[5]);
			BT_DBG("\t nameLen = %d , name = %s \n",param->params.remDevName.nameLen, param->params.remDevName.name);
		}
		break;

		case BT_STACK_EVENT_COMMON_ACCESS_MODE:
		{
			//DBG("AccesMode = %d\n", param->params.accessMode);
			if(param->params.accessMode == BtAccessModeNotAccessible)
			{
				BT_DBG("ACCESS MODE: Bluetooth no discover no connectable.\r\n");
				SetBtDeviceConnMode(BT_DEVICE_CONNECTION_MODE_NONE);
				if(bt_powerup_flag)
				{
					bt_powerup_flag = 0;
					
					BTSetAccessMode(BtAccessModeGeneralAccessible); // < General discoverable and connectable >
				}
			}
			else if(param->params.accessMode == BtAccessModeDiscoverbleOnly)
			{
				BT_DBG("ACCESS MODE: only Discoverable\r\n");
				SetBtDeviceConnMode(BT_DEVICE_CONNECTION_MODE_DISCOVERIBLE);
			}
			else if(param->params.accessMode == BtAccessModeConnectableOnly)
			{
				BT_DBG("ACCESS MODE: only Connectable\r\n");
				SetBtDeviceConnMode(BT_DEVICE_CONNECTION_MODE_CONNECTABLE);
			}
			else
			{
				BT_DBG("ACCESS MODE: WaitForPairing/Connection\r\n");
				SetBtDeviceConnMode(BT_DEVICE_CONNECTION_MODE_ALL);
				
				if(GetBtStackState() == BT_STACK_STATE_INITAILIZING)
				{
					SetBtStackState(BT_STACK_STATE_READY);
				}
				
				//更新inquiry scan的相关参数
				//WriteInquiryScanActivity(btStackConfigParams->bt_InquiryScanInterval, btStackConfigParams->bt_InquiryScanWindow);
				
#ifdef BT_RECONNECTION_FUNC
				BtReconnectDevice();
#endif

				//enable bb enter sleep
				Set_rwip_sleep_enable(1);

				btManager.btLastAddrUpgradeIgnored = 0;
			}
		}
		break;

		case BT_STACK_EVENT_COMMON_CONNECTION_ABORTED:
			BT_DBG("BT_STACK_EVENT_COMMON_CONNECTION_ABORTED\n");
		break;

		case BT_STACK_EVENT_COMMON_PAGE_TIME_OUT:
			BT_DBG("BT_STACK_EVENT_COMMON_PAGE_TIME_OUT\n");
#ifdef BT_RECONNECTION_FUNC
			BtReconnectCB();
#endif
		break;
		
		case BT_STACK_EVENT_TRUST_LIST_ADD_NEW_RECORD:
		{
			BT_DBG("A new device has been added into db : bt address = %02x:%02x:%02x:%02x:%02x:%02x\n",
					(param->params.bd_addr)[0],
					(param->params.bd_addr)[1],
					(param->params.bd_addr)[2],
					(param->params.bd_addr)[3],
					(param->params.bd_addr)[4],
					(param->params.bd_addr)[5]);
		}
		break;

		case BT_STACK_EVENT_TRUST_LIST_DELETE_RECORD:
		break;

		case BT_STACK_EVENT_COMMON_BB_LOST_CONNECTION:
		{
			BT_DBG("BB LOST\n");
#ifdef BT_RECONNECTION_FUNC
			if(!btManager.btLastAddrUpgradeIgnored)
				BtStartReconnectDevice(2);
#endif
		}
		break;

		default:
		break;
	}
}

/**************************************************************************************
 **************************************************************************************/
void ClearBtManagerReg(void)
{
	
	memset(&btManager, 0, sizeof(BT_MANAGER_ST));
}

BT_MANAGER_ST * GetBtManager(void)
{
	return &btManager;
}

void SetBtStackState(BT_STACK_STATE state)
{
	GetBtManager()->stackState = state;
}

BT_STACK_STATE GetBtStackState(void)
{
	return GetBtManager()->stackState;
}

bool SetBtDeviceConnMode(BT_DEVICE_CONNECTION_MODE mode)
{
	//if(GetBtStackState() != BT_STACK_STATE_READY)
	//	return FALSE;

	GetBtManager()->deviceConMode = mode;

	return TRUE;
}

BT_DEVICE_CONNECTION_MODE GetBtDeviceConnMode(void)
{
	//if(GetBtStackState() != BT_STACK_STATE_READY)
	//	return BT_DEVICE_CONNECTION_MODE_NONE;

	return GetBtManager()->deviceConMode;
}
/**
* Bt device connected profile manage. 
*/
void SetBtConnectedProfile(uint16_t connectedProfile)
{
	btManager.btConnectedProfile |= connectedProfile;

	//连接到一个蓝牙设备的一个profile时,就更新最后1次连接的蓝牙记录信息
	if(GetNumOfBtConnectedProfile() == 1)
	{
		//配对地址一致,不需要重复保存
		if(memcmp(btManager.btDdbLastAddr, btManager.remoteAddr, 6) == 0)
			return ;

		memcpy(btManager.btDdbLastAddr, btManager.remoteAddr, 6);
		BtDdb_UpgradeLastBtAddr(btManager.remoteAddr);
	}

	SetBtCurConnectFlag(1);
}

void SetBtDisconnectProfile(uint16_t disconnectProfile)
{
	btManager.btConnectedProfile &= ~disconnectProfile;

	if(!btManager.btConnectedProfile)
	{
		SetBtCurConnectFlag(0);
	}
}

uint8_t GetNumOfBtConnectedProfile(void)
{
	uint8_t		number_of_profile = 0;

	if(btManager.btConnectedProfile & BT_CONNECTED_A2DP_FLAG)
	{
		number_of_profile++;
	}

	if(btManager.btConnectedProfile & BT_CONNECTED_HFP_FLAG)
	{
		number_of_profile++;
	}

	if(btManager.btConnectedProfile & BT_CONNECTED_SPP_FLAG)
	{
		number_of_profile++;
	}
		
	if(btManager.btConnectedProfile & BT_CONNECTED_AVRCP_FLAG)
	{
		number_of_profile++;
	}

	return 	number_of_profile;
}

#ifdef BT_RECONNECTION_FUNC
void BtReconnectDevice(void)
{
	if(btManager.BtPowerOnFlag)
		return;

	btManager.BtPowerOnFlag = 1;
	//reconnect last bluetooth device
	if((GetBtManager()->btDdbLastAddr[0]==0)
		&&(GetBtManager()->btDdbLastAddr[1]==0)
		&&(GetBtManager()->btDdbLastAddr[2]==0)
		&&(GetBtManager()->btDdbLastAddr[3]==0)
		&&(GetBtManager()->btDdbLastAddr[4]==0)
		&&(GetBtManager()->btDdbLastAddr[5]==0))
	{
		BT_DBG("Last Addr is NULL\n");
		return ;
	}
	
	//reconnect remote device
	BT_DBG("Reconnect:");
	{
		uint8_t i;
		for(i=0; i<6; i++)
		{
			BT_DBG("%x ", GetBtManager()->btDdbLastAddr[i]);
		}
	}
	BT_DBG("\n");

	BtStartReconnectDevice(1);
}

extern bool IsBtReconnectReady(void);

static void BtReconnectDelay(void)
{
	DBG("Busy\n");
	TimeOutSet(&btManager.btReconnectTimer.timerHandle, 500);
	btManager.btReconnectTimer.timerFlag |= TIMER_STARTED;
}

uint32_t BtReconnectProfile(void)
{
	uint32_t reconnectProfile = 0;
	reconnectProfile = GetSupportProfiles();
	reconnectProfile &= (BT_PROFILE_SUPPORTED_HFP | BT_PROFILE_SUPPORTED_A2DP | BT_PROFILE_SUPPORTED_AVRCP);

	//在回连手机时,打开手机蓝牙功能;安卓手机会自动回连;
	//某些安卓手机回连非常快,会导致sdp channel复用,导致异常;
	//在发起回连时,确定sdp server是不是被使用,未使用则开始进行回连
	if((reconnectProfile & BT_PROFILE_SUPPORTED_HFP)&&(GetHfpState() < BT_HFP_STATE_CONNECTED))
	{
		if(!IsBtReconnectReady())
		{
			BtReconnectDelay();
			return reconnectProfile;
		}
		BtHfpConnect(GetBtManager()->btDdbLastAddr);
	}
	else if((reconnectProfile & BT_PROFILE_SUPPORTED_A2DP)&&(GetA2dpState() < BT_A2DP_STATE_CONNECTED))
	{
		reconnectProfile &= ~BT_PROFILE_SUPPORTED_HFP;
		
		if(!IsBtReconnectReady())
		{
			BtReconnectDelay();
			return reconnectProfile;
		}
		BtA2dpConnect(GetBtManager()->btDdbLastAddr);
	}
	else if((reconnectProfile & BT_PROFILE_SUPPORTED_AVRCP)&&(GetAvrcpState() != BT_AVRCP_STATE_CONNECTED))
	{
		reconnectProfile &= ~(BT_PROFILE_SUPPORTED_A2DP | BT_PROFILE_SUPPORTED_HFP);
		
		if(!IsBtReconnectReady())
		{
			BtReconnectDelay();
			return reconnectProfile;
		}
		BtAvrcpConnect(GetBtManager()->btDdbLastAddr);
	}
	else
	{
		reconnectProfile &= ~(BT_PROFILE_SUPPORTED_HFP | BT_PROFILE_SUPPORTED_A2DP | BT_PROFILE_SUPPORTED_AVRCP);
		BtStopReconnect();
	}

	return reconnectProfile;
}

//1.在开机的时候，需要回连连接过的设备
//2.在BB连接丢失(设备拿远后)，需要回连设备
void BtStartReconnectDevice(uint8_t mode)
{
	if(mode == 1)
	{
	#ifdef BT_POWERON_RECONNECTION
		//power on: reconnect device
		btManager.btReconnectTryCount = BT_POR_TRY_COUNTS;
		btManager.btReconnectIntervalTime = BT_POR_INTERNAL_TIME;
		btManager.btReconnectTimer.timerFlag = TIMER_USED;
		btManager.btReconnectedFlag = 0;
		BtReconnectProfile();
	#endif
	}
	else if(mode == 2)
	{
	#ifdef BT_BB_LOST_RECONNECTION
		//bb lost: reconnect device
		btManager.btReconnectTryCount = BT_BLR_TRY_COUNTS;
		btManager.btReconnectIntervalTime = BT_BLR_INTERNAL_TIME;
		btManager.btReconnectTimer.timerFlag = TIMER_USED;
		btManager.btReconnectedFlag = 0;
		BtReconnectProfile();
	#endif
	}
}

void BtStartReconnectProfile(void)
{
	if(!btManager.btReconnectTimer.timerFlag)
		return;
		
	btManager.btReconnectedFlag = 1;
	TimeOutSet(&btManager.btReconnectTimer.timerHandle, 200);
	btManager.btReconnectTimer.timerFlag |= TIMER_STARTED;
}

//1.在连接设备成功后，需要停止回连
//2.在连接次数超时后，需要停止回连
//3.在有其他设备连接成功后，停止回连
void BtStopReconnect(void)
{
	btManager.btReconnectTimer.timerFlag = TIMER_UNUSED;
	btManager.btReconnectTryCount = 0;
	btManager.btReconnectIntervalTime = 0;
	btManager.btReconnectedFlag = 0;
	BT_DBG("BtStopReconnect\n");
}


void BtReconnectCB(void)
{
	if(!btManager.btReconnectTimer.timerFlag)
		return;

	if(--btManager.btReconnectTryCount)
	{
		TimeOutSet(&btManager.btReconnectTimer.timerHandle, btManager.btReconnectIntervalTime*1000);
		btManager.btReconnectTimer.timerFlag |= TIMER_STARTED;
		BT_DBG("[BT] BtReConnect will be started in %d seconds\n", btManager.btReconnectIntervalTime);
	}
	else
	{
		BtStopReconnect();
	}
}

void CheckBtReconnectTimer(void)
{
	btManager.btReconnectTimer.timerFlag &= ~TIMER_STARTED;

	if(!BtReconnectProfile())
	{
		if(btManager.btReconnectedFlag)
		{
			BtStopReconnect();
		}
	}
}

#endif

