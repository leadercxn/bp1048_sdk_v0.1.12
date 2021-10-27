/**
 **************************************************************************************
 * @file    rtc_service_api.c
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
#include "app_config.h"
#include "debug.h"
#include "EOSDEF.h"
#include "main_task.h"
#include "irqn.h"
#include "rtc.h"
#include "rtc_timer.h"
#include "rtc_lunar.h"
#include "rtc_alarm.h"
#include "rtc_ctrl.h"
#include "rtos_api.h"
#include "timeout.h"
#ifdef CFG_FUNC_RTC_EN
uint8_t  RtcAutOutTimeCount = 0;
RTC_STATE RtcState = 0;
RTC_SUB_STATE RtcSubState = 0;
RTC_DATE_TIME gRtcTime;
TIMER RtcAutoOutTimer;
static TIMER RtcReadTimer;
uint8_t RtcFlag = 0;
uint8_t RtcMode = 1;//0 = hh:mm  ,       1 = yy:mm:dd:hh:mm 
uint8_t RtcUpdateDisplay = FALSE;
uint8_t gRtcRdWrFlag; 


__attribute__((section(".driver.isr")))void RtcInterrupt(void)
{
	if(RTC_IntFlagGet() == TRUE)
	{
#ifdef CFG_FUNC_ALARM_EN
		mainAppCt.AlarmFlag = TRUE;
		mainAppCt.AlarmID = RTC_IntAlarmIDGet();//��ǰֻ��rtcģʽ������Ч����flag��ϡ�
#endif
		RTC_IntFlagClear();
	}
}

#ifdef CFG_FUNC_ALARM_EN
///����һ ���ڶ� ������ ������ ������ ������ Sunday
/*    AlarmWdayData
 *bit : 7      6         5        4         3            2      1        0
 *week:     Saturday  Friday   Thursday  Wednesday  Tuesday   Monday  Sunday
 */
ALARM_TIME_INFO AlarmList[gMAX_ALARM_NUM] = {
/**  AlarmHour AlarmMin,	RTC_ALARM_STATUS,		RTC_ALARM_MODE,		AlarmWdayData */
	{00,		02,			ALARM_STATUS_CLOSED,	ALARM_MODE_ONCE,	 0x7f},
	{10,		26,			ALARM_STATUS_CLOSED,	ALARM_MODE_CONTINUE, 0x00},
	{10,		25,			ALARM_STATUS_CLOSED,	ALARM_MODE_CONTINUE, 0x00},
	{10,		27,			ALARM_STATUS_CLOSED,	ALARM_MODE_CONTINUE, 0x00},
	{10,		29,			ALARM_STATUS_CLOSED,	ALARM_MODE_CONTINUE, 0x00},
	{10,		27,			ALARM_STATUS_CLOSED,	ALARM_MODE_CONTINUE, 0x00},
	{10,		30,			ALARM_STATUS_CLOSED,	ALARM_MODE_CONTINUE, 0x00},
	{10,		31,			ALARM_STATUS_CLOSED,	ALARM_MODE_CONTINUE, 0x00},
};
const uint8_t ModeDisp[2][16] = 
{
    {"ALARM_ONCE_ONLY"},
    {"ALARM_CONTINUE"},
};
ALARM_TIME_INFO *gpAlarmList_temp;
RTC_DATE_TIME gAlarmTime;
uint8_t  RtcAlarmNum = 1;
uint8_t gAlarm1State, gAlarm2State;
uint8_t gAlarm1OnFlag = FALSE;
uint8_t gAlarm2OnFlag = FALSE;
uint8_t gAlarmVolume  =  CFG_PARA_SYS_VOLUME_DEFAULT;
uint8_t gAlarm1Volume = CFG_PARA_SYS_VOLUME_DEFAULT;
uint8_t gAlarm2Volume = CFG_PARA_SYS_VOLUME_DEFAULT; 
RTC_ALARM_SOURCE_STATE gAlarmSource;
RTC_ALARM_SOURCE_STATE gAlarm1Source = RTC_ALARM_SOURCE_BT;
RTC_ALARM_SOURCE_STATE gAlarm2Source = RTC_ALARM_SOURCE_BT;
#endif

