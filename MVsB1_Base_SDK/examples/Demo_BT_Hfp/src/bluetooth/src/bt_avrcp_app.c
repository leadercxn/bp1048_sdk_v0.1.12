/**
 *******************************************************************************
 * @file    bt_avrcp_app.c
 * @author  Halley
 * @version V1.0.1
 * @date    27-Apr-2016
 * @brief   Avrcp callback events and actions
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
#include "timeout.h"

#include "bt_manager.h"
#ifdef	FUNC_OS_EN
#include "rtos_api.h"
#endif
#include "bt_app_interface.h"
//#include "bt_play_api.h"

TIMER	AvrcpConnectTimer;

static void SetAvrcpState(BT_AVRCP_STATE state);

void BtAvrcpCallback(BT_AVRCP_CALLBACK_EVENT event, BT_AVRCP_CALLBACK_PARAMS * param)
{
	switch(event)
	{

		case BT_STACK_EVENT_AVRCP_CONNECTED:
			{
				BT_DBG("Avrcp Connected : bt address = %02x:%02x:%02x:%02x:%02x:%02x\n",
						(param->params.bd_addr)[0],
						(param->params.bd_addr)[1],
						(param->params.bd_addr)[2],
						(param->params.bd_addr)[3],
						(param->params.bd_addr)[4],
						(param->params.bd_addr)[5]);

				SetAvrcpState(BT_AVRCP_STATE_CONNECTED);
				
				memcpy(GetBtManager()->remoteAddr, param->params.bd_addr, 6);

				SetBtConnectedProfile(BT_CONNECTED_AVRCP_FLAG);

#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
				GetBtManager()->avrcpSyncEnable = 0;
				GetBtManager()->avrcpSyncVol = 0xff; //初始值
#endif

#ifdef BT_RECONNECTION_FUNC
				if(GetBtManager()->btReconnectTimer.timerFlag)
					CheckBtReconnectTimer();
#endif
			}
			break;

		case BT_STACK_EVENT_AVRCP_DISCONNECTED:
			{
				BT_DBG("Avrcp disconnect\n");
				SetAvrcpState(BT_AVRCP_STATE_NONE);
				
				SetBtDisconnectProfile(BT_CONNECTED_AVRCP_FLAG);

				BtLinkDisconnect();
			}
			break;

		case BT_STACK_EVENT_AVRCP_ADV_PLAY_STATUS_CHANGED:
			{
				BT_DBG("A2DP Play State [%d]\n", param->params.avrcpAdv.avrcpAdvMediaStatus);
#if zsq
				BtMidMessageSend(MSG_BT_MID_PLAY_STATE_CHANGE, param->params.avrcpAdv.avrcpAdvMediaStatus);
				
				if(CheckTimerStart_BtPlayStatus())
				{
				}
				else
				{
					if(param->params.avrcpAdv.avrcpAdvMediaStatus == AVRCP_ADV_MEDIA_PLAYING)
					{
						BTCtrlGetPlayStatus();
						TimerStart_BtPlayStatus();
					}
				}
#endif
			}
			break;

#if (BT_AVRCP_ADVANCED == ENABLE)
#if (BT_AVRCP_SONG_PLAY_STATE == ENABLE)
		case BT_STACK_EVENT_AVRCP_ADV_PLAY_STATUS:
			{
				uint8_t curPlayStatus = param->params.avrcpAdv.avrcpAdvPlayStatus.CurPlayStatus;
				uint32_t curPlayTimes = param->params.avrcpAdv.avrcpAdvPlayStatus.CurPosInMs;
				uint32_t tatolPlayTimes = param->params.avrcpAdv.avrcpAdvPlayStatus.TotalLengthInMs;
				
				switch(curPlayStatus)
				{
					case AVRCP_ADV_MEDIA_STOPPED:
						APP_DBG("Stopped \n");
						break;
						
					case AVRCP_ADV_MEDIA_PLAYING:
						APP_DBG("Playing > %02d:%02d / %02d:%02d \n",
								((int)curPlayTimes/1000)/60,
								((int)curPlayTimes/1000)%60,
								((int)tatolPlayTimes/1000)/60,
								((int)tatolPlayTimes/1000)%60);
						break;

					case AVRCP_ADV_MEDIA_PAUSED:
						APP_DBG("Paused || %02d:%02d / %02d:%02d  \n",
                        		((int)curPlayTimes/1000)/60,
                        		((int)curPlayTimes/1000)%60,
                        		((int)tatolPlayTimes/1000)/60,
                        		((int)tatolPlayTimes/1000)%60);
						break;

					case AVRCP_ADV_MEDIA_FWD_SEEK:
						APP_DBG("FF >> %02d:%02d / %02d:%02d  \n",
                        		((int)curPlayTimes/1000)/60,
                        		((int)curPlayTimes/1000)%60,
                        		((int)tatolPlayTimes/1000)/60,
                        		((int)tatolPlayTimes/1000)%60);
						break;

					case AVRCP_ADV_MEDIA_REV_SEEK:
						APP_DBG("FB << %02d:%02d / %02d:%02d  \n",
                        		((int)curPlayTimes/1000)/60,
                        		((int)curPlayTimes/1000)%60,
                        		((int)tatolPlayTimes/1000)/60,
                        		((int)tatolPlayTimes/1000)%60);
						break;

					default:
						break;
				}
			}
			break;
#endif

		case BT_STACK_EVENT_AVRCP_ADV_TRACK_INFO:
			//BT_DBG("TRACK_INFO\n");
			/*{
				APP_DBG("Playing > %02d:%02d / %02d:%02d \n",
            		(param->params.avrcpAdv.avrcpAdvTrack.ms/1000)/60,
            		(param->params.avrcpAdv.avrcpAdvTrack.ms/1000)%60,
            		(param->params.avrcpAdv.avrcpAdvTrack.ls/1000)/60,
            		(param->params.avrcpAdv.avrcpAdvTrack.ls/1000)%60);
			}*/
			break;

		case BT_STACK_EVENT_AVRCP_ADV_ADDRESSED_PLAYERS:
			//BT_DBG("AVRCP Event: BT_STACK_EVENT_AVRCP_ADV_ADDRESSED_PLAYERS\n");
			//BT_DBG("PlayId[%x],UidCount[%x]\n", param->params.avrcpAdv.avrcpAdvAddressedPlayer.playerId, param->params.avrcpAdv.avrcpAdvAddressedPlayer.uidCounter);
			break;

		case BT_STACK_EVENT_AVRCP_ADV_CAPABILITY_COMPANY_ID:
			/*BT_DBG("COMPANY_ID: ");
			{
				uint8_t len;
				for(len=0; len< param->paramsLen; len++)
				{
					BT_DBG("[0x%02x] ",param->params.avrcpAdv.avrcpAdvCompanyId[len]);
				}
			}
			BT_DBG("\n");*/
			break;
			
		case BT_STACK_EVENT_AVRCP_ADV_CAPABILITY_EVENTS_SUPPORTED:
			//BT_DBG("AVRCP SUPPORTED FEATRUE = [0x%04x]\n", param->params.avrcpAdv.avrcpAdvEventMask);
			break;

