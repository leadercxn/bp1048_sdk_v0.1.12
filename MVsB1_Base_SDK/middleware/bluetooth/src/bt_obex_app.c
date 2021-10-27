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
#include "bt_config.h"
#include "bt_manager.h"
#include "bt_app_interface.h"
#include "bt_obex_api.h"
#include "app_message.h"

#if(BT_OBEX_SUPPORT == ENABLE)

#ifdef MVA_BT_OBEX_UPDATE_FUNC_SUPPORT
#include "bt_obex_upgrade.h"
#endif

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static void SetObexState(BT_OBEX_STATE state)
{
	GetBtManager()->obexState = state;
}

BT_OBEX_STATE GetObexState(void)
{
	return GetBtManager()->obexState;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//callback
void BtObexCallback(BT_OBEX_CALLBACK_EVENT event, BT_OBEX_CALLBACK_PARAMS * param)
{
	uint16_t i;
	switch(event)
	{
		case BT_STACK_EVENT_OBEX_CONNECTED:
			APP_DBG("OBEX EVENT:connected\n");
			SetObexState(BT_OBEX_STATE_CONNECTED);
			break;
		
		case BT_STACK_EVENT_OBEX_DISCONNECTED:
			APP_DBG("OBEX EVENT:disconnect\n");
			SetObexState(BT_OBEX_STATE_NONE);
		
#ifdef MVA_BT_OBEX_UPDATE_FUNC_SUPPORT
			ObexUpgradeEnd();
#endif
			break;
		
		case BT_STACK_EVENT_OBEX_DATA_RECEIVED:
			/*APP_DBG("====\n OBEX EVENT:data ind \n");
			APP_DBG("type:%d\n", param->type);
			APP_DBG("total len:%d\n", param->total);
			APP_DBG("seg off:%d\n", param->segoff);
			APP_DBG("seg len:%d\n", param->seglen);

			APP_DBG("seg data:");
			for(i=0;i<param->seglen;i++)
			{
				APP_DBG("%02x ", param->segdata[i]);
			}
			APP_DBG("\n ====\n");*/
#ifdef MVA_BT_OBEX_UPDATE_FUNC_SUPPORT
			ObexUpdateProc(param);
#endif
			break;
		
		default:
			break;
	}
}

#endif