static void RtcTimeDown(RTC_DATE_TIME* Time)
{
	switch(RtcSubState)
	{
#ifdef CFG_FUNC_ALARM_EN
		case RTC_SET_ALARM_NUM:
			RtcAlarmNum--;	
			if(RtcAlarmNum < 1)
			{
				RtcAlarmNum = MAX_ALARM_NUM;    
			}
			gpAlarmList_temp = AlarmList;
            gpAlarmList_temp = gpAlarmList_temp + (RtcAlarmNum - 1);
            gpAlarmList_temp = RTC_AlarmTimeGet(RtcAlarmNum);
			if(gpAlarmList_temp->AlarmHour > 23)
			{
				gpAlarmList_temp->AlarmHour = 0;
			}
			if(gpAlarmList_temp->AlarmMin > 59)
			{
				gpAlarmList_temp->AlarmMin = 0;
			}
			if((gpAlarmList_temp->AlarmMode > ALARM_MODE_CONTINUE) || (gpAlarmList_temp->AlarmMode < ALARM_MODE_ONCE))
			{
				gpAlarmList_temp->AlarmMode = ALARM_MODE_ONCE;
			}
			APP_DBG("RtcAlarmNum:%d\n", RtcAlarmNum);
			break;
       
		case RTC_SET_ALARM_MODE:
			if(gpAlarmList_temp->AlarmMode == ALARM_MODE_ONCE)
			{
				gpAlarmList_temp->AlarmMode = ALARM_MODE_CONTINUE;
			}
			else
			{
				gpAlarmList_temp->AlarmMode = ALARM_MODE_ONCE;
			}			
			APP_DBG("gAlarmMode[%u]=%s\n", gpAlarmList_temp->AlarmMode, ModeDisp[gpAlarmList_temp->AlarmMode - 1]);
			break;
		 
		 case RTC_SET_ALARM_ONOFF:
			 if(gpAlarmList_temp->AlarmStatus == ALARM_STATUS_OPEN)
			 {
				 gpAlarmList_temp->AlarmStatus = ALARM_STATUS_CLOSE;
				 APP_DBG("ALARM_STATUS_CLOSE\n");
			 }
			 else
			 {
				 gpAlarmList_temp->AlarmStatus = ALARM_STATUS_OPEN;
				 APP_DBG("ALARM_STATUS_OPEN\n");
			 }
			 break;
#endif

		case RTC_SET_YEAR:
			Time->Year--;
			if(Time->Year < RTC_START_YEAR)
			{
				Time->Year = RTC_END_YEAR;
			}
			Time->WDay = RTC_CurrWeekDay2BasedDateGet(RTC_Days2BasedDayGet(Time));
			break;
	
		case RTC_SET_MON:
			Time->Mon--;
			if(Time->Mon == 0)
			{
				Time->Mon = 12;
			}
			if(Time->Date > RTC_MonthDaysGet(Time->Year, Time->Mon))
			{
				Time->Date = RTC_MonthDaysGet(Time->Year, Time->Mon);
			}
			Time->WDay = RTC_CurrWeekDay2BasedDateGet(RTC_Days2BasedDayGet(Time));
			break;
	
		case RTC_SET_DATE:
			Time->Date--;
			if(Time->Date == 0)
			{
				Time->Date = RTC_MonthDaysGet(Time->Year, Time->Mon);
			}	
			Time->WDay = RTC_CurrWeekDay2BasedDateGet(RTC_Days2BasedDayGet(Time));
			break;
	
#ifdef CFG_FUNC_ALARM_EN	
		case RTC_SET_WEEK:
			if(Time->WDay > 0)
			{
				Time->WDay--;			
			}
			else
			{
				Time->WDay = 6;	
			}
			break;
#endif
	
		case RTC_SET_HR:
			Time->Sec = 0;	
			Time->Hour--;
			if(Time->Hour > 23)
			{
				Time->Hour = 23;
			}
			break;
	
		case RTC_SET_MIN:
			Time->Sec = 0;		
			Time->Min--;
			if(Time->Min > 59)
			{
				Time->Min = 59;
			}
			break;
#ifdef CFG_FUNC_ALARM_EN
		case RTC_SET_ALARM_SOURCE:
			if(gAlarmSource > RTC_ALARM_SOURCE_BT)
			{
				gAlarmSource--;
			}
			break;

		case RTC_SET_ALARM_VOLUME:
			if(gAlarmVolume)
			{
				gAlarmVolume--; 
			}		
			break;
#endif
		default:
			break;
	}
}


