/**
 **************************************************************************************
 * @file    sadc_interface.h
 * @brief   adc interface
 *
 * @author  Sam
 * @version V1.1.0
 *
 * $Created: 2018-05-06 16:50:43$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef		__SADC_INTERFACE_H__
#define		__SADC_INTERFACE_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

void SarADC_Init(void);

int16_t SarADC_LDOINVolGet(void);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__SADC_INTERFACE_H__

