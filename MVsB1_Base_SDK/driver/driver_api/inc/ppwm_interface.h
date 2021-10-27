/**
 **************************************************************************************
 * @file    ppwm_interface.h
 * @brief   audio ppwm interface
 *
 * @author  
 * @version V1.1.0
 *
 * $Created: 2019-05-23 14:01:05$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef		__PPWM_INTERFACE_H__
#define		__PPWM_INTERFACE_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

void AudioPPWM_Init(uint32_t SampleRate, void* Buf, uint16_t Len);
void AudioPPWM_SampleRateChange(uint32_t SampleRate);
uint16_t AudioPPWM_DataSpaceLenGet(void);
uint16_t AudioPPWM_DataSet(void* Buf, uint16_t Len);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__PPWM_INTERFACE_H__

