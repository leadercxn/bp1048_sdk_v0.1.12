#include "app_config.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "gpio.h"
#include "timeout.h"
#include "audio_adc.h"
#include "dac.h"
#include "clk.h"
#include "chip_info.h"
#include "otg_device_hcd.h"
#include "rtc.h"
#include "irqn.h"
#include "debug.h"
#include "adc_key.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adc_key.h"
//#include "OrioleReg.h"//for test
#include "hdmi_in_api.h"
#include "sys.h"
#include "sadc_interface.h"
#include "watchdog.h"
#include "backup.h"
#include "ir_key.h"
#include "app_message.h"
#include "reset.h"
#include "bt_stack_service.h"
#include "key.h"
#include "main_task.h"

#define DPLL_QUICK_START_SLOPE	(*(volatile unsigned long *) 0x40026028)

//DPLL 快速启动参数获取。
void Clock_GetDPll(uint32_t* NDAC, uint32_t* OS, uint32_t* K1, uint32_t* FC);

#if	defined(CFG_FUNC_DEEPSLEEP_EN) || defined(CFG_FUNC_MAIN_DEEPSLEEP_EN)
HDMIInfo  			 *gHdmiCt;

inline void GIE_ENABLE(void);
inline void SystemOscConfig(void);
inline void SleepMainAppTask(void);
void SleepAgainConfig(void);

void WakeupMain(void);

inline void SleepMain(void);
void LogUartConfig(bool InitBandRate);

#define CHECK_SCAN_TIME				5000		//醒来确认有效唤醒源 扫描限时ms。

static uint32_t sources;
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
uint32_t alarm = 0;
#endif

__attribute__((section(".driver.isr")))void WakeupInterrupt(void)
{
	sources |= Power_WakeupSourceGet();
	Power_WakeupSourceClear();

	if(Power_WakeupSourceGet() == SYSWAKEUP_SOURCE13_BT)
	{
		Power_WakeupDisable(SYSWAKEUP_SOURCE13_BT);
		NVIC_DisableIRQ(Wakeup_IRQn);
	}
}


void SystermGPIOWakeupConfig(PWR_SYSWAKEUP_SOURCE_SEL source,PWR_WAKEUP_GPIO_SEL gpio,PWR_SYSWAKEUP_SOURCE_EDGE_SEL edge)
{
	if(gpio < 32)
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   (1 << gpio));
		GPIO_RegOneBitClear(GPIO_A_OE, (1 << gpio));
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			GPIO_RegOneBitSet(GPIO_A_PU, (1 << gpio));//因为芯片的GPIO有内部上下拉电阻,选择下降沿触发时要将指定的GPIO唤醒管脚配置为上拉
			GPIO_RegOneBitClear(GPIO_A_PD, (1 << gpio));
		}
		else if(edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			GPIO_RegOneBitClear(GPIO_A_PU, (1 << gpio));//因为芯片的GPIO有内部上下拉电阻，所以选择上升沿触发时要将指定的GPIO唤醒管脚配置为下拉
			GPIO_RegOneBitSet(GPIO_A_PD, (1 << gpio));
		}
	}
	else if(gpio < 41)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   (1 << (gpio - 32)));
		GPIO_RegOneBitClear(GPIO_B_OE, (1 << (gpio - 32)));
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			GPIO_RegOneBitSet(GPIO_B_PU, (1 << (gpio - 32)));//因为芯片的GPIO有内部上下拉电阻,选择下降沿触发时要将指定的GPIO唤醒管脚配置为上拉
			GPIO_RegOneBitClear(GPIO_B_PD, (1 << (gpio - 32)));
		}
		else if( edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			GPIO_RegOneBitClear(GPIO_B_PU, (1 << (gpio - 32)));//因为芯片的GPIO有内部上下拉电阻，所以选择上升沿触发时要将指定的GPIO唤醒管脚配置为下拉
			GPIO_RegOneBitSet(GPIO_B_PD, (1 << (gpio - 32)));
		}
	}
	else if(gpio == 41)
	{

		BACKUP_WriteEnable();

		BACKUP_C0RegSet(BKUP_GPIO_C0_REG_IE_OFF, TRUE);
		BACKUP_C0RegSet(BKUP_GPIO_C0_REG_OE_OFF, FALSE);
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PU_OFF, TRUE);
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PD_OFF, FALSE);
		}
		else if( edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PU_OFF, FALSE);//因为芯片的GPIO有内部上下拉电阻，所以选择上升沿触发时要将指定的GPIO唤醒管脚配置为下拉
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PD_OFF, TRUE);
		}
		BACKUP_WriteDisable();
	}

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(source, gpio, edge);
	Power_WakeupEnable(source);

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();
}

