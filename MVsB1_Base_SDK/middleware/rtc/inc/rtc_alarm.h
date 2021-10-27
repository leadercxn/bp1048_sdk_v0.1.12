/**
 *****************************************************************************
 * @file     rtc_alarm.h
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
 * @defgroup rtc_alarm rtc_alarm.h
 * @{
 */

#ifndef __RTC_ALARM_H__
#define __RTC_ALARM_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "rtc_lunar.h"
#include "rtc_timer.h"


////////////////////////////////�����������йص�API///////////////////////////////////////////////

/**
 * ����ÿ��������ռ�õ�memory�ֽ������ϲ����ɾݴ˼���Ҫ֧�ֵ��������������memory�ռ�
 */
#define   MEM_SIZE_PER_ALARM     10

/**
 * ��������������������֧��8�����ӣ�����8��ʱ��ǰ8��Ϊ׼
 */
#define   gMAX_ALARM_NUM			8

/**
 * ��������״̬
 */
typedef enum _RTC_ALARM_STATUS
{
	ALARM_STATUS_OPENED = 1,    /**< ���Ӵ�״̬  */
	ALARM_STATUS_CLOSED,        /**<���ӹر�״̬ */

} RTC_ALARM_STATUS;

/**
 * ��������ģʽ
 */
typedef enum _RTC_ALARM_MODE
{
	ALARM_MODE_ONCE = 1,	/**< �������� */
	ALARM_MODE_CONTINUE,	/**< �������� */

} RTC_ALARM_MODE;

typedef struct _ALARM_TIME_INFO
{
	uint8_t AlarmHour;/**< ����Сʱ��24Сʱ�ƣ� */
	uint8_t AlarmMin;/**< ���ӷ���(60������) */
	RTC_ALARM_STATUS  AlarmStatus : 2;/**< ����״̬�������� */
	RTC_ALARM_MODE  AlarmMode : 4;/**< ���õ�����ģʽ�� �������ӡ���������   */
	uint8_t  AlarmData; /**< ���ӵľ������ڣ� [bit0~bit6]�ֱ��ʾ����~���� */

} ALARM_TIME_INFO;

extern ALARM_TIME_INFO *gpAlarmList;//�����б�ָ�롣
///////////////////////////////////////////////////////////
//���������أ��ò��ִ������������NVM,���ᱣ����FLASH
//��׼SDKδʵ�����Ӵ����ò��ִ����ɿͻ��Լ�������
typedef struct _NVM_ALARM_INFO_
{
	uint8_t RingType : 3; /**< �������� : TNTER_RING_TYPE:��������; USB_RING_TYPE: U������; SD_RING_TYPE:SD������*/
	uint8_t Duration : 2; /**< �������ʱ��: 0:30s; 1:1����; 2:2����; 3:3���� */
	uint8_t RepeatCnt: 3; /**< �����ظ����� */
	uint32_t FileAddr;    /**< �ļ������� */
	uint8_t CRC8;         /**< �ļ���У���� */

} ALARM_RING_INFO;

/**
 * @brief  ����ǰ�������Ч���ӵ�Load���Ĵ�����
 * @param  pAlarmList �����б�
 * @return NONE
 * #note   ʹ��RTC���ӹ���ʱ����ʼ����ʱ�������б���Ҫ��Ӧ�ò㴫�ݣ����ܴ���NULL������������ʱ��������ӣ������봫NULL
 */
void RTC_LoadCurrAlarm(ALARM_TIME_INFO *pAlarmList);

/**
 * @brief  ��������״̬
 * @param  AlarmID�����Ӻ�
 * @param  AlarmStatus��Ҫ���õ�����״̬
 * @return ���ִ�гɹ�����TRUE�����򷵻�FALSE��
 */
bool RTC_AlarmStatusSet(uint8_t AlarmID, uint8_t AlarmStatus);

/**
 * @brief  ��ȡ���ӵ�״̬��
 * @param  AlarmID�����Ӻ�
 * @return �������ӵ�״̬(��/�ر�)��
 */
RTC_ALARM_STATUS RTC_AlarmStatusGet(uint8_t AlarmID);

/**
 * @brief  ��ȡ���ӵ�����ʱ�䣬����ģʽ����������Ϣ�����Ӻ�
 * @param  AlarmID�����Ӻ�
 * @return ALARM_TIME_INFO*���͵Ľṹ��
 */
ALARM_TIME_INFO* RTC_AlarmTimeGet(uint8_t AlarmID);

/**
 * @brief  ����ĸ����ӵ��ˡ�
 * @param  NONE
 * @return ����0��ʾû�����ӵ������ش���0��ֵ��ʾ��Ӧ�����ӵ���
 * @NONE   [bit0~bit7]�ֱ��ʾ��0��~��8�����ӣ� ��Ӧ��bitλ����1���ʾ�����ӣ���0��������
 */
uint8_t RTC_IntAlarmIDGet(void);

/**
 * @brief  �ж������������Ƿ񳬹����ֵ
 * @param  Seconds������������
 * @return TRUE:�������ֵ��
 *         FALSE: û�г������ֵ
 */
bool IsRtcCurrentTimeCompliant(uint32_t *Seconds);

/**
 * @brief  ��ȡ��ǰ����Ч���Ӻš�
 * @param  NONE
 * @return ����0��ʾû����Ч���ӡ����ش���0��ֵ��ʾ����Ч����
 */
uint8_t RTC_AlarmIDGet(void);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__RTC_H__

/**
 * @}
 * @}
 */
