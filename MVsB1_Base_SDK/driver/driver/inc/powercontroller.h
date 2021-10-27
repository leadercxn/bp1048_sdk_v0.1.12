/**
 *******************************************************************************
 * @file    powercontroller.h
 * @brief	powercontroller module driver interface

 * @author  Sean
 * @version V1.0.0

 * $Created: 2017-11-13 16:51:05$
 * @Copyright (C) 2017, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *******************************************************************************
 */

/**
 * @addtogroup POWERCONTROLLER
 * @{
 * @defgroup powercontroller powercontroller.h
 * @{
 */
 
#ifndef __POWERCONTROLLER_H__
#define __POWERCONTROLLER_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

typedef enum __PWR_SYSWAKEUP_SOURCE_SEL{
	SYSWAKEUP_SOURCE0_GPIO = (1 << 0),				/**<system wakeup source0 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE1_GPIO = (1 << 1),				/**<system wakeup source1 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE2_GPIO = (1 << 2),				/**<system wakeup source2 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE3_GPIO = (1 << 3),				/**<system wakeup source3 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE4_GPIO = (1 << 4),				/**<system wakeup source4 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE5_GPIO = (1 << 5),				/**<system wakeup source5 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE6_POWERKEY = (1 << 6),				/**<system wakeup source6 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE7_RTC = (1 << 7),				/**<system wakeup source7 macro,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE8_LVD = (1 << 8),				/**<system wakeup source8 macro,一般不用,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE9_IR = (1 << 9),				/**<system wakeup source9 macro,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE10_TIMER3_INT = (1 << 10),        /**<system wakeup source10 macro,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE11_UART0_RX = (1 << 11),          /**<system wakeup source11 macro,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE12_UART1_RX = (1 << 12),          /**<system wakeup source12 macro,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE13_BT = (1 << 13)				/**<system wakeup source13 macro,should be waked up by rise edge*/

}PWR_SYSWAKEUP_SOURCE_SEL;

typedef enum __PWR_WAKEUP_GPIO_SEL{
	WAKEUP_GPIOA0 = 0,          /**<wakeup by GPIOA0 macro*/
	WAKEUP_GPIOA1,              /**<wakeup by GPIOA1 macro*/
	WAKEUP_GPIOA2,              /**<wakeup by GPIOA2 macro*/
	WAKEUP_GPIOA3,              /**<wakeup by GPIOA3 macro*/
	WAKEUP_GPIOA4,              /**<wakeup by GPIOA4 macro*/
	WAKEUP_GPIOA5, 				/**<wakeup by GPIOA5 macro*/
	WAKEUP_GPIOA6, 				/**<wakeup by GPIOA6 macro*/
	WAKEUP_GPIOA7, 				/**<wakeup by GPIOA7 macro*/
	WAKEUP_GPIOA8,			    /**<wakeup by GPIOA8 macro*/
	WAKEUP_GPIOA9, 				/**<wakeup by GPIOA9 macro*/
	WAKEUP_GPIOA10, 			/**<wakeup by GPIOA10 macro*/
	WAKEUP_GPIOA11, 			/**<wakeup by GPIOA11 macro*/
	WAKEUP_GPIOA12, 			/**<wakeup by GPIOA12 macro*/
	WAKEUP_GPIOA13, 			/**<wakeup by GPIOA13 macro*/
	WAKEUP_GPIOA14, 			/**<wakeup by GPIOA14 macro*/
	WAKEUP_GPIOA15, 			/**<wakeup by GPIOA15 macro*/
	WAKEUP_GPIOA16,				/**<wakeup by GPIOA16 macro*/
	WAKEUP_GPIOA17,			    /**<wakeup by GPIOA17 macro*/
	WAKEUP_GPIOA18,				/**<wakeup by GPIOA18 macro*/
	WAKEUP_GPIOA19,             /**<wakeup by GPIOA19 macro*/
	WAKEUP_GPIOA20,			    /**<wakeup by GPIOA20 macro*/
	WAKEUP_GPIOA21, 			/**<wakeup by GPIOA21 macro*/
	WAKEUP_GPIOA22,				/**<wakeup by GPIOA22 macro*/
	WAKEUP_GPIOA23,				/**<wakeup by GPIOA23 macro*/
	WAKEUP_GPIOA24, 			/**<wakeup by GPIOA24 macro*/
	WAKEUP_GPIOA25,		    	/**<wakeup by GPIOA25 macro*/
	WAKEUP_GPIOA26,			    /**<wakeup by GPIOA26 macro*/
	WAKEUP_GPIOA27,             /**<wakeup by GPIOA27 macro*/
	WAKEUP_GPIOA28,             /**<wakeup by GPIOA28 macro*/
	WAKEUP_GPIOA29,             /**<wakeup by GPIOA29 macro*/
	WAKEUP_GPIOA30,             /**<wakeup by GPIOA30 macro*/
	WAKEUP_GPIOA31,             /**<wakeup by GPIOA31 macro*/
	WAKEUP_GPIOB0,              /**<wakeup by GPIOB0 macro*/
	WAKEUP_GPIOB1,              /**<wakeup by GPIOB1 macro*/
	WAKEUP_GPIOB2,              /**<wakeup by GPIOB2 macro*/
	WAKEUP_GPIOB3,              /**<wakeup by GPIOB3 macro*/
	WAKEUP_GPIOB4,              /**<wakeup by GPIOB4 macro*/
	WAKEUP_GPIOB5,              /**<wakeup by GPIOB5 macro*/
	WAKEUP_GPIOB6,              /**<wakeup by GPIOB6 macro*/
	WAKEUP_GPIOB7,              /**<wakeup by GPIOB7 macro*/
	WAKEUP_GPIOB8,              /**<wakeup by GPIOB8 macro*/
	WAKEUP_GPIOC0               /**<wakeup by GPIOC0 macro*/

}PWR_WAKEUP_GPIO_SEL;