static void RtcTimeUp(RTC_DATE_TIME* Time)
{
	switch(RtcSubState)
	{
#ifdef CFG_FUNC_ALARM_EN
		case RTC_SET_ALARM_NUM:
			RtcAlarmNum++;	
			if(RtcAlarmNum > MAX_ALARM_NUM)
			{
			    RtcAlarmNum = 1;    
			}
			gpAlarmList_temp = AlarmList;
			gpAlarmList_temp = gpAlarmList_temp + (RtcAlarmNum -1);
			gpAlarmList_temp = RTC_AlarmTimeGet(RtcAlarmNum);
			if(gpAlarmList_temp->AlarmHour > 23)
			{
				gpAlarmList_temp->AlarmHour = 0;
			}
			if(gpAlarmList_temp->AlarmMin > 59)
			{
				gpAlarmList_temp->AlarmMin = 0;
			}
			if((gpAlarmList_temp->AlarmMode > ALARM_MODE_CONTINUE) || (gpAlarmList_temp->AlarmMode < ALARM_MODE_ONCE))
			{
				gpAlarmList_temp->AlarmMode = ALARM_MODE_ONCE;
			}
			APP_DBG("RtcAlarmNum:%d\n", RtcAlarmNum);
			break;

		case RTC_SET_ALARM_MODE:
			if(gpAlarmList_temp->AlarmMode == ALARM_MODE_ONCE)
			{
				gpAlarmList_temp->AlarmMode = ALARM_MODE_CONTINUE;
			}
			else
			{
				gpAlarmList_temp->AlarmMode = ALARM_MODE_ONCE;
			}
			APP_DBG("gAlarmMode[%u]=%s\n", gpAlarmList_temp->AlarmMode,ModeDisp[gpAlarmList_temp->AlarmMode-1]);
			break;
		
		case RTC_SET_ALARM_ONOFF:	
			if(gpAlarmList_temp->AlarmStatus == ALARM_STATUS_CLOSED)
			{
				gpAlarmList_temp->AlarmStatus = ALARM_STATUS_OPENED;
				APP_DBG("ALARM_STATUS_OPEN\n");
			}
			else
			{
				gpAlarmList_temp->AlarmStatus = ALARM_STATUS_CLOSED;
				APP_DBG("ALARM_STATUS_CLOSE\n");
			}
   		 	break;
#endif

		case RTC_SET_YEAR:
			APP_DBG("Time->Year:%u\n", (int)Time->Year);
			Time->Year++;
			if(Time->Year > RTC_END_YEAR)
			{
				Time->Year = RTC_START_YEAR;
			}
			Time->WDay = RTC_CurrWeekDay2BasedDateGet(RTC_Days2BasedDayGet(Time));
			break;
	
		case RTC_SET_MON:
			Time->Mon++;
			if(Time->Mon > 12)
			{
				Time->Mon = 1;
			}
			if(Time->Date > RTC_MonthDaysGet(Time->Year, Time->Mon))
			{
				Time->Date = RTC_MonthDaysGet(Time->Year, Time->Mon);
			}
			Time->WDay = RTC_CurrWeekDay2BasedDateGet(RTC_Days2BasedDayGet(Time));
			break;
	
		case RTC_SET_DATE:
			Time->Date++;
			if(Time->Date > RTC_MonthDaysGet(Time->Year, Time->Mon))
			{
				Time->Date = 1;
			}
			Time->WDay = RTC_CurrWeekDay2BasedDateGet(RTC_Days2BasedDayGet(Time));
			break;

#ifdef CFG_FUNC_ALARM_EN	
		case RTC_SET_WEEK:
			if(Time->WDay < 6)
			{
				Time->WDay++;			
			}
			else
			{
				Time->WDay = 0;	
			}
			break;
#endif
	
		case RTC_SET_HR:
			Time->Sec = 0;
			Time->Hour++;
			if(Time->Hour > 23)
			{
				Time->Hour = 0;
			}
			break;
	
		case RTC_SET_MIN:
			Time->Sec = 0;
			Time->Min++;
			if(Time->Min > 59)
			{
				Time->Min = 0;
			}
			break;
#ifdef CFG_FUNC_ALARM_EN
		case RTC_SET_ALARM_SOURCE:
			if(gAlarmSource < RTC_ALARM_ONOFF)
			{
				gAlarmSource++;
			}
			break;

		case RTC_SET_ALARM_VOLUME:
			if(gAlarmVolume < CFG_PARA_MAX_VOLUME_NUM)
			{
				gAlarmVolume++;	
			}		
			break;
#endif
		default:
			break;
	}
}

static void RtcNextSubState(void)
{
	switch(RtcSubState)
	{
#ifdef CFG_FUNC_ALARM_EN
		case RTC_SET_ALARM_NUM:
			APP_DBG("RTC_SET_ALARM_MODE:%2X\n", gpAlarmList_temp->AlarmMode);
			RtcSubState = RTC_SET_ALARM_MODE;
			break;

		case RTC_SET_ALARM_MODE:
			switch(gpAlarmList_temp->AlarmMode)
			{
			    case 1:
			        APP_DBG("RTC_SET_YEAR\n");
			        RtcSubState = RTC_SET_YEAR;
			        break;
			        
			    case 2:
			        APP_DBG("RTC_SET_HR\n");
			        RtcSubState = RTC_SET_HR;
			        break;
			        
			    case 3:
			        APP_DBG("RTC_SET_WEEK\n");
			        RtcSubState = RTC_SET_WEEK;
			        break;
			        
			    default:
			        break;   
			}
			break;
		
		case RTC_SET_ALARM_ONOFF:			
			RtcState = RTC_STATE_SET_SOURCE;
			break;
#endif

		case RTC_SET_YEAR:
			APP_DBG("RTC_SET_MON\n");
			RtcSubState = RTC_SET_MON;
			break;
	
		case RTC_SET_MON:
			APP_DBG("RTC_SET_DATE\n");
			RtcSubState = RTC_SET_DATE;
			break;
	
		case RTC_SET_DATE:
			APP_DBG("RTC_SET_HR\n");
			RtcSubState = RTC_SET_HR;
			break;

		case RTC_SET_WEEK:
			APP_DBG("RTC_SET_HR\n");
			RtcSubState = RTC_SET_HR;
			break;

		case RTC_SET_HR:
			APP_DBG("RTC_SET_MIN\n");
			RtcSubState = RTC_SET_MIN;
			break;

		case RTC_SET_MIN:
#ifdef CFG_FUNC_ALARM_EN
			if(RtcState == RTC_STATE_SET_ALARM)
			{
				RtcSubState = RTC_SET_ALARM_SOURCE;
				if(RtcAlarmNum == 1)
				{
					gAlarmSource = gAlarm1Source;
				}
				else 
				{
					gAlarmSource = gAlarm2Source;
				}
				break;
			}
#endif
			RtcSubState = RTC_SET_NONE;
			RtcAutOutTimeCount = 0;
			break;
#ifdef CFG_FUNC_ALARM_EN
		case RTC_SET_ALARM_SOURCE:
			if(RtcAlarmNum == 1)
			{
				gAlarm1Source = gAlarmSource;
				gAlarmVolume = gAlarm1Volume;
			}
			else 
			{
				gAlarm2Source = gAlarmSource;
				gAlarmVolume = gAlarm2Volume;
			}
			
			RtcSubState = RTC_SET_ALARM_VOLUME;
			break;

		case RTC_SET_ALARM_VOLUME:
			if(RtcAlarmNum == 1)
			{
				gAlarm1Volume = gAlarmVolume;
			}
			else 
			{
				gAlarm2Volume = gAlarmVolume;
			}			
			RtcSubState = RTC_SET_NONE;
			break;
#endif
		case RTC_SET_NONE:
		default:
			if(RtcState == RTC_STATE_SET_TIME)
			{
				if(RtcMode)
				{
					APP_DBG("RTC_SET_YEAR\n");
					RtcSubState = RTC_SET_YEAR;
				}
				else
				{
					APP_DBG("RTC_SET_HR\n");
					RtcSubState = RTC_SET_HR;
				}
				
			}
#ifdef CFG_FUNC_ALARM_EN
			else if(RtcState == RTC_STATE_SET_ALARM)
			{
				APP_DBG("RTC_SET_ALARM_NUM\n");
				APP_DBG("RtcAlarmNum:%d\n", RtcAlarmNum);
				APP_DBG("gAlarmMode:%d\n", gpAlarmList_temp->AlarmMode);
				RtcSubState = RTC_SET_ALARM_NUM;				
			}
#endif	
			break;
	}
}


