/**
 *******************************************************************************
 * @file    clk.h
 * @brief	Clock driver interface
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2015-11-05 10:46:11$
 *
 * @Copyright (C) 2015, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 ******************************************************************************* 
 */
 
 
/**
 * @addtogroup CLOCK
 * @{
 * @defgroup clk clk.h
 * @{
 */
 
#ifndef __CLK_H__
#define __CLK_H__
 
#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "spi_flash.h"
/**
 * CLK module switch macro define
 */
typedef enum __CLOCK_MODULE1_SWITCH
{
	MCLK_PLL_CLK0_EN = (1 << 0),		/**<MCLK PLL0 module clk switch */
	MCLK_PLL_CLK1_EN = (1 << 1),		/**<MCLK PLL1 module clk switch */
	FLASH_CONTROL_PLL_CLK_EN = (1 << 2),/**<Flash Control PLL module clk switch */
	SPDIF_PLL_CLK_EN = (1 << 3),  		/**<SPDIF PLL module clk switch */
	USB_UART_PLL_CLK_EN = (1 << 4),		/**<USB/UART PLL module clk switch */
	AUDIO_DAC0_CLK_EN = (1 << 5),		/**<Audio DAC port0 module clk switch */
	AUDIO_DAC1_CLK_EN = (1 << 6),       /**<Audio DAC port1 module clk switch */
	AUDIO_ADC0_CLK_EN = (1 << 7),		/**<Audio ADC port0 module clk switch */
	AUDIO_ADC1_CLK_EN = (1 << 8),       /**<Audio ADC port1 module clk switch */
	I2S0_CLK_EN = (1 << 9),				/**<I2S port0 module clk switch */
	I2S1_CLK_EN = (1 << 10),			/**<I2S port1 module clk switch */
	PPWM_CLK_EN = (1 << 11),			/**<PPWM module clk switch */
	FLASH_CONTROL_CLK_EN = (1 << 12),	/**<Flash control module clk switch */
	
	USB_CLK_EN = (1 << 14),           	/**<USB module clk switch */
	UART0_CLK_EN = (1 << 15),           /**<UART0 module clk switch */
	UART1_CLK_EN = (1 << 16),           /**<UART1 module clk switch */
	DMA_CLK_EN = (1 << 17),   			/**<DMA control module clk switch */
	SPDIF_CLK_EN = (1 << 18),           /**<SPDIF module clk switch */
	FFT_CLK_EN = (1 << 19),            	/**<FFT module clk switch */
	ADC_CLK_EN = (1 << 20),				/**<sarADC module clk switch */
	EFUSE_CLK_EN = (1 << 21),          	/**<EFUSE module clk switch */
	I2C_CLK_EN = (1 << 22),          	/**<I2C module clk switch */
	SPIM_CLK_EN = (1 << 23),          	/**<SPI master module clk switch */
	SDIO_CLK_EN = (1 << 24),          	/**<SDIO module clk switch */
	ROM_CLK_EN = (1 << 25),          	/**<ROM module clk switch */
	TIMER1_CLK_EN = (1 << 26),          /**<TIMER1 module clk switch */
	TIMER2_CLK_EN = (1 << 27),          /**<TIMER2 module clk switch */
	TIMER3_CLK_EN = (1 << 28),          /**<TIMER3 module clk switch */
	TIMER4_CLK_EN = (1 << 29),          /**<TIMER4 module clk switch */
	TIMER5_CLK_EN = (1 << 30),          /**<TIMER5 module clk switch */
	TIMER6_CLK_EN = (1 << 31),          /**<TIMER6 module clk switch */

	ALL_MODULE1_CLK_SWITCH = (0xFFFFFFFF),/**<all module clk SWITCH*/
} CLOCK_MODULE1_SWITCH;

typedef enum __CLOCK_MODULE2_SWITCH
{
	BTDM_HCLK_EN = (1 << 0),			/**< module clk switch */
	MDM_CLK_EN = (1 << 1),				/**< module clk switch */
	BTDM_MST_CLK_EN = (1 << 2),			/**< module clk switch */
	MDM_PCLK_EN = (1 << 3),				/**< module clk switch */
	PPWM_DBL_CLK_EN = (1 << 4),			/**< module clk switch */
	ALL_MODULE2_CLK_SWITCH = (0x1F),	/**<all module clk SWITCH*/
} CLOCK_MODULE2_SWITCH;