#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
		case BT_STACK_EVENT_AVRCP_ADV_VOLUME_CHANGE:
			BT_DBG("BTVOL_CHANGE = [%d]\n", param->params.avrcpAdv.avrcpAdvVolumePercent);
			GetBtManager()->avrcpSyncEnable = 1;
			{
				uint16_t VolumePercent = param->params.avrcpAdv.avrcpAdvVolumePercent;
				VolumePercent = VolumePercent*32/100;

				GetBtManager()->avrcpSyncVol = VolumePercent;
				
				BtMidMessageSend(MSG_BT_MID_VOLUME_CHANGE, (uint8_t)VolumePercent);
			}
			break;
#endif

#if (BT_AVRCP_SONG_TRACK_INFOR == ENABLE)
		case BT_STACK_EVENT_AVRCP_ADV_MEDIA_INFO:
			BT_DBG("AVRCP Event: BT_STACK_EVENT_AVRCP_ADV_MEDIA_INFO\n");

			if(GetMediaInfo)
				GetMediaInfo((void*)param->params.avrcpAdv.avrcpAdvMediaInfo);
			
			break;
#endif
#endif

		default:
			break;
	}
}

static void SetAvrcpState(BT_AVRCP_STATE state)
{
	GetBtManager()->avrcpState = state;
}

BT_AVRCP_STATE GetAvrcpState(void)
{
	return GetBtManager()->avrcpState;
}


bool IsAvrcpConnected(void)
{
	return (GetAvrcpState() == BT_AVRCP_STATE_CONNECTED);
}

void BtAvrcpConnect(uint8_t *addr)
{
	if(!IsAvrcpConnected())
	{
		AvrcpConnect(addr);
	}
}


//针对部分手机不能主动连接AVRCP协议,在A2DP连接成功2S后主动发起AVRCP连接
void BtAvrcpConnectStart(void)
{
	if((!GetBtManager()->avrcpConnectStart)&&(!IsAvrcpConnected()))
	{
		GetBtManager()->avrcpConnectStart = 1;
		TimeOutSet(&AvrcpConnectTimer, 2000);
	}
}


void CheckBtAvrcpState(void)
{
	if(GetBtManager()->avrcpConnectStart)
	{
		if(IsTimeOut(&AvrcpConnectTimer))
		{
			GetBtManager()->avrcpConnectStart=0;

			if(!IsAvrcpConnected())
			{
				memcpy(GetBtManager()->remoteAvrcpAddr, GetBtManager()->remoteA2dpAddr, 6);
				BtAvrcpConnect(GetBtManager()->remoteAvrcpAddr);
			}
		}
	}
}


//AVRCP handle
void BtAvrcpConnectedHandle(uint8_t *addr)
{
	SetAvrcpState(BT_AVRCP_STATE_CONNECTED);
}


void BtAvrcpDisconnectHandle(void)
{
	SetAvrcpState(BT_AVRCP_STATE_NONE);
}

//timer: get bt play status
void TimerStart_BtPlayStatus(void)
{
	TimeOutSet(&GetBtManager()->avrcpPlayStatusTimer.timerHandle, 900);
	GetBtManager()->avrcpPlayStatusTimer.timerFlag = 1;
}

void TimerStop_BtPlayStatus(void)
{
	GetBtManager()->avrcpPlayStatusTimer.timerFlag = 0;
}

uint8_t CheckTimerStart_BtPlayStatus(void)
{
	return GetBtManager()->avrcpPlayStatusTimer.timerFlag;
}
