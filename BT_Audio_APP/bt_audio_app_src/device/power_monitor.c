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

#define LDOIN_SAMPLE_COUNT			10		//获取LDOIN幅度时用来平均的采样次数
#define LDOIN_SAMPLE_PERIOD			50		//获取LDOIN幅度时获取采样值的间隔(ms)
#define LOW_POWEROFF_TIME			10000		//低电检测关机连续检测时间(ms)


//以下定义不同的电压检测事件的触发电压(单位mV)，用户根据自身系统电池的特点来配置
#define LDOIN_VOLTAGE_FULL			4200
#define LDOIN_VOLTAGE_HIGH			3800
#define LDOIN_VOLTAGE_MID			3600
#define LDOIN_VOLTAGE_LOW			3500
#define LDOIN_VOLTAGE_OFF			3300	//低于此电压值进入关机powerdown状态

//电压检测时不同的显示处理
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

//主要用于电压状态显示的变量
static TIMER BlinkTimer;
static PWR_MNT_DISP PwrMntDisp;


//用于电压检测的变量
TIMER PowerMonitorTimer;
uint32_t LdoinSampleSum = 0; 
uint16_t  LdoinSampleCnt = LDOIN_SAMPLE_COUNT;
uint32_t LdoinLevelAverage = 0;		//当前LDOIN电压平均值

static PWR_LEVEL PowerLevel = PWR_LEVEL_4;


#ifdef	CFG_FUNC_OPTION_CHARGER_DETECT
//硬件检测PC 或充电器连接状态
//使能内部下拉。无PC或充电器连接时，检测口为低电平，有时检测口为高电平
bool IsInCharge(void)
{
//设为输入，无上下拉

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
			//闪烁充电ICON,表示正在充电
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
			//可以显示满格电量，请添加显示代码
			break;
			
		case PWR_MNT_DISP_MID_V:
			//APP_DBG("BAT HALF\n");		
			DispIcon(ICON_BATFUL, FALSE);
			DispIcon(ICON_BATHAF, TRUE);
			//可以显示2格电量，请添加显示代码
			break;

		case PWR_MNT_DISP_LOW_V:
			DispIcon(ICON_BATFUL, FALSE);
			DispIcon(ICON_BATHAF, TRUE);
			//可以显示1格电量，请添加显示代码
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
			//可以显示0格电量，请添加显示代码
			break;
		
		case PWR_MNT_DISP_SYS_OFF:
			//APP_DBG("BAT OFF\n");
			//ClearScreen();			//清除显示				
			//DispString(" LO ");
			break;
			
		default:
			break;
	}