typedef enum __CLOCK_MODULE3_SWITCH
{
	USB_REG_CLK_EN = (1 << 0),			/**< module clk switch */
	SDIO_REG_CLK_EN = (1 << 1),			/**< module clk switch */
	SPIM_REG_CLK_EN = (1 << 2),			/**< module clk switch */
	SPIS_REG_CLK_EN = (1 << 3),			/**< module clk switch */
	UART0_REG_CLK_EN = (1 << 4),		/**< module clk switch */
	UART1_REG_CLK_EN = (1 << 5),		/**< module clk switch */
	SPDIF_REG_CLK_EN = (1 << 6),		/**< module clk switch */
	ALL_MODULE3_CLK_SWITCH = (0x7F),	/**<all module clk SWITCH*/
} CLOCK_MODULE3_SWITCH;


typedef enum __CLK_MODE{
	RC_CLK_MODE,	/*RC 12M source*/
	PLL_CLK_MODE,	/*DPLL source*/
	APLL_CLK_MODE,	/*APLL source*/
	SYSTEM_CLK_MODE /*System Clock*/
}CLK_MODE;

typedef enum _CLK_32K_MODE{
	RC_CLK32_MODE,			/*RC32K(RC 12M div) source*/
	OSC_32K_MODE,			/*OSC32K source(HOSC_DIV_32K || LOSC_32K)*/
	HOSC_DIV_32K_CLK_MODE,	/*HOSC DIV source*/
	LOSC_32K_MODE,			/*LOSC 32K source*/
}CLK_32K_MODE;


typedef enum __PLL_CLK_INDEX{
	PLL_CLK_1 = 0,	/**PLL分频时钟器1,软件配置生成为11.2896M，软件可以修改*/
	PLL_CLK_2		/**PLL分频时钟器2,软件配置生成为12.288M，软件可以修改*/
}PLL_CLK_INDEX;

typedef enum __AUDIO_MODULE{
	AUDIO_DAC0,
	AUDIO_DAC1,
	AUDIO_ADC0,
	AUDIO_ADC1,
	AUDIO_I2S0,
	AUDIO_I2S1,
	AUDIO_PPWM
}AUDIO_MODULE;

typedef enum __MCLK_CLK_SEL{
	PLL_CLOCK1 = 0,	/**PLL分频1时钟，软件配置PLL分频时钟生产成频率为11.2896M，或者其他频率*/
	PLL_CLOCK2,		/**PLL分频2时钟，软件配置PLL分频时钟生产成频率为12.288M，或者其他频率*/
	GPIO_IN0,		/**GPIO MCLK in0输入*/
	GPIO_IN1		/**GPIO MCLK in1输入*/
}MCLK_CLK_SEL;


typedef enum __CLOCK_GPIO_Port
{
	GPIO_CLK_OUT_A29,		/**GPIO A29输出时钟观测端口*/
	GPIO_CLK_OUT_B6,		/**GPIO B6输出时钟观测端口*/
	GPIO_CLK_OUT_B7,		/**GPIO B7输出时钟观测端口*/
}CLK_GPIO_Port;

typedef enum __CLOCK_OUT_MODE
{
	MCLK_PLL_CLK = 0,		//0:   mclk_pll_clk0
	MCLK_PLL_CLK_DIV,	//1:   mclk_pll_clk0_div
	SPDIF_CLK,			//2:   spdif_clk
	Flash_Control_CLK,	//3:   fshc_clk
	USB_CLK,			//4:   usb_clk
	APB_CLK,			//5:   apb_clk
	SPIM_CLK,			//6:   spim_clk
	SDIO_CLK,			//7:   sdio_clk
	RC_CLK,				//8:   rc_clk
	BTDM_CLK,			//9:   btdm_hclk
	HOSC_CLK_DIV,		//10:  hosc_clk_div, 晶体时钟固定2分频之后输出12M，可用于FM时钟源
	BTDM_LP_CLK,		//11:  btdm_lp_clk
	OSC_32K_CLK,		//12:  osc_32k_clk，仅仅芯片内部RTC使用，用于观测，不可用于FM时钟源
	DPLL_CLK_DIV,		//13:  pll_clk_div
	APLL_CLK_DIV		//14:  apll_clk_div
}CLOCK_OUT_MODE;

//建议MCLK0 配置为11.2896M，MCLK1配置为12.288M
#define		AUDIO_PLL_CLK1_FREQ		11289600//PLL1,11.2896MHz
#define		AUDIO_PLL_CLK2_FREQ		12288000//PLL2,12.288MHz