void SystermIRWakeupConfig(IR_MODE_SEL ModeSel, IR_IO_SEL GpioSel, IR_CMD_LEN_SEL CMDLenSel)
{
	Clock_BTDMClkSelect(RC_CLK32_MODE);
	Reset_FunctionReset(IR_FUNC_SEPA);
#ifdef CFG_RES_IR_KEY_USE
	IRKeyInit();
#endif
	IR_WakeupEnable();


	if(GpioSel == IR_GPIOB6)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX6);
		GPIO_RegOneBitSet(GPIO_B_IN,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OUT, GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX6);
	}
	else if(GpioSel == IR_GPIOB7)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX7);
		GPIO_RegOneBitSet(GPIO_B_IN,   GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_OUT, GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX7);
	}
	else
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX29);
		GPIO_RegOneBitSet(GPIO_A_IN,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OUT, GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX29);
	}
	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE9_IR, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE9_IR);
}



#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
//RTC唤醒 并不会进入RTC中断
void SystermRTCWakeupConfig(uint32_t SleepSecond)
{
	//RTC_REG_TIME_UNIT start;
	RTC_ClockSrcSel(OSC_24M);
	RTC_IntDisable();
	RTC_IntFlagClear();
	RTC_WakeupDisable();

	alarm = RTC_SecGet() + SleepSecond;
	RTC_SecAlarmSet(alarm);
	RTC_WakeupEnable();
	RTC_IntEnable();

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	NVIC_EnableIRQ(Rtc_IRQn);
	NVIC_SetPriority(Rtc_IRQn, 1);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE7_RTC, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE7_RTC);
}
#endif //CFG_PARA_WAKEUP_RTC


void DeepSleepIOConfig()
{
	//cansle all IO AF
	{
		int IO_cnt = 0;
		for(IO_cnt = 0;IO_cnt < 32;IO_cnt++)
		{
			if(IO_cnt == 30)
				continue;//A30是sw口
			if(IO_cnt == 31)
				continue;//A31是sw口

			GPIO_PortAModeSet(GPIOA0 << IO_cnt,0);
		}
		for(IO_cnt = 0;IO_cnt < 8;IO_cnt++)
		{
			if(IO_cnt == 0)
				continue;//B0是sw口
			if(IO_cnt == 1)
				continue;//B1是sw口

			GPIO_PortBModeSet(GPIOB0 << IO_cnt,0);
		}
	}

	SleepAgainConfig();//配置相同
}


void SystermDeepSleepConfig(void)
{
	DeepSleepIOConfig();

	SleepMain();

#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	if(gHdmiCt == NULL)
	{
		HDMI_CEC_DDC_Init();
		gHdmiCt->hdmiReportStatus = 0;
	}
//	osTaskDelay(10);
	while(!HDMI_CEC_IsReadytoDeepSleep(10));//拉低10ms，保障后续18ms没有通信和中断响应。
#endif
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
	alarm = 0;
#else

	//Clock_LOSCDisable(); //若有RTC应用则不关闭32K晶振 BKD mark sleep
//	BACKUP_32KDisable(OSC32K_SOURCE);// bkd add

#endif
}

//启用多个唤醒源时，source通道配置灵活，但不可重复。
void WakeupSourceSet(void)
{
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
	SystermRTCWakeupConfig(CFG_PARA_WAKEUP_TIME_RTC);
#endif	
#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY)&& defined(CFG_PARA_WAKEUP_GPIO_ADCKEY)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_ADCKEY, CFG_PARA_WAKEUP_GPIO_ADCKEY, SYSWAKEUP_SOURCE_NEGE_TRIG);
#ifdef CFG_PARA_WAKEUP_SOURCE_POWERKEY
	SystermGPIOWakeupConfig(SYSWAKEUP_SOURCE6_POWERKEY, 42, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_IOKEY1) && defined(CFG_PARA_WAKEUP_GPIO_IOKEY1)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_IOKEY1, CFG_PARA_WAKEUP_GPIO_IOKEY1, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_IOKEY2) && defined(CFG_PARA_WAKEUP_GPIO_IOKEY2)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_IOKEY2, CFG_PARA_WAKEUP_GPIO_IOKEY2, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_IR) && defined(CFG_RES_IR_KEY_USE)
	SystermIRWakeupConfig(CFG_PARA_IR_SEL, CFG_RES_IR_PIN, CFG_PARA_IR_BIT);
