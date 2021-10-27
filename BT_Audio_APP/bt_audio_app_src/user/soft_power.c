#include <stdint.h>
#include "gpio.h"
#include "irqn.h"
//#include "config_opus.h"
//#include "code_key.h"
//#include "key_process.h"
#include "timeout.h"
#include "hw_interface.h"
#include "watchdog.h"
//#include "key_comm.h"
//#include "gpio_key.h"
#include "io_key.h"
#include "app_config.h"
#include "app_message.h"
#include "dac.h"
#include "delay.h"

#ifdef CFG_SOFT_POWER_KEY_EN
//#include "adc_key.h"

/*************************************************
* 外部软开关电路功能初始化配置函数
*
* 注：此功能需要打开CFG_GPIO_KEY_EN；    
***************************************************/
void SoftPowerInit(void)
{
	MUTE_ON();
	POWER_OFF(); 
	#if CFG_LED_EN
	POWER_LED_OFF(); 
	#endif
#ifdef CFG_RES_IO_KEY_SCAN
	IOKeyInit();
#endif
}

/*************************************************
* 外部软开关电路功能处理函数
*
* 注：1.此功能需要打开CFG_GPIO_KEY_EN； 
      2.此函数在系统初始化时调用
***************************************************/
void WaitSoftKey(void)
{
#ifdef CFG_RES_IO_KEY_SCAN//bkd add
	IOKeyMsg ioKeyMsg;
#endif
	MessageId KeyMsg = MSG_NONE;

#if 1
    while(1)
	{
		WDG_Feed();
		WaitMs(1000);//长按1S开机
		if(!GPIO_RegOneBitGet(POWER_KEY_IN,POWER_KEY_PIN))
		{
			POWER_ON();
			#if CFG_LED_EN
			POWER_LED_ON(); 
			#endif
			//APP_DBG("power ON\n");
			break;
		}
	}
#else
	while(1)
	{
		WDG_Feed();
#ifdef CFG_RES_IO_KEY_SCAN
		ioKeyMsg = IOKeyScan();
		if(ioKeyMsg.index != IO_CHANNEL_EMPTY && ioKeyMsg.type != IO_KEY_UNKOWN_TYPE)
		{
			SetGlobalKeyValue(ioKeyMsg.type,ioKeyMsg.index);
		}
#endif

#if CFG_ADC_KEY_EN
        //AdcKeyProcess();
#endif
		KeyMsg = GetGlobalKeyValue();

		ClrGlobalKeyValue();

		if(KeyMsg == MSG_SOFT_POWER)
		{
			POWER_ON();
			#if CFG_LED_EN
			POWER_LED_ON(); 
			#endif
			APP_DBG("power ON\n");
			break;
		}
	}
#endif
}

/*************************************************
*  关机处理函数
*
*
***************************************************/
void SoftKeyPowerOff(void)
{
	uint32_t msg;
	APP_DBG("power off\n");
	MUTE_ON();
	WaitMs(10);
#if CFG_DAC0_EN
	AudioDAC_DigitalMute(DAC0,1,1);
#endif
#if CFG_DAC1_EN
	AudioDAC_DigitalMute(DAC1,1,1);
#endif 			 

	//LedDisplayOff();

//#ifdef CFG_FUNC_BREAKPOINT_EN
//	BreakPointSave(BACKUP_SYS_INFO,0);
//#endif

	while(1)
	{
#if CFG_WATCH_DOG_EN
	WDG_Feed();
#endif
#if CFG_LED_EN        
	POWER_LED_OFF();
#endif
	POWER_OFF();	
	}
}
#endif



