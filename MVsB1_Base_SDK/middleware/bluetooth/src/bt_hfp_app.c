/**
 *******************************************************************************
 * @file    bt_hfp_app.c
 * @author  Halley
 * @version V1.0.1
 * @date    27-Apr-2016
 * @brief   Hfp callback events and actions
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
#include "type.h"
#include "debug.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#include "main_task.h"
#include "mode_switch_api.h"
#include "bt_hf_mode.h"
#include "audio_vol.h"
#endif
#include "bt_config.h"

#include "bt_hfp_api.h"
#include "bt_manager.h"
#include "bt_app_interface.h"
#include "app_message.h"
#include "dac.h"

#define BT_MANAGER_HFP_ERROR_NONE					0

#define BT_MANAGER_ERROR_PARAMETER_INVAILD			-1
#define BT_MANAGER_ERROR_NOT_INITIALIZED			-2
#define BT_MANAGER_HFP_ERROR_NOT_CONNECTED			-3


/*
* Previous declare
*/
//static void SetHfpState(BT_HFP_STATE state);
static void SetScoConnectFlag(bool flag);
static int16_t SetBtCallInPhoneNumber(const uint8_t * number, uint16_t len);
static int16_t SetBtCallWaitingNotify(bool flag);
static int16_t SetBtBatteryLevel(uint8_t level);
static int16_t SetBtHfpSignalLevel(uint8_t level);
static int16_t SetBtHfpRoam(bool flag);
//static int16_t SetBtHfpVoiceRecognition(bool flag);
static int16_t SetBtHfpSpeakerVolume(uint8_t gain);

#if  defined(CFG_APP_CONFIG)
extern void BtHfMsgToParent(uint16_t id);
extern void ExitBtHfMode(void);
extern void EnterBtHfMode(void);
extern uint32_t hfModeRestart;
extern uint32_t hfModeSuspend;
extern uint32_t gHfCodecTypeUpdate;
#else
#define BtHfMsgToParent
#define ExitBtHfMode()				// 用于注销代码
#define EnterBtHfMode()
#define gHfCodecTypeUpdate
#endif

static uint16_t testRecvLen = 0;

extern uint32_t gSpecificDevice;

//#define EVENT_SCO_CONNECT		0x01
uint32_t gHfFuncState = 0;

#ifdef BT_HFP_MODE_DISABLE
uint32_t gHfpCallNoneWaiting;
#endif

#ifdef CFG_FUNC_POWER_MONITOR_EN
#include "power_monitor.h"
static PWR_LEVEL powerLevelBak = PWR_LEVEL_0;//PWR_LEVEL_4;
void SetBtHfpBatteryLevel(PWR_LEVEL level, uint8_t flag)
{
#ifdef BT_HFP_BATTERY_SYNC
	uint8_t HfpBatLevel = 0;

	if(GetHfpState() < BT_HFP_STATE_CONNECTED)
		return;

	if((flag == 0)&&(powerLevelBak == level))
		return;

	powerLevelBak = level;
	switch(level)
	{
		case PWR_LEVEL_4:
			HfpBatLevel = 9;
			break;

		case PWR_LEVEL_3:
			HfpBatLevel = 6;
			break;
		
		case PWR_LEVEL_2:
			HfpBatLevel = 3;
			break;
		
		case PWR_LEVEL_1:
			HfpBatLevel = 1;
			break;
		
		case PWR_LEVEL_0:
		default:
			HfpBatLevel = 0;
			break;
	}
	
	HfpSetBatteryState(HfpBatLevel,0);
#endif
}
#endif

#ifdef BT_RECORD_FUNC_ENABLE
uint32_t gSysRecordMode2HfMode = 0;
#endif