#endif	
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_CEC, CFG_PARA_WAKEUP_GPIO_CEC, SYSWAKEUP_SOURCE_BOTH_EDGES_TRIG);
#endif	
}
void DeepSleeping(void)
{

	uint32_t GpioAPU_Back,GpioAPD_Back,GpioBPU_Back,GpioBPD_Back;

	WDG_Disable();

#ifdef CFG_APP_BT_MODE_EN
	BT_ModuleClose();
#endif
	
	GpioAPU_Back = GPIO_RegGet(GPIO_A_PU);
	GpioAPD_Back = GPIO_RegGet(GPIO_A_PD);
	GpioBPU_Back = GPIO_RegGet(GPIO_B_PU);
	GpioBPD_Back = GPIO_RegGet(GPIO_B_PD);
	SystermDeepSleepConfig();
	WakeupSourceSet();
	Power_GotoDeepSleep();
	while(!SystermWackupSourceCheck())
	{
		SleepAgainConfig();
		Power_WakeupDisable(0xff);
		WakeupSourceSet();
		Power_GotoDeepSleep();
	}
	Power_WakeupDisable(0xff);
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	HDMI_CEC_DDC_DeInit();
#endif
	//GPIO恢复上下拉
	GPIO_RegSet(GPIO_A_PU, GpioAPU_Back);
	GPIO_RegSet(GPIO_A_PD, GpioAPD_Back);
	GPIO_RegSet(GPIO_B_PU, GpioBPU_Back);
	GPIO_RegSet(GPIO_B_PD, GpioBPD_Back);
	GPIO_PortAModeSet(GPIOA30, 0x0005);//调试口 SW恢复 方便下载
	GPIO_PortAModeSet(GPIOA31, 0x0004);

	WDG_Feed();
	WakeupMain();
	SysTickInit();
	WDG_Feed();
#if defined(CFG_FUNC_DISPLAY_EN)
    DispInit(0);
#endif
}


