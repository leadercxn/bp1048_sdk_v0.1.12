/**
 **************************************************************************************
 * @file    adc_key.c
 * @brief   
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-01-11 17:30:47$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "type.h"
#include "app_config.h"
//driver
#include "adc.h"
#include "clk.h"
#include "gpio.h"
#include "timeout.h"
#include "adc_key.h"
#include "debug.h"
#include "sadc_interface.h"

/*
 * ******************************************
 * PWER KEY标准电阻值： 
 * 0 = 16K(1.40V),  1 = 27K(1.80V), 2 = 43K(2.2V), 3= 82K(2.6V), 4 = 220K(3.0V), 
 *
 ********************************************
*/

/*
 * ******************************************
 * ADC KEY标准电阻值： 上拉 10k (3.30V)
 * 0 = 100(0.00V), 1 = 1K(0.30V),  2 = 2.2K(0.59V), 3 = 3.6K(0.87V), 4= 5.6K(1.18V), 5 = 8.2K(1.48V), 
 * 6 = 12K(1.80V), 7 = 18K(2.10V), 8 = 27K(2.40V),  9 = 47K(2.70V),  10=100K(3.00V),
 *
 ********************************************
*/

#ifndef CFG_APP_CONFIG 
/*********** default config, determined by the hardware circuit **************/
#define 	CFG_RES_ADC_KEY_USE
#define		CFG_PARA_ADC_KEY_COUNT				11 		//key count per adc channel 
#define		CFG_RES_ADC_KEY_PORT_CH1			ADC_CHANNEL_GPIOA20_A23
#define		CFG_RES_ADC_KEY_CH1_ANA_EN			GPIO_A_ANA_EN
#define		CFG_RES_ADC_KEY_CH1_ANA_MASK		GPIO_INDEX23

/****************************************************************************/
#endif //no CFG_APP_CONFIG

#define     NORMAL_ADKEY                (1)    ///1 = 标准的ADKEY值处理，0 = 用户自定义ADKEY值处理
#define     POWERKEY_11_ADKEY           (0)    ///1 = 标准的11个 ADKEY值处理，0 = 5个 ADKEY值处理+1个POWER KEY功能键


#define 	ADC_KEY_SCAN_TIME			10
#define 	ADC_KEY_JITTER_TIME			30
#define 	ADC_KEY_CP_TIME				1000 	//for hold or long press
#define 	ADC_KEY_CPH_TIME			200		//for long long press
#define	    ADC_KEY_COUNT				11
#define		ADC_KEY_FULL_VAL			4096		

typedef enum _ADC_KEY_STATE
{
	ADC_KEY_STATE_IDLE,
	ADC_KEY_STATE_JITTER,
	ADC_KEY_STATE_PRESS_DOWN,
	ADC_KEY_STATE_CP

} ADC_KEY_STATE;

#define PWRKEY_0    ADC_030V
#define PWRKEY_1    ADC_160V
#define PWRKEY_2    ADC_200V
#define PWRKEY_3    ADC_240V
#define PWRKEY_4    ADC_280V
#define PWRKEY_5    ADC_310V

const uint16_t UserPWRKey_Tab[6]=
{
	PWRKEY_0,
	PWRKEY_1,
	PWRKEY_2,
	PWRKEY_3,
	PWRKEY_4,
	PWRKEY_5,
};

#define PWR_ADCKEY_COUNT    11

#define ADKEY_0    (ADC_000V+ADC_030V)/2
#define ADKEY_1    (ADC_030V+ADC_060V)/2
#define ADKEY_2    (ADC_060V+ADC_085V)/2
#define ADKEY_3    (ADC_085V+ADC_110V)/2
#define ADKEY_4    (ADC_110V+ADC_140V)/2
#define ADKEY_5    (ADC_140V+ADC_165V)/2
#define ADKEY_6    (ADC_165V+ADC_180V)/2
#define ADKEY_7    (ADC_180V+ADC_200V)/2
#define ADKEY_8    (ADC_200V+ADC_230V)/2
#define ADKEY_9    (ADC_230V+ADC_260V)/2
#define ADKEY_10   (ADC_260V+ADC_290V)/2
#define ADKEY_11   (ADC_000V+ADC_030V)/2
#define ADKEY_12   (ADC_000V+ADC_030V)/2
#define ADKEY_13   (ADC_000V+ADC_030V)/2
#define ADKEY_14   (ADC_000V+ADC_030V)/2

