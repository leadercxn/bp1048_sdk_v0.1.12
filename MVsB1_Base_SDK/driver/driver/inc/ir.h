/**
 **************************************************************************************
 * @file    ir.h
 * @brief   IR Module API
 *
 * @author  TaoWen
 * @version V1.0.0
 *
 * $Created: 2019-06-03 15:32:38$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
/**
 * @addtogroup IR
 * @{
 * @defgroup ir ir.h
 * @{
 */
 
#ifndef __IR_H__
#define __IR_H__
 
#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

typedef enum __IR_IO_SEL{
	IR_GPIOA29 = 1,    /**ir_0(i)*/
	IR_GPIOB6,         /**ir_1(i)*/
	IR_GPIOB7          /**ir_2(i)*/
}IR_IO_SEL;

typedef enum __IR_MODE_SEL{
	IR_MODE_NEC = 1,    /**NEC mode*/
	IR_MODE_SONY,         /**SONY mode*/
}IR_MODE_SEL;

typedef enum __IR_CMD_LEN_SEL{
	IR_NEC_16BITS = 1,    /**NEC mode*/
	IR_NEC_32BITS,        /**NEC mode*/
	IR_SONY_12BITS,       /**SONY mode*/
	IR_SONY_15BITS,       /**SONY mode*/
	IR_SONY_20BITS        /**SONY mode*/
}IR_CMD_LEN_SEL;

/**
 * @brief  IR功能开启
 * @param  无
 * @return 无
 */
void IR_Enable(void);

/**
 * @brief  IR功能禁止
 * @param  无
 * @return 无
 */
void IR_Disable(void);

/**
 * @brief  获取IR中断标志
 * @param  无
 * @return TRUE: 当前中断标志的状态为1，有中断产生
 *         FALSE:当前中断标志的状态为0，即没有中断产生
 */
bool IR_IntFlagGet(void);

/**
 * @brief  使能IR中断
 * @param  无
 * @return 无
 */
void IR_InterruptEnable(void);

/**
 * @brief  禁能IR中断
 * @param  无
 * @return 无
 */
void IR_InterruptDisable(void);

/**
 * @brief  清除IR中断标志
 * @param  无
 * @return 无
 */
void IR_IntFlagClear(void);

/**
 * @brief  使能IR将系统从DeepSleep状态唤醒的功能，
 * @param  无
 * @return 无
 */
void IR_WakeupEnable(void);

/**
 * @brief  关闭IR将系统从DeepSleep状态唤醒的功能，
 * @param  无
 * @return 无
 */
void IR_WakeupDisable(void);

/**
 * @brief  IR功能开启后，判断IR是否接收到数据的状态标志
 * @param  无
 * @return TRUE: 当前接收到数据的状态标志为1，有接收到IR数据
 *         FALSE:当前接收到数据的状态标志为1，没有接收到IR数据
 */
bool IR_CommandFlagGet(void);

/**
 * @brief  IR功能开启后，清除CommandFlag寄存器和RepeatCount寄存器
 * @param  无
 * @return 无
 */
void IR_CommandFlagClear(void);

/**
 * @brief  IR功能开启后，获取所接收到的IR数据
 * @param  无
 * @return 所获取到的IR数据
 */
uint32_t IR_CommandDataGet(void);

/**
 * @brief  IR功能开启后，获取IR信号的repeat计数
 * @param  无
 * @return IR信号的repeat计数
 */
uint8_t IR_RepeatCountGet(void);

/**
 * @brief  IR功能开启之前需要的配置
 * @param  ModeSel  NEC or SONY
 * @param  GpioSel  GPIOA29 or GPIOB6 or GPIOB7
 * @param  CmdLenSel  when NEC mode, support 32 bits or 16bits;
 *                    when SONY mode, support 12, 15 or 20 bits
 * @return NONE
 */
void IR_Config(IR_MODE_SEL ModeSel, IR_IO_SEL GpioSel, IR_CMD_LEN_SEL CmdLenSel);




#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__IR_H__

/**
 * @}
 * @}
 */