bool SystermWackupSourceCheck(void)
{
#ifdef CFG_RES_ADC_KEY_SCAN
	AdcKeyMsg AdcKeyVal;
#endif

#ifdef CFG_PARA_WAKEUP_SOURCE_IR
	IRKeyMsg IRKeyMsg;

#endif
	TIMER WaitScan;

#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY) && defined(CFG_RES_ADC_KEY_USE)
	SarADC_Init();
	AdcKeyInit();
#endif

//********************
	//串口IO设置
	LogUartConfig(FALSE);//此处如果重配clk波特率，较为耗时，不重配。
	SysTickInit();
	APP_DBG("Scan:%x", (int)sources);
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
	if(sources & CFG_PARA_WAKEUP_SOURCE_RTC)
	{
		sources = 0;//唤醒源清零
		APP_DBG("Alarm!%d", RTC_SecGet());
		return TRUE;
	}
	else if(alarm)//避免RTC唤醒事件错失
	{
		uint32_t NowTime;
		NowTime = RTC_SecGet();
		if(NowTime + 2 + CHECK_SCAN_TIME / 1000 > alarm)//如果存在多个唤醒源，rtc可以提前唤醒()，以避免丢失
		{
			APP_DBG("Timer");
			sources = 0;//唤醒源清零
			alarm = 0;
			return TRUE;
		}
	}
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	bool CecRest = FALSE;//cec空闲状态(18ms消抖)。以免影响后续解析和唤醒。
	HDMI_HPD_CHECK_IO_INIT();
#endif

	TimeOutSet(&WaitScan, CHECK_SCAN_TIME);
	while(!IsTimeOut(&WaitScan)
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
			|| !CecRest
#endif
		)
	{
#if defined(CFG_PARA_WAKEUP_SOURCE_IR) && defined(CFG_RES_IR_KEY_USE)
		if(sources & CFG_PARA_WAKEUP_SOURCE_IR)
		{
			IRKeyMsg = IRKeyScan();			
			if(IRKeyMsg.index != IR_KEY_NONE && IRKeyMsg.type != IR_KEY_UNKOWN_TYPE)
			{
				APP_DBG("IRID:%d,type:%d\n", IRKeyMsg.index, IRKeyMsg.type);
				SetIrKeyValue(IRKeyMsg.type,IRKeyMsg.index);
#ifdef CFG_APP_REST_MODE_EN
				if(GetGlobalKeyValue() == MSG_POWER)
#else
				if(GetGlobalKeyValue() == MSG_DEEPSLEEP)
#endif
				{
					sources = 0;
					ClrGlobalKeyValue();
					return TRUE;
				}
				ClrGlobalKeyValue();
			}
		}
#endif
#ifdef CFG_PARA_WAKEUP_SOURCE_POWERKEY
		if(sources & SYSWAKEUP_SOURCE6_POWERKEY)
		{
			sources = 0;
			return TRUE;
		}
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY)  && defined(CFG_RES_ADC_KEY_USE)
		if(sources & (CFG_PARA_WAKEUP_SOURCE_ADCKEY))
		{
			AdcKeyVal = AdcKeyScan();
			if(AdcKeyVal.index != ADC_CHANNEL_EMPTY && AdcKeyVal.type != ADC_KEY_UNKOWN_TYPE)
			{
				APP_DBG("KeyID:%d,type:%d\n", AdcKeyVal.index, AdcKeyVal.type);
				SetAdcKeyValue(AdcKeyVal.type,AdcKeyVal.index);
#ifdef CFG_APP_REST_MODE_EN
				if(GetGlobalKeyValue() == MSG_POWER)
#else
				if(GetGlobalKeyValue() == MSG_DEEPSLEEP)
#endif
				{
					sources = 0;
					ClrGlobalKeyValue();
					return TRUE;
				}
				ClrGlobalKeyValue();
			}
		}
#endif
#ifdef CFG_RES_IO_KEY_SCAN
#ifdef CFG_PARA_WAKEUP_SOURCE_IOKEY1
		if(sources & CFG_PARA_WAKEUP_SOURCE_IOKEY1)
		{
			return TRUE;
		}
#endif
#ifdef CFG_PARA_WAKEUP_SOURCE_IOKEY2
		if(sources & CFG_PARA_WAKEUP_SOURCE_IOKEY2)
		{
			return TRUE;
		}
#endif
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
		HDMI_CEC_Scan(0);
		if(gHdmiCt->hdmi_poweron_flag == 1)
		{
			SoftFlagRegister(SoftFlagWakeUpSouceIsCEC);
			APP_DBG("CEC PowerOn\n");
			return TRUE;
		}
		if(IsTimeOut(&WaitScan))//超时之后等待下拉电平，同时保持scan。
		{
			CecRest = HDMI_CEC_IsReadytoDeepSleep(6);
		}
#endif

	}
	sources = 0;
//	SysTickDeInit();
	return FALSE;
}


void SleepAgainConfig(void)
{
	GPIO_RegSet(GPIO_A_IE,0x00000000);
	GPIO_RegSet(GPIO_A_OE,0x00000000);
	GPIO_RegSet(GPIO_A_OUTDS,0x00000000);//bkd GPIO_A_REG_OUTDS
	GPIO_RegSet(GPIO_A_PD,0xffffffff
#if defined(CFG_PARA_WAKEUP_GPIO_CEC) && defined(CFG_PARA_WAKEUP_SOURCE_CEC)//cec端口不做上下拉配置，需要cec状态保障。
			& ~ BIT(CFG_PARA_WAKEUP_GPIO_CEC)
#endif
			);
	GPIO_RegSet(GPIO_A_PU,0x00000000);//此时的flash的CS必须拉高0x00400000
	GPIO_RegSet(GPIO_A_ANA_EN,0x00000000);
	GPIO_RegSet(GPIO_A_PULLDOWN0,0x00000000);//bkd
	GPIO_RegSet(GPIO_A_PULLDOWN1,0x00000000);//bkd

	GPIO_RegSet(GPIO_B_IE,0x00);
	GPIO_RegSet(GPIO_B_OE,0x00); 
	GPIO_RegSet(GPIO_B_OUTDS,0x00); // bkd mark GPIO_B_REG_OUTDS
	GPIO_RegSet(GPIO_B_PD,0xff);//B2、B3下拉，B4,B5高阻 0x1cc
	GPIO_RegSet(GPIO_B_PU,0x00);//B0、B1上拉 0x03
	GPIO_RegSet(GPIO_B_ANA_EN,0x00);
	GPIO_RegSet(GPIO_B_PULLDOWN,0x00);//bkd mark GPIO_B_REG_PULLDOWN
}