void BtHfpCallback(BT_HFP_CALLBACK_EVENT event, BT_HFP_CALLBACK_PARAMS * param)
{
	switch(event)
	{

		case BT_STACK_EVENT_HFP_CONNECTED:
			{
				APP_DBG("Hfp Connected : bt address = %02x:%02x:%02x:%02x:%02x:%02x\n",
						(param->params.bd_addr)[0],
						(param->params.bd_addr)[1],
						(param->params.bd_addr)[2],
						(param->params.bd_addr)[3],
						(param->params.bd_addr)[4],
						(param->params.bd_addr)[5]);

				if(GetHfpState() > BT_HFP_STATE_CONNECTED)
				{
#if !defined(BT_HFP_MODE_DISABLE) && defined(CFG_APP_CONFIG)
					EnterBtHfMode();
#endif
				}
				else
					SetHfpState(BT_HFP_STATE_CONNECTED);

				if((param->params.bd_addr)[0] || (param->params.bd_addr)[1] || (param->params.bd_addr)[2] 
					|| (param->params.bd_addr)[3] || (param->params.bd_addr)[4] || (param->params.bd_addr)[5])
				{
					memcpy(GetBtManager()->remoteAddr, param->params.bd_addr, 6);
				}
				
				SetBtConnectedProfile(BT_CONNECTED_HFP_FLAG);
#ifdef BT_RECONNECTION_FUNC
				BtStartReconnectProfile();
#endif

#if (defined(CFG_FUNC_POWER_MONITOR_EN)&&defined(BT_HFP_BATTERY_SYNC))
				SetBtHfpBatteryLevel(PowerLevelGet(), 1);
#endif
				
				btManager.hfpVoiceState = 0;

				BtMidMessageSend(MSG_BT_MID_HFP_CONNECTED, 0);
			}
			break;

		case BT_STACK_EVENT_HFP_DISCONNECTED:
			{
				APP_DBG("Hfp disconnect\n");
				gSpecificDevice = 0;
				btManager.appleDeviceFlag = 0;
				SetHfpState(BT_HFP_STATE_NONE);
				
				SetBtDisconnectProfile(BT_CONNECTED_HFP_FLAG);

#if !defined(BT_HFP_MODE_DISABLE) && defined(CFG_APP_CONFIG)
				//断开HFP，同步退出通话模式
				if(GetSystemMode() == AppModeBtHfPlay)
				{
					ExitBtHfMode();
				}
			#ifdef BT_RECORD_FUNC_ENABLE
				else if(GetSystemMode() == AppModeBtRecordPlay)
				{
					BtMidMessageSend(MSG_BT_MID_HFP_RECORD_MODE_EXIT, 0);
				}
			#endif
#endif

				BtLinkStateDisconnect();
			}
			break;

		case BT_STACK_EVENT_HFP_SCO_CONNECTED:
			{
				uint8_t flag = 0;
				uint8_t type = GetHfpScoAudioCodecType();
				APP_DBG("Hfp sco connect\n");
				
				if(GetHfpState() < BT_HFP_STATE_CONNECTED)
					break;
#if defined(CFG_BT_RING_LOCAL)
				SetBtScoState(1);
				APP_DBG("SCO connected, so set the Hfp sco state is 1\n");
#endif
#ifdef BT_HFP_MODE_DISABLE
				HfpAudioDisconnect();
#else
				SetScoConnectFlag(TRUE);
				DelayExitBtHfModeCancel();
				
				APP_DBG("&&&sco audio type:%d\n", type);
				GetBtHfpVoiceRecognition(&flag);
				if(flag)
				{	
					EnterBtHfMode();
				}
				else if(type == 0)
				{
					//CVSD格式,直接进入通话模式
					EnterBtHfMode();
				}
				else
				{
			#ifdef BT_RECORD_FUNC_ENABLE
					BtMidMessageSend(MSG_BT_MID_HFP_RECORD_MODE_ENTER, 0);
			#else
					EnterBtHfMode();
			#endif
				}
#if  defined(CFG_APP_CONFIG)
				hfModeRestart = 1;
				hfModeSuspend = 0;
#endif
			BtMidMessageSend(MSG_BT_MID_HFP_TASK_RESUME, 0);
#endif
			}
			break;

		case BT_STACK_EVENT_HFP_SCO_DISCONNECTED:
			{
				bool flag = 0;
				APP_DBG("Hfp sco disconnect\n");
#ifndef BT_HFP_MODE_DISABLE		
				if(GetHfpState() < BT_HFP_STATE_CONNECTED)
					break;
				
				GetBtHfpVoiceRecognition(&flag);
				if(flag)
				{
					SetHfpState(BT_HFP_STATE_CONNECTED);
					#ifdef BT_RECORD_FUNC_ENABLE
						if(GetSystemMode() == AppModeBtRecordPlay)
						{
							BtMidMessageSend(MSG_BT_MID_HFP_RECORD_MODE_EXIT, 0);
						}
						else
						{
#if(BT_HFP_SUPPORT == ENABLE)
							DelayExitBtHfModeSet();
#endif
							break;
						}
					#else
						//ExitBtHfMode();
//						DelayExitBtHfModeSet();
					#endif
				}
				else
				{
					SetScoConnectFlag(FALSE);

					if(GetHfpState() == BT_HFP_STATE_CONNECTED)
					{
					#ifdef BT_RECORD_FUNC_ENABLE
						if(GetSystemMode() == AppModeBtRecordPlay)
						{
							BtMidMessageSend(MSG_BT_MID_HFP_RECORD_MODE_EXIT, 0);
						}
						else
						{
							ExitBtHfMode();
							break;
						}
					#else
						ExitBtHfMode();
						break;
					#endif
					}
#if  defined(CFG_APP_CONFIG)
#ifdef BT_RECORD_FUNC_ENABLE
					if(!gSysRecordMode2HfMode)
#endif
						BtHfTaskPause();
#endif
				}
				
			#ifdef BT_RECORD_FUNC_ENABLE
				BtMidMessageSend(MSG_BT_MID_HFP_RECORD_MODE_EXIT, 0);
			#endif
#endif
			}
			break;

#ifndef BT_HFP_MODE_DISABLE
		case BT_STACK_EVENT_HFP_SCO_STREAM_PAUSE:
			{
				APP_DBG("Hfp sco stream pause\n");
#if  defined(CFG_APP_CONFIG)
				hfModeSuspend = 1;
#endif
			}

// 下面逻辑移到bt_hf_mode中去处理	
//#ifdef CFG_RES_AUDIO_DAC0_EN
			//AudioDAC_SoftMute(DAC0, 1, 1);	//增加mute， 消除下通话断开有一下杂音
//#endif

//#ifdef CFG_RES_AUDIO_DACX_EN
            //AudioDAC_SoftMute(DAC1, 1, 1);

//#endif		
			break;
#endif

		case BT_STACK_EVENT_HFP_CALL_CONNECTED:
			{
				APP_DBG("Hfp call connected\n");

				//if(GetHfpState() < BT_HFP_STATE_CONNECTED)
				//	break;
#ifndef BT_HFP_MODE_DISABLE				
				if(GetHfpState() > BT_HFP_STATE_ACTIVE)
				{
					SetHfpState(BT_HFP_STATE_3WAY_ATCTIVE_CALL);
				}
				else
				{
					SetHfpState(BT_HFP_STATE_ACTIVE);
				}

				if(GetBtConnectedProfile()&BT_CONNECTED_HFP_FLAG)
				{
#ifdef CFG_FUNC_REMIND_SOUND_EN
					if(btManager.localringState == 1) //表示CallRingTmr还在等待计数的过程中，已经接通电话了，此时需要置位remind标志位，要不然会出现通话无声问题
					{
						btManager.localringState = 0;
						SoftFlagRegister(SoftFlagDecoderRemind);
					}
					BtMidMessageSend(MSG_BT_MID_HFP_PLAY_REMIND_END, 0);
#endif
#ifdef CFG_APP_CONFIG
#ifdef BT_REMOTE_AEC_DISABLE
					if(GetSystemMode() == AppModeBtHfPlay)
					 {
						HfpDisableNREC();
					 }
#endif
#endif
				}
#endif
			}
			break;

		case BT_STACK_EVENT_HFP_CALL_DISCONNECTED:
			{
				BT_HFP_STATE	hfpState;
				APP_DBG("Hfp call disconnect\n");
#if defined(CFG_BT_RING_LOCAL)
				APP_DBG("The call finished, soc disconnected, set the sco state to 0\n");
				SetBtScoState(0);
#endif
#ifndef BT_HFP_MODE_DISABLE
				if(GetHfpState() < BT_HFP_STATE_CONNECTED)
					break;

				hfpState = GetHfpState();
				switch(hfpState)
				{
					case BT_HFP_STATE_ACTIVE:
						SetHfpState(BT_HFP_STATE_CONNECTED);
						break;

					default:
						break;
				}
				SetHfpState(BT_HFP_STATE_CONNECTED);
				ExitBtHfMode();
#endif
			}
			break;

#ifndef BT_HFP_MODE_DISABLE
		case BT_STACK_EVENT_HFP_SCO_DATA_RECEIVED:
#ifdef CFG_BT_RING_LOCAL
			if(GetHfpState() == BT_HFP_STATE_INCOMING && !GetBtScoState())
			{
				BtMidMessageSend(MSG_BT_MID_HFP_PLAY_REMIND, 0);
			}
			else			
#endif
			//if(GetHfpState() == BT_HFP_STATE_ACTIVE)
			{
				//将数据缓存到bt sco fifo
				if(btManager.hfpScoCodecType != HFP_AUDIO_DATA_mSBC)
				{
					if(testRecvLen != param->paramsLen)
					{
						testRecvLen = param->paramsLen;
						APP_DBG("CVSD len:%d\n", testRecvLen);
					}
				}
				else
				{
					if(testRecvLen != param->paramsLen)
					{
						testRecvLen = param->paramsLen;
						APP_DBG("MSBC len:%d\n", testRecvLen);
					}
				}

				if(SaveHfpScoDataToBuffer)
					SaveHfpScoDataToBuffer(param->params.scoReceivedData,param->paramsLen);
			}
			break;
#endif

		case BT_STACK_EVENT_HFP_CALLSETUP_NONE:
			{
				BT_HFP_STATE	hfpState;
				
				APP_DBG("Hfp call setup none\n");
#ifdef BT_HFP_MODE_DISABLE
				//在呼入电话时,对方挂断电话,避免残留的铃声通过A2DP传过来,进行标识
				gHfpCallNoneWaiting = 1;
#else
				if(GetHfpState() < BT_HFP_STATE_CONNECTED)
					break;
				
				hfpState = GetHfpState();
				switch(hfpState)
				{
					case BT_HFP_STATE_3WAY_INCOMING_CALL:
					case BT_HFP_STATE_3WAY_OUTGOING_CALL:
						APP_DBG("3way calling,\n");
						SetHfpState(BT_HFP_STATE_3WAY_ATCTIVE_CALL);
						break;
					
					case BT_HFP_STATE_INCOMING:
					case BT_HFP_STATE_OUTGOING:
						SetHfpState(BT_HFP_STATE_CONNECTED);
						ExitBtHfMode();
						break;
						
					case BT_HFP_STATE_3WAY_ATCTIVE_CALL:
					case BT_HFP_STATE_ACTIVE:
						break;
						
					default:
						break;
				}
#endif
			}
			break;

#ifndef BT_HFP_MODE_DISABLE
		case BT_STACK_EVENT_HFP_CALLSETUP_IN:
			{
				if(GetHfpState() < BT_HFP_STATE_CONNECTED)
					break;
				
				if(GetHfpState()>= BT_HFP_STATE_ACTIVE)
				{
					APP_DBG("3 way incoming\n");
					SetHfpState(BT_HFP_STATE_3WAY_INCOMING_CALL);
				}
				else
				{
					APP_DBG("Hfp call setup incoming\n");
					SetHfpState(BT_HFP_STATE_INCOMING);
				}
#ifdef BT_RECORD_FUNC_ENABLE
				if(GetSystemMode() == AppModeBtRecordPlay)
				{
					gSysRecordMode2HfMode = 1;
				}
				
				BtMidMessageSend(MSG_BT_MID_HFP_RECORD_MODE_DEREGISTER, 0);
#endif
				EnterBtHfMode();

				btManager.hfpVoiceState = 0;
			}
			break;

		case BT_STACK_EVENT_HFP_CALLSETUP_OUT:
			{
				if(GetHfpState() < BT_HFP_STATE_CONNECTED)
					break;
				
				if(GetHfpState() >= BT_HFP_STATE_ACTIVE)
				{
					APP_DBG("3 way outgoing\n");
					SetHfpState(BT_HFP_STATE_3WAY_OUTGOING_CALL);
				}
				else
				{
					APP_DBG("Hfp call setup outgoing\n");
					SetHfpState(BT_HFP_STATE_OUTGOING);
				}
			
		#ifdef BT_RECORD_FUNC_ENABLE
				BtMidMessageSend(MSG_BT_MID_HFP_RECORD_MODE_ENTER, 0);
		#else
				EnterBtHfMode();
		#endif
				btManager.hfpVoiceState = 1;

				HfpGetCurrentCalls();
			}
			break;

		case BT_STACK_EVENT_HFP_CALLSETUP_ALERT:
			{
				if(GetHfpState() < BT_HFP_STATE_CONNECTED)
					break;
				
				APP_DBG("Hfp call setup alert\n");
				SetHfpState(BT_HFP_STATE_OUTGOING);
#ifdef BT_RECORD_FUNC_ENABLE
				BtMidMessageSend(MSG_BT_MID_HFP_RECORD_MODE_DEREGISTER, 0);
#endif
				EnterBtHfMode();
				
				btManager.hfpVoiceState = 0;
			}
			break;

		case BT_STACK_EVENT_HFP_RING:
			{
				APP_DBG("Hfp RING...\n");
				if(GetHfpState() < BT_HFP_STATE_CONNECTED)
					break;
				
				SetHfpState(BT_HFP_STATE_INCOMING);
				EnterBtHfMode();	// 来电铃声没建立SCO的这类手机，如果重启后有这个信号则自动进入HFP Mode
			}
			break;

		case BT_STACK_EVENT_HFP_CALLER_ID_NOTIFY:
			{
				APP_DBG("Hfp caller id : %s\n", param->params.hfpPhoneNumber);

				SetBtCallInPhoneNumber((uint8_t *)param->params.hfpPhoneNumber, param->paramsLen);
#ifdef CFG_FUNC_REMIND_SOUND_EN
				BtMidMessageSend(MSG_BT_MID_HFP_PLAY_REMIND, 0);
#endif
			}
			break;

		case BT_STACK_EVENT_HFP_CURRENT_CALLS:
			{
				APP_DBG("Hfp caller %d, state :%d, num: %s\n", param->params.callListParms.index, param->params.callListParms.state, param->params.callListParms.number);
			}
			break;

		case BT_STACK_EVENT_HFP_CALL_WAIT_NOTIFY:
			{
				APP_DBG("Hfp call wait : %s\n", param->params.hfpPhoneNumber);

				SetBtCallWaitingNotify(TRUE);

				SetBtCallInPhoneNumber((uint8_t *)param->params.hfpPhoneNumber, param->paramsLen);
			}
			break;

		case BT_STACK_EVENT_HFP_BATTERY:
			{
				APP_DBG("Hfp battery level : [%d]\n", param->params.hfpBattery);
				SetBtBatteryLevel(param->params.hfpBattery);
			}
			break;

		case BT_STACK_EVENT_HFP_SIGNAL:
			{
				APP_DBG("Hfp signal level : [%d]\n", param->params.hfpSignal);

				SetBtHfpSignalLevel(param->params.hfpSignal);
			}
			break;

		case BT_STACK_EVENT_HFP_ROAM:
			{
				if(param->params.hfpRoam)
					APP_DBG("Hfp roam TRUE\n");
				else
					APP_DBG("Hfp roam FALSE\n");

				SetBtHfpRoam(param->params.hfpRoam);
			}
			break;

		case BT_STACK_EVENT_HFP_VOICE_RECOGNITION:
			{
				if(GetHfpState() < BT_HFP_STATE_CONNECTED)
					break;
				
				if(param->params.hfpVoiceRec)
				{
					APP_DBG("Hfp vocie recognition TRUE\n");
					SetHfpState(BT_HFP_STATE_ACTIVE);
#if(BT_HFP_SUPPORT == ENABLE)
					DelayExitBtHfModeCancel();
#endif
				}
				else
				{
					APP_DBG("Hfp vocie recognition FALSE\n");
					SetHfpState(BT_HFP_STATE_CONNECTED);
					//ExitBtHfMode();
#if(BT_HFP_SUPPORT == ENABLE)
					DelayExitBtHfModeSet();
#endif
				}

				SetBtHfpVoiceRecognition(param->params.hfpVoiceRec);
			}
			break;

		case BT_STACK_EVENT_HFP_SPEAKER_VOLUME:
			{
				APP_DBG("Hfp speaker vol : [%d]\n", param->params.hfpVolGain);

				SetBtHfpSpeakerVolume(param->params.hfpVolGain);
			}
			break;

		case BT_STACK_EVENT_HFP_IN_BAND_RING:
			{
				if(param->params.hfpInBandRing)
					APP_DBG("Hfp inBandRing TRUE\n");
				else
					APP_DBG("Hfp inBandRing FALSE\n");
			}
			break;
		case BT_STACK_EVENT_HFP_MANUFACTORY_INFO:
			{
				APP_DBG("%s\n", param->params.hfpRemoteManufactory);
				btManager.appleDeviceFlag = 0;
				if(strstr(param->params.hfpRemoteManufactory,"Apple Inc."))
					btManager.appleDeviceFlag = 1;
			}
			break;

		case BT_STACK_EVENT_HFP_DEVICE_TYPE:
			{
				APP_DBG("%s\n", param->params.hfpRemoteDeviceType);
			}
			break;

		case BT_STACK_EVENT_HFP_UNSOLICITED_DATA:
			{
				/*uint16_t i;
				
				APP_DBG("Hfp unsolicited data (char): ");
				for(i = 0; i < param->paramsLen; i++)
				{
					APP_DBG("%c", param->params.hfpUnsolicitedData[i]);
				}
				APP_DBG("\n");
				*/
			}
			break;

		case BT_STACK_EVENT_HFP_CODEC_TYPE:
			{
				//type: 1=CVSD, 2=MSBC
				if(param->params.scoCodecType == HFP_AUDIO_DATA_mSBC)
				{
					APP_DBG("+++mSBC+++\n");
					btManager.hfpScoCodecType = HFP_AUDIO_DATA_mSBC;
				}
				else //if(param->params.scoCodecType == HFP_AUDIO_DATA_PCM)
				{
					APP_DBG("+++CVSD+++\n");
					btManager.hfpScoCodecType = HFP_AUDIO_DATA_PCM;
				}

				BtMidMessageSend(MSG_BT_MID_HFP_CODEC_TYPE_UPDATE, btManager.hfpScoCodecType);
			}
			break;
#endif

		default:
			break;
	}
}

