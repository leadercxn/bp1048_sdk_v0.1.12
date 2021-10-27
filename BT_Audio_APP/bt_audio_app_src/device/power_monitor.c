///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//                       All rights reserved.
//  Filename: power_management.c

///////////////////////////////////////////////////////////////////////////////

#include "type.h"
#include "app_config.h"
#include "adc.h"
#include "clk.h"
#include "gpio.h"
#include "timeout.h"
#include "adc_key.h"
#include "debug.h"
#include "sadc_interface.h"
#include "delay.h"
#include "bt_config.h"

#ifdef CFG_FUNC_POWER_MONITOR_EN

#include "power_monitor.h"

#define LDOIN_SAMPLE_COUNT			10		//��ȡLDOIN����ʱ����ƽ���Ĳ�������
#define LDOIN_SAMPLE_PERIOD			50		//��ȡLDOIN����ʱ��ȡ����ֵ�ļ��(ms)
#define LOW_POWEROFF_TIME			10000		//�͵���ػ��������ʱ��(ms)


//���¶��岻ͬ�ĵ�ѹ����¼��Ĵ�����ѹ(��λmV)���û���������ϵͳ��ص��ص�������
#define LDOIN_VOLTAGE_FULL			4200
#define LDOIN_VOLTAGE_HIGH			3800
#define LDOIN_VOLTAGE_MID			3600
#define LDOIN_VOLTAGE_LOW			3500
#define LDOIN_VOLTAGE_OFF			3300	//���ڴ˵�ѹֵ����ػ�powerdown״̬

//��ѹ���ʱ��ͬ����ʾ����
typedef enum _PWR_MNT_DISP
{
	PWR_MNT_DISP_NONE = 0,
	PWR_MNT_DISP_CHARGE,		 
	PWR_MNT_DISP_HIGH_V, 
	PWR_MNT_DISP_MID_V, 
	PWR_MNT_DISP_LOW_V, 
	PWR_MNT_DISP_EMPTY_V, 
	PWR_MNT_DISP_SYS_OFF
	 
} PWR_MNT_DISP;

//��Ҫ���ڵ�ѹ״̬��ʾ�ı���
static TIMER BlinkTimer;
static PWR_MNT_DISP PwrMntDisp;


//���ڵ�ѹ���ı���
TIMER PowerMonitorTimer;
uint32_t LdoinSampleSum = 0; 
uint16_t  LdoinSampleCnt = LDOIN_SAMPLE_COUNT;
uint32_t LdoinLevelAverage = 0;		//��ǰLDOIN��ѹƽ��ֵ

static PWR_LEVEL PowerLevel = PWR_LEVEL_4;


#ifdef	CFG_FUNC_OPTION_CHARGER_DETECT
//Ӳ�����PC ����������״̬
//ʹ���ڲ���������PC����������ʱ������Ϊ�͵�ƽ����ʱ����Ϊ�ߵ�ƽ
bool IsInCharge(void)
{
//��Ϊ���룬��������

	GPIO_PortAModeSet(CHARGE_DETECT_GPIO, 0x0);
	GPIO_RegOneBitSet(CHARGE_DETECT_PORT_PU, CHARGE_DETECT_GPIO);
	GPIO_RegOneBitClear(CHARGE_DETECT_PORT_PD, CHARGE_DETECT_GPIO);
	GPIO_RegOneBitClear(CHARGE_DETECT_PORT_OE, CHARGE_DETECT_GPIO);
	GPIO_RegOneBitSet(CHARGE_DETECT_PORT_IE, CHARGE_DETECT_GPIO);
	WaitUs(2);
	if(GPIO_RegOneBitGet(CHARGE_DETECT_PORT_IN,CHARGE_DETECT_GPIO))
	{
		return TRUE;
	}   	

	return FALSE;
}
#endif
void PowerMonitorDisp(void)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN) ||defined(FUNC_TM1628_LED_EN))
	static uint8_t  ShowStep = 0;
	static bool IsToShow = FALSE;

	switch(PwrMntDisp)
	{
		case PWR_MNT_DISP_CHARGE:
			//��˸���ICON,��ʾ���ڳ��
			if(IsTimeOut(&BlinkTimer))
			{
				TimeOutSet(&BlinkTimer, 500);
				switch(ShowStep)
				{

					case 0:
						DispIcon(ICON_BAT1, FALSE);
						DispIcon(ICON_BAT2, FALSE);
						DispIcon(ICON_BAT3, FALSE);
						break;
					case 1:
						DispIcon(ICON_BAT1, TRUE);
						DispIcon(ICON_BAT2, FALSE);
						DispIcon(ICON_BAT3, FALSE);
						break;
					case 2:
						DispIcon(ICON_BAT1, TRUE);
						DispIcon(ICON_BAT2, TRUE);
						DispIcon(ICON_BAT3, FALSE);
						break;
					case 3:
						DispIcon(ICON_BAT1, TRUE);
						DispIcon(ICON_BAT2, TRUE);
						DispIcon(ICON_BAT3, TRUE);
						break;
				}		
				if(ShowStep < 3)
				{
					ShowStep++;
				}
				else
				{
					ShowStep = 0;
				}
			}
			
			break;

		case PWR_MNT_DISP_HIGH_V:
			//APP_DBG("BAT FULL\n");			
			DispIcon(ICON_BATFUL, TRUE);
			DispIcon(ICON_BATHAF, FALSE);
			//������ʾ����������������ʾ����
			break;
			
		case PWR_MNT_DISP_MID_V:
			//APP_DBG("BAT HALF\n");		
			DispIcon(ICON_BATFUL, FALSE);
			DispIcon(ICON_BATHAF, TRUE);
			//������ʾ2��������������ʾ����
			break;

		case PWR_MNT_DISP_LOW_V:
			DispIcon(ICON_BATFUL, FALSE);
			DispIcon(ICON_BATHAF, TRUE);
			//������ʾ1��������������ʾ����
			break;
			
		case PWR_MNT_DISP_EMPTY_V:
			//APP_DBG("BAT EMPTY\n");				
			DispIcon(ICON_BATFUL, FALSE);

			if(IsTimeOut(&BlinkTimer))
			{
				TimeOutSet(&BlinkTimer, 300);
				if(IsToShow)
				{
					DispIcon(ICON_BATHAF, TRUE);
				}
				else
				{
					DispIcon(ICON_BATHAF, FALSE);
				}
				IsToShow = !IsToShow;
			}
			//������ʾ0��������������ʾ����
			break;
		
		case PWR_MNT_DISP_SYS_OFF:
			//APP_DBG("BAT OFF\n");
			//ClearScreen();			//�����ʾ				
			//DispString(" LO ");
			break;
			
		default:
			break;
	}
