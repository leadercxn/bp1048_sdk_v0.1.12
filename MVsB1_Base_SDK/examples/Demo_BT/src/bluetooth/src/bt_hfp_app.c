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
//#include "app_config.h"


#include "bt_app_interface.h"
#include "bt_hfp_api.h"
#include "bt_manager.h"
#include "app_message.h"
//#include "mode_switch_api.h"
//#include "audio_vol.h"
//#include "bt_hf_mode.h"

#define BT_MANAGER_HFP_ERROR_NONE					0

#define BT_MANAGER_ERROR_PARAMETER_INVAILD			-1
#define BT_MANAGER_ERROR_NOT_INITIALIZED			-2
#define BT_MANAGER_HFP_ERROR_NOT_CONNECTED			-3


/*
* Previous declare
*/
static void SetHfpState(BT_HFP_STATE state);
static void SetHfpConnectedAddr(uint8_t * addr);
static void SetScoConnectFlag(bool flag);
static int16_t SetBtCallInPhoneNumber(const uint8_t * number, uint16_t len);
static int16_t SetBtCallWaitingNotify(bool flag);
static int16_t SetBtBatteryLevel(uint8_t level);
static int16_t SetBtHfpSignalLevel(uint8_t level);
static int16_t SetBtHfpRoam(bool flag);
static int16_t SetBtHfpVoiceRecognition(bool flag);
static int16_t SetBtHfpSpeakerVolume(uint8_t gain);

extern void BtHfMsgToParent(uint16_t id);
extern void ExitBtHfMode(void);
extern void EnterBtHfMode(void);

static uint16_t testRecvLen = 0;

extern uint32_t hfModeRestart;
extern uint32_t hfModeSuspend;
extern uint32_t gHfCodecTypeUpdate;

#ifdef CFG_FUNC_POWER_MONITOR_EN
#include "power_monitor.h"
static PWR_LEVEL powerLevelBak = PWR_LEVEL_4;
void SetBtHfpBatteryLevel(PWR_LEVEL level)
{
	uint8_t HfpBatLevel = 0;

	if(powerLevelBak == level)
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
}
#endif