typedef enum __PWR_SYSWAKEUP_SOURCE_EDGE_SEL{
	SYSWAKEUP_SOURCE_NEGE_TRIG = 0,  /**negedge trigger*/
	SYSWAKEUP_SOURCE_POSE_TRIG,       /**posedge trigger*/
	SYSWAKEUP_SOURCE_BOTH_EDGES_TRIG       /**both edges trigger*/
	                                 /**只在SourSel为[0~6]才有上升沿或者下降沿触发之分，其他SourSel都是默认上升沿触发*/

}PWR_SYSWAKEUP_SOURCE_EDGE_SEL;

typedef enum __PWR_LVD_Threshold_SEL{
	PWR_LVD_Threshold_2V3 = 0,  /**LVD threshold select:2.3V */
	PWR_LVD_Threshold_2V4,       /**LVD threshold select:2.4V*/
	PWR_LVD_Threshold_2V5,       /**LVD threshold select:2.5V*/
	PWR_LVD_Threshold_2V6,       /**LVD threshold select:2.6V*/
	PWR_LVD_Threshold_2V7,       /**LVD threshold select:2.7V*/
	PWR_LVD_Threshold_2V8,       /**LVD threshold select:2.8V*/
	PWR_LVD_Threshold_2V9,       /**LVD threshold select:2.9V*/
	PWR_LVD_Threshold_3V0,       /**LVD threshold select:3.0V*/

}PWR_LVD_Threshold_SEL;

/**
 * @brief  系统进入sleep模式
 * @param  无
 * @return 无
 */
void Power_GotoSleep(void);

/**
 * @brief  系统进入deepsleep模式
 * @param  无
 * @return 无
 */
void Power_GotoDeepSleep(void);

/**
 * @brief  配置DeepSleep唤醒源
 * @param  SourSel 配置唤醒源,分别为：GPIO，PowerKey,RTC,LVD(一般不用),IR,BT为蓝牙事件唤醒
 * @param  Gpio   配置gpio唤醒引脚，[0~41],分别对应GPIOA[31:0]、GPIOB[8:0]、GPIOC[0]
 * @param  Edge   gpio的边沿触发方式选择：1上升沿0下降沿
 * @note   参数gpio，    只在SourSel为GPIO才有效
 *         参数edge，    只在SourSel为GPIO和PowerKey才有上升沿或者下降沿触发之分，其他SourSel都是默认上升沿触发
 * @return 无
 */