#ifdef CFG_FUNC_LUNAR_EN
static void DisplayLunarDate(void)
{
	//ũ���������
	const uint8_t  LunarYearName[12][2] = {"��", "ţ", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��"};
	
	//ũ���·�����
	const uint8_t  LunarMonthName[12][2] = {"��", "��", "��", "��", "��", "��", "��", "��", "��", "ʮ", "��", "��"};
	
	//ũ����������
	const uint8_t  LunarDateName[30][4] = {"��һ", "����", "����", "����", "����", "����", "����", "����", "����", "��ʮ",
									  "ʮһ", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "��ʮ",
									  "إһ", "إ��", "إ��", "إ��", "إ��", "إ��", "إ��", "إ��", "إ��", "��ʮ"};
	
	//ũ���������
	const uint8_t  HeavenlyStemName[10][2] = {"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"};
	
	//ũ����֧����
	const uint8_t  EarthlyBranchName[12][2] = {"��", "��", "��", "î", "��", "��", "��", "δ", "��", "��", "��", "��"};
	
	RTC_LUNAR_DATE LunarDate;

	LunarDate = RTC_SolarToLunar(&gRtcTime);	
	APP_DBG("ũ�� %d�� ", (uint16_t)LunarDate.Year);
	APP_DBG("%-.2s%-.2s�� ", HeavenlyStemName[RTC_HeavenlyStemGet(LunarDate.Year)], 
						  EarthlyBranchName[RTC_EarthlyBranchGet(LunarDate.Year)]);
	APP_DBG("%-.2s�� ", LunarYearName[RTC_EarthlyBranchGet(LunarDate.Year)]);
	if(LunarDate.IsLeapMonth)
	{
		APP_DBG("��");
	}
	APP_DBG("%-.2s��", LunarMonthName[LunarDate.Month - 1]);
	
	if(LunarDate.MonthDays == 29)
	{
		APP_DBG("(С)");
	}
	else
	{
		APP_DBG("(��)");
	}
	
	APP_DBG("%-.4s ", LunarDateName[LunarDate.Date - 1]);

	if((LunarDate.Month == 1) && (LunarDate.Date == 1))			//����
	{
		APP_DBG("����");
	}
	else if((LunarDate.Month == 1) && (LunarDate.Date == 15))	//Ԫ����
	{
		APP_DBG("Ԫ����");
	}
	else if((LunarDate.Month == 5) && (LunarDate.Date == 5))	//�����
	{
		APP_DBG("�����");
	}
	else if((LunarDate.Month == 7) && (LunarDate.Date == 7))	//��Ϧ���˽�
	{
		APP_DBG("��Ϧ���˽�");
	}
	else if((LunarDate.Month == 7) && (LunarDate.Date == 15))	//��Ԫ��
	{
		APP_DBG("��Ԫ��");
	}
	else if((LunarDate.Month == 8) && (LunarDate.Date == 15))	//�����
	{
		APP_DBG("�����");
	}
	else if((LunarDate.Month == 9) && (LunarDate.Date == 9))	//������
	{
   		APP_DBG("������");
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == 8))	//���˽�
	{
	 	APP_DBG("���˽�");
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == 23))	//С��
	{
		APP_DBG("С��");
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == LunarDate.MonthDays))	//��Ϧ
	{
		APP_DBG("��Ϧ");
	}
}
#endif

