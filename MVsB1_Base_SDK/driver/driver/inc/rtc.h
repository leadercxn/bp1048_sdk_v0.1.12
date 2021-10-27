/**
 *****************************************************************************
 * @file     rtc.h
 * @author   Sean
 * @version  V1.0.0
 * @date     29-Nov-2016
 * @brief    rtc module driver header file
 * @maintainer: Sean
 * change log:
 *			 Add by Sean -20161129
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */
/**
 * @addtogroup RTC
 * @{
 * @defgroup rtc rtc.h
 * @{
 */

#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

/**
 *  RTC Clock Source Select
 */
typedef enum _RTC_CLOCK_SEL
{
    OSC_32K = 0,/**<RTC时钟选择晶体晶振32.768kHZ，时钟频率为32768HZ*/
    OSC_24M = 1,/**<RTC时钟选择晶体晶振24MHZ并且进行分频分频，时钟频率可配*/
    RC_32K = 2  /**<RTC时钟选择RC 32.768kHZ，时钟频率为32768HZ，RC误差较大，禁用*/
} RTC_CLOCK_SEL;

/**
 * @brief  RTC中断使能
 * @param  无
 * @return 无
 */
void RTC_IntEnable(void);

/**
 * @brief  RTC中断禁止
 * @param  无
 * @return 无
 */
void RTC_IntDisable(void);

/**
 * @brief  RTC中断标志清除
 * @param  无
 * @return 无
 */
void RTC_IntFlagClear(void);

/**
 * @brief  RTC中断标志获取
 * @param  无
 * @return 无
 */
bool RTC_IntFlagGet(void);

/**
 * @brief  RTC唤醒功能开启
 * @param  无
 * @return 无
 */
void RTC_WakeupEnable(void);

/**
 * @brief  RTC唤醒功能关闭
 * @param  无
 * @return 无
 */
void RTC_WakeupDisable(void);

/**
 * @brief  设置Frac（小于秒的计时单位）
 * @param  FracTime  设置的值
 * @return 无
 */
void RTC_FracSet(uint32_t FracTime);

/**
 * @brief  设置Sec
 * @param  Sec  设置的值（以秒为计时单位）
 * @return 无
 */
void RTC_SecSet(uint32_t Sec);

/**
 * @brief  获取Frac（小于秒的计时单位）
 * @param  无
 * @return 当前FracTime
 */
uint32_t RTC_FracGet(void);

/**
 * @brief  获取Sec
 * @param  无
 * @return 当前Sec（以秒为计时单位）
 */
uint32_t RTC_SecGet(void);

/**
 * @brief  配置SecAlarm
 * @param  SecAlarm值（计时单位为秒）
 * @return 无
 */
void RTC_SecAlarmSet(uint32_t SecAlarm);

/**
 * @brief  设置FracAlarm（小于秒的计时单位）
 * @param  FracTime  设置的值
 * @return 无
 */
void RTC_FracAlarmSet(uint32_t FracAlarm);

/**
 * @brief  获取SecAlarm
 * @param  无
 * @return 当前SecAlarm（计时单位为秒）
 */
uint32_t RTC_SecAlarmGet(void);

/**
 * @brief  获取FracAlarm
 * @param  无
 * @return 当前FracAlarm（小于秒的计时单位）
 */
uint32_t FracAlarmGet(void);

/**
 * @brief  配置RTC模块的时钟源和频率
 * @param  ClkSel 配置RTC模块的时钟源
 *	@arg OSC_32K:选择32.768kHZ晶体晶振作为RTC时钟，时钟频率为32768HZ
 *	@arg OSC_24M:选择24MHZ晶体晶振并且对其进行分频作为RTC时钟，时钟频率由用户配置，单位为HZ
 *	@arg RC_32K: 选择32.768kHZ晶体晶振作为rtc时钟，时钟频率为32768HZ,RC时钟误差较大
 * @return 无
 */
void RTC_ClockSrcSel(RTC_CLOCK_SEL ClkSel);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__RTC_H__

/**
 * @}
 * @}
 */
