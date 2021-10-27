/**
 **************************************************************************************
 * @file    IO_key.c
 * @brief   
 *
 * @author  ken bu/bkd
 * @version V0.0.1
 *
 * $Created: 2018-09-03 
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "type.h"
#include "app_config.h"
//driver
//#include "adc.h"
#include "clk.h"
#include "gpio.h"
#include "timeout.h"
#include "io_key.h"
#include "debug.h"
#include "hw_interface.h"

#ifndef CFG_APP_CONFIG 
/*********** default config, determined by the hardware circuit **************/
//#define 	CFG_RES_IO_KEY_SCAN
#define		CFG_PARA_IO_KEY_COUNT				11 		//key count per IO channel 
//#define		CFG_RES_IO_KEY_PORT_CH1			IO_CHANNEL_GPIOA23
#define		CFG_RES_IO_KEY_CH1_ANA_EN			GPIO_A_ANA_EN
#define		CFG_RES_IO_KEY_CH1_ANA_MASK		GPIO_INDEX23

/****************************************************************************/
#endif //no CFG_APP_CONFIG


#define 	IO_KEY_SCAN_TIME			20
#define 	IO_KEY_JITTER_TIME			30
#define 	IO_KEY_CP_TIME				1000 	//for hold or long press
#define 	IO_KEY_CPH_TIME			    200		//for long long press

#define		IO_KEY_FULL_VAL			    4096		//

typedef enum _IO_KEY_STATE
{
	IO_KEY_STATE_IDLE,
	IO_KEY_STATE_JITTER,
	IO_KEY_STATE_PRESS_DOWN,
	IO_KEY_STATE_CP

} IO_KEY_STATE;

#ifdef CFG_RES_IO_KEY_SCAN
TIMER			IOKeyWaitTimer;
TIMER			IOKeyScanTimer;
IO_KEY_STATE	IOKeyState;

const uint32_t GPIOKey_Init_Tab[][5] =
{
	#ifdef CFG_SOFT_POWER_KEY_EN
	{POWER_KEY_OE,POWER_KEY_IE,POWER_KEY_PU,POWER_KEY_PD,POWER_KEY_PIN},
	#endif
	#ifdef CFG_GPIO_KEY1_EN
	{GPIO_KEY1_OE,GPIO_KEY1_IE,GPIO_KEY1_PU,GPIO_KEY1_PD,GPIO_KEY1},
	#endif
	#ifdef CFG_GPIO_KEY2_EN
	{GPIO_KEY2_OE,GPIO_KEY2_IE,GPIO_KEY2_PU,GPIO_KEY2_PD,GPIO_KEY2},
	#endif	   
};

int32_t IOKeyInit(void)
{
	volatile uint32_t i,pu,pd,ie,oe,pin;
	
	IOKeyState = IO_KEY_STATE_IDLE;

	TimeOutSet(&IOKeyScanTimer, 0);
	
	#if defined(CFG_SOFT_POWER_KEY_EN) || defined(CFG_GPIO_KEY1_EN) || defined(CFG_GPIO_KEY2_EN)
	for(i = 0; i < sizeof(GPIOKey_Init_Tab)/sizeof(GPIOKey_Init_Tab[0]); i++)
	{
		oe    = GPIOKey_Init_Tab[i][0];
		ie    = GPIOKey_Init_Tab[i][1];
		pu    = GPIOKey_Init_Tab[i][2];
		pd    = GPIOKey_Init_Tab[i][3];
		pin   = GPIOKey_Init_Tab[i][4];

		///input en
		GPIO_RegOneBitSet(ie, pin);
		GPIO_RegOneBitClear(oe, pin);
		///pull enable
		GPIO_RegOneBitSet(pu, pin);
		GPIO_RegOneBitClear(pd, pin);  
	}
	#endif

	return 0;
}


static uint8_t IOChannelKeyGet(void)
{
	uint8_t	KeyIndex = IO_CHANNEL_EMPTY;

#ifdef CFG_SOFT_POWER_KEY_EN
	if(!GPIO_RegOneBitGet(POWER_KEY_IN,POWER_KEY_PIN))
	{
		KeyIndex = 0;
		return KeyIndex;
	}
#endif
#ifdef CFG_GPIO_KEY1_EN
	if(!GPIO_RegOneBitGet(GPIO_KEY1_IN,GPIO_KEY1))
	{
		KeyIndex = 1;
	}
#endif
#ifdef CFG_GPIO_KEY2_EN	
	else if(!GPIO_RegOneBitGet(GPIO_KEY2_IN,GPIO_KEY2))
	{
		KeyIndex = 2;
	}
#endif
	return KeyIndex;
}