void RtcDisplay(void)
{
	static uint8_t TempSec = -1;
	
	//Display RTC time
	if(IsTimeOut(&RtcReadTimer))
	{
		TimeOutSet(&RtcReadTimer, 500);
		//RtcAutOutTimeCount++;
		if(RtcState != RTC_STATE_SET_TIME)
		{
			gRtcTime = RTC_DateTimerGet();
		}
		if(gRtcTime.Sec != TempSec)
		{
			TempSec = gRtcTime.Sec;
			RtcUpdateDisplay = TRUE;
			#ifdef CFG_FUNC_SNOOZE_EN
			if(mainAppCt.SnoozeOn)
			{
				mainAppCt.SnoozeCnt++;
			}
			#endif
		}
	}

	if(RtcUpdateDisplay)
	{
		RtcUpdateDisplay = FALSE;

		if(RtcFlag == 1)
		{
			//YYYY-MM-DD(W) HH:MM:SS
			APP_DBG("%d-%-.2d-%-.2d(����%d) ",
			(uint16_t)gRtcTime.Year,
			(uint8_t)gRtcTime.Mon,
			(uint8_t)gRtcTime.Date,
			(uint8_t)(gRtcTime.WDay));

			#ifdef CFG_FUNC_LUNAR_EN
			DisplayLunarDate();
			#endif

			APP_DBG(" %-.2d:%-.2d:%-.2d  ",
			(uint8_t)gRtcTime.Hour,
			(uint8_t)gRtcTime.Min,
			(uint8_t)gRtcTime.Sec);
			APP_DBG("\n");
		}
#if 0//def CFG_FUNC_ALARM_EN
		else if(RtcFlag == 2)
		{
			switch(gpAlarmList_temp->AlarmMode)
			{
				case ALARM_MODE_ONCE:
					APP_DBG("����ģʽ:����(once only) ");
					break;

				case ALARM_MODE_CONTINUE:
					APP_DBG("����ģʽ:ÿ��һ��(every day)");
					break;

				default:
					APP_DBG("ģʽ����(mode error)\n");
					break;
			}
			APP_DBG(" %-.2d:%-.2d  ", (WORD)gpAlarmList_temp->AlarmHour, (WORD)gpAlarmList_temp->AlarmMin);
			APP_DBG("\n");
			APP_DBG("RtcAlarmNum = :%2BX\n",RtcAlarmNum);
		}		
#endif
	}
}


bool CheckTimeIsValid(RTC_DATE_TIME* Time)
{
	bool TimeModifyFlag = TRUE;
	
	if((Time->Year > RTC_END_YEAR) || (Time->Year < RTC_START_YEAR))
	{
		Time->Year = 1980;
		TimeModifyFlag = FALSE;
	}
	
	if((Time->Mon > 12) || (Time->Mon == 0))
	{
		Time->Mon = 1;
		TimeModifyFlag = FALSE;
	}
	
	if((Time->Date == 0) || (Time->Date > RTC_MonthDaysGet(Time->Year, Time->Mon)))
	{
		Time->Date = 1;
		TimeModifyFlag = FALSE;
	}

	if(Time->WDay > 6)
	{
		Time->WDay = RTC_CurrWeekDay2BasedDateGet(RTC_Days2BasedDayGet(Time));
		TimeModifyFlag = FALSE;
	}

	if(Time->Hour > 23)
	{
		Time->Hour = 0;
		TimeModifyFlag = FALSE;
	}

	if(Time->Min > 59)
	{
		Time->Min  = 0;
		TimeModifyFlag = FALSE;
	}

	if(Time->Sec > 59)
	{
		Time->Sec = 0;
		TimeModifyFlag = FALSE;
	}

	return TimeModifyFlag;
}