const uint16_t UserADKey_Tab[11]=
{
	ADKEY_0,
	ADKEY_1,
	ADKEY_2,
	ADKEY_3,
	ADKEY_4,
	ADKEY_5,
	ADKEY_6,
	ADKEY_7,
	ADKEY_8,
	ADKEY_9,
	ADKEY_10,
};

static char *mp_adc_key_channel[] = {
	"ADC_CHANNEL_VIN",	
	"ADC_CHANNEL_VBK",		
	"ADC_CHANNEL_VDD1V2",		
	"ADC_CHANNEL_VCOM",		
	"ADC_CHANNEL_GPIOA20_A23",
	"ADC_CHANNEL_GPIOA21_A24",
	"ADC_CHANNEL_GPIOA22_A25",
	"ADC_CHANNEL_GPIOA26",	
	"ADC_CHANNEL_GPIOA27",	
	"ADC_CHANNEL_GPIOA28",	
	"ADC_CHANNEL_GPIOA29",	
	"ADC_CHANNEL_GPIOA30",	
	"ADC_CHANNEL_GPIOA31",	
	"ADC_CHANNEL_GPIOB0",		
	"ADC_CHANNEL_GPIOB1",		
	"ADC_CHANNEL_POWERKEY",	
};

#ifdef CFG_RES_ADC_KEY_USE
TIMER			AdcKeyWaitTimer;
TIMER			AdcKeyScanTimer;
ADC_KEY_STATE	AdcKeyState;							//adc按键状态
static 	uint8_t 	PreKeyIndex = ADC_CHANNEL_EMPTY;	//前一次adc按键键值

int32_t AdcKeyInit(void)
{
	AdcKeyState = ADC_KEY_STATE_IDLE;
	PreKeyIndex = ADC_CHANNEL_EMPTY;
	TimeOutSet(&AdcKeyScanTimer, 0);

#ifdef CFG_RES_ADC_KEY_PORT_CH1
	GPIO_RegOneBitSet(CFG_RES_ADC_KEY_CH1_ANA_EN, CFG_RES_ADC_KEY_CH1_ANA_MASK);
#endif 

#ifdef CFG_RES_ADC_KEY_PORT_CH2
	GPIO_RegOneBitSet(CFG_RES_ADC_KEY_CH2_ANA_EN, CFG_RES_ADC_KEY_CH2_ANA_MASK);
#endif 

	return 0;
}