void BtHfpCallback(BT_HFP_CALLBACK_EVENT event, BT_HFP_CALLBACK_PARAMS * param)
{
	switch(event)
	{

		case BT_STACK_EVENT_HFP_CONNECTED:
			{
				BT_DBG("Hfp Connected : bt address = %02x:%02x:%02x:%02x:%02x:%02x\n",
						(param->params.bd_addr)[0],
						(param->params.bd_addr)[1],
						(param->params.bd_addr)[2],
						(param->params.bd_addr)[3],
						(param->params.bd_addr)[4],
						(param->params.bd_addr)[5]);

				if(GetHfpState() > BT_HFP_STATE_CONNECTED)
				{
#if defined(CFG_APP_CONFIG)
					if(GetSystemMode() != AppModeBtHfPlay)
					{
						ResourceRegister(AppResourceBtHf);
						BtHfMsgToParent(MSG_DEVICE_SERVICE_BTHF_IN);
					}
#endif
				}
				else
					SetHfpState(BT_HFP_STATE_CONNECTED);

				//SetHfpConnectedAddr(param->params.bd_addr);
				
				memcpy(GetBtManager()->remoteAddr, param->params.bd_addr, 6);

				SetBtConnectedProfile(BT_CONNECTED_HFP_FLAG);
#ifdef BT_RECONNECTION_FUNC
				BtStartReconnectProfile();
#endif

#ifdef CFG_FUNC_POWER_MONITOR_EN
				SetBtHfpBatteryLevel(PowerLevelGet());
#endif

				btManager.hfpVoiceState = 0;
			}
			break;

		case BT_STACK_EVENT_HFP_DISCONNECTED:
			{
				BT_DBG("Hfp disconnect\n");

				SetHfpState(BT_HFP_STATE_NONE);

				SetHfpConnectedAddr(NULL); // clear connected address
				
				SetBtDisconnectProfile(BT_CONNECTED_HFP_FLAG);
#if defined(CFG_APP_CONFIG)
				//断开HFP，同步退出通话模式
				if(GetSystemMode() == AppModeBtHfPlay)
				{
					ExitBtHfMode();
				}
#endif
				BtLinkDisconnect();
			}
			break;

		case BT_STACK_EVENT_HFP_SCO_CONNECTED:
			{
				BT_DBG("Hfp sco connect\n");
				SetScoConnectFlag(TRUE);

				DBG("&&&sco audio type:%d\n", GetHfpScoAudioCodecType());
#if defined(CFG_APP_CONFIG)
				if(GetSystemMode() != AppModeBtHfPlay)
				{
					ResourceRegister(AppResourceBtHf);
					BtHfMsgToParent(MSG_DEVICE_SERVICE_BTHF_IN);
				}
#endif
				//else
				{
					hfModeRestart = 1;
				}

				hfModeSuspend = 0;
			}
			break;

		case BT_STACK_EVENT_HFP_SCO_DISCONNECTED:
			{
				BT_DBG("Hfp sco disconnect\n");

				SetScoConnectFlag(FALSE);
				
				//ExitBtHfMode();
				BtHfTaskPause();
			}
			break;

		case BT_STACK_EVENT_HFP_SCO_STREAM_PAUSE:
			{
				BT_DBG("Hfp sco stream pause\n");
				
				hfModeSuspend = 1;
			}
			break;

		case BT_STACK_EVENT_HFP_CALL_CONNECTED:
			{
				BT_DBG("Hfp call connected\n");

				SetHfpState(BT_HFP_STATE_ACTIVE);
			}
			break;

		case BT_STACK_EVENT_HFP_CALL_DISCONNECTED:
			{
				BT_HFP_STATE	hfpState;
				BT_DBG("Hfp call disconnect\n");

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
			}
			break;

		case BT_STACK_EVENT_HFP_SCO_DATA_RECEIVED:
			//if(GetHfpState() == BT_HFP_STATE_ACTIVE)
			{
				//将数据缓存到bt sco fifo
				if(btManager.hfpScoCodecType != HFP_AUDIO_DATA_mSBC)
				{
					if(testRecvLen != param->paramsLen)
					{
						testRecvLen = param->paramsLen;
						DBG("CVSD len:%d\n", testRecvLen);
					}
				}
				else
				{
					if(testRecvLen != param->paramsLen)
					{
						testRecvLen = param->paramsLen;
						DBG("MSBC len:%d\n", testRecvLen);
					}
				}

				if(SaveHfpScoDataToBuffer)
					SaveHfpScoDataToBuffer(param->params.scoReceivedData,param->paramsLen);
			}
			break;

		case BT_STACK_EVENT_HFP_CALLSETUP_NONE:
			{
				BT_HFP_STATE	hfpState;
				
				BT_DBG("Hfp call setup none\n");

				hfpState = GetHfpState();
				switch(hfpState)
				{
					case BT_HFP_STATE_INCOMING:
					case BT_HFP_STATE_OUTGOING:
						SetHfpState(BT_HFP_STATE_CONNECTED);
						ExitBtHfMode();
						break;

					case BT_HFP_STATE_ACTIVE:
						break;
						
					default:
						break;
				}
			}
			break;

		case BT_STACK_EVENT_HFP_CALLSETUP_IN:
			{
				BT_DBG("Hfp call setup incoming\n");
				SetHfpState(BT_HFP_STATE_INCOMING);
				EnterBtHfMode();

				btManager.hfpVoiceState = 0;
			}
			break;

		case BT_STACK_EVENT_HFP_CALLSETUP_OUT:
			{
				BT_DBG("Hfp call setup outgoing\n");
				SetHfpState(BT_HFP_STATE_OUTGOING);
				EnterBtHfMode();

				btManager.hfpVoiceState = 1;
			}
			break;

		case BT_STACK_EVENT_HFP_CALLSETUP_ALERT:
			{
				BT_DBG("Hfp call setup alert\n");
				SetHfpState(BT_HFP_STATE_OUTGOING);
				EnterBtHfMode();
				
				btManager.hfpVoiceState = 0;
			}
			break;

		case BT_STACK_EVENT_HFP_RING:
			{
				BT_DBG("Hfp RING...\n");
				SetHfpState(BT_HFP_STATE_INCOMING);
			}
			break;

		case BT_STACK_EVENT_HFP_CALLER_ID_NOTIFY:
			{
				BT_DBG("Hfp caller id : %s\n", param->params.hfpPhoneNumber);

				SetBtCallInPhoneNumber((uint8_t *)param->params.hfpPhoneNumber, param->paramsLen);
			}
			break;

		case BT_STACK_EVENT_HFP_CURRENT_CALLS:
			{
				BT_DBG("Hfp current caller id : %s\n", param->params.hfpPhoneNumber);
			}
			break;

		case BT_STACK_EVENT_HFP_CALL_WAIT_NOTIFY:
			{
				BT_DBG("Hfp call wait : %s\n", param->params.hfpPhoneNumber);

				SetBtCallWaitingNotify(TRUE);

				SetBtCallInPhoneNumber((uint8_t *)param->params.hfpPhoneNumber, param->paramsLen);
			}
			break;

		case BT_STACK_EVENT_HFP_BATTERY:
			{
				BT_DBG("Hfp battery level : [%d]\n", param->params.hfpBattery);
				SetBtBatteryLevel(param->params.hfpBattery);
			}
			break;

		case BT_STACK_EVENT_HFP_SIGNAL:
			{
				BT_DBG("Hfp signal level : [%d]\n", param->params.hfpSignal);

				SetBtHfpSignalLevel(param->params.hfpSignal);
			}
			break;

		case BT_STACK_EVENT_HFP_ROAM:
			{
				if(param->params.hfpRoam)
					BT_DBG("Hfp roam TRUE\n");
				else
					BT_DBG("Hfp roam FALSE\n");

				SetBtHfpRoam(param->params.hfpRoam);
			}
			break;

		case BT_STACK_EVENT_HFP_VOICE_RECOGNITION:
			{
				if(param->params.hfpVoiceRec)
					BT_DBG("Hfp vocie recognition TRUE\n");
				else
					BT_DBG("Hfp vocie recognition FALSE\n");

				SetBtHfpVoiceRecognition(param->params.hfpVoiceRec);
			}
			break;

		case BT_STACK_EVENT_HFP_SPEAKER_VOLUME:
			{
				BT_DBG("Hfp speaker vol : [%d]\n", param->params.hfpVolGain);

				SetBtHfpSpeakerVolume(param->params.hfpVolGain);
			}
			break;

		case BT_STACK_EVENT_HFP_IN_BAND_RING:
			{
				if(param->params.hfpInBandRing)
					BT_DBG("Hfp inBandRing TRUE\n");
				else
					BT_DBG("Hfp inBandRing FALSE\n");
			}
			break;
		case BT_STACK_EVENT_HFP_MANUFACTORY_INFO:
			{
				BT_DBG("%s\n", param->params.hfpRemoteManufactory);
			}
			break;

		case BT_STACK_EVENT_HFP_DEVICE_TYPE:
			{
				BT_DBG("%s\n", param->params.hfpRemoteDeviceType);
			}
			break;

		case BT_STACK_EVENT_HFP_UNSOLICITED_DATA:
			{
				/*uint16_t i;
				
				BT_DBG("Hfp unsolicited data (char): ");
				for(i = 0; i < param->paramsLen; i++)
				{
					BT_DBG("%c", param->params.hfpUnsolicitedData[i]);
				}
				BT_DBG("\n");
				*/
			}
			break;

		case BT_STACK_EVENT_HFP_CODEC_TYPE:
			{
				//type: 1=CVSD, 2=MSBC
				if(param->params.scoCodecType == HFP_AUDIO_DATA_mSBC)
				{
					DBG("+++mSBC+++\n");
					btManager.hfpScoCodecType = HFP_AUDIO_DATA_mSBC;
				}
				else //if(param->params.scoCodecType == HFP_AUDIO_DATA_PCM)
				{
					DBG("+++CVSD+++\n");
					btManager.hfpScoCodecType = HFP_AUDIO_DATA_PCM;
				}
			}
			break;
			

		default:
			break;
	}
}

