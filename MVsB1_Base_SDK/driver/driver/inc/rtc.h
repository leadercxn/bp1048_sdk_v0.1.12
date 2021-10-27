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
    OSC_32K = 0,/**<RTCʱ��ѡ���徧��32.768kHZ��ʱ��Ƶ��Ϊ32768HZ*/
    OSC_24M = 1,/**<RTCʱ��ѡ���徧��24MHZ���ҽ��з�Ƶ��Ƶ��ʱ��Ƶ�ʿ���*/
    RC_32K = 2  /**<RTCʱ��ѡ��RC 32.768kHZ��ʱ��Ƶ��Ϊ32768HZ��RC���ϴ󣬽���*/
} RTC_CLOCK_SEL;

/**
 * @brief  RTC�ж�ʹ��
 * @param  ��
 * @return ��
 */
void RTC_IntEnable(void);

/**
 * @brief  RTC�жϽ�ֹ
 * @param  ��
 * @return ��
 */
void RTC_IntDisable(void);

/**
 * @brief  RTC�жϱ�־���
 * @param  ��
 * @return ��
 */
void RTC_IntFlagClear(void);

/**
 * @brief  RTC�жϱ�־��ȡ
 * @param  ��
 * @return ��
 */
bool RTC_IntFlagGet(void);

/**
 * @brief  RTC���ѹ��ܿ���
 * @param  ��
 * @return ��
 */
void RTC_WakeupEnable(void);

/**
 * @brief  RTC���ѹ��ܹر�
 * @param  ��
 * @return ��
 */
void RTC_WakeupDisable(void);

/**
 * @brief  ����Frac��С����ļ�ʱ��λ��
 * @param  FracTime  ���õ�ֵ
 * @return ��
 */
void RTC_FracSet(uint32_t FracTime);

/**
 * @brief  ����Sec
 * @param  Sec  ���õ�ֵ������Ϊ��ʱ��λ��
 * @return ��
 */
void RTC_SecSet(uint32_t Sec);

/**
 * @brief  ��ȡFrac��С����ļ�ʱ��λ��
 * @param  ��
 * @return ��ǰFracTime
 */
uint32_t RTC_FracGet(void);

/**
 * @brief  ��ȡSec
 * @param  ��
 * @return ��ǰSec������Ϊ��ʱ��λ��
 */
uint32_t RTC_SecGet(void);

/**
 * @brief  ����SecAlarm
 * @param  SecAlarmֵ����ʱ��λΪ�룩
 * @return ��
 */
void RTC_SecAlarmSet(uint32_t SecAlarm);

/**
 * @brief  ����FracAlarm��С����ļ�ʱ��λ��
 * @param  FracTime  ���õ�ֵ
 * @return ��
 */
void RTC_FracAlarmSet(uint32_t FracAlarm);

/**
 * @brief  ��ȡSecAlarm
 * @param  ��
 * @return ��ǰSecAlarm����ʱ��λΪ�룩
 */
uint32_t RTC_SecAlarmGet(void);

/**
 * @brief  ��ȡFracAlarm
 * @param  ��
 * @return ��ǰFracAlarm��С����ļ�ʱ��λ��
 */
uint32_t FracAlarmGet(void);

/**
 * @brief  ����RTCģ���ʱ��Դ��Ƶ��
 * @param  ClkSel ����RTCģ���ʱ��Դ
 *	@arg OSC_32K:ѡ��32.768kHZ���徧����ΪRTCʱ�ӣ�ʱ��Ƶ��Ϊ32768HZ
 *	@arg OSC_24M:ѡ��24MHZ���徧���Ҷ�����з�Ƶ��ΪRTCʱ�ӣ�ʱ��Ƶ�����û����ã���λΪHZ
 *	@arg RC_32K: ѡ��32.768kHZ���徧����Ϊrtcʱ�ӣ�ʱ��Ƶ��Ϊ32768HZ,RCʱ�����ϴ�
 * @return ��
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