#endif	
}
//���LDOIN�ĵ�ѹֵ��ִ�ж�Ӧ����Ĵ���
//PowerOnInitFlag: TRUE--��һ���ϵ�ִ�е�Դ��ؼ��
static uint8_t PowerLdoinLevelMonitor(bool PowerOnInitFlag)
{	
	//bool PowerOffFlag = FALSE;
	uint8_t ret = 0;

	if(LdoinSampleCnt > 0)
	{
		LdoinSampleSum += SarADC_LDOINVolGet();
		LdoinSampleCnt--;
	}

	//������LDOIN_SAMPLE_COUNT��������LDOINƽ��ֵ
	if(LdoinSampleCnt == 0)
	{
		LdoinLevelAverage = LdoinSampleSum / LDOIN_SAMPLE_COUNT;

		APP_DBG("LDOin 5V Volt: %lu\n", (uint32_t)LdoinLevelAverage);

		//Ϊ�������LDOIN����ʼ������
		LdoinSampleCnt = LDOIN_SAMPLE_COUNT;
		LdoinSampleSum = 0;

#ifdef	CFG_FUNC_OPTION_CHARGER_DETECT
		if(IsInCharge())		//������Ѿ�����Ĵ���
		{		
			PowerMonitorDisp();
			return ret ;
		}
#endif
		
		if(LdoinLevelAverage > LDOIN_VOLTAGE_HIGH)	  
		{
			//������ʾ�������������PowerMonitorDisp�������ʾ����
			//PowerMonitorDisp(PWR_MNT_DISP_HIGH_V);
			PwrMntDisp = PWR_MNT_DISP_HIGH_V;
			//APP_DBG("bat full\n");

			PowerLevel = PWR_LEVEL_4;
		}

		else if(LdoinLevelAverage > LDOIN_VOLTAGE_MID)
		{
			//������ʾ2�����������PowerMonitorDisp�������ʾ����
			//PowerMonitorDisp(PWR_MNT_DISP_MID_V);
			PwrMntDisp = PWR_MNT_DISP_MID_V;

			PowerLevel = PWR_LEVEL_3;
		}
		else if(LdoinLevelAverage > LDOIN_VOLTAGE_LOW)
		{
			//������ʾ1�����������PowerMonitorDisp�������ʾ����
			//PowerMonitorDisp(PWR_MNT_DISP_LOW_V);
			//ret = PWR_MNT_LOW_V;
			PwrMntDisp = PWR_MNT_DISP_LOW_V;

			PowerLevel = PWR_LEVEL_2;
			APP_DBG("bat LOW\n");
		}
		else if(LdoinLevelAverage > LDOIN_VOLTAGE_OFF)
		{
			//������ʾ0�����������PowerMonitorDisp�������ʾ����
			//PowerMonitorDisp(PWR_MNT_DISP_EMPTY_V);
			PwrMntDisp = PWR_MNT_DISP_EMPTY_V;

			PowerLevel = PWR_LEVEL_1;
			ret = PWR_MNT_LOW_V;
		}
#if 0
		if(LdoinLevelAverage > LDOIN_VOLTAGE_HIGH)	  
		{
			PowerLevel = PWR_LEVEL_4;
		}

		else if(LdoinLevelAverage > LDOIN_VOLTAGE_MID)
		{
			PowerLevel = PWR_LEVEL_3;
		}
		else if(LdoinLevelAverage > LDOIN_VOLTAGE_LOW)
		{
			PowerLevel = PWR_LEVEL_2;
			APP_DBG("bat LOW\n");
		}
		else if(LdoinLevelAverage > LDOIN_VOLTAGE_OFF)
		{
			PowerLevel = PWR_LEVEL_1;
			ret = PWR_MNT_LOW_V;
		}
#endif
		

		if(LdoinLevelAverage <= LDOIN_VOLTAGE_OFF)
		{
			//���ڹػ���ѹ������ػ�����
			//����������ʾ�͵�ѹ��ʾ��Ȼ��ִ�йػ�����
#ifdef	CFG_FUNC_OPTION_CHARGER_DETECT			
			PwrMntDisp = PWR_MNT_DISP_SYS_OFF;
			PowerMonitorDisp();
#endif			
			//osTaskDelay(1000);
			//ֹͣ�����������̣���������ʾ����DAC���ع��ŵ�Դ�ȶ���
			APP_DBG("PowerMonitor, Low Voltage!PD.\n");	
			ret = PWR_MNT_OFF_V;
			
			//low level voltage detect in power on sequence, power down system directly
            #ifdef CFG_FUNC_BACKUP_EN
			  SystemPowerDown();
			#endif

            #ifdef CFG_FUNC_DEEPSLEEP_EN
			  //MainAppServicePause();
			  ret = 1;
            #endif

			PowerLevel = PWR_LEVEL_0;
		}

#ifdef CFG_APP_BT_MODE_EN
#if (BT_HFP_SUPPORT == ENABLE)
		SetBtHfpBatteryLevel(PowerLevel, 0);
#endif
#endif
	}
	PowerMonitorDisp();

	return ret;
}