/**
 * @brief	系统参考时钟源配置选择
 * @param	IsOsc TURE：晶体时钟；FALSE：XIN端口外灌方波时钟
 * @param   Freq  系统参考时钟的工作频率：32.768K,1M,2M,...40M,单位HZ
 * @return	无
 * @note	如果使用PLL时钟则必须要先调用该函数
 */
void Clock_Config(bool IsOsc, uint32_t Freq);

/**
 * @brief	获取Core工作的时钟频率
 * @param	无
 * @return	Core工作频率
 * @note    Core时钟和系统时钟可以同频，此时最高运行144MHz
 */
uint32_t Clock_CoreClockFreqGet(void);

/**
 * @brief  设置Core时钟分频系数
 * @param  DivVal [1-256]，Freq = Fpll/Div
 * @return 无
 * @note    Core时钟和系统时钟可以同频，此时最高运行144MHz
 */
void Clock_CoreClkDivSet(uint32_t DivVal);

/**
 * @brief   获取Core分频系数
 * @param   无
 * @return  系统分频系数[0-255]
 * @note    Core时钟和系统时钟可以同频，此时最高运行144MHz
 */
uint32_t Clock_CoreClkDivGet(void);

/**
 * @brief	获取系统工作的时钟频率
 * @param	无
 * @return	系统工作频率
 * @note    Core时钟和系统时钟可以同频，此时最高运行144MHz
 */
uint32_t Clock_SysClockFreqGet(void);

/**
 * @brief   设置系统时钟分频系数
 * @param   DivVal [0-64]，0关闭分频功能, 1不分频
 * @return  无
 * @note    Core时钟和系统时钟可以同频，此时最高运行144MHz
 */
void Clock_SysClkDivSet(uint32_t DivVal);
    
/**
 * @brief   获取系统时钟分频系数
 * @param   NONE
 * @return  系统分频系数[1-64]
 */
uint32_t Clock_SysClkDivGet(void);

/**
 * @brief   设置APB总线分频系数，基于系统总线分频
 * @param   DivVal [2-14]
 * @return  无
 */
void Clock_ApbClkDivSet(uint32_t DivVal);

/**
 * @brief   获取APB总线分频系数，基于系统总线分频
 * @param   无
 * @return  APB总线分频系数[2-14]
 * @note	APB时钟不能大于37.5MHz
 */
uint32_t Clock_ApbClkDivGet(void);

/**
 * @brief   设置Spdif时钟分频系数，基于PLL时钟分频
 * @param   DivVal [2-32]
 * @return  无
 */
void Clock_SpdifClkDivSet(uint32_t DivVal);

/**
 * @brief   获取Spdif时钟分频系数，基于PLL时钟分频
 * @param   无
 * @return  Spdif时钟分频系数[2-32]
 */
uint32_t Clock_SpdifClkDivGet(void);

/**
 * @brief   设置USB时钟分频系数，基于PLL时钟分频
 * @param   DivVal [2-16]
 * @return  无
 * #note	
 */
void Clock_USBClkDivSet(uint32_t DivVal);

/**
 * @brief   获取USB时钟分频系数，基于PLL时钟分频
 * @param   无
 * @return  USB时钟分频系数[2-16]
 */
uint32_t Clock_USBClkDivGet(void);

/**
 * @brief  	设置SPI master工作时钟分频系数，从系统时钟分频
 * @param  	DivVal 分频系数[1-15]
 * @return  无
 * @note	SPIM时钟不高于120M,配置为1时，SPIM时钟为系统时钟，需要在SPIM接口内去降低频率
 */
void Clock_SPIMClkDivSet(uint32_t DivVal);

/**
 * @brief  获取SPI master工作时钟分频系数，从系统时钟分频
 * @param  无
 * @return  分频系数[2-15]
 */
uint32_t Clock_SPIMClkDivGet(void);

/**
 * @brief   设置SDIO0工作时钟分频系数，从系统时钟分频
 * @param   DivVal 分频系数[2-15]
 * @return  无
 * @note	SDIO时钟不能大于60MHz
 */
void Clock_SDIOClkDivSet(uint32_t DivVal);

/**
 * @brief  获取SDIO0工作时钟分频系数，从系统时钟分频
 * @param  无
 * @return  分频系数[2-15]
 */