/***************************************************************************
 **************************************************************************/
void BtHfpConnect(uint8_t * addr)
{
	SetHfpState(BT_HFP_STATE_CONNECTING);
	HfpConnect(addr);
}

void BtHfpDisconnect(void)
{
	HfpDisconnect();
}

/***************************************************************************
 **************************************************************************/
void SetHfpState(BT_HFP_STATE state)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return;

	tempBtManager->hfpState = state;
}

BT_HFP_STATE GetHfpState(void)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_HFP_STATE_NONE;

	return GetBtManager()->hfpState;
}

/***************************************************************************
 **************************************************************************/
static void SetScoConnectFlag(bool flag)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return;

	tempBtManager->scoConnected = flag;
}

bool GetScoConnectFlag(void)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return FALSE;

	return tempBtManager->scoConnected;
}

/***************************************************************************
 **************************************************************************/
static int16_t SetBtCallInPhoneNumber(const uint8_t * number, uint16_t len)
{
	BT_MANAGER_ST *	tempBtManager = NULL;
	uint16_t		tempLen;

	tempBtManager = GetBtManager();

	if(number == NULL || len == 0 )
		return BT_MANAGER_ERROR_PARAMETER_INVAILD;

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	if(tempBtManager->hfpState < BT_HFP_STATE_CONNECTED)
		return BT_MANAGER_HFP_ERROR_NOT_CONNECTED;

	tempLen = len > (MAX_PHONE_NUMBER_LENGTH - 1) ? (MAX_PHONE_NUMBER_LENGTH - 1) : len;
	
	memset(tempBtManager->phoneNumber, 0, MAX_PHONE_NUMBER_LENGTH);
	strncpy((char*)tempBtManager->phoneNumber, (char*)number, tempLen);

	return tempLen;
}

