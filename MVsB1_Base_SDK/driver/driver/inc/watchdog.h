/**
 *******************************************************************************
 * @file    watchdog.h
 * @brief	watchdog module driver interface

 * @author  Sam
 * @version V1.0.0

 * $Created: 2017-10-31 16:51:05$
 * @Copyright (C) 2017, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *******************************************************************************
 */

/**
 * @addtogroup WATCHDOG
 * @{
 * @defgroup watchdog watchdog.h
 * @{
 */
 
#ifndef __WATCH_DOG_H__
#define __WATCH_DOG_H__
 
#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
 *  ���忴�Ź��ĸ�λ����
 */
typedef enum _WDG_STEP_SEL
{
    WDG_STEP_1S = 0,/**<���Ź��ĸ�λ����:1S*/
    WDG_STEP_3S = 1,/**<���Ź��ĸ�λ����:3S*/
    WDG_STEP_4S = 2 /**<���Ź��ĸ�λ����:4S*/
} WDG_STEP_SEL;


/**
 * @brief  ���Ź�ʹ��
 * @param  Mode ���ÿ��Ź��ĸ�λ����
 *     @arg WDG_STEP_1S
 *     @arg WDG_STEP_3S
 *     @arg WDG_STEP_4S
 * @return ��
 */
void WDG_Enable(WDG_STEP_SEL Mode);


/**
 * @brief  ���Ź���ֹ
 * @param  ��
 * @return ��
 */
void WDG_Disable(void);


/**
 * @brief  ι��
 * @param  ��
 * @return ��
 */
void WDG_Feed(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__WATCH_DOG_H__

/**
 * @}
 * @}
 */
