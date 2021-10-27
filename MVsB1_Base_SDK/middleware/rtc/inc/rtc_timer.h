/**
 *****************************************************************************
 * @file     rtc_timer.h
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
 * @defgroup rtc_timer rtc_timer.h
 * @{
 */

#ifndef __RTC_TIMER_H__
#define __RTC_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"


#define RTC_START_YEAR 		1980            /**< 勿改，起始年份，用来做计算基准 */
#define DAYS_PER_YEAR 		365  			/**< 勿改， 一年的天数（非闰年） */
#define DAYS_PER_4YEARS		1461			/**< 勿改，四年的天数 */
#define SECONDS_PER_DAY		(24*3600)		/**< 勿改，每天的秒数 */
#define	SECONDS_PER_WEEK	(7*24*3600)  	/**< 勿改，每周的秒数 */
#define SECONDS_PER_HOUR	(3600)			/**< 勿改，每小时的秒数 */
#define SECONDS_PER_MIN		(60)			/**< 勿改，每分钟的秒数 */
#define	RTC_BASE_WDAY		2				/**< 勿改，1980年元旦是周日二*/

/**
 * 公历时间结构体指针
 */
typedef struct _RTC_DATE_TIME
{
	uint32_t	Year;  /**< 公历年*/
	uint8_t	    Mon;   /**< 公历月 */
	uint8_t	    Date;  /**< 公历日 */
	uint8_t   	Hour;  /**< 小时数 */
	uint8_t 	Min;   /**< 分钟数 */
	uint8_t	    Sec;   /**< 秒数 */
	uint8_t	    WDay;  /**< 星期号，0代表周日；1~6代表 周一到周六*/
} RTC_DATE_TIME;

/**
 * @brief  判断是否是闰年
 * @param  Year： 阳历年份
 * @return TRUE if it is leap year, otherwise, return FALSE.
 */
bool RTC_IsLeapYear(uint16_t Year);

/**
 * @brief  根据距离1980年元旦的偏移天数来计算年月日
 * @param  Days:  距离1980年元旦的偏移天数
 * @return 年月日
 */
RTC_DATE_TIME RTC_CurrDate2BasedDateGet(uint16_t Days);

/**
 * @brief  根据年月日时分秒信息来计算距离1980年1月1日0时0分0秒的秒数
 * @param  Time: 指向时间结构体的指针
 * @return 返回秒数
 */
uint32_t RTC_DateTimer2SecondsGet(RTC_DATE_TIME* Time);

/**
 * @brief  设置当前日期时间信息（包括：年月日时分秒）
 * @param  Time: 指向时间结构体的指针
 * @return TRUE:设置成功 ；FALSE:设置失败
 */
bool RTC_DateTimerSet(RTC_DATE_TIME* Time);

/**
 * @brief  获取当前日期时间信息（包括：年月日时分秒）
 * @param  无
 * @return RTC_DATE_TIME： 返回时间结构体
 */
RTC_DATE_TIME RTC_DateTimerGet(void);

/**
 * @brief  获取公历年份的指定月份的天数
 * @param  Year:公历年份
 * @param  Month:指定月份
 * @return 返回公历年份的指定月份的天数
 */
uint8_t RTC_MonthDaysGet(uint16_t Year, uint8_t Month);

/**
 * @brief  根据当前提供的年月日来计算距离1980年1月1日的天数
 * @param  timePtr: 指向时间结构体的指针
 * @return 距离1980年1月1日的天数
 */
uint16_t RTC_Days2BasedDayGet(RTC_DATE_TIME* time);
/**
 * @brief  根据距离1980年元旦的偏移天数来计算周天数
 * @param  DaysOffset: 距离1980年元旦的偏移天数
 * @return 周天数
 */
uint8_t RTC_CurrWeekDay2BasedDateGet(uint16_t DaysOffset);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__RTC_H__

/**
 * @}
 * @}
 */
