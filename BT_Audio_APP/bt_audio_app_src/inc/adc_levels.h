/**
 *************************************************************************************
 * @file	adc_levels.h
 * @author	ken bu/bkd
 * @version	v0.0.1
 * @date    2019/04/24
 * @brief	 for  Sliding rheostat
 * @ maintainer: 
 * Copyright (C) Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *************************************************************************************
 */
#include "app_config.h"
#include <stdint.h>

#ifdef CFG_ADC_LEVEL_KEY_EN

#define ADCLevelsTimer  			2//35
#define how_many_times_have_effect	3

enum
{
	FUNC_ID_BYPASS_VOL_VALUE,
	FUNC_ID_ECHO_VOL_VALUE,
	FUNC_ID_REVERB_VOL_VALUE,
	FUNC_ID_BASS_VALUE,
	FUNC_ID_TREB_VALUE,

};

typedef struct _ADCLevelsMsg
{
    uint16_t channel_index;
    uint16_t step_value;
} ADCLevelsMsg;

void ADCLevelsKeyInit(void);
void ADCLevelsScan(void);
uint16_t  AdcLevelKeyOneChanScan(uint8_t ADCLevelsScanNum);
uint16_t AdcLevelKeyProcess(void);
ADCLevelsMsg SliderResistor;


#endif//__POTENTIOMETER_KEY_H__