void Power_WakeupSourceSet(PWR_SYSWAKEUP_SOURCE_SEL SourSel, PWR_WAKEUP_GPIO_SEL Gpio, PWR_SYSWAKEUP_SOURCE_EDGE_SEL Edge);

/**
 * @brief  设置某个通道唤醒使能
 * @param  SourSel 唤醒源:GPIO，PowerKey,RTC,LVD(一般不用),IR,BT为蓝牙事件唤醒
 * @return 无
 */
void Power_WakeupEnable(PWR_SYSWAKEUP_SOURCE_SEL SourSel);

/**
 * @brief  设置某个通道唤醒禁止
 * @param  SourSel 唤醒源:GPIO，PowerKey,RTC,LVD(一般不用),IR,BT为蓝牙事件唤醒
 * @return 无
 */
void Power_WakeupDisable(PWR_SYSWAKEUP_SOURCE_SEL SourSel);

/**
 * @brief  获取唤醒通道标志
 * @param  无
 * @return 获取唤醒通道标志
 */
uint32_t Power_WakeupSourceGet(void);

/**
 * @brief  清除唤醒通道标志
 * @param  无
 * @return 无
 */
void Power_WakeupSourceClear(void);

/**
 * @brief  通过唤醒源标志查询gpio唤醒引脚：[0~41],分别对应GPIOA[31:0]、GPIOB[8:0]、GPIOC[0]
 * @param  SourSel为所获取的唤醒源，分别为：GPIO，PowerKey,RTC,LVD(一般不用),IR,Timer3中断（定时或者PWC），UART0或者UART1
 * @return gpio唤醒引脚
 * @note   必须先获取唤醒源
 *         只在SourSel为GPIO才有效
 */
uint32_t Power_WakeupGpioGet(PWR_SYSWAKEUP_SOURCE_SEL SourSel);

/**
 * @brief  通过唤醒源标志查询gpio唤醒的触发边沿
 * @param  SourSel:
 * @return 唤醒源的触发边沿      0：下降沿； 1：上升沿
 * @note   必须先获取唤醒源
 *         只在SourSel为GPIO和PowerKey才有上升沿或者下降沿触发之分，其他SourSel都是默认上升沿触发
 */
uint8_t Power_WakeupEdgeGet(PWR_SYSWAKEUP_SOURCE_SEL SourSel);

/**
 * @brief  配置限流值
 * @param  Mode = 1，限流值280mA； Mode = 0，限流值60mA
 * @return 无
 */
void Power_CurrentLimitcConfig(uint32_t Mode);

/**
 * @brief  配置限流值
 * @param  PowerMode: 1高电压输入，大于3.6V; 0:低于3.6V
 * @param  IsChipWork:
 * @return 无
 */
void Power_PowerModeConfig(uint32_t PowerMode, bool IsChipWork);

/**
 * @brief  设置Rtc32K闹钟
 * @param  alarm:闹钟时间（单位秒）
 * @param  start:时钟计数初值：单位秒，一般为0
 * @param  stop时钟计数初值：单位秒，一般为0
 * @return 无
 */
void Power_AlarmSet(uint32_t alarm,uint32_t start,uint32_t stop);

/**
 * @brief  配置underpower的LDO电压需要调用此函数
 * @param  src:systerm core src base address0
 * @param  dest:systerm core dest base address0
 * @param  size:systerm core base address0 map size
 * @return 无
 */
void Power_FlashRemap2Sram(uint32_t src, uint32_t dest, uint32_t size);

/**
 * @brief  配置underpower的LDO电压需要调用此函数
 * @param  TRUE:进入DeepSleep关闭RC时钟; FALSE:进入DeepSleep不会关闭RC时钟
 * @return 无
 */
void Power_DeepSleepLDO12Config(bool IsAudoCutRcClk);

/**
 * @brief  配置LDO12电压需要调用此函数
 * @param  value:单位：mV；
 *               范围：1000mV~1300mV
 * @return 无
 * @note  芯片个体之间会有稍许差异和偏差，偏差范围在0mV~9mV之间；
 */