#ifdef CFG_FUNC_ALARM_EN
bool CheckAlarmTime(uint8_t AlarmNum, uint8_t AlarmMode, RTC_DATE_TIME* Time)
{
	if(RTC_AlarmStatusGet(AlarmNum) == 1) 
	{
		APP_DBG("�����ɹرյ���,ע���������ʱ��\n");
		if((AlarmMode <= ALARM_MODE_CONTINUE) && (AlarmMode >= ALARM_MODE_ONCE))
		{
			gpAlarmList_temp->AlarmMode = AlarmMode;
		}
		else
		{
			gpAlarmList_temp->AlarmMode = ALARM_MODE_ONCE;
		}
	}
	
	if(AlarmMode == ALARM_MODE_ONCE)
	{
		if(CheckTimeIsValid(Time) == FALSE)
		{
			APP_DBG("����ʱ��Ƿ��Ѿ���\n");
		}
	}	
	else if(AlarmMode == ALARM_MODE_CONTINUE)
	{
		Time->Year = 1980;
		Time->Mon = 1;
		if(Time->WDay > 6)
		{			
			Time->WDay = RTC_CurrWeekDay2BasedDateGet(RTC_Days2BasedDayGet(Time));
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}
#endif


static uint8_t RtcAlarmSetWaitTime(void)
{
	if(RtcAutOutTimeCount == RTC_AUTO_OUT_TIME_COUNT)
	{
		TimeOutSet(&RtcAutoOutTimer, RTC_AUTO_OUT_TIME_UNIT);
		RtcAutOutTimeCount--;
	}
	else if(IsTimeOut(&RtcAutoOutTimer))
	{
		if(RtcAutOutTimeCount > 0)
		{
			TimeOutSet(&RtcAutoOutTimer, RTC_AUTO_OUT_TIME_UNIT);
			RtcAutOutTimeCount--;
		}
		else
		{
			RtcState = RTC_STATE_IDLE;
			RtcSubState = RTC_SET_NONE;
			return TRUE;
		}
	}
	return FALSE;;
}

//����кʹ˺��������÷�(Ƕ�뵽����ģʽ)�ĺ���λ����󣬴˺�����Ҫ��default����ֹ���غ������õ���Ϣ
void RtcMsgPro(MessageContext MsgId)
{	
	uint32_t Msg = MsgId.msgId;

	if(RtcState == RTC_STATE_IDLE)
	{		
		switch(Msg)
		{	
			case MSG_RTC_SET_TIME:
				APP_DBG("RTC_SET\n");
				RtcFlag = 1;
				RtcUpdateDisplay = TRUE;
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				RtcState = RTC_STATE_SET_TIME;
				RtcSubState = RTC_SET_NONE;
				RtcNextSubState();
				break;

			#ifdef CFG_FUNC_ALARM_EN
			case MSG_RTC_SET_ALARM:
				APP_DBG("ALARM_SET\n");
				RtcFlag = 2;
				RtcUpdateDisplay = TRUE;
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				if(RtcState != RTC_STATE_SET_ALARM)
				{
					RtcSubState = RTC_SET_NONE;
					gpAlarmList_temp = AlarmList;
					RtcAlarmNum = 1;
					gpAlarmList_temp = RTC_AlarmTimeGet(RtcAlarmNum);
					if(gpAlarmList_temp->AlarmHour > 23)
					{
						gpAlarmList_temp->AlarmHour = 0;
					}
					if(gpAlarmList_temp->AlarmMin > 59)
					{
						gpAlarmList_temp->AlarmMin = 0;
					}
					if(gpAlarmList_temp->AlarmMode > ALARM_MODE_CONTINUE)
					{
						gpAlarmList_temp->AlarmMode = ALARM_MODE_ONCE;
					}
					gAlarmTime.Hour = gpAlarmList_temp->AlarmHour;
					gAlarmTime.Min = gpAlarmList_temp->AlarmMin;
				}
			
				if((RtcAlarmNum > 0) && (RtcAlarmNum <= MAX_ALARM_NUM))
				{
					//gpAlarmList_temp->AlarmMode = GetAlarmTime(RtcAlarmNum, &gAlarmTime);
					//if((gpAlarmList_temp->AlarmMode < ALARM_MODE_ONCE_ONLY) || (gpAlarmList_temp->AlarmMode > ALARM_MODE_PER_WEEK))
					//{
					//	gpAlarmList_temp->AlarmMode = ALARM_MODE_PER_DAY;	
					//}
					CheckAlarmTime(RtcAlarmNum, gpAlarmList_temp->AlarmMode, &gAlarmTime);									
				}
				else
				{
					APP_DBG("���Ӵ���\n");
				}
				
				RtcState = RTC_STATE_SET_ALARM;
				RtcSubState = RTC_SET_NONE;
				RtcNextSubState();
				break;
			#endif					
			default:
				break;
		}
	}
	else if(RtcState == RTC_STATE_SET_TIME)
	{
		switch(Msg)
		{
			case MSG_RTC_DOWN:
				APP_DBG("RTC_DOWN\n");
				RtcUpdateDisplay = TRUE;
				gRtcTime = RTC_DateTimerGet();
				RtcTimeDown(&gRtcTime);
				RTC_DateTimerSet(&gRtcTime);
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				break;

			case MSG_RTC_UP:
				APP_DBG("RTC_UP\n");
				RtcUpdateDisplay = TRUE;
				gRtcTime = RTC_DateTimerGet();
				RtcTimeUp(&gRtcTime);
				RTC_DateTimerSet(&gRtcTime);
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				break;

			case MSG_RTC_SET_TIME:
				APP_DBG("MSG_RTC_SET_TIME\n");
				if(RtcSubState == RTC_SET_MIN)
				{
					RtcState = RTC_STATE_IDLE;
					RtcSubState = RTC_SET_NONE;
					RtcAutOutTimeCount = 0;
					break;
				}
				RtcUpdateDisplay = TRUE;
				RtcNextSubState();
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				break;

			default:
				break;
		}
	}
#ifdef CFG_FUNC_ALARM_EN
	else if(RtcState == RTC_STATE_SET_ALARM)
	{
		switch(Msg)
		{
			case MSG_RTC_DOWN:
				APP_DBG("ALARM RTC_DOWN\n");
				RtcUpdateDisplay = TRUE;      				
				RtcTimeDown(&gAlarmTime);
				CheckAlarmTime(RtcAlarmNum, gpAlarmList_temp->AlarmMode, &gAlarmTime);
				gpAlarmList_temp->AlarmHour = gAlarmTime.Hour;
				gpAlarmList_temp->AlarmMin  = gAlarmTime.Min;
				RTC_LoadCurrAlarm(AlarmList);
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
//#ifdef FUNC_BREAK_POINT_EN
//				BP_SaveInfo((BYTE*)&gBreakPointInfo.PowerMemory.AlarmList2Store, sizeof(gBreakPointInfo.PowerMemory.AlarmList2Store));
//#endif
				break;

			case MSG_RTC_UP:
				APP_DBG("ALARM RTC_UP\n");
				RtcUpdateDisplay = TRUE;               				
				RtcTimeUp(&gAlarmTime);
				CheckAlarmTime(RtcAlarmNum, gpAlarmList_temp->AlarmMode, &gAlarmTime);
				gpAlarmList_temp->AlarmHour = gAlarmTime.Hour;
				gpAlarmList_temp->AlarmMin  = gAlarmTime.Min;
				RTC_LoadCurrAlarm(AlarmList);
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
//#ifdef FUNC_BREAK_POINT_EN
//				BP_SaveInfo((BYTE*)&gBreakPointInfo.PowerMemory.AlarmList2Store, sizeof(gBreakPointInfo.PowerMemory.AlarmList2Store));
//#endif
				break;

			case MSG_RTC_SET_ALARM:
				APP_DBG("MSG_RTC_SET_ALARM\n");

				if(RtcSubState == RTC_SET_ALARM_VOLUME)
				{					
					RtcState = RTC_STATE_IDLE;
					RtcSubState = RTC_SET_NONE;
					RtcAutOutTimeCount = 0;
					break;
				}
				RtcUpdateDisplay = TRUE;
				RtcNextSubState();		
				RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
				break;

			default:
				break;
		}
	}
#endif
}

void RTC_ServiceInit(uint16_t RstFflag)
{
	APP_DBG("Rtc init------------------\n");
	RtcState = RTC_STATE_IDLE;
	RtcSubState = RTC_SET_NONE;
	RtcUpdateDisplay = FALSE;
	RtcFlag = 0;
	RtcAutOutTimeCount = 0;
	TimeOutSet(&RtcReadTimer, 0);
    #ifdef CFG_FUNC_ALARM_EN
    gpAlarmList = AlarmList;
    gpAlarmList_temp = AlarmList;
    #endif
    if(RstFflag==0) return;//not power on

	gRtcTime.Year = 2001;
	gRtcTime.Mon  = 1;
	gRtcTime.Date  = 1;
	gRtcTime.Hour = 12;
	gRtcTime.Min = 0;
	gRtcTime.Sec = 0;
	RTC_DateTimerSet(&gRtcTime);

	gRtcTime = RTC_DateTimerGet();
	APP_DBG("%d  ,%d  ,%d  \n",(uint16_t)gRtcTime.Year,gRtcTime.Mon,gRtcTime.Date);

	#ifdef CFG_FUNC_ALARM_EN
	//init all alarm//
	mainAppCt.AlarmFlag = FALSE;
	mainAppCt.AlarmID = 0;
	gpAlarmList = AlarmList;
	gpAlarmList_temp = AlarmList;

	gAlarmTime.Hour = 0;
	gAlarmTime.Min = 0;
	gAlarmTime.Sec = 0;
	gpAlarmList_temp->AlarmHour = gAlarmTime.Hour;
	gpAlarmList_temp->AlarmMin  = gAlarmTime.Min;
	gpAlarmList_temp->AlarmMode = ALARM_MODE_ONCE;
	RTC_LoadCurrAlarm(gpAlarmList_temp);
	#endif
}

void RTC_AlarmParameterInit(void)
{
#ifdef CFG_FUNC_ALARM_EN

#endif
}
/*
 * SelSet: 0 = set rtc time , 1 = set rtc alarm, 2 = display current time
 * mode:   0 = hh:mm  ,       1 = yy:mm:dd:hh:mm 
 */
void RTC_ServiceModeSelect(uint8_t mode, uint8_t SelSet)
{
	 MessageContext Msg;

	 RtcMode = mode;
	 if(SelSet == 0 ) //SET RTC
	 {
		 if(RtcState == RTC_STATE_SET_ALARM)
		 {
			 RtcState =  RTC_STATE_IDLE;
	     }
		 
		 Msg.msgId  = MSG_RTC_SET_TIME;		 
		 RtcMsgPro(Msg);
	 }
	 else if(SelSet == 1 ) //SET ALARM
	 {
		 if(RtcState == RTC_STATE_SET_TIME)
		 {
			 RtcState =  RTC_STATE_IDLE;
	     }
		 Msg.msgId  = MSG_RTC_SET_ALARM;
		 RtcMsgPro(Msg);
	 }
	 else 
	 {
		 APP_DBG("RTC display current time!\n");
		 RtcFlag = 1;
		 RtcAutOutTimeCount = RTC_AUTO_OUT_TIME_COUNT;
	 }
}

void RTC_RtcUp(void)
{
	 MessageContext Msg;//

	 if(RtcState == RTC_STATE_IDLE) return;

	 Msg.msgId  = MSG_RTC_UP;

	 RtcMsgPro(Msg);

}

void RTC_RtcDown(void)
{
	 MessageContext Msg;

	 if(RtcState == RTC_STATE_IDLE) return;

	 Msg.msgId  = MSG_RTC_DOWN;

	 RtcMsgPro(Msg);
}

RTC_STATE RTC_GetCurrServiceMode(void)
{
	return RtcState;
}

RTC_SUB_STATE RTC_GetCurrSetState(void)
{
	return RtcSubState;
}

void RTC_SetCurrtime(RTC_DATE_TIME *time)
{
	RTC_DateTimerSet(time);
}

void RTC_GetCurrtime(RTC_DATE_TIME *time)
{
	*time = RTC_DateTimerGet();
}

void RTC_SetCurralarm(ALARM_TIME_INFO *alarm, uint8_t alarm_id)
{
#ifdef CFG_FUNC_ALARM_EN
	gpAlarmList_temp = AlarmList;
	AlarmList[alarm_id].AlarmHour = alarm->AlarmHour;
	AlarmList[alarm_id].AlarmMin = alarm->AlarmMin;
	AlarmList[alarm_id].AlarmStatus = alarm->AlarmStatus;
	AlarmList[alarm_id].AlarmMode = alarm->AlarmMode;
	AlarmList[alarm_id].AlarmData = alarm->AlarmData;
	RTC_LoadCurrAlarm(gpAlarmList_temp);
#endif
}

void RTC_GetCurralarm(ALARM_TIME_INFO *alarm, uint8_t alarm_id)
{

#ifdef CFG_FUNC_ALARM_EN
    uint8_t AlarmStatus;
	gpAlarmList_temp = AlarmList;
    gpAlarmList_temp = gpAlarmList_temp + alarm_id;
    gpAlarmList_temp = RTC_AlarmTimeGet(alarm_id+1);
	if(gpAlarmList_temp->AlarmHour > 23)
	{
		gpAlarmList_temp->AlarmHour = 0;
	}
	if(gpAlarmList_temp->AlarmMin > 59)
	{
		gpAlarmList_temp->AlarmMin = 0;
	}
	if(gpAlarmList_temp->AlarmMode > ALARM_MODE_CONTINUE)
	{
		gpAlarmList_temp->AlarmMode = ALARM_MODE_ONCE;
	}
	AlarmStatus = RTC_AlarmStatusGet(alarm_id+1);

	alarm->AlarmHour   = AlarmList[alarm_id].AlarmHour;/**< ����Сʱ��24Сʱ�ƣ� */
	alarm->AlarmMin    = AlarmList[alarm_id].AlarmMin;/**< ���ӷ���(60������) */
	alarm->AlarmStatus = AlarmStatus;/**< ����״̬�������� */
	alarm->AlarmMode   = AlarmList[alarm_id].AlarmMode;/**< ���õ�����ģʽ�� �������ӡ���������   */
	alarm->AlarmData   = AlarmList[alarm_id].AlarmData; /**< ���ӵľ������ڣ� [bit0~bit6]�ֱ��ʾ����~���� */
#endif
}

void RtcStateCtrl(void)
{
#ifdef CFG_FUNC_ALARM_EN
	MessageContext		msgSend;

	if(RtcSubState == RTC_SET_NONE)
	{
		//Check alarm
		if(mainAppCt.AlarmFlag)
		{
			mainAppCt.AlarmFlag = FALSE;

			if(mainAppCt.AlarmID < gMAX_ALARM_NUM)
		    {
		    	//if(AlarmList[mainAppCt.AlarmID-1].AlarmStatus == ALARM_STATUS_OPENED)
		    	{		    		
		    		if(mainAppCt.AlarmID == 1)
					{
						gAlarm1OnFlag = TRUE;
						APP_DBG("\nRTC ALARM1 COME!\n");
					}
					else
					{
						gAlarm2OnFlag = TRUE;
						APP_DBG("\nRTC ALARM2 COME!\n");
					}
		    		RTC_LoadCurrAlarm(NULL);
					mainAppCt.AlarmRemindStart = 1;
					mainAppCt.AlarmRemindCnt = 10;
					msgSend.msgId		= MSG_RTC_ALARMING;
			   		MessageSend(GetMainMessageHandle(), &msgSend);
		    	}
		    }
//#ifdef FUNC_BREAK_POINT_EN
//			BP_SaveInfo((BYTE*)&gBreakPointInfo.PowerMemory.AlarmList2Store, sizeof(gBreakPointInfo.PowerMemory.AlarmList2Store));
//#endif			
		}
		#ifdef CFG_FUNC_SNOOZE_EN
		if(mainAppCt.SnoozeOn)
		{
			if(mainAppCt.SnoozeCnt >= 60*5)//̰˯5����ʱ�䵽
			{
				APP_DBG("\nSNOOZE COME!\n");
				mainAppCt.AlarmRemindStart = 1;
				mainAppCt.AlarmRemindCnt = 10;
				mainAppCt.SnoozeOn = 0;
				mainAppCt.SnoozeCnt = 0;
				msgSend.msgId		= MSG_RTC_ALARMING;
			   	MessageSend(GetMainMessageHandle(), &msgSend);
			}
		}
		#endif
	}
#endif

	if(TRUE == RtcAlarmSetWaitTime())
	{
		if(RtcFlag)
		{
			APP_DBG("EXIT RTC_STATE_SET_TIME!\n");
			RtcState = RTC_STATE_IDLE;
			RtcSubState = RTC_SET_NONE;
			RtcFlag = 0;
		}
	}
	
	//Display time
	RtcDisplay();
}
#endif /// end of 	#ifdef  CFG_FUNC_RTC_EN
