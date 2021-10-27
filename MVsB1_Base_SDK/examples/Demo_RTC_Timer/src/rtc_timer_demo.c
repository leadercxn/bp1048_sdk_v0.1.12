/**
 **************************************************************************************
 * @file    rtc_example.c
 * @brief   rtc example
 *
 * @author  TaoWen
 * @version V1.0.0
 *
 * $Created: 2018-10-10 13:25:00$
 *
 * @Copyright (C) 2017, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <nds32_intrinsic.h>
//#include <string.h>
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "remap.h"
#include "rtc.h"
#include "rtc_timer.h"
#include "rtc_lunar.h"
#include "rtc_alarm.h"
#include "irqn.h"
#include "chip_info.h"

extern ALARM_TIME_INFO *gpAlarmList;
ALARM_TIME_INFO AlarmList[gMAX_ALARM_NUM] = {

	/**<  AlarmHour,   AlarmMin,     RTC_ALARM_STATUS,    RTC_ALARM_MODE,          AlarmWdayData */

		{ 23,          57,            ALARM_STATUS_OPENED, ALARM_MODE_ONCE,          0x0d},//1
		{ 23,          2,            ALARM_STATUS_OPENED, ALARM_MODE_ONCE,          0x14},//2
		{ 0,          1,            ALARM_STATUS_OPENED, ALARM_MODE_ONCE,          0x4e},//3
		{ 0,          0,            ALARM_STATUS_OPENED, ALARM_MODE_ONCE,          0x1c},//4
		{ 23,          57,            ALARM_STATUS_OPENED, ALARM_MODE_CONTINUE,          0x7F},//5
		{ 23,          58,            ALARM_STATUS_OPENED, ALARM_MODE_CONTINUE,          0x1F},//6
		{ 0,          2,            ALARM_STATUS_OPENED, ALARM_MODE_CONTINUE,          0x35},//7
		{ 0,          0,            ALARM_STATUS_OPENED, ALARM_MODE_CONTINUE,          0x2d},//8
};


uint32_t SecLast = 0;
uint32_t time = 0;
uint32_t alarm =0;
static RTC_DATE_TIME stDateTimer;
static RTC_DATE_TIME LastDateTimer;
uint8_t LunarLastSecCnt = 60;
uint8_t LunarSecCnt = 60;
static RTC_LUNAR_DATE CurrentLunar;
static uint32_t AlarmID;
static bool AlarmFlag = FALSE;