int16_t GetBtCallInPhoneNumber(uint8_t * number)
{
	BT_MANAGER_ST *	tempBtManager = NULL;
	uint16_t		tempLen;

	tempBtManager = GetBtManager();

	if(number == NULL)
		return BT_MANAGER_ERROR_PARAMETER_INVAILD;

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	if(tempBtManager->hfpState < BT_HFP_STATE_CONNECTED)
		return BT_MANAGER_HFP_ERROR_NOT_CONNECTED;

	tempLen = strlen((const char*)tempBtManager->phoneNumber);

	strncpy((char*)number, (const char*)tempBtManager->phoneNumber, tempLen);

	return tempLen;
}

/***************************************************************************
 **************************************************************************/
static int16_t SetBtCallWaitingNotify(bool flag)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	tempBtManager->callWaitingFlag = flag;

	return BT_MANAGER_HFP_ERROR_NONE;
	
}

int16_t GetBtCallWaitingNotify(bool * flag)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	if(flag == NULL)
		return BT_MANAGER_ERROR_PARAMETER_INVAILD;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	*flag = tempBtManager->callWaitingFlag;

	return BT_MANAGER_HFP_ERROR_NONE;
}

/***************************************************************************
 **************************************************************************/
static int16_t SetBtBatteryLevel(uint8_t level)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	tempBtManager->batteryLevel = level;

	return BT_MANAGER_HFP_ERROR_NONE;
}

