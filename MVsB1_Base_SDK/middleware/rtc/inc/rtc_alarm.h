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


////////////////////////////////以下是闹钟有关的API///////////////////////////////////////////////

/**
 * 定义每个闹钟需占用的memory字节数，上层代码可据此及需要支持的最大闹钟数分配memory空间
 */
#define   MEM_SIZE_PER_ALARM     10

/**
 * 定义闹钟最大数量，最多支持8组闹钟，超过8组时以前8组为准
 */
#define   gMAX_ALARM_NUM			8

/**
 * 定义闹钟状态
 */
typedef enum _RTC_ALARM_STATUS
{
	ALARM_STATUS_OPENED = 1,    /**< 闹钟打开状态  */
	ALARM_STATUS_CLOSED,        /**<闹钟关闭状态 */

} RTC_ALARM_STATUS;

/**
 * 定义闹钟模式
 */
typedef enum _RTC_ALARM_MODE
{
	ALARM_MODE_ONCE = 1,	/**< 单次闹钟 */
	ALARM_MODE_CONTINUE,	/**< 连续闹钟 */

} RTC_ALARM_MODE;

typedef struct _ALARM_TIME_INFO
{
	uint8_t AlarmHour;/**< 闹钟小时（24小时制） */
	uint8_t AlarmMin;/**< 闹钟分钟(60分钟制) */
	RTC_ALARM_STATUS  AlarmStatus : 2;/**< 闹钟状态：开、关 */
	RTC_ALARM_MODE  AlarmMode : 4;/**< 设置的闹钟模式： 单次闹钟、连续闹钟   */
	uint8_t  AlarmData; /**< 闹钟的具体日期： [bit0~bit6]分别表示周日~周六 */

} ALARM_TIME_INFO;

extern ALARM_TIME_INFO *gpAlarmList;//闹钟列表指针。
///////////////////////////////////////////////////////////
//掉电记忆相关，该部分代码仅仅保存于NVM,不会保存于FLASH
//标准SDK未实现闹钟处理，该部分代码由客户自己添加完成
typedef struct _NVM_ALARM_INFO_
{
	uint8_t RingType : 3; /**< 闹铃类型 : TNTER_RING_TYPE:内置铃声; USB_RING_TYPE: U盘铃声; SD_RING_TYPE:SD卡铃声*/
	uint8_t Duration : 2; /**< 闹铃持续时间: 0:30s; 1:1分钟; 2:2分钟; 3:3分钟 */
	uint8_t RepeatCnt: 3; /**< 闹铃重复次数 */
	uint32_t FileAddr;    /**< 文件扇区号 */
	uint8_t CRC8;         /**< 文件名校验码 */

} ALARM_RING_INFO;

/**
 * @brief  将当前最近的有效闹钟的Load到寄存器中
 * @param  pAlarmList 闹钟列表
 * @return NONE
 * #note   使用RTC闹钟功能时，初始化的时候闹钟列表需要重应用层传递，不能传递NULL；正常工作的时候更新闹钟，参数请传NULL
 */
void RTC_LoadCurrAlarm(ALARM_TIME_INFO *pAlarmList);

/**
 * @brief  设置闹钟状态
 * @param  AlarmID：闹钟号
 * @param  AlarmStatus：要设置的闹钟状态
 * @return 如果执行成功返回TRUE，否则返回FALSE。
 */
bool RTC_AlarmStatusSet(uint8_t AlarmID, uint8_t AlarmStatus);

/**
 * @brief  获取闹钟的状态。
 * @param  AlarmID：闹钟号
 * @return 返回闹钟的状态(打开/关闭)。
 */
RTC_ALARM_STATUS RTC_AlarmStatusGet(uint8_t AlarmID);

/**
 * @brief  获取闹钟的闹钟时间，闹钟模式，周闹钟信息，闹钟号
 * @param  AlarmID：闹钟号
 * @return ALARM_TIME_INFO*类型的结构体
 */
ALARM_TIME_INFO* RTC_AlarmTimeGet(uint8_t AlarmID);

/**
 * @brief  检查哪个闹钟到了。
 * @param  NONE
 * @return 返回0表示没有闹钟到。返回大于0的值表示对应的闹钟到了
 * @NONE   [bit0~bit7]分别表示第0组~第8组闹钟， 对应的bit位被置1则表示有闹钟，置0则无闹钟
 */
uint8_t RTC_IntAlarmIDGet(void);

/**
 * @brief  判断闹钟总秒数是否超过最大值
 * @param  Seconds：闹钟总秒数
 * @return TRUE:超过最大值，
 *         FALSE: 没有超过最大值
 */
bool IsRtcCurrentTimeCompliant(uint32_t *Seconds);

/**
 * @brief  获取当前的有效闹钟号。
 * @param  NONE
 * @return 返回0表示没有有效闹钟。返回大于0的值表示有有效闹钟
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
