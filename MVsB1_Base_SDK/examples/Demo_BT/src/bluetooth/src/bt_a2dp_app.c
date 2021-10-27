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

#include "bt_app_interface.h"
#include "bt_manager.h"




static void SetA2dpState(BT_A2DP_STATE state);

void BtA2dpCallback(BT_A2DP_CALLBACK_EVENT event, BT_A2DP_CALLBACK_PARAMS * param)
{
	switch(event)
	{
		case BT_STACK_EVENT_A2DP_CONNECTED:
		{
			BT_DBG("A2dp Connected : bt address = %02x:%02x:%02x:%02x:%02x:%02x\n",
					(param->params.bd_addr)[0],
					(param->params.bd_addr)[1],
					(param->params.bd_addr)[2],
					(param->params.bd_addr)[3],
					(param->params.bd_addr)[4],
					(param->params.bd_addr)[5]);

			SetA2dpState(BT_A2DP_STATE_CONNECTED);

			memcpy(GetBtManager()->remoteAddr, param->params.bd_addr, 6);

			SetBtConnectedProfile(BT_CONNECTED_A2DP_FLAG);
#ifdef BT_RECONNECTION_FUNC
			BtStartReconnectProfile();
#endif
			BtLinkConnect();
		}

		break;

		case BT_STACK_EVENT_A2DP_DISCONNECTED:
		{
			BT_DBG("A2dp disconnect\n");
			SetA2dpState(BT_A2DP_STATE_NONE);
			
			SetBtDisconnectProfile(BT_CONNECTED_A2DP_FLAG);

			//重新更新蓝牙decoder相关参数
			if(RefreshSbcDecoder)
				RefreshSbcDecoder();

			BtLinkDisconnect();
		}
		break;

		case BT_STACK_EVENT_A2DP_CONNECT_TIMEOUT:
		{
			BT_DBG("A2dp connect timeout\n");
		}
		break;

		case BT_STACK_EVENT_A2DP_STREAM_START:
		{
			BT_DBG("A2dp streaming...\n");
			SetA2dpState(BT_A2DP_STATE_STREAMING);
			
			//BtMidMessageSend(MSG_BT_MID_PLAY_STATE_CHANGE, 1);
		}
		break;

		case BT_STACK_EVENT_A2DP_STREAM_SUSPEND:
		{
			BT_DBG("A2dp suspend\n");
			SetA2dpState(BT_A2DP_STATE_CONNECTED);

			//BtMidMessageSend(MSG_BT_MID_PLAY_STATE_CHANGE, 2);
		}
		break;

		case BT_STACK_EVENT_A2DP_STREAM_DATA_IND:
		{
			//data size: 591bytes
//			BT_DBG("====data_ind: len=[%d], [0x%x],[0x%x]\n", param->paramsLen,
//				param->params.a2dpStreamData[0], param->params.a2dpStreamData[1]);
//			SaveDataToSbcBuffer(param->params.a2dpStreamData, param->paramsLen);
			if(SaveA2dpStreamDataToBuffer)
			{
				SaveA2dpStreamDataToBuffer(param->params.a2dpStreamData, param->paramsLen);
			}
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

