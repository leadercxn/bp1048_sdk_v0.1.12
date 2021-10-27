/**
 *****************************************************************************
 * @file     rtc_lunar.h
 * @author   TaoWen
 * @version  V1.0.0
 * @date     10-Oct-2018
 * @brief    rtc module driver header file
 * @maintainer: TaoWen
 * change log:
 *			 Add by TaoWen -20181010
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */
/**
 * @addtogroup RTC
 * @{
 * @defgroup rtc_lunar rtc_lunar.h
 * @{
 */

#ifndef __RTC_LUNAR_H__
#define __RTC_LUNAR_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "rtc_timer.h"

/**
 *  农历时间结构体
 */
typedef struct _RTC_LUNAR_DATE
{
	uint16_t Year;         /**< 农历年份*/
	uint8_t Month;        /**< 农历月份*/
	uint8_t Date;         /**< 农历日期*/
	uint8_t MonthDays;    /**< 农历的各个月份的天数*/
	bool IsLeapMonth;  	  /**< 是否是农历闰月*/
} RTC_LUNAR_DATE;

/**
 * @brief  将某公历年的农历信息提取到全局变量中
 * @param  Year:某公历年份
 * @return NONE
 */
void RTC_LunarInfoGet(uint16_t Year);

/**
 * @brief  获取农历年份的天干
 * @param  Year:农历年份作为输入参数
 * @return 返回0~9，分别对应天干：{“甲”,“乙”,“丙”,“丁”,“戊”,“已”,“庚”,“辛”,“壬”,“癸”}
 */
uint8_t RTC_HeavenlyStemGet(uint16_t Year);

/**
 * @brief  获取农历年份的地支
 * @param  Year:农历年份作为输入参数
 * @return 返回0~11，分别对应地支： {“子”,“丑”,“寅”,“牟”,“辰”,“巳”,“午”,“未”,“申”,“酉”,“戌”,“亥”}
 *         分别对应生肖: {“鼠”,“牛”,“虎”,“兔”,“龙”,“蛇”,“马”,“羊”,“猴”,“鸡”,“狗”,“猪”}
 */
uint8_t RTC_EarthlyBranchGet(uint16_t Year);

/**
 * @brief  公历日期转化为农历日期
 * @param  DateTime:阳历时间结构体指针，作为输入参数
 * @return 返回RTC_LUNAR_DATE*类型的农历时间结构体指针
 */
RTC_LUNAR_DATE  RTC_SolarToLunar(RTC_DATE_TIME* DateTime);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__RTC_LUNAR_H__

/**
 * @}
 * @}
 */