__attribute__((used))
static void System_LunarDisplay(void)
{
		if( LunarSecCnt != LunarLastSecCnt)
		{
			LunarSecCnt = LunarLastSecCnt;

			//ũ���������
			const uint8_t LunarYearName[12][2] = {"��", "ţ", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��"};

			//ũ���·�����
			const uint8_t LunarMonthName[12][4] = {"һ", "��", "��", "��", "��", "��", "��", "��", "��", "ʮ", "ʮһ", "ʮ��"};

			//ũ����������
			const uint8_t LunarDateName[30][6] = {"��һ", "����", "����", "����", "����", "����", "����", "����", "����", "��ʮ", "ʮһ", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "��ʮ", "��ʮһ", "��ʮ��", "��ʮ��", "��ʮ��", "��ʮ��", "��ʮ��", "��ʮ��", "��ʮ��", "��ʮ��", "��ʮ"};

			//ũ���������
			const uint8_t HeavenlyStemName[10][2] = {"��", "��", "��","��","��","��","��","��","��","��"};

			//ũ����֧����
			const uint8_t EarthlyBranchName[12][2] = {"��", "��","��","î","��","��","��","δ","��","��","��","��"};

			RTC_LunarInfoGet(stDateTimer.Year);
			CurrentLunar = RTC_SolarToLunar(&stDateTimer);
			DBG("\nũ�� %d��", CurrentLunar.Year);
			DBG("%-.2s", HeavenlyStemName[RTC_HeavenlyStemGet(CurrentLunar.Year)]);
			DBG("%-.2s", EarthlyBranchName[RTC_EarthlyBranchGet(CurrentLunar.Year)]);
			DBG("%-.2s��", LunarYearName[RTC_EarthlyBranchGet(CurrentLunar.Year)]);

			if(CurrentLunar.IsLeapMonth)
			{
				DBG("��");
			}
			DBG("%-.4s��",LunarMonthName[CurrentLunar.Month - 1]);

			if(CurrentLunar.MonthDays == 29)
			{
				DBG("(С)");
			}
			else
			{
				DBG("(��)");
			}

			DBG(" %-.6s�� ", LunarDateName[CurrentLunar.Date - 1]);
			if( ( CurrentLunar.Month == 1 ) && ( CurrentLunar.Date == 1 ))
			{
				DBG("����");
			}
			else if( ( CurrentLunar.Month == 1 ) && ( CurrentLunar.Date == 15 ) )
			{
				DBG("Ԫ����");
			}
			else if( ( CurrentLunar.Month == 5 ) && ( CurrentLunar.Date == 5 ) )
			{
				DBG("�����");
			}
			else if( ( CurrentLunar.Month == 7 ) && ( CurrentLunar.Date == 7 ) )
			{
				DBG("��Ϧ���˽�");
			}
			else if( ( CurrentLunar.Month == 7 ) && ( CurrentLunar.Date == 15 ) )
			{
				DBG("��Ԫ��");
			}
			else if( ( CurrentLunar.Month == 8 ) && ( CurrentLunar.Date == 15 ) )
			{
				DBG("�����");
			}
			else if( ( CurrentLunar.Month == 9 ) && ( CurrentLunar.Date == 9 ) )
			{
				DBG("������");
			}
			else if( ( CurrentLunar.Month == 12 ) && ( CurrentLunar.Date == 8 ) )
			{
				DBG("���˽�");
			}
			else if( ( CurrentLunar.Month == 12 ) && ( CurrentLunar.Date == 23 ) )
			{
				DBG("С��");
			}
			else if( ( CurrentLunar.Month == 12 ) && ( CurrentLunar.Date == CurrentLunar.MonthDays ) )
			{
				DBG("��Ϧ");
			}

			DBG("\n\n");
		}
}

static void System_AlarmDisplay(void)
{
	uint8_t i;
	uint8_t temp = 0;
	uint8_t AlarmIDTemp;

	if(AlarmFlag == TRUE)
	{
		temp = AlarmID;
		DBG("there is an alarm(polling mode)********************************\n");
		for( i = 0; i < 8; i++ )
		{
			temp >>= i;
			if( temp &= 0x1 )
			{
				AlarmIDTemp = i;
				DBG("AlarmID: %d\n",(AlarmIDTemp + 1));
			}
			temp = AlarmID;
		}
		AlarmFlag = FALSE;
		/* �������µ����ӱ����ˣ�����load��������� */
		RTC_LoadCurrAlarm(NULL);
	}
}

static void System_TimerDisplay(void)
{
	//��ʾ��������ʱ�����Ϣ
	stDateTimer = RTC_DateTimerGet();
	if( LastDateTimer.Sec != stDateTimer.Sec)
	{
		LastDateTimer.Sec = stDateTimer.Sec;
		LunarLastSecCnt = stDateTimer.Sec;
		DBG("\n%04d-%02d-%02d Week:%d  %02d:%02d:%02d \n", (int)stDateTimer.Year, (int)stDateTimer.Mon, (int)stDateTimer.Date, (int)stDateTimer.WDay, (int)stDateTimer.Hour, (int)stDateTimer.Min, (int)stDateTimer.Sec);
	}
}

bool System_TimerReSet(RTC_DATE_TIME* stDateTimer)
{
	bool Flag = FALSE;

	stDateTimer->Year = 2018;//2018
	stDateTimer->Mon = 11;    //11��
	stDateTimer->Date = 15;//15��

	stDateTimer->Hour = 23;//23ʱ
	stDateTimer->Min = 56;//50��
	stDateTimer->Sec = 0;//0��

	Flag = RTC_DateTimerSet(stDateTimer);

	return Flag;
}

extern void SysTickInit(void);

int main(void)
{
	static RTC_DATE_TIME CurrTime;
	bool ReSetFlag = FALSE;

	Chip_Init(1);
	WDG_Disable();

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_APllLock(288000);
	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_UARTClkSelect(PLL_CLK_MODE);

	GPIO_PortAModeSet(GPIOA24, 1);//Rx, A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx, A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);

	SysTickInit();

	Remap_InitTcm(0, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	GIE_ENABLE();	//�������ж�

	//DMA_ChannelAllocTableSet(DmaChannelMap);

	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                    RTC_Timer Example                    |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n\n");

    DBG("RTC Version: %s\n", GetLibVersionRTC());

    RTC_ClockSrcSel(OSC_24M);//�˺����Ĳ���ѡ����������ϵͳ��ʼ��ѡ��ľ��񣨡�Clock_Config()��������һ��
	RTC_IntDisable();//Ĭ�Ϲر�RTC�ж�
	RTC_IntFlagClear();
	RTC_WakeupDisable();
    GIE_ENABLE();	//�������ж�
	RTC_IntEnable();
	NVIC_EnableIRQ(Rtc_IRQn);

	GPIO_PortAModeSet(GPIOA16, 0x3); //HOSC OUT
	GPIO_PortAModeSet(GPIOA0, 0x3); //LOSC OUT

    ReSetFlag = System_TimerReSet(&CurrTime);

	/* load��������� */
	RTC_LoadCurrAlarm((ALARM_TIME_INFO *)&AlarmList);
    if( ( ReSetFlag == TRUE) )
    {
		LastDateTimer = RTC_DateTimerGet();
		DBG("\nthere is timer display \n");
		while(1)
		{
			//��ʾ��������ʱ�����Ϣ
			System_TimerDisplay();

			//��ʾũ��������Ϣ
			//System_LunarDisplay();

			//���ӵ�����ʾ������Ϣ
			System_AlarmDisplay();
		}
    }
	return 0;
}

//������Ҫ��RTC�жϺ�������������жϱ�־���������
__attribute__((section(".driver.isr")))void RtcInterrupt(void)
{
	if(RTC_IntFlagGet() == TRUE)
	{
        AlarmID = RTC_IntAlarmIDGet();
		RTC_IntFlagClear();
		AlarmFlag = TRUE;
	}
}