//���ܼ��ӳ�ʼ��
//ʵ��ϵͳ���������еĵ͵�ѹ��⴦���Լ���ʼ������豸������IO��
void PowerMonitorInit(void)
{
	TimeOutSet(&PowerMonitorTimer, 0);	
	TimeOutSet(&BlinkTimer, 0);	
#ifdef CFG_FUNC_OPTION_CHARGER_DETECT
	//���ϵͳ����ʱ������豸�Ѿ����룬�Ͳ�ִ��������ε͵�ѹ���ʹ������
	if(!IsInCharge())
#endif
	{
#ifdef POWERON_DETECT_VOLTAGE	// ��������ѹ
		//ϵͳ���������еĵ͵�ѹ���
		//����ʱ��ѹ��⣬���С�ڿ�����ѹ���������豸���Ͳ������̣�ֱ�ӹػ�
		//������Ϊʱ50ms�������ж�Ӧ�Ĵ���
		while(LdoinSampleCnt)
		{
			LdoinSampleCnt--;
			LdoinSampleSum += SarADC_LDOINVolGet();
			osTaskDelay(5);
		}		
		//Ϊ��ߵ�LDOIN����ʼ������
		PowerLdoinLevelMonitor(TRUE);
#endif
	}
}

//ϵͳ��Դ״̬��غʹ���
//ϵͳ���������LDOIN���ڿ�����ѹ������ϵͳ���������м��LDOIN
uint8_t PowerMonitor(void)
{
	if(IsTimeOut(&PowerMonitorTimer))
	{
		TimeOutSet(&PowerMonitorTimer, LDOIN_SAMPLE_PERIOD);

#ifdef CFG_FUNC_OPTION_CHARGER_DETECT
		if(IsInCharge())		//������Ѿ�����Ĵ���
		{
			if(LdoinLevelAverage >= LDOIN_VOLTAGE_FULL) 
			{
				//PowerMonitorDisp(PWR_MNT_DISP_HIGH_V);		//��ʾ���״̬
				PwrMntDisp = PWR_MNT_DISP_HIGH_V;
				//APP_DBG("charger full\n");
			}
			else
			{
				//PowerMonitorDisp(PWR_MNT_DISP_CHARGE);		//��ʾ���״̬
				PwrMntDisp = PWR_MNT_DISP_CHARGE;
				//APP_DBG("charger.....\n");
			}
		}
	  else
#endif
	    {

	    }
		//û�в�����LDOIN_SAMPLE_COUNT��������������
		return PowerLdoinLevelMonitor(FALSE);
	}
   return FALSE;
}

//
//��ȡ��ǰ��ص���
//return: level(0-3)
//
PWR_LEVEL PowerLevelGet(void)
{
	return PowerLevel;
}


#endif
