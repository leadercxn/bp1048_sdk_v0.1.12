/**
 *************************************************************************************
 * @file	adc_levels.c
 * @author	ken bu/bkd
 * @version	v0.0.1
 * @date    2019/04/24
 * @brief	 for  Sliding rheostat
 * @ maintainer: 
 * Copyright (C) Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#include "app_config.h"
#include "app_message.h"

#ifdef CFG_ADC_LEVEL_KEY_EN
#include "adc_levels.h"
#include "adc.h"
//#include "config.h"
#include "timeout.h"
#include "gpio.h"
#include "debug.h"
#include "key.h"

static TIMER   ADCLevelsScanTimer;
static uint8_t ADCLevelsScanCount        = 0;
static uint8_t ADCLevelTotalNum          = 0;
static uint8_t ADCLevelChannelTotal      = 0;
static uint16_t ADCLevelsChannel[14];
static uint8_t repeat_count[14]          = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static uint16_t ADCLevels_STEP_Store[14]  = {0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff};
static uint16_t ADCLevels_Msg_Ch[14]  = {MSG_ADC_LEVEL_CH1,MSG_ADC_LEVEL_CH2,MSG_ADC_LEVEL_CH3,MSG_ADC_LEVEL_CH4,MSG_ADC_LEVEL_CH5,MSG_ADC_LEVEL_CH6,MSG_ADC_LEVEL_CH7,MSG_ADC_LEVEL_CH8,MSG_ADC_LEVEL_CH9,MSG_ADC_LEVEL_CH10,MSG_ADC_LEVEL_CH11};

/*
****************************************************************
* 全部ADC通道初始化列表
*
*
****************************************************************
*/
const uint32_t ADC_CHANNEL_Init_Tab[14*2]=
{

	GPIO_A_ANA_EN, GPIO_INDEX20,/**channel 0*/

	GPIO_A_ANA_EN, GPIO_INDEX21,/**channel 1*/

	GPIO_A_ANA_EN, GPIO_INDEX22,/**channel 2*/

	GPIO_A_ANA_EN, GPIO_INDEX23,/**channel 3*/

	GPIO_A_ANA_EN, GPIO_INDEX24,/**channel 4*/

	GPIO_A_ANA_EN, GPIO_INDEX25,/**channel 5*/

	GPIO_A_ANA_EN, GPIO_INDEX26,/**channel 6*/

	GPIO_A_ANA_EN, GPIO_INDEX27,/**channel 7*/

	GPIO_A_ANA_EN, GPIO_INDEX28,/**channel 8*/

	GPIO_A_ANA_EN, GPIO_INDEX29,/**channel 9*/

	GPIO_A_ANA_EN, GPIO_INDEX30,/**channel 10*/

	GPIO_A_ANA_EN, GPIO_INDEX31,/**channel 11*/

	GPIO_B_ANA_EN, GPIO_INDEX0,	/**channel 12*/

	GPIO_B_ANA_EN, GPIO_INDEX1,	/**channel 13*/

};
	
/*
****************************************************************
*  ADC通道选择列表
*
*
****************************************************************
*/
const uint32_t ADC_CHANNEL_Select_Tab[14]=
{
	ADC_GPIOA20,
	ADC_GPIOA21,
	ADC_GPIOA22,
	ADC_GPIOA23,
	ADC_GPIOA24,
	ADC_GPIOA25,
	ADC_GPIOA26,
	ADC_GPIOA27,
	ADC_GPIOA28,
	ADC_GPIOA29,
	ADC_GPIOA30,
	ADC_GPIOA31,
	ADC_GPIOB0,
	ADC_GPIOB1,
};

