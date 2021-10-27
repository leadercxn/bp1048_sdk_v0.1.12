/**
 **************************************************************************************
 * @file    adc_interface.h
 * @brief   audio adc interface
 *
 * @author  Sam
 * @version V1.1.0
 *
 * $Created: 2015-09-23 14:01:05$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef		__ADC_INTERFACE_H__
#define		__ADC_INTERFACE_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 
#include "audio_adc.h"
void AudioADC_DMARestart(ADC_MODULE Module, void* Buf, uint16_t Len);
void AudioADC_DigitalInit(ADC_MODULE Module, uint32_t SampleRate, void* Buf, uint16_t Len);
void AudioADC_DeInit(ADC_MODULE Module);
uint16_t AudioADC_DataLenGet(ADC_MODULE Module);
uint16_t AudioADC_DataGet(ADC_MODULE Module, void* Buf, uint16_t Len);


uint16_t AudioADC0DataLenGet(void);
uint16_t AudioADC1DataLenGet(void);

uint16_t AudioADC0DataGet(void* Buf, uint16_t Len);
uint16_t AudioADC1DataGet(void* Buf, uint16_t Len);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__ADC_INTERFACE_H__