int16_t GetBtBatteryLevel(uint8_t * level)
{

	BT_MANAGER_ST *	tempBtManager = NULL;

	if(level == NULL)
		return BT_MANAGER_ERROR_PARAMETER_INVAILD;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	*level = tempBtManager->batteryLevel;

	return BT_MANAGER_HFP_ERROR_NONE;
}


/***************************************************************************
 **************************************************************************/
static int16_t SetBtHfpSignalLevel(uint8_t level)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	tempBtManager->signalLevel = level;

	return BT_MANAGER_HFP_ERROR_NONE;
}

int16_t GetBtSignalLevel(uint8_t * level)
{

	BT_MANAGER_ST *	tempBtManager = NULL;

	if(level == NULL)
		return BT_MANAGER_ERROR_PARAMETER_INVAILD;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	*level = tempBtManager->signalLevel;

	return BT_MANAGER_HFP_ERROR_NONE;
}


/***************************************************************************
 **************************************************************************/
static int16_t SetBtHfpRoam(bool flag)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	tempBtManager->roamFlag = flag;

	return BT_MANAGER_HFP_ERROR_NONE;
	
}

int16_t GetBtHfpRoam(bool * flag)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	if(flag == NULL)
		return BT_MANAGER_ERROR_PARAMETER_INVAILD;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	*flag = tempBtManager->roamFlag;

	return BT_MANAGER_HFP_ERROR_NONE;
}