/*
****************************************************************
* 获取对应ADC通道的ADC采样值
*
*
****************************************************************
*/
uint16_t GetAdcVal(uint32_t adc_ch)
{
    uint16_t Val = 0x00;
	
	if(adc_ch == ADC_GPIOA20)	         
	{
		//GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX20);//若是用到复用的ADC口，例如A20和A23口做ADC，需要打开此代码
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA20_A23);
		//GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX20);  //若是用到复用的ADC口，例如A20和A23口做ADC，需要打开此代码
		//GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX23);  //若是用到复用的ADC口，例如A20和A23口做ADC，需要打开此代码
	}
	else if(adc_ch == ADC_GPIOA21)	     
	{
		//GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX21);//若是用到复用的ADC口，例如A21和A24口做ADC，需要打开此代码
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA21_A24);
		//GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX21);  //若是用到复用的ADC口，例如A21和A24口做ADC，需要打开此代码
		//GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX24);  //若是用到复用的ADC口，例如A21和A24口做ADC，需要打开此代码
	}
	else if(adc_ch == ADC_GPIOA22)	 
	{
		//GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX22);//若是用到复用的ADC口，例如A22和A25口做ADC，需要打开此代码
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA22_A25);
		//GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX22);  //若是用到复用的ADC口，例如A22和A25口做ADC，需要打开此代码
		//GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX25);  //若是用到复用的ADC口，例如A22和A25口做ADC，需要打开此代码
	}	
	else if(adc_ch == ADC_GPIOA23)	 
	{
		//GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX23);//若是用到复用的ADC口，例如A20和A23口做ADC，需要打开此代码
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA20_A23);
		//GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX20);  //若是用到复用的ADC口，例如A20和A23口做ADC，需要打开此代码
		//GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX23);  //若是用到复用的ADC口，例如A20和A23口做ADC，需要打开此代码
	}
	else if(adc_ch == ADC_GPIOA24)  
	{
		//GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX24);//若是用到复用的ADC口，例如A21和A24口做ADC，需要打开此代码
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA21_A24);
		//GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX21);  //若是用到复用的ADC口，例如A21和A24口做ADC，需要打开此代码
		//GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX24);  //若是用到复用的ADC口，例如A21和A24口做ADC，需要打开此代码
	}
	else if(adc_ch == ADC_GPIOA25) 
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA22_A25);
	}
	else if(adc_ch == ADC_GPIOA26) 
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA26);
	}
	else if(adc_ch == ADC_GPIOA27)  
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA27);
	}
	else if(adc_ch == ADC_GPIOA28)  
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA28);
	}
	else if(adc_ch == ADC_GPIOA29) 
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA29);
	}
	else if(adc_ch == ADC_GPIOA30)  
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA30);
	}
	else if(adc_ch == ADC_GPIOA31) 
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA31);
	}
	else if(adc_ch == ADC_GPIOB0) 
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOB0);
	}
	else if(adc_ch == ADC_GPIOB1)
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOB1);
	}
	else
	{
		Val = 0x00;
	}
	return Val;

}


/*
****************************************************************
* adc电位器初始化函数
*
*
****************************************************************
*/
void ADCLevelsKeyInit(void)
{
    uint8_t k;

    uint32_t adc_ch;

	ADCLevelChannelTotal = 0;

    adc_ch = ADCLEVL_CHANNEL_MAP;
	
	for(k = 0; k < (sizeof(ADC_CHANNEL_Select_Tab)/sizeof(ADC_CHANNEL_Select_Tab[0])); k++)
	{
		if( adc_ch & ADC_CHANNEL_Select_Tab[k] )
		{
			GPIO_RegOneBitSet(ADC_CHANNEL_Init_Tab[k*2], ADC_CHANNEL_Init_Tab[k*2 + 1]);

			ADCLevelsChannel[ADCLevelChannelTotal] = ADC_CHANNEL_Select_Tab[k];

			ADCLevelChannelTotal++;
		}
	}

	TimeOutSet(&ADCLevelsScanTimer, ADCLevelsTimer);
}
/*
****************************************************************
* 扫描一路adc电位器处理
*
*
****************************************************************
*/
uint16_t  AdcLevelKeyOneChanScan(uint8_t ADCLevelsScanNum)
{
	uint16_t     Val;
	uint8_t     i_count;

	if(ADCLevelChannelTotal == 0) return;
	
	Val = GetAdcVal(ADCLevelsChannel[ADCLevelsScanNum]); 	

	for(i_count=0;i_count < MAX_ADCLEVL_STEP_NUMBER;i_count++)
	{
		uint32_t min = MAX_ADCLEVL_LEVEL_VAL/MAX_ADCLEVL_STEP_NUMBER*i_count - DISTANCE_BETWEEN_STEP;
		uint32_t max = MAX_ADCLEVL_LEVEL_VAL/MAX_ADCLEVL_STEP_NUMBER*(i_count+1) + DISTANCE_BETWEEN_STEP;
		if(i_count == 0)
		{
			min = 0;
		}
		if(i_count == (MAX_ADCLEVL_STEP_NUMBER-1))
		{
			max = MAX_ADCLEVL_LEVEL_VAL;
		}
		if(Val >= min && Val <= max)
		{
			break;
		}
	}
	ADCLevels_STEP_Store[ADCLevelsScanNum] =  i_count;
	DBG("Val = %d\n", Val);
	DBG("i_count = %d\n", i_count);
	DBG("ADCLevelsScanNum = %d\n", ADCLevelsScanNum);
	return i_count;
}
/*
****************************************************************
* adc电位器扫描处理
*
*
****************************************************************
*/
uint16_t  AdcLevelKeyProcess(void)
{
	uint16_t     Val;
	uint8_t     i_count;

    //if(!IsTimeOut(&ADCLevelsScanTimer))
	//{
	//	return ;
	//}
	//TimeOutSet(&ADCLevelsScanTimer, ADCLevelsTimer);

    if(ADCLevelChannelTotal == 0) return;

	ADCLevelsScanCount++;

	if(ADCLevelsScanCount >= ADCLevelChannelTotal)
	{
		ADCLevelsScanCount = 0;
	}

    Val = GetAdcVal(ADCLevelsChannel[ADCLevelsScanCount]); 	

	for(i_count=0;i_count < MAX_ADCLEVL_STEP_NUMBER;i_count++)
	{
		uint32_t min = MAX_ADCLEVL_LEVEL_VAL/MAX_ADCLEVL_STEP_NUMBER*i_count - DISTANCE_BETWEEN_STEP;
		uint32_t max = MAX_ADCLEVL_LEVEL_VAL/MAX_ADCLEVL_STEP_NUMBER*(i_count+1) + DISTANCE_BETWEEN_STEP;
		if(i_count == 0)
		{
			min = 0;
		}
		if(i_count == (MAX_ADCLEVL_STEP_NUMBER-1))
		{
			max = MAX_ADCLEVL_LEVEL_VAL;
		}
		if(Val >= min && Val <= max)
		{
			break;
		}
	}

	if(i_count != MAX_ADCLEVL_STEP_NUMBER)
	{
		if(i_count != ADCLevels_STEP_Store[ADCLevelsScanCount] )
		{
			repeat_count[ADCLevelsScanCount]++;
			if(repeat_count[ADCLevelsScanCount] > how_many_times_have_effect)
			{
				ADCLevels_STEP_Store[ADCLevelsScanCount] =  i_count;
				APP_DBG("Val = %d\n", Val);
				APP_DBG("i_count = %d\n", i_count);
				APP_DBG("ADCLevelsScanCount = %d\n", ADCLevelsScanCount);
				return (ADCLevels_Msg_Ch[ADCLevelsScanCount]+i_count);
			}
		}
		else
		{
			repeat_count[ADCLevelsScanCount] = 0;
			return MSG_NONE;
		}
	}
	return MSG_NONE;
}
#endif