static uint8_t GetIOKeyIndex(void)
{
	uint8_t KeyIndex = IO_CHANNEL_EMPTY;

	KeyIndex = IOChannelKeyGet();

	return KeyIndex;
}


/**
 * @func        IOKeyScan
 * @brief       IOKeyScan 按下有消抖，弹起没有消抖处理。
 * @param       void  
 * @Output      None
 * @return      IOKeyMsg, IO_CHANNEL_EMPTY or--KeyType & KeyIndex: 0 ~ CFG_PARA_ADC_KEY_COUNT * Channels
 * @Others      
 * Record
 * 1.Date        : 20180123
 *   Author      : pi.wang
 *   Modification: Created function
*/
IOKeyMsg IOKeyScan(void)
{
	static 	uint8_t 	PreKeyIndex = IO_CHANNEL_EMPTY;
	uint8_t				KeyIndex;
	IOKeyMsg Msg = {IO_CHANNEL_EMPTY, IO_KEY_UNKOWN_TYPE};

	if(!IsTimeOut(&IOKeyScanTimer))
	{
		return Msg;
	}
	TimeOutSet(&IOKeyScanTimer, IO_KEY_SCAN_TIME);

	KeyIndex = GetIOKeyIndex();

	switch(IOKeyState)
	{
		case IO_KEY_STATE_IDLE:
			if(KeyIndex == IO_CHANNEL_EMPTY)
			{
				return Msg;
			}
			PreKeyIndex = KeyIndex;
			TimeOutSet(&IOKeyWaitTimer, IO_KEY_JITTER_TIME);
			//APP_DBG("GOTO JITTER!\n");
			IOKeyState = IO_KEY_STATE_JITTER;

		case IO_KEY_STATE_JITTER:
			if(PreKeyIndex != KeyIndex)
			{
				//APP_DBG("GOTO IDLE Because jitter!\n");
				PreKeyIndex = IO_CHANNEL_EMPTY;
				IOKeyState = IO_KEY_STATE_IDLE;
			}
			else if(IsTimeOut(&IOKeyWaitTimer))
			{
				//APP_DBG("GOTO PRESS_DOWN!\n");
				TimeOutSet(&IOKeyWaitTimer, IO_KEY_CP_TIME);
				IOKeyState = IO_KEY_STATE_PRESS_DOWN;
				Msg.index = PreKeyIndex;
				Msg.type = IO_KEY_PRESSED;
			}
			break;

		case IO_KEY_STATE_PRESS_DOWN:
			if(PreKeyIndex != KeyIndex)
			{
				//APP_DBG("IO KEY SP%bu\n", PreKeyIndex);
				IOKeyState = IO_KEY_STATE_IDLE;
				Msg.index = PreKeyIndex;
				Msg.type = IO_KEY_RELEASED;
				PreKeyIndex = IO_CHANNEL_EMPTY;
			}
			else if(IsTimeOut(&IOKeyWaitTimer))
			{
				//return key cp value
				//APP_DBG("IO KEY CPS!\n");
				TimeOutSet(&IOKeyWaitTimer, IO_KEY_CPH_TIME);
				IOKeyState = IO_KEY_STATE_CP;
				Msg.index = PreKeyIndex;
				Msg.type = IO_KEY_LONG_PRESSED;
			}
			break;

		case IO_KEY_STATE_CP:
			if(PreKeyIndex != KeyIndex)
			{
				//return key cp value
				//APP_DBG("IO KEY CPR!\n");
				IOKeyState = IO_KEY_STATE_IDLE;
				Msg.index = PreKeyIndex;
				Msg.type  = IO_KEY_LONG_RELEASED;
				PreKeyIndex = IO_CHANNEL_EMPTY;
			}
			else if(IsTimeOut(&IOKeyWaitTimer))
			{
				//return key cph value
				//APP_DBG("IO KEY CPH!\n");
				TimeOutSet(&IOKeyWaitTimer, IO_KEY_CPH_TIME);
				Msg.index = PreKeyIndex;
				Msg.type  = IO_KEY_LONG_PRESS_HOLD;
			}
			break;

		default:
			IOKeyState = IO_KEY_STATE_IDLE;
			PreKeyIndex = IO_CHANNEL_EMPTY;
			break;
	}

	return Msg;
}

#endif //CFG_RES_IO_KEY_SCAN


