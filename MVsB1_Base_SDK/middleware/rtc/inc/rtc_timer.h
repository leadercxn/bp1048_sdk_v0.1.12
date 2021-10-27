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


#define RTC_START_YEAR 		1980            /**< ��ģ���ʼ��ݣ������������׼ */
#define DAYS_PER_YEAR 		365  			/**< ��ģ� һ��������������꣩ */
#define DAYS_PER_4YEARS		1461			/**< ��ģ���������� */
#define SECONDS_PER_DAY		(24*3600)		/**< ��ģ�ÿ������� */
#define	SECONDS_PER_WEEK	(7*24*3600)  	/**< ��ģ�ÿ�ܵ����� */
#define SECONDS_PER_HOUR	(3600)			/**< ��ģ�ÿСʱ������ */
#define SECONDS_PER_MIN		(60)			/**< ��ģ�ÿ���ӵ����� */
#define	RTC_BASE_WDAY		2				/**< ��ģ�1980��Ԫ�������ն�*/

/**
 * ����ʱ��ṹ��ָ��
 */
typedef struct _RTC_DATE_TIME
{
	uint32_t	Year;  /**< ������*/
	uint8_t	    Mon;   /**< ������ */
	uint8_t	    Date;  /**< ������ */
	uint8_t   	Hour;  /**< Сʱ�� */
	uint8_t 	Min;   /**< ������ */
	uint8_t	    Sec;   /**< ���� */
	uint8_t	    WDay;  /**< ���ںţ�0�������գ�1~6���� ��һ������*/
} RTC_DATE_TIME;

/**
 * @brief  �ж��Ƿ�������
 * @param  Year�� �������
 * @return TRUE if it is leap year, otherwise, return FALSE.
 */
bool RTC_IsLeapYear(uint16_t Year);

/**
 * @brief  ���ݾ���1980��Ԫ����ƫ������������������
 * @param  Days:  ����1980��Ԫ����ƫ������
 * @return ������
 */
RTC_DATE_TIME RTC_CurrDate2BasedDateGet(uint16_t Days);

/**
 * @brief  ����������ʱ������Ϣ���������1980��1��1��0ʱ0��0�������
 * @param  Time: ָ��ʱ��ṹ���ָ��
 * @return ��������
 */
uint32_t RTC_DateTimer2SecondsGet(RTC_DATE_TIME* Time);

/**
 * @brief  ���õ�ǰ����ʱ����Ϣ��������������ʱ���룩
 * @param  Time: ָ��ʱ��ṹ���ָ��
 * @return TRUE:���óɹ� ��FALSE:����ʧ��
 */
bool RTC_DateTimerSet(RTC_DATE_TIME* Time);

/**
 * @brief  ��ȡ��ǰ����ʱ����Ϣ��������������ʱ���룩
 * @param  ��
 * @return RTC_DATE_TIME�� ����ʱ��ṹ��
 */
RTC_DATE_TIME RTC_DateTimerGet(void);

/**
 * @brief  ��ȡ������ݵ�ָ���·ݵ�����
 * @param  Year:�������
 * @param  Month:ָ���·�
 * @return ���ع�����ݵ�ָ���·ݵ�����
 */
uint8_t RTC_MonthDaysGet(uint16_t Year, uint8_t Month);

/**
 * @brief  ���ݵ�ǰ�ṩ�����������������1980��1��1�յ�����
 * @param  timePtr: ָ��ʱ��ṹ���ָ��
 * @return ����1980��1��1�յ�����
 */
uint16_t RTC_Days2BasedDayGet(RTC_DATE_TIME* time);
/**
 * @brief  ���ݾ���1980��Ԫ����ƫ������������������
 * @param  DaysOffset: ����1980��Ԫ����ƫ������
 * @return ������
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