void Power_LDO12Config(uint32_t value);

/**
 * @brief  配置LDO_33D电压需要调用此函数
 * @param  value:单位：mV；
 *               范围：2930mV~3350mV
 * @return 无
 * @note  芯片个体之间会有稍许差异和偏差，偏差范围在0mV~40mV之间；
 */
void Power_LDO33DConfig(uint32_t value);

/**
 * @brief  配置LDO_33A电压需要调用此函数
 * @param  value:单位：mV；
 *               范围：2950mV~3600mV
 * @return 无
 * @note  芯片个体之间会有稍许差异和偏差，偏差范围在0mV~20mV之间；
 * @note  33A电压调整之后对33D电压有影响
 */
void Power_LDO33AConfig(uint32_t value);

/**
 * @brief  使能LVD低电压检测（VIN电压）
 * @param  无
 * @return 无
 * @note   无
 */
void Power_LVDEnable(void);

/**
 * @brief  禁能LVD低电压检测（VIN电压）
 * @param  无
 * @return 无
 * @note   无
 */
void Power_LVDDisable(void);

/**
 * @brief  LVD低电压检测的水位选择（VIN电压）
 * @param  Lvd_Threshold_Sel  水位选择档位[2.3~3.0]（默认水位为2.3V）
 * @return 无
 * @note   无
 */
void Power_LVDThresholdSel(PWR_LVD_Threshold_SEL Lvd_Threshold_Sel);

/**
 * @brief  使用DeepSleep功能时配置LVD低电压检测的水位（VIN电压）
 * @param  Lvd_Threshold_Sel  LVD低电压检测的水位选择（默认水位为2.3V）
 * @return 无
 * @note   无
 */
void Power_LVDWakeupConfig(PWR_LVD_Threshold_SEL Lvd_Threshold_Sel);

/**
 * @brief  使用DeepSleep功能时决定是否开启LPM模式
 * @param  IsLDO33A_LPM  TRUE:开启LDO33A的LPM模式；FALSE:默认值（开启LDO33A的HPM模式）
 * @param  IsLDO33D_LPM  TRUE:开启LDO33D的LPM模式；FALSE:默认值（开启LDO33D的HPM模式）
 * @param  IsLDO12_LPM   TRUE:开启LDO12的LPM模式；FALSE:默认值（开启LDO12的HPM模式）
 * @return 无
 * @note   无
 */
void Power_PowerModeConfigTest(bool IsLDO33A_LPM,bool IsLDO33D_LPM, bool IsLDO12_LPM);

/**
 * @brief  使用DeepSleep功能时调节LDO12的电压
 * @param  LDO1V2_HV_Val  LDO12往高电压方向调节的参数(只有1个档位，即1增加1V)
 * @param  LDO1V2_LV_Val  LDO12往低电压方向调节的参数(0~15共16个档位，电压按照线性降低，每档位降低40mV)
 * @param  LDO1V2_TRIM_Val  LDO12电压会向两个方向变化；0~31按照线性降低，每个档位3mV；32电压最大，32~63按照线性降低，每个档位3mV；
 * @return 无
 * @note   无
 */
void Power_DeepSleepLDO12ConfigTest(uint8_t LDO1V2_LV_Val, uint8_t LDO1V2_HV_Val, uint8_t LDO1V2_TRIM_Val);

/**
 * @brief   Is HRC run during deepsleep?
 * @param   IsOpen  TRUE: Run HRC during deepsleep
 *                  FALSE: Not run HRC during deepsleep
 * @return TRUE:配置成功
 *         FALSE:配置失败
 * @note   若返回值是FALSE则说明前面已经配置的唤醒源中有需要用到HRC的Source,所以禁止关闭
 */
bool Power_HRCCtrlByHwDuringDeepSleep(bool IsOpen);

/**
 * @brief  Close RF3V3D for DeepSleep
 * @param  无
 * @return 无
 * @note   无
 */
void RF_PowerDown(void);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__POWERCONTROLLER_H__

/**
 * @}
 * @}
 */