uint32_t Clock_SDIOClkDivGet(void);

/**
 * @brief  设置RC32K分频系数，基于RC12M分频，该时钟主要给WDG使用
 * @param  DivVal 分频系数[2-2048]
 * @return  无
 */
void Clock_RC32KClkDivSet(uint32_t DivVal);

/**
 * @brief  获取RC32K分频系数，基于RC12M分频，该时钟主要给WDG使用
 * @param  无
 * @return  分频系数[2-512]
 */
uint32_t Clock_RC32KClkDivGet(void);

/**
 * @brief	设置OSC分频系数，分频之后的时钟提供给RTC使用
 * @param	DivVal 分频系数[1-2048]。
 * @return  无
 */
void Clock_OSCClkDivSet(uint32_t DivVal);

/**
 * @brief	获取OSC分频系数，分频之后的时钟提供给RTC使用
 * @param	无
 * @return  OSC分频系数[1-2048]
 */
uint32_t Clock_OSCClkDivGet(void);

/**
 * @brief	设置pll工作频率,等待pll lock
 * @param	PllFreq pll频率,单位KHz[120000K-288000K]
 * @return  PLL锁定情况  TRUE:按设定目标频率锁定
 */
bool Clock_PllLock(uint32_t PllFreq);

/**
 * @brief	设置apll工作频率,等待pll lock
 * @param	APllFreq pll频率,单位KHz[120000K-240000K]
 * @return  PLL锁定情况  TRUE:按设定目标频率锁定
 */
bool Clock_APllLock(uint32_t PllFreq);

/**
 * @brief	获取rc频率
 * @param	IsReCount 是否再次获取硬件计数器值。TRUE：再次启动硬件计数器。FALSE：获取上次记录值。
 * @return  rc频率，单位Hz
 */	
uint32_t Clock_RcFreqGet(bool IsReCount);

/**
 * @brief	获取pll频率
 * @param	无
 * @return  pll频率
 * @note    该函数调用需要应用确保pll已经lock，否则读数错误
 */
uint32_t Clock_PllFreqGet(void);

/**
 * @brief	设置apll工作频率,等待apll lock
 * @param	PllFreq pll频率,单位KHz[120000K-360000K]
 * @return  PLL锁定情况  TRUE:按设定目标频率锁定
 */
uint32_t Clock_APllFreqGet(void);

/**
 * @brief	获取pll频率
 * @param	Win 计数器窗口【3:2048】
 * @return  无
 */
void Clock_RcCntWindowSet(uint32_t Win);

/**
 * @brief	关闭rc频率硬件自动更新功能
 * @param	无
 * @return  无
 */
void Clock_RcFreqAutoCntDisable(void);

/**
 * @brief	启动rc频率硬件自动更新功能
 * @param	无
 * @return  无
 */
void Clock_RcFreqAutoCntStart(void);

/**
 * @brief	获取rc频率（硬件自动更新）
 * @param	无
 * @return  无
 */
uint32_t Clock_RcFreqAutoCntGet(void);

/**
 * @brief	设置pll工作频率,快速锁定模式，等待pll lock
 * @param	PllFreq pll频率,单位KHz[240000K-480000K]
 * @param	K1 [0-15]
 * @param	OS [0-31]
 * @param	NDAC [0-4095]
 * @param	FC [0-2]
 * @param	Slope [0-16777216]
 * @return  PLL锁定情况  TRUE:按设定目标频率锁定
 */
bool Clock_PllQuicklock(uint32_t PllFreq, uint8_t K1, uint8_t OS, uint32_t NDAC, uint32_t FC, uint32_t Slope);

/**
 * @brief	设置pll工作频率,开环锁定模式(不需要晶体，无自校准)
 * @param	PllFreq pll频率,单位KHz[240000K-480000K]
 * @param	K1 [0-15]
 * @param	OS [0-31]
 * @param	NDAC [0-4095]
 * @param	FC [0-2]
 * @return  无
 */
void Clock_PllFreeRun(uint32_t PllFreq, uint32_t K1, uint32_t OS, uint32_t NDAC, uint32_t FC);

/**
 * @brief	pll free run 一般只需要调此函数即可
 * @param	无
 * @return  TRUE, 配置成功
 */
bool Clock_PllFreeRunEfuse(void);
/**
 * @brief	pll模块关闭
 * @param	无
 * @return  无
 */
void Clock_PllClose(void);

