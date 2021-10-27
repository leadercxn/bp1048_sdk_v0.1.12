/**
 *******************************************************************************
 * @file    bt_mfi_app.c
 * @author  Halley
 * @version V1.0.1
 * @date    27-Apr-2016
 * @brief   MFi callback events and actions
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

#include "bt_mfi_api.h"
#include "bt_manager.h"

#if (BT_MFI_SUPPORT == ENABLE)

void BtMfiCallback(BT_MFI_CALLBACK_EVENT event, BT_MFI_CALLBACK_PARAMS * param)
{
	switch(event)
	{
		case BT_STACK_EVENT_MFI_CONNECTED:
			{
				APP_DBG("MFi Connected : bt address = %02x:%02x:%02x:%02x:%02x:%02x\n",
						(param->params.bd_addr)[0],
						(param->params.bd_addr)[1],
						(param->params.bd_addr)[2],
						(param->params.bd_addr)[3],
						(param->params.bd_addr)[4],
						(param->params.bd_addr)[5]);
			}
			break;

		case BT_STACK_EVENT_MFI_DISCONNECTED:
			{
				APP_DBG("MFi Disconnected\n");
			}
			break;

		case BT_STACK_EVENT_MFI_DATA_RECEIVED:
			{
				uint16_t		i;

				APP_DBG("BT_STACK_EVENT_MFI_DATA_RECEIVED:\n");
				for(i = 0; i < param->paramsLen; i++)
				{
					APP_DBG("0x%02x ", param->params.mfiReceivedData[i]);
				}
				APP_DBG("\n");
			}
			break;

		case BT_STACK_EVENT_MFI_DATA_SENT:
			{
				
			}
			break;

		default:
			break;
	}
}

static void SetMfiState(BT_MFI_STATE state)
{
	GetBtManager()->mfiState = state;
}

BT_MFI_STATE GetMfiState(void)
{
	return GetBtManager()->mfiState;
}

#endif

