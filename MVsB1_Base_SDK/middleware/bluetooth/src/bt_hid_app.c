/**
 *******************************************************************************
 * @file    bt_hid_app.h
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
#include "bt_config.h"

#include "bt_hid_api.h"

#if (BT_HID_SUPPORT == ENABLE)
#include "rtos_api.h"

void BtHidCallback(BT_HID_CALLBACK_EVENT event, BT_HID_CALLBACK_PARAMS * param)
{
	switch(event)
	{
		case BT_STACK_EVENT_HID_CONNECTED:
			APP_DBG("BT_STACK_EVENT_HID_CONNECTED\n");
			break;
			
		case BT_STACK_EVENT_HID_DISCONNECTED:
			APP_DBG("BT_STACK_EVENT_HID_DISCONNECTED\n");
			break;
			
		case BT_STACK_EVENT_HID_DATA_RECEIVED:
			APP_DBG("BT_STACK_EVENT_HID_DATA_RECEIVED\n");
			{
				uint32_t i;
				APP_DBG("data len: %d:", param->params.hidData.dataLen);
				for(i=0;i<param->params.hidData.dataLen;i++)
				{
					APP_DBG("0x%x ", param->params.hidData.data[i]);
				}
				APP_DBG("\n");
			}
			break;
			
		case BT_STACK_EVENT_HID_DATA_SENT:
			break;
			
	}
}

////////////////////////////////////////////////////////////
//HID Command
#define HID_CMD_RELEASE 0x00
#define HID_VOL_UP      BIT(0) 
#define HID_VOL_DN      BIT(1)
#define HID_NEXT        BIT(2) 
#define HID_PREV        BIT(3) 
#define HID_MUTE        BIT(4)
#define HID_STOP        BIT(7) 

#define HID_Camera      BIT(1) //用于自拍

uint8_t hidUserCmd[2];
void BtHidUserKey(uint8_t cmd)
{
	HidData sHidData;
	
	sHidData.channel = HidIntrChannel;
	hidUserCmd[0] = 0xa1;
	hidUserCmd[1] = cmd;
	sHidData.data = &hidUserCmd[0];
	sHidData.dataLen = 2;

	HidSendData(&sHidData);
}

void HidCmd_VolUp(void)
{
	BtHidUserKey(HID_VOL_UP);
	osTaskDelay(500);
	BtHidUserKey(HID_CMD_RELEASE);
}

void HidCmd_VolDn(void)
{
	BtHidUserKey(HID_VOL_DN);
	osTaskDelay(500);
	BtHidUserKey(HID_CMD_RELEASE);
}

void HidCmd_Next(void)
{
	BtHidUserKey(HID_NEXT);
	osTaskDelay(500);
	BtHidUserKey(HID_CMD_RELEASE);
}

void HidCmd_Prev(void)
{
	BtHidUserKey(HID_PREV);
	osTaskDelay(500);
	BtHidUserKey(HID_CMD_RELEASE);
}

void HidCmd_Mute(void)
{
	BtHidUserKey(HID_MUTE);
	osTaskDelay(500);
	BtHidUserKey(HID_CMD_RELEASE);
}

void HidCmd_Stop(void)
{
	BtHidUserKey(HID_STOP);
	osTaskDelay(500);
	BtHidUserKey(HID_CMD_RELEASE);
}

//自拍
void HidCmd_Camera(void)
{
	BtHidUserKey(HID_Camera);
	osTaskDelay(500);
	BtHidUserKey(HID_CMD_RELEASE);
}

#endif