static uint8_t AdcChannelKeyGet(uint8_t Channel)
{
	uint16_t	Val,i;
	uint8_t	KeyIndex = ADC_CHANNEL_EMPTY;

	Val = ADC_SingleModeDataGet(Channel);

	if( Channel == ADC_CHANNEL_POWERKEY )
	{
		//APP_DBG("P Val = %d\n", Val);
		//if(Val < 500)
		//{
		//	return ADC_CHANNEL_EMPTY;
		//}
		if(Val >= (ADC_KEY_FULL_VAL - (ADC_KEY_FULL_VAL / ADC_KEY_COUNT) / 2))
		{
			return ADC_CHANNEL_EMPTY;
		}
	}
	else
	{
		//APP_DBG("A Val = %d\n", Val);
		if(Val >= (ADC_KEY_FULL_VAL - (ADC_KEY_FULL_VAL / ADC_KEY_COUNT) / 2))
		{
			return ADC_CHANNEL_EMPTY;
		}
	}

#ifdef CFG_RES_POWERKEY_ADC_EN
	if( Channel == ADC_CHANNEL_POWERKEY )
	{
	#if POWERKEY_11_ADKEY
		KeyIndex = (Val + (ADC_KEY_FULL_VAL / ADC_KEY_COUNT) / 2) * ADC_KEY_COUNT / ADC_KEY_FULL_VAL;
	#else
		for(i = 0; i < sizeof(UserPWRKey_Tab)/sizeof(UserPWRKey_Tab[0]); i++)
		{
			if(Val < UserPWRKey_Tab[i])
			{
				KeyIndex = i;
				break;
			}
		}	
	#endif
	}
#endif

#ifdef CFG_RES_ADC_KEY_PORT_CH1
	if(Channel == CFG_RES_ADC_KEY_PORT_CH1)
	{
		#if NORMAL_ADKEY
		KeyIndex = PWR_ADCKEY_COUNT + (Val + (ADC_KEY_FULL_VAL / ADC_KEY_COUNT) / 2) * ADC_KEY_COUNT / ADC_KEY_FULL_VAL;
		#else
		for(i = 0; i < sizeof(UserADKey_Tab)/sizeof(UserADKey_Tab[0]); i++)
		{
			if(Val < UserADKey_Tab[i])
			{
				KeyIndex = PWR_ADCKEY_COUNT + i;
				break;
			}
		}		
		#endif
	}
#endif

#ifdef CFG_RES_ADC_KEY_PORT_CH2
	if(Channel == CFG_RES_ADC_KEY_PORT_CH2)
	{
		#if NORMAL_ADKEY
		KeyIndex = PWR_ADCKEY_COUNT + ADC_KEY_COUNT + (Val + (ADC_KEY_FULL_VAL / ADC_KEY_COUNT) / 2) * ADC_KEY_COUNT / ADC_KEY_FULL_VAL;
		#else
		for(i = 0; i < sizeof(UserADKey_Tab)/sizeof(UserADKey_Tab[0]); i++)
		{
			if(Val < UserADKey_Tab[i])
			{
				KeyIndex = PWR_ADCKEY_COUNT + ADC_KEY_COUNT + i;
				break;
			}
		}		
		#endif
	}
#endif

	//if( KeyIndex != ADC_CHANNEL_EMPTY )
	//{
		//APP_DBG("KeyIndex = %d\n", KeyIndex);
	//}

	trace_verboseln("adc channel %s value %d, KeyIndex = %d",mp_adc_key_channel[Channel], Val, KeyIndex);

	return KeyIndex;
}


static uint8_t GetAdcKeyIndex(void)
{
	uint8_t KeyIndex = ADC_CHANNEL_EMPTY;

#ifdef CFG_RES_POWERKEY_ADC_EN
	if( KeyIndex ==  ADC_CHANNEL_EMPTY )
	{
		KeyIndex = AdcChannelKeyGet(ADC_CHANNEL_POWERKEY);
	}
#endif

#ifdef CFG_RES_ADC_KEY_PORT_CH1
	if(KeyIndex == ADC_CHANNEL_EMPTY)
	{
		//GPIO_RegOneBitSet(CFG_RES_ADC_KEY_CH1_ANA_EN, CFG_RES_ADC_KEY_CH1_ANA_MASK);  //若是用到复用的ADC口，例如A20和A23口做ADC，需要打开此代码
		KeyIndex = AdcChannelKeyGet(CFG_RES_ADC_KEY_PORT_CH1);
		//GPIO_RegOneBitClear(CFG_RES_ADC_KEY_CH1_ANA_EN, GPIO_INDEX20);                //若是用到复用的ADC口，例如A20和A23口做ADC，需要打开此代码
	    //GPIO_RegOneBitClear(CFG_RES_ADC_KEY_CH1_ANA_EN, CFG_RES_ADC_KEY_CH1_ANA_MASK);//若是用到复用的ADC口，例如A20和A23口做ADC，需要打开此代码
	}
#endif

#ifdef CFG_RES_ADC_KEY_PORT_CH2
	if(KeyIndex == ADC_CHANNEL_EMPTY)
	{
		KeyIndex = AdcChannelKeyGet(CFG_RES_ADC_KEY_PORT_CH2);
	}
#endif

	return KeyIndex;
}


