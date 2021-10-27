/**
 *******************************************************************************
 * @file    ppwm.h
 * @brief	ppwm module driver interface

 * @author  Peter
 * @version V1.0.0

 * $Created: 2019-05-26 14:01:05$
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *******************************************************************************
 */

/**
 * @addtogroup PPWM
 * @{
 * @defgroup ppwm ppwm.h
 * @{
 */

#ifndef MODULES_PPWM_H_
#define MODULES_PPWM_H_

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include "type.h"

typedef enum
{
	AUPLL_CLK = 0,
	PLL_CLK   = 1,
	GPIO_IN00 = 2,
	GPIO_IN01 = 3,
}PPWM_CLK_SRC;

/**
 * @brief   ʹ��PPWM
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_Enable(void);

/**
 * @brief   ����PPWM
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_Disable(void);

/**
 * @brief   ʹ��PPWM Mute����
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_DigitalMuteEnable(void);

/**
 * @brief   ����PPWM Mute����
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_DigitalMuteDisable(void);

/**
 * @brief   ��������
 * @param	Vol ����ֵ��ȡֵ��Χ0~0x3FFF
 * @return  ��
 * @note
 */
void PPWM_VolSet(uint16_t Vol);

/**
 * @brief   ��ȡ����ֵ
 * @param	��
 * @return  ����ֵ
 * @note
 */
uint16_t PPWM_VolGet(void);

/**
 * @brief   PPWM��ͣ
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_Pause(void);

/**
 * @brief   PPWM����
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_Run(void);

/**
 * @brief   ���error��־
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_LoadErrorFlagClear(void);

/**
 * @brief   ��ȡerror��־
 * @param	��
 * @return  ��
 * @note
 */
bool PPWM_LoadErrorFlagGet(void);

/**
 * @brief   ʹ��PPWM Fade����
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_FadeEnable(void);

/**
 * @brief   ����PPWM Fade����
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_FadeDisable(void);

/**
 * @brief   ʹ��PPWMʱ��
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_ClkEnable(void);

/**
 * @brief   ����PPWMʱ��
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_ClkDisable(void);

/**
 * @brief   PPWMʱ��Դѡ��
 * @param	sel ʱ��Դ����ο�PPWM_CLK_SRC�ṹ��
 * @return  ��
 * @note
 */
void PPWM_SelectClkSource(PPWM_CLK_SRC sel);

/**
 * @brief   ����PPWM Fadeʱ��
 * @param	FadeTime
 * @return  ��
 * @note
 */
void PPWM_FadeTimeSet(uint8_t FadeTime);

/**
 * @brief   ��ȡPPWM Fadeʱ��
 * @param	��
 * @return  Fadeʱ��
 * @note
 */
uint32_t PPWM_FadeTimeGet(void);

/**
 * @brief   ʹ��DSM����
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_DsmOutdisModeEnable(void);

/**
 * @brief   ����DSM����
 * @param	��
 * @return  ��
 * @note
 */
void PPWM_DsmOutdisModeDisable(void);

/**
 * @brief   ����Zero����
 * @param	sel ȡֵ��Χ0~7
 * 			@arg	0~zeros number value: 512
 *          @arg    1~zeros number value: 1024
 *          @arg    2~zeros number value: 2048
 *			@arg    3~zeros number value: 4096
 *          @arg    4~zeros number value: 8192
 *          @arg    5~zeros number value: 16384
 *          @arg    6~zeros number value: 32768
 *          @arg    7~zeros number value: 65535
 * @return  ��
 * @note
 */
void PPWM_ZeroNumSet(uint8_t sel);

/**
 * @brief   ����DutyCycle
 * @param	sel ȡֵ��Χ0~2
 * @return  ��
 * @note
 */
void PPWM_PwmDutyCycleSet(uint8_t sel);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // MODULES_PPWM_H_
/**
 * @}
 * @}
 */