/**
 * @brief	AudioPll模块关闭
 * @param	无
 * @return  无
 */
void Clock_AUPllClose(void);

/**
 * @brief	选择系统工作时钟
 * @param	ClkMode, RC_CLK_MODE: RC12M时钟; PLL_CLK_MODE:pll时钟;
 * @return  是否成功切换系统时钟，TRUE：工程切换；FALSE：切换时钟失败。
 * @note    系统时钟工作于pll时钟时，为pll时钟的4分频
 */
bool Clock_SysClkSelect(CLK_MODE ClkMode);

/**
 * @brief	Spdif模块时钟选择,DPLL时钟还是APLL时钟
 * @param	ClkMode	时钟源选择
 *   @arg	PLL_CLK_MODE:DPLL时钟（PLL分频之后）
 *   @arg	APLL_CLK_MODE:APLL时钟（APLL分频之后）
 * @return  无
 */
void Clock_SpdifClkSelect(CLK_MODE ClkMode);

/**
 * @brief	USB模块时钟选择,DPLL时钟还是APLL时钟
 * @param	ClkMode	时钟源选择
 *   @arg	PLL_CLK_MODE:DPLL时钟（PLL分频之后）
 *   @arg	APLL_CLK_MODE:APLL时钟（APLL分频之后）
 * @return  无
 * @note	时钟频率不要超过60M，一般设置为60M
 */
void Clock_USBClkSelect(CLK_MODE ClkMode);

/**
 * @brief	Uart模块时钟选择,pll时钟还是RC时钟
 * @param	ClkMode 时钟源选择 
 *   @arg	RC_CLK_MODE: RC时钟;
 *   @arg	PLL_CLK_MODE:DPLL时钟（PLL分频之后）
 *   @arg	APLL_CLK_MODE:APLL时钟（APLL分频之后）
 * @return  无
 * @note	注意该时钟源如果选择PLL或者APLL时钟，是和USB时钟源同源
 */
void Clock_UARTClkSelect(CLK_MODE ClkMode);

/**
 * @brief	Timer3模块时钟选择,系统时钟时钟还是RC时钟
 * @param	ClkMode SYSTEM_CLK_MODE: 系统时钟; RC_CLK_MODE:RC 12M时钟
 * @return  无
 */
void Clock_Timer3ClkSelect(CLK_MODE ClkMode);

/**
 * @brief	晶体32K时钟选择,系统时钟时钟还是RC时钟
 * @param	ClkMode 32K晶体时钟源选择
 *   @arg	HOSC_DIV_32K_CLK_MODE: 高频晶体时钟分频到32;
 *   @arg	LOSC_32K_MODE: 32K晶体时钟;
 * @return  无
 */
void Clock_OSC32KClkSelect(CLK_32K_MODE ClkMode);

/**
 * @brief	BTDM模块时钟选择,系统时钟时钟还是RC时钟
 * @param	ClkMode BTDM模块
 *   @arg	RC_CLK32_MODE: RC12M时钟分频到32K;
 *   @arg	OSC_32K_MODE: 晶体32K时钟(注意来源);
 * @return  无
 */
void Clock_BTDMClkSelect(CLK_32K_MODE ClkMode);
CLK_32K_MODE Clock_BTDMClkSelectGet( void);

/**
 * @brief	模块时钟使能
 * @param	ClkSel 模块源，根据CLOCK_MODULE1_SWITCH选择
 * @return  无
 */
void Clock_Module1Enable(CLOCK_MODULE1_SWITCH ClkSel);

/**
 * @brief	模块时钟禁能
 * @param	ClkSel 模块源，根据CLOCK_MODULE1_SWITCH选择
 * @return  无
 */
void Clock_Module1Disable(CLOCK_MODULE1_SWITCH ClkSel);

/**
 * @brief	模块时钟使能
 * @param	ClkSel 模块源，根据CLOCK_MODULE2_SWITCH选择
 * @return  无
 */
void Clock_Module2Enable(CLOCK_MODULE2_SWITCH ClkSel);

/**
 * @brief	模块时钟禁能
 * @param	ClkSel 模块源，根据CLOCK_MODULE2_SWITCH选择
 * @return  无
 */
void Clock_Module2Disable(CLOCK_MODULE2_SWITCH ClkSel);

/**
 * @brief	模块时钟使能
 * @param	ClkSel 模块源，根据CLOCK_MODULE3_SWITCH选择
 * @return  无
 */