/**
 * @func        AdcKeyScan
 * @brief       AdcKeyScan 按下有消抖，弹起没有消抖处理。
 * @param       void  
 * @Output      None
 * @return      AdcKeyMsg, ADC_CHANNEL_EMPTY or--KeyType & KeyIndex: 0 ~ CFG_PARA_ADC_KEY_COUNT * Channels
 * @Others      
 * Record
 * 1.Date        : 20180123
 *   Author      : pi.wang
 *   Modification: Created function
*/
AdcKeyMsg AdcKeyScan(void)
{
	uint8_t				KeyIndex=ADC_CHANNEL_EMPTY;
	AdcKeyMsg Msg = {ADC_CHANNEL_EMPTY, ADC_KEY_UNKOWN_TYPE};

	if(!IsTimeOut(&AdcKeyScanTimer))
	{
		return Msg;
	}
	TimeOutSet(&AdcKeyScanTimer, ADC_KEY_SCAN_TIME);
	
	KeyIndex = GetAdcKeyIndex();

	switch(AdcKeyState)
	{
		case ADC_KEY_STATE_IDLE:
			if(KeyIndex == ADC_CHANNEL_EMPTY)
			{
				return Msg;
			}
			PreKeyIndex = KeyIndex;
			TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_JITTER_TIME);
			//APP_DBG("GOTO JITTER!\n");
			AdcKeyState = ADC_KEY_STATE_JITTER;		//消抖

		case ADC_KEY_STATE_JITTER:
			if(PreKeyIndex != KeyIndex)
			{
				//APP_DBG("GOTO IDLE Because jitter!\n");
				PreKeyIndex = ADC_CHANNEL_EMPTY;
				AdcKeyState = ADC_KEY_STATE_IDLE;
			}
			else if(IsTimeOut(&AdcKeyWaitTimer))
			{
				//APP_DBG("GOTO PRESS_DOWN!\n");
				TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_CP_TIME);
				AdcKeyState = ADC_KEY_STATE_PRESS_DOWN;
				Msg.index = PreKeyIndex;
				Msg.type = ADC_KEY_PRESSED;
			}
			break;

		case ADC_KEY_STATE_PRESS_DOWN:
			if(PreKeyIndex != KeyIndex)
			{
				APP_DBG("key release %d\n", PreKeyIndex);
				AdcKeyState = ADC_KEY_STATE_IDLE;
				Msg.index = PreKeyIndex;
				Msg.type = ADC_KEY_RELEASED;
				PreKeyIndex = ADC_CHANNEL_EMPTY;
			}
			else if(IsTimeOut(&AdcKeyWaitTimer))
			{
				//return key cp value
				//APP_DBG("ADC KEY CPS!\n");
				TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_CPH_TIME);
				AdcKeyState = ADC_KEY_STATE_CP;
				Msg.index = PreKeyIndex;
				Msg.type = ADC_KEY_LONG_PRESSED;
			}
			break;

		case ADC_KEY_STATE_CP:
			if(PreKeyIndex != KeyIndex)
			{
				//return key cp value
				//APP_DBG("ADC KEY CPR!\n");
				AdcKeyState = ADC_KEY_STATE_IDLE;
				Msg.index = PreKeyIndex;
				Msg.type  = ADC_KEY_LONG_RELEASED;
				PreKeyIndex = ADC_CHANNEL_EMPTY;
			}
			else if(IsTimeOut(&AdcKeyWaitTimer))
			{
				//return key cph value
				//APP_DBG("ADC KEY CPH!\n");
				TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_CPH_TIME);
				Msg.index = PreKeyIndex;
				Msg.type  = ADC_KEY_LONG_PRESS_HOLD;
			}
			break;

		default:
			AdcKeyState = ADC_KEY_STATE_IDLE;
			PreKeyIndex = ADC_CHANNEL_EMPTY;
			break;
	}

	return Msg;
}

#endif //CFG_RES_ADC_KEY_USE