#endif	
}
//监测LDOIN的电压值，执行对应检测点的处理
//PowerOnInitFlag: TRUE--第一次上电执行电源监控检测
static uint8_t PowerLdoinLevelMonitor(bool PowerOnInitFlag)
{	
	//bool PowerOffFlag = FALSE;
	uint8_t ret = 0;

	if(LdoinSampleCnt > 0)
	{
		LdoinSampleSum += SarADC_LDOINVolGet();
		LdoinSampleCnt--;
	}

	//采样够LDOIN_SAMPLE_COUNT次数，求LDOIN平均值
	if(LdoinSampleCnt == 0)
	{
		LdoinLevelAverage = LdoinSampleSum / LDOIN_SAMPLE_COUNT;

		APP_DBG("LDOin 5V Volt: %lu\n", (uint32_t)LdoinLevelAverage);

		//为继续检测LDOIN，初始化变量
		LdoinSampleCnt = LDOIN_SAMPLE_COUNT;
		LdoinSampleSum = 0;

#ifdef	CFG_FUNC_OPTION_CHARGER_DETECT
		if(IsInCharge())		//充电器已经接入的处理
		{		
			PowerMonitorDisp();
			return ret ;
		}
#endif
		
		if(LdoinLevelAverage > LDOIN_VOLTAGE_HIGH)	  
		{
			//可以显示满格电量，请在PowerMonitorDisp中添加显示代码
			//PowerMonitorDisp(PWR_MNT_DISP_HIGH_V);
			PwrMntDisp = PWR_MNT_DISP_HIGH_V;
			//APP_DBG("bat full\n");

			PowerLevel = PWR_LEVEL_4;
		}

		else if(LdoinLevelAverage > LDOIN_VOLTAGE_MID)
		{
			//可以显示2格电量，请在PowerMonitorDisp中添加显示代码
			//PowerMonitorDisp(PWR_MNT_DISP_MID_V);
			PwrMntDisp = PWR_MNT_DISP_MID_V;

			PowerLevel = PWR_LEVEL_3;
		}
		else if(LdoinLevelAverage > LDOIN_VOLTAGE_LOW)
		{
			//可以显示1格电量，请在PowerMonitorDisp中添加显示代码
			//PowerMonitorDisp(PWR_MNT_DISP_LOW_V);
			//ret = PWR_MNT_LOW_V;
			PwrMntDisp = PWR_MNT_DISP_LOW_V;

			PowerLevel = PWR_LEVEL_2;
			APP_DBG("bat LOW\n");
		}
		else if(LdoinLevelAverage > LDOIN_VOLTAGE_OFF)
		{
			//可以显示0格电量，请在PowerMonitorDisp中添加显示代码
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
			//低于关机电压，进入关机流程
			//可以先行显示低电压提示，然后执行关机动作
#ifdef	CFG_FUNC_OPTION_CHARGER_DETECT			
			PwrMntDisp = PWR_MNT_DISP_SYS_OFF;
			PowerMonitorDisp();
#endif			
			//osTaskDelay(1000);
			//停止正常工作流程，包括关显示、关DAC、关功放电源等动作
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


//电能监视初始化
//实现系统启动过程中的低电压检测处理，以及初始化充电设备接入检测IO等
void PowerMonitorInit(void)
{
	TimeOutSet(&PowerMonitorTimer, 0);	
	TimeOutSet(&BlinkTimer, 0);	
#ifdef CFG_FUNC_OPTION_CHARGER_DETECT
	//如果系统启动时，充电设备已经接入，就不执行下面这段低电压检测和处理过程
	if(!IsInCharge())
#endif
	{
#ifdef POWERON_DETECT_VOLTAGE	// 开机检测电压
		//系统启动过程中的低电压检测
		//开机时电压检测，如果小于开机电压，不进入设备检测和播放流程，直接关机
		//检测过程为时50ms，并进行对应的处理
		while(LdoinSampleCnt)
		{
			LdoinSampleCnt--;
			LdoinSampleSum += SarADC_LDOINVolGet();
			osTaskDelay(5);
		}		
		//为后边的LDOIN检测初始化变量
		PowerLdoinLevelMonitor(TRUE);
#endif
	}
}

//系统电源状态监控和处理
//系统启动后如果LDOIN大于开机电压，放在系统正常运行中监测LDOIN
uint8_t PowerMonitor(void)
{
	if(IsTimeOut(&PowerMonitorTimer))
	{
		TimeOutSet(&PowerMonitorTimer, LDOIN_SAMPLE_PERIOD);

#ifdef CFG_FUNC_OPTION_CHARGER_DETECT
		if(IsInCharge())		//充电器已经接入的处理
		{
			if(LdoinLevelAverage >= LDOIN_VOLTAGE_FULL) 
			{
				//PowerMonitorDisp(PWR_MNT_DISP_HIGH_V);		//显示充电状态
				PwrMntDisp = PWR_MNT_DISP_HIGH_V;
				//APP_DBG("charger full\n");
			}
			else
			{
				//PowerMonitorDisp(PWR_MNT_DISP_CHARGE);		//显示充电状态
				PwrMntDisp = PWR_MNT_DISP_CHARGE;
				//APP_DBG("charger.....\n");
			}
		}
	  else
#endif
	    {

	    }
		//没有采样够LDOIN_SAMPLE_COUNT次数，继续采样
		return PowerLdoinLevelMonitor(FALSE);
	}
   return FALSE;
}

//
//获取当前电池电量
//return: level(0-3)
//
PWR_LEVEL PowerLevelGet(void)
{
	return PowerLevel;
}


#endif