/***************************************************************************
 **************************************************************************/
int16_t SetBtHfpVoiceRecognition(bool flag)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	tempBtManager->voiceRecognition = flag;

	return BT_MANAGER_HFP_ERROR_NONE;
	
}

int16_t GetBtHfpVoiceRecognition(bool * flag)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	if(flag == NULL)
		return BT_MANAGER_ERROR_PARAMETER_INVAILD;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	*flag = tempBtManager->voiceRecognition;

	return BT_MANAGER_HFP_ERROR_NONE;
}

void OpenBtHfpVoiceRecognitionFunc(void)
{
	if(GetHfpState() == BT_HFP_STATE_CONNECTED)
	{
		APP_DBG("test open voicerecognition\n");
		HfpVoiceRecognition(1);
		SetBtHfpVoiceRecognition(1);
	}
}

/***************************************************************************
 **************************************************************************/
static int16_t SetBtHfpSpeakerVolume(uint8_t gain)
{
	BT_MANAGER_ST *	tempBtManager = NULL;
	uint32_t hfpVolume = 0;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	tempBtManager->volGain = gain;
#if defined(CFG_APP_CONFIG)
	if(GetSystemMode() != AppModeBtHfPlay)
		return BT_MANAGER_HFP_ERROR_NONE;

	hfpVolume = (tempBtManager->volGain*CFG_PARA_MAX_VOLUME_NUM)/15;

	AudioHfVolSet((uint8_t)hfpVolume);
#endif
	return BT_MANAGER_HFP_ERROR_NONE;
}

int16_t GetBtHfpSpeakerVolume(uint8_t * gain)
{

	BT_MANAGER_ST *	tempBtManager = NULL;

	if(gain == NULL)
		return BT_MANAGER_ERROR_PARAMETER_INVAILD;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	*gain = tempBtManager->volGain;

	return BT_MANAGER_HFP_ERROR_NONE;
}