void BtHfpConnect(uint8_t * addr)
{
	SetHfpState(BT_HFP_STATE_CONNECTING);
	HfpConnect(addr);
}

void BtHfpDisconnect(void)
{
	HfpDisconnect();
}





static void SetHfpState(BT_HFP_STATE state)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return;

	tempBtManager->hfpState = state;
}

static void SetHfpConnectedAddr(uint8_t * addr)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return;

	if(addr == NULL) // clear bt address
		memset(tempBtManager->btDevAddr, 0, sizeof(tempBtManager->btDevAddr));
	else
		memcpy(tempBtManager->btDevAddr, addr, 6);
}

BT_HFP_STATE GetHfpState(void)
{
	BT_MANAGER_ST *	tempBtManager = NULL;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_HFP_STATE_NONE;

	return GetBtManager()->hfpState;
}

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


static int16_t SetBtHfpVoiceRecognition(bool flag)
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


static int16_t SetBtHfpSpeakerVolume(uint8_t gain)
{
	BT_MANAGER_ST *	tempBtManager = NULL;
	uint32_t hfpVolume = 0;

	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return BT_MANAGER_ERROR_NOT_INITIALIZED;

	tempBtManager->volGain = gain;

	hfpVolume = (tempBtManager->volGain*32)/15;

	AudioMusicVolSet((uint8_t)hfpVolume);

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
