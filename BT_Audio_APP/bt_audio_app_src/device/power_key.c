/**
 **************************************************************************************
 * @file    power_key.c
 * @brief   power key
 *
 * @author  Tony
 * @version V1.0.0
 *
 * $Created: 2019-10-12 14:00:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include "type.h"
#include "adc.h"
#include "app_config.h"
#include "gpio.h"
#include "timeout.h"
#include "backup.h"
#include "power_key.h"
#include "debug.h"



#if (defined(CFG_FUNC_BACKUP_EN) && defined(USE_POWERKEY_SOFT_PUSH_BUTTON))


#define 	POWER_KEY_JITTER_TIME		100			//����ʱ�䣬��ʱ������ؿ��ػ�Ӳ��ʱ���й�
#define 	POWER_KEY_CP_TIME			1000



typedef enum _POWER_KEY_STATE
{
	POWER_KEY_STATE_IDLE,
	POWER_KEY_STATE_JITTER,
	POWER_KEY_STATE_PRESS_DOWN,
	POWER_KEY_STATE_CP,

} POWER_KEY_STATE;


TIMER			PowerKeyWaitTimer;
POWER_KEY_STATE	PowerKeyState;



// Initialize POWER_KEY scan operation.
void PowerKeyScanInit(void)
{
	APP_DBG("PowerKeyScanInit*******\n");
	PowerKeyState = POWER_KEY_STATE_IDLE;
}


// POWER_KEY����ͨ�İ�����ͬ�����Ӱ�ť���أ����أ�ʱ����Ҫ���û���ϵͳ���ػ�����Ȼ��Ҳ�����ö̰����ܡ�
// �̰�����ʱ�����Ͷ̰���Ϣ�������̰����䣬�˴������κδ�����ϵͳӲ�����ػ���
PWRKeyMsg PowerKeyScan(void)
{
	PWRKeyMsg 			Msg = {0xff, PWR_KEY_UNKOWN_TYPE};

	switch(PowerKeyState)
	{
		case POWER_KEY_STATE_IDLE:
			if(BACKUP_PowerKeyPinStateGet())
			{
				Msg.type = PWR_KEY_UNKOWN_TYPE;
				return Msg;
			}
			else
			{	
				TimeOutSet(&PowerKeyWaitTimer, POWER_KEY_JITTER_TIME);
				PowerKeyState = POWER_KEY_STATE_JITTER;
			}
			break;
		case POWER_KEY_STATE_JITTER:
			if(BACKUP_PowerKeyPinStateGet())
			{
				PowerKeyState = POWER_KEY_STATE_IDLE;
			}
			else if(IsTimeOut(&PowerKeyWaitTimer))
			{
				PowerKeyState = POWER_KEY_STATE_PRESS_DOWN;
				TimeOutSet(&PowerKeyWaitTimer, POWER_KEY_CP_TIME);
				Msg.type = PWR_KEY_UNKOWN_TYPE;
				return Msg;
			}
			break;
			
		case POWER_KEY_STATE_PRESS_DOWN:
			if(BACKUP_PowerKeyPinStateGet())
			{
				PowerKeyState = POWER_KEY_STATE_IDLE;
				Msg.type = PWR_KEY_SP;
				return Msg;
			}
			else if(IsTimeOut(&PowerKeyWaitTimer))
			{
				PowerKeyState = POWER_KEY_STATE_CP;
				Msg.type = PWR_KEY_UNKOWN_TYPE;
				return Msg;
			}
			break;
			
		case POWER_KEY_STATE_CP:
			//�˴�����֤һ�ΰ���������Ӧ��ζ̰�
			if(BACKUP_PowerKeyPinStateGet())
			{
				PowerKeyState = POWER_KEY_STATE_IDLE;
				Msg.type = PWR_KEY_UNKOWN_TYPE;
				return Msg;
			}
			else
			{
				//do no thing
			}
			break;
			
		default:
			PowerKeyState = POWER_KEY_STATE_IDLE;
			break;
	}
	Msg.type = PWR_KEY_UNKOWN_TYPE;
	return Msg;
}

#endif
