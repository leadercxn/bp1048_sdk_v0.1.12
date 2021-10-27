/**
 *******************************************************************************
 * @file    bt_spp_app.h
 * @author  Halley
 * @version V1.0.1
 * @date    27-Apr-2016
 * @brief   Spp callback events and actions
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
#include "bt_spp_api.h"
#include "bt_config.h"
//#include "app_config.h"


#if(BT_SPP_SUPPORT == ENABLE)


#ifdef CFG_FUNC_AI
#include "ai.h"
#endif

static void SetSppState(BT_SPP_STATE state);

void BtSppCallback(BT_SPP_CALLBACK_EVENT event, BT_SPP_CALLBACK_PARAMS * param)
{
	switch(event)
	{
		case BT_STACK_EVENT_SPP_CONNECTED:
			DBG("SPP EVENT:connected\n");
			SetSppState(BT_SPP_STATE_CONNECTED);
		break;
		
		case BT_STACK_EVENT_SPP_DISCONNECTED:
			DBG("SPP EVENT:disconnect\n");
			SetSppState(BT_SPP_STATE_NONE);
		break;
		
		case BT_STACK_EVENT_SPP_DATA_RECEIVED:
			DBG("SPP EVENT:receive len:%d\n", param->paramsLen);
			{
				uint16_t i;
				for(i=0;i<param->paramsLen;i++)
				{
					DBG("0x%02x ", param->params.sppReceivedData[i]);
					if((i!=0)&&(i%16 == 0))
						DBG("\n");
				}
				#ifdef CFG_FUNC_AI
				spp_rcv_data_proess(param->params.sppReceivedData,param->paramsLen);
				#endif
			}
		break;
		
		case BT_STACK_EVENT_SPP_DATA_SENT:
		break;
		
		default:
		break;
	}
}

void BtSppHookFunc(void)
{
	//在此函数中可以处理spp需要发送的数据
	if(GetBtManager()->sppState == BT_SPP_STATE_CONNECTED)
	{
		if(SppIsReady())
		{
			//客户增加
		}
	}
}

static void SetSppState(BT_SPP_STATE state)
{
	GetBtManager()->sppState = state;
}

BT_SPP_STATE GetSppState(void)
{
	return GetBtManager()->sppState;
}

#endif