#if 0
/*
****************************************************************
* adc电位器处理主函数
*
*
****************************************************************
*/
uint16_t AdcLevelKeyProcess(void)
{
	//uint16_t msg;
	SliderResistor.step_value = 255;
	SliderResistor.channel_index = 255;
	ADCLevelsScan();
	if(SliderResistor.step_value!=255)
	{
		if(SliderResistor.channel_index == 0)
		{
			return MSG_BT_ADC_LEVEL_CH1+SliderResistor.step_value;
		}
		else if(SliderResistor.channel_index == 1)
		{
			return MSG_BT_ADC_LEVEL_CH2+SliderResistor.step_value;
		}
		else if(SliderResistor.channel_index == 2)
		{
			return MSG_BT_ADC_LEVEL_CH3+SliderResistor.step_value;
		}
		else
		{
			return MSG_NONE;
		}
	}
	return MSG_NONE;
}

/*
****************************************************************
* adc电位器扫描处理
*
*
****************************************************************
*/
void ADCLevelsScan(void)
{
	uint16_t     Val;

    if(!IsTimeOut(&ADCLevelsScanTimer))
	{
		return ;
	}
	TimeOutSet(&ADCLevelsScanTimer, ADCLevelsTimer);

	if(ADCLevelChannelTotal == 0) return;

	ADCLevelsScanCount++;

	if(ADCLevelsScanCount >= ADCLevelChannelTotal)
	{
		ADCLevelsScanCount = 0;
	}

	if(gCtrlVars.init_param & 0x01)
	{
		gCtrlVars.init_param &= ~0x01;

		for(Val = 0; Val < ADCLevelChannelTotal; Val++)
		{
			ADCLevelsValue[Val] = 0xffff;
		}
	}

    Val = GetAdcVal(ADCLevelsChannel[ADCLevelsScanCount]);

	if(CompareJitter(Val,ADCLevelsValue[ADCLevelsScanCount]))
	{
		if(Val != ADCLevelsValue[ADCLevelsScanCount])/////
		{
			ADCLevelsValue[ADCLevelsScanCount] =  Val;
			//----0-0f  ,maybe modfiy-----------//
			if(Val > 1800)
			{
				Val = (MAX_ADCLEVL_STEP-1);
			}
			else
			{
				Val >>= 7;
			}

			Val &= (MAX_ADCLEVL_STEP-1);
			//--------------------------------------//
			SliderResistor.index = ADCLevelsScanCount;

			SliderResistor.value = Val;
		}
	}
}
/*
****************************************************************
* adc电位器处理主函数
*
*
****************************************************************
*/
uint16_t AdcLevelKeyProcess(void)
{
	uint16_t msg;

	msg = MSG_NONE;

	SliderResistor.index = 100;
	SliderResistor.value = 0;

	ADCLevelsScan();

	if(SliderResistor.index < ADCLevelChannelTotal)
	{
		msg = SliderResistor.index;
		msg <<= 16;
		msg += MSG_ADC_LEVEL_MSG_START;
		msg += SliderResistor.value;

	}

	return msg;
}
#endif





