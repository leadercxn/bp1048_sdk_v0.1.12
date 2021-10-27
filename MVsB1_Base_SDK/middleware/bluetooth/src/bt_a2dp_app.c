/**
 *******************************************************************************
 * @file    bt_a2dp_app.h
 * @author  Halley
 * @version V1.0.1
 * @date    27-Apr-2016
 * @brief   A2dp callback events and actions
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

#include "bt_manager.h"
#include "bt_app_interface.h"
#include "bt_config.h"
#include "mcu_circular_buf.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#include "main_task.h"
#endif
#include <nds32_intrinsic.h>

#ifdef 	CFG_XIAOAI_AI_EN
	#include "xm_xiaoai_api.h"
	#include "xm_auth.h"
#endif

#if (BT_A2DP_SUPPORT == ENABLE)
static void SetA2dpState(BT_A2DP_STATE state);

#ifndef BT_TWS_SUPPORT
uint32_t a2dp_avrcp_connect_flag = 0;//A2DP连接成功后，AVRCP状态更新判断标志//主要区分A2DP未播放,AVRCP反馈播放状态

extern uint32_t gSpecificDevice;
#endif

#ifdef BT_HFP_MODE_DISABLE
extern uint32_t gHfpCallNoneWaiting;
#endif

#ifdef BT_TWS_SUPPORT
#include "bt_tws_app_func.h"
uint32_t a2dp_avrcp_connect_flag;

void a2dp_sbc_decoer_init(void);
void a2dp_sbc_save(uint8_t *p,uint32_t len);
static void SetA2dpState(BT_A2DP_STATE state);
#endif

void BtA2dpCallback(BT_A2DP_CALLBACK_EVENT event, BT_A2DP_CALLBACK_PARAMS * param)
{
	switch(event)
	{
		case BT_STACK_EVENT_A2DP_CONNECTED:
		{
#ifdef BT_TWS_SUPPORT
			a2dp_sbc_decoer_init();
			if((btManager.twsState == BT_TWS_STATE_CONNECTED)&&(btManager.twsRole == BT_TWS_SLAVE))
			{
				//从机已经组网成功,此时被手机连接上,则断开手机
				BTHciDisconnectCmd(param->params.bd_addr);
				break;
			}

#endif
			APP_DBG("A2dp Connected : bt address = %02x:%02x:%02x:%02x:%02x:%02x\n",
					(param->params.bd_addr)[0],
					(param->params.bd_addr)[1],
					(param->params.bd_addr)[2],
					(param->params.bd_addr)[3],
					(param->params.bd_addr)[4],
					(param->params.bd_addr)[5]);

			SetA2dpState(BT_A2DP_STATE_CONNECTED);

#ifdef 	CFG_XIAOAI_AI_EN
			{
				if(return_ble_conn_state() && (get_communicate_way() == CURRENT_COMMUNICATE_BLE))
				{
					aivs_conn_edr_status();
				}
			}
#endif
			
			if((param->params.bd_addr)[0] || (param->params.bd_addr)[1] || (param->params.bd_addr)[2] 
				|| (param->params.bd_addr)[3] || (param->params.bd_addr)[4] || (param->params.bd_addr)[5])
			{
				memcpy(GetBtManager()->remoteAddr, param->params.bd_addr, 6);
			}

			SetBtConnectedProfile(BT_CONNECTED_A2DP_FLAG);

			if(!btManager.btReconnectTimer.timerFlag)
			{
				//Remote Device主动连接BP10,A2DP连接成功,AVRCP未连接上,主动发起1次avrcp连接
				if(GetAvrcpState() < BT_AVRCP_STATE_CONNECTED)
				{
					btEventListB2Count = 1500;//50;//延时50ms
					btCheckEventList |= BT_EVENT_AVRCP_CONNECT;
				}
			}
#ifdef BT_RECONNECTION_FUNC
			BtStartReconnectProfile();
#endif
			BtLinkStateConnect();
			a2dp_avrcp_connect_flag = 1;
		}

		break;

		case BT_STACK_EVENT_A2DP_DISCONNECTED:
		{
			APP_DBG("A2dp disconnect\n");
			SetA2dpState(BT_A2DP_STATE_NONE);
			SetBtDisconnectProfile(BT_CONNECTED_A2DP_FLAG);
			//重新更新蓝牙decoder相关参数
			if(RefreshSbcDecoder)
				RefreshSbcDecoder();
			
			BtLinkStateDisconnect();
			
			//A2DP断开后，开启检测AVRCP断开机制(5S超时)
			if(IsAvrcpConnected())
			{
				btEventListB0Count = btEventListCount;
				btEventListB0Count += 5000;//延时5s
				btCheckEventList |= BT_EVENT_AVRCP_DISCONNECT;
			}
		}
		break;

		case BT_STACK_EVENT_A2DP_CONNECT_TIMEOUT:
		{
			APP_DBG("A2dp connect timeout\n");
			if(GetA2dpState()>BT_A2DP_STATE_NONE)
			{
				SetA2dpState(BT_A2DP_STATE_NONE);
				SetBtDisconnectProfile(BT_CONNECTED_A2DP_FLAG);

				//重新更新蓝牙decoder相关参数
				if(RefreshSbcDecoder)
					RefreshSbcDecoder();
				
				BtLinkStateConnect();

				//A2DP断开后，开启检测AVRCP断开机制(5S超时)
				if(IsAvrcpConnected())
				{
					btEventListB0Count = btEventListCount;
					btEventListB0Count += 5000;//延时5s
					btCheckEventList |= BT_EVENT_AVRCP_DISCONNECT;
				}
			}
		}
		break;

		case BT_STACK_EVENT_A2DP_STREAM_START:
		{
			SetA2dpState(BT_A2DP_STATE_STREAMING);
			
			APP_DBG("A2dp streaming...\n");

#ifdef BT_TWS_SUPPORT
			tws_init();
#endif
			if(GetAvrcpState() != BT_AVRCP_STATE_CONNECTED)
				BtMidMessageSend(MSG_BT_MID_PLAY_STATE_CHANGE, 1);
			
#ifndef BT_TWS_SUPPORT
			a2dp_avrcp_connect_flag = 0;
#if (BT_HFP_SUPPORT == ENABLE)
			if(gSpecificDevice)
			{
				extern void SpecialDeviceFunc(void);
				SpecialDeviceFunc();
			}
#endif
#else
			a2dp_sbc_decoer_init();
#endif
		}
		break;

		case BT_STACK_EVENT_A2DP_STREAM_SUSPEND:
		{
			APP_DBG("A2dp suspend\n");
#ifdef BT_TWS_SUPPORT
			if(IsBtAudioMode())
			{
				AudioCoreSourceDisable(1);
			}
#endif
			SetA2dpState(BT_A2DP_STATE_CONNECTED);

			BtMidMessageSend(MSG_BT_MID_PLAY_STATE_CHANGE, 2);
		}
		break;

		case BT_STACK_EVENT_A2DP_STREAM_DATA_IND:
		{
#ifdef BT_HFP_MODE_DISABLE
			if(gHfpCallNoneWaiting)
				break;
#endif
#ifdef MVA_BT_OBEX_UPDATE_FUNC_SUPPORT
			extern uint32_t volatile obex_start;
			if(obex_start)
			{
				break;
			}
#endif

#ifndef BT_TWS_SUPPORT	
			//SBC or AAC
			if(SaveA2dpStreamDataToBuffer)
			{
				//SaveA2dpStreamDataToBuffer(param->params.a2dpStreamParams.a2dpStreamData, param->paramsLen);
				SaveA2dpStreamDataToBuffer(param->params.a2dpStreamParams.a2dpStreamData, param->params.a2dpStreamParams.a2dpStreamDataLen);
			}
#else
			a2dp_sbc_save(param->params.a2dpStreamParams.a2dpStreamData,param->params.a2dpStreamParams.a2dpStreamDataLen);

#endif
		}
		break;

		case BT_STACK_EVENT_A2DP_STREAM_DATA_TYPE:
		{
			APP_DBG("A2dp stream type: ");
			if(param->params.a2dpStreamDataType)
			{
				APP_DBG("AAC\n");
				GetBtManager()->a2dpStreamType = BT_A2DP_STREAM_TYPE_AAC;
			}
			else
			{
				APP_DBG("SBC\n");
				GetBtManager()->a2dpStreamType = BT_A2DP_STREAM_TYPE_SBC;
			}
			if(RefreshSbcDecoder)
				RefreshSbcDecoder();
			
		}
		break;

		default:
		break;
	}
}

static void SetA2dpState(BT_A2DP_STATE state)
{
	GetBtManager()->a2dpState = state;
}
#endif

BT_A2DP_STATE GetA2dpState(void)
{
	return GetBtManager()->a2dpState;
}

void BtA2dpConnect(uint8_t *addr)
{
	if(GetA2dpState() == BT_A2DP_STATE_NONE)
	{
		A2dpConnect(addr);
	}
}

