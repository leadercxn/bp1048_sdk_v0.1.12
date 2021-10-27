/**
 **************************************************************************************
 * @file    rtc_ctrl.h
 * @brief
 *
 * @author
 * @version
 *
 * $Created:
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __RTC_CTRL_H__
#define __RTC_CTRL_H__

#include "app_config.h"
#include "rtc_timer.h"
#include "rtc_alarm.h"
// this calendar supports from 1980.01.01 to 2099.12.31 year
#define		RTC_START_YEAR		1980	
#define		RTC_END_YEAR		2099

//ÄÖÖÓºÅÎª 1 ~ MAX_ALARM_NUM
#define		MAX_ALARM_NUM		2


#define ALARM_STATUS_OPEN        0x01
#define ALARM_STATUS_CLOSE       0x02


// RTC control state.
typedef enum _RTC_STATE
{
	RTC_STATE_IDLE,
	RTC_STATE_SET_TIME,
	RTC_STATE_SET_ALARM,
	RTC_STATE_SET_SOURCE,
} RTC_STATE;


typedef enum _RTC_SUB_STATE
{
	RTC_SET_NONE = 0,
	RTC_SET_ALARM_NUM,
	RTC_SET_ALARM_MODE,
	RTC_SET_YEAR,
	RTC_SET_MON,
	RTC_SET_DATE,
	RTC_SET_WEEK,
	RTC_SET_HR,
	RTC_SET_MIN,
	RTC_SET_ALARM_SOURCE,
	RTC_SET_ALARM_VOLUME,
	RTC_SET_ALARM_ONOFF

} RTC_SUB_STATE;

typedef enum _RTC_SOURCE_STATE
{
	RTC_SOURCE_CLOCK,
	RTC_SOURCE_YEAR,
	RTC_SOURCE_MON,
	RTC_SOURCE_ALARM,
	RTC_SOURCE_ONOFF

} RTC_SOURCE_STATE;

typedef enum _RTC_ALARM_SOURCE_STATE
{
	RTC_ALARM_SOURCE_BT,
	RTC_ALARM_SOURCE_RADIO,
	RTC_ALARM_SOURCE_USB,
	RTC_ALARM_SOURCE_CARD,
	RTC_ALARM_SOURCE_AUX,
	RTC_ALARM_ONOFF

} RTC_ALARM_SOURCE_STATE;

#define RTC_AUTO_OUT_TIME_UNIT	          250//ms
#define RTC_AUTO_OUT_TIME_COUNT           20



extern RTC_STATE RtcState;
extern RTC_SUB_STATE RtcSubState;
extern uint8_t  RtcAutOutTimeCount;
extern RTC_DATE_TIME gRtcTime;
 

#ifdef CFG_FUNC_ALARM_EN
extern uint8_t gAlarm1State, gAlarm2State;
extern uint8_t gAlarm1OnFlag;
extern uint8_t gAlarm2OnFlag;
extern uint8_t gAlarmVolume;
extern uint8_t gAlarm1Volume;
extern uint8_t gAlarm2Volume;
extern RTC_ALARM_SOURCE_STATE gAlarmSource;
extern RTC_ALARM_SOURCE_STATE gAlarm1Source;
extern RTC_ALARM_SOURCE_STATE gAlarm2Source;
extern uint8_t RtcAlarmNum;
extern RTC_DATE_TIME gAlarmTime;
#endif

/*
* Rtc initial
*/
void RTC_ServiceInit(uint16_t RstFflag);
/*
* alarm parameter from BP INFO
*/
void RTC_AlarmParameterInit(void);
/*
 * SelSet: 0 = set time , 1 = set alarm
 * mode:   0 = hh:mm  , 1 = yy:mm:dd:hh:mm £¬2 =
 */
void RTC_ServiceModeSelect(uint8_t mode, uint8_t SelSet);
/*
 * set -
 */
void RTC_RtcDown(void);
/*
 * set +
 */
void RTC_RtcUp(void);
/*
 *YYYY-MM-DD-HH-MM-SS
 */
void RTC_SetCurrtime(RTC_DATE_TIME *time);
/*
 *YYYY-MM-DD-HH-MM-SS
 */
void RTC_GetCurrtime(RTC_DATE_TIME *time);
/*
 *alarm_id :0 ~ (gMAX_ALARM_NUM-1)
 */
void RTC_SetCurralarm(ALARM_TIME_INFO *alarm, uint8_t alarm_id);
/*
 *alarm_id :0 ~ (gMAX_ALARM_NUM-1)
 */
void RTC_GetCurralarm(ALARM_TIME_INFO *alarm, uint8_t alarm_id);

RTC_STATE RTC_GetCurrServiceMode(void);

RTC_SUB_STATE RTC_GetCurrSetState(void);
/*
 *rtc process loop
 */
void RtcStateCtrl(void);
#endif
