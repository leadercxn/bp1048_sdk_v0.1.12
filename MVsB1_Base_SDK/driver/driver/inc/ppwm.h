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
 * @brief   使能PPWM
 * @param	无
 * @return  无
 * @note
 */
void PPWM_Enable(void);

/**
 * @brief   禁能PPWM
 * @param	无
 * @return  无
 * @note
 */
void PPWM_Disable(void);

/**
 * @brief   使能PPWM Mute功能
 * @param	无
 * @return  无
 * @note
 */
void PPWM_DigitalMuteEnable(void);

/**
 * @brief   禁能PPWM Mute功能
 * @param	无
 * @return  无
 * @note
 */
void PPWM_DigitalMuteDisable(void);

/**
 * @brief   设置音量
 * @param	Vol 音量值，取值范围0~0x3FFF
 * @return  无
 * @note
 */
void PPWM_VolSet(uint16_t Vol);

/**
 * @brief   获取音量值
 * @param	无
 * @return  音量值
 * @note
 */
uint16_t PPWM_VolGet(void);

/**
 * @brief   PPWM暂停
 * @param	无
 * @return  无
 * @note
 */
void PPWM_Pause(void);

/**
 * @brief   PPWM继续
 * @param	无
 * @return  无
 * @note
 */
void PPWM_Run(void);

/**
 * @brief   清除error标志
 * @param	无
 * @return  无
 * @note
 */
void PPWM_LoadErrorFlagClear(void);

/**
 * @brief   获取error标志
 * @param	无
 * @return  无
 * @note
 */
bool PPWM_LoadErrorFlagGet(void);

/**
 * @brief   使能PPWM Fade功能
 * @param	无
 * @return  无
 * @note
 */
void PPWM_FadeEnable(void);

/**
 * @brief   禁能PPWM Fade功能
 * @param	无
 * @return  无
 * @note
 */
void PPWM_FadeDisable(void);

/**
 * @brief   使能PPWM时钟
 * @param	无
 * @return  无
 * @note
 */
void PPWM_ClkEnable(void);

/**
 * @brief   禁能PPWM时钟
 * @param	无
 * @return  无
 * @note
 */
void PPWM_ClkDisable(void);

/**
 * @brief   PPWM时钟源选择
 * @param	sel 时钟源，请参考PPWM_CLK_SRC结构体
 * @return  无
 * @note
 */
void PPWM_SelectClkSource(PPWM_CLK_SRC sel);

/**
 * @brief   设置PPWM Fade时间
 * @param	FadeTime
 * @return  无
 * @note
 */
void PPWM_FadeTimeSet(uint8_t FadeTime);

/**
 * @brief   获取PPWM Fade时间
 * @param	无
 * @return  Fade时间
 * @note
 */
uint32_t PPWM_FadeTimeGet(void);

/**
 * @brief   使能DSM功能
 * @param	无
 * @return  无
 * @note
 */
void PPWM_DsmOutdisModeEnable(void);

/**
 * @brief   禁能DSM功能
 * @param	无
 * @return  无
 * @note
 */
void PPWM_DsmOutdisModeDisable(void);

/**
 * @brief   设置Zero个数
 * @param	sel 取值范围0~7
 * 			@arg	0~zeros number value: 512
 *          @arg    1~zeros number value: 1024
 *          @arg    2~zeros number value: 2048
 *			@arg    3~zeros number value: 4096
 *          @arg    4~zeros number value: 8192
 *          @arg    5~zeros number value: 16384
 *          @arg    6~zeros number value: 32768
 *          @arg    7~zeros number value: 65535
 * @return  无
 * @note
 */
void PPWM_ZeroNumSet(uint8_t sel);

/**
 * @brief   设置DutyCycle
 * @param	sel 取值范围0~2
 * @return  无
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
