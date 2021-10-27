/**
 **************************************************************************************
 * @file    adc_key.h
 * @brief   adc key 
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-1-11 18:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */


#ifndef __ADC_KEY_H__
#define __ADC_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
#include "type.h"

#define ADC_000V    00*4096/330
#define ADC_010V    10*4096/330
#define ADC_020V    20*4096/330
#define ADC_030V    30*4096/330
#define ADC_040V    40*4096/330
#define ADC_050V    50*4096/330
#define ADC_060V    60*4096/330
#define ADC_070V    70*4096/330
#define ADC_080V    80*4096/330
#define ADC_085V    85*4096/330
#define ADC_090V    90*4096/330
#define ADC_100V    100*4096/330
#define ADC_110V    110*4096/330
#define ADC_120V    120*4096/330
#define ADC_130V    130*4096/330
#define ADC_140V    140*4096/330
#define ADC_150V    150*4096/330
#define ADC_160V    160*4096/330
#define ADC_165V    165*4096/330
#define ADC_170V    170*4096/330
#define ADC_180V    180*4096/330
#define ADC_190V    190*4096/330

#define ADC_200V    200*4096/330
#define ADC_210V    210*4096/330
#define ADC_220V    220*4096/330
#define ADC_230V    230*4096/330
#define ADC_240V    240*4096/330
#define ADC_250V    250*4096/330
#define ADC_260V    260*4096/330
#define ADC_270V    270*4096/330
#define ADC_280V    280*4096/330
#define ADC_290V    290*4096/330


#define ADC_300V    300*4096/330
#define ADC_310V    310*4096/330
#define ADC_320V    320*4096/330
#define ADC_330V    330*4096/330
#define ADC_340V    340*4096/330
#define ADC_350V    350*4096/330
#define ADC_360V    360*4096/330
#define ADC_370V    370*4096/330
#define ADC_380V    380*4096/330
#define ADC_390V    390*4096/330

#define ADC_400V    400*4096/330
#define ADC_410V    410*4096/330
#define ADC_420V    420*4096/330
#define ADC_430V    430*4096/330
#define ADC_440V    440*4096/330
#define ADC_450V    450*4096/330
#define ADC_460V    460*4096/330
#define ADC_470V    470*4096/330
#define ADC_480V    480*4096/330
#define ADC_490V    490*4096/330

#define 	ADC_CHANNEL_EMPTY               0xFF

typedef enum _AdcKeyType
{
	ADC_KEY_UNKOWN_TYPE = 0,
	ADC_KEY_PRESSED,
	ADC_KEY_RELEASED,
	ADC_KEY_LONG_PRESSED,
	ADC_KEY_LONG_PRESS_HOLD,
	ADC_KEY_LONG_RELEASED,
}AdcKeyType;

typedef struct _AdcKeyMsg
{
    uint16_t index;
    uint16_t type;
}AdcKeyMsg;

uint8_t GetAdcKeyIndex(void);

AdcKeyMsg AdcKeyScan(void);

int32_t AdcKeyInit(void);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__ADC_KEY_H__