void Clock_Module3Enable(CLOCK_MODULE3_SWITCH ClkSel);

/**
 * @brief	模块时钟禁能
 * @param	ClkSel 模块源，根据CLOCK_MODULE3_SWITCH选择
 * @return  无
 */
void Clock_Module3Disable(CLOCK_MODULE3_SWITCH ClkSel);

/**
 * @brief	配置高频晶体的内置起振电容
 * @param	XICap 晶体输入端起振电容
 * @param	XOCap 晶体输出端起振电容
 * @return  无
 */
void Clock_HOSCCapSet(uint32_t XICap, uint32_t XOCap);

/**
 * @brief	配置高频晶体的内置起振电容偏置电流
 * @param	Current 电流值，【0x0-0xF】，0xF最大
 * @return  无
 * @note	注意，电流值的调整会影响24M晶体的震荡频率
 */
void Clock_HOSCCurrentSet(uint32_t Current);

/**
 * @brief	音频PLL时钟生成
 * @param	CLK_MODE PLL源: PLL_CLK 还是 APLL_CLK
 * @param	Index PLL源，PLL_CLK_1:11.2896M;PLL_CLK_2:12.288M;
 * @param	TargetFreq 锁定的音频MCLK频率，推荐配置PLL1： 11 2896M；PLL2: 12.288M。Unit:Hz
 * @return  无
 */
void Clock_AudioPllClockSet(CLK_MODE ClkMode, PLL_CLK_INDEX Index, uint32_t TargetFreq);

/**
 * @brief	音频模块主时钟源选择
 * @param	Module 音频模块
 * @param	ClkSel 时钟来源类型选择
 * @return  无
 */
void Clock_AudioMclkSel(AUDIO_MODULE Module, MCLK_CLK_SEL ClkSel);

/**
 * @brief	音频时钟源采样率微调，PLL1和PLL2
 * @param	Index PLL源，PLL_CLK_1:11.2896M;PLL_CLK_2:12.288M;
 * @param	Sign  0：调慢；1：调快
 * @param	Ppm 时钟微调参数，为0时硬件时钟无微调功能。
 * @return  无
 */
void Clock_AudioPllClockAdjust(PLL_CLK_INDEX Index,uint8_t Sign, uint8_t Ppm);

/**
 * @brief	使用DeepSleep功能时的系统时钟选择和flash时钟选择
 * @param	ClockSelect
 * @param	FlashClockSel
 * @param	IsEnterDeepSeep  TRUE:系统进入DeepSleep  FALSE:系统不进DeepSleep
 * @return  bool  TRUE:时钟切换成功     FALSE:时钟切换失败
 */
bool Clock_DeepSleepSysClkSelect(CLK_MODE SysClockSelect, FSHC_CLK_MODE FlashClockSel, bool IsEnterDeepSeep);

/**
 * @brief	HOSC晶振关闭
 * @param	无
 * @return  无
 */
void Clock_HOSCDisable(void);

/**
 * @brief	LOSC晶振关闭
 * @param	无
 * @return  无
 */
void Clock_LOSCDisable(void);

/**
 * @brief	时钟输出到GPIO端口上
 * @param	CLK_GPIO_Port GPIO复用输出时钟端口，GPIOA29，GPIOB6，GPIOB7
 * @param	CLOCK_OUT_MODE 时钟输出的源
 * @return  无
 */
void Clock_GPIOOutSel(CLK_GPIO_Port Port, CLOCK_OUT_MODE mode);

/**
 * @brief	关闭GPIO端口上的时钟输出功能
 * @param	CLK_GPIO_Port GPIO复用输出时钟端口，GPIOA29，GPIOB6，GPIOB7
 * @return  无
 */
void Clock_GPIOOutDisable(CLK_GPIO_Port Port);

/**
 * @brief	开启BB控制HOSC的功能，默认为1
 * @param	0 : 关闭BB控制HOSC
 * @param	1 : 开启BB控制HOSC
 * @return  无
 */
void Clock_BBCtrlHOSCInDeepsleep(uint8_t set);
uint8_t Clock_BBCtrlHOSCInDeepsleepGet(void);

/**
 * @brief	HOSC对32K的分频系数
 * @param	clk_div : 分频系数值(除法)
 * @return  无
 */
void Clock_32KClkDivSet(uint32_t clk_div);
uint32_t Clock_32KClkDivGet(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif

/**
 * @}
 * @}
 */
 