#endif//CFG_FUNC_DEEPSLEEP_EN

#ifdef BT_SNIFF_ENABLE

void BtWakeupConfigUsr()//蓝牙唤醒配置入口
{
	//打开默认串口
//	Clock_UARTClkSelect(PLL_CLK_MODE);//先切换log clk。避免后续慢速处理
//	LogUartConfig(TRUE);
}
void BtDeepSleepForUsr(void)//蓝牙休眠配置入口，目前没做处理
{
//	uint32_t SLOPE, NDAC, OS, K1, FC;
	uint32_t GpioAPU_Back,GpioAPD_Back,GpioBPU_Back,GpioBPD_Back;

//	Clock_GetDPll(&NDAC, &OS, &K1, &FC);
//	SLOPE = DPLL_QUICK_START_SLOPE;//获取快速启动，参数。

	GpioAPU_Back = GPIO_RegGet(GPIO_A_PU);//保存上下拉
	GpioAPD_Back = GPIO_RegGet(GPIO_A_PD);
	GpioBPU_Back = GPIO_RegGet(GPIO_B_PU);
	GpioBPD_Back = GPIO_RegGet(GPIO_B_PD);

	GPIO_PortAModeSet(GPIOA30,0);		  //去掉Sw复用。调试口
	GPIO_PortAModeSet(GPIOA31,0);

	GIE_DISABLE();
	DeepSleepIOConfig();
	Power_DeepSleepLDO12ConfigTest(0,5,0);//进入deepsleep时电压降为1V0
	SysTickDeInit();
	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE13_BT, 0, 0);//设置蓝牙为唤醒源，无需IO所以随便填了0
	Power_WakeupEnable(SYSWAKEUP_SOURCE13_BT);

	Clock_AUPllClose();

	Clock_DeepSleepSysClkSelect(RC_CLK_MODE, FSHC_RC_CLK_MODE, TRUE);

	Clock_PllClose();

#if BT_SNIFF_CLK_SEL != BT_SNIFF_LOSC_CLK
	Clock_LOSCDisable();
#endif

#if BT_SNIFF_CLK_SEL == BT_SNIFF_RC_CLK
	//RC
	Power_HRCCtrlByHwDuringDeepSleep(1);
	Clock_HOSCDisable();
#endif

	GIE_ENABLE();

	Power_GotoDeepSleep();

	GIE_DISABLE();
#if BT_SNIFF_CLK_SEL == BT_SNIFF_RC_CLK
	Clock_Config(1, 24000000);
	Clock_RcFreqAutoCntStart();
#endif
//	Clock_PllQuicklock(288000, K1, OS, NDAC, FC, SLOPE);
	Clock_PllLock(288000);
	Clock_DeepSleepSysClkSelect(PLL_CLK_MODE,FSHC_PLL_CLK_MODE,FALSE);

	Clock_UARTClkSelect(PLL_CLK_MODE);//先切换log clk。避免后续慢速处理
	LogUartConfig(TRUE); //scan不打印时 可屏蔽
	GIE_ENABLE();

	GPIO_RegSet(GPIO_A_PU, GpioAPU_Back);
	GPIO_RegSet(GPIO_A_PD, GpioAPD_Back);
	GPIO_RegSet(GPIO_B_PU, GpioBPU_Back);
	GPIO_RegSet(GPIO_B_PD, GpioBPD_Back);
	GPIO_PortAModeSet(GPIOA30, 0x0005);//调试口 SW恢复 方便下载
	GPIO_PortAModeSet(GPIOA31, 0x0004);

	SysTickInit();

}


#endif //BT_SNIFF_ENABLE
















