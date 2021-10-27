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
 * @brief  IR���ܿ���
 * @param  ��
 * @return ��
 */
void IR_Enable(void);

/**
 * @brief  IR���ܽ�ֹ
 * @param  ��
 * @return ��
 */
void IR_Disable(void);

/**
 * @brief  ��ȡIR�жϱ�־
 * @param  ��
 * @return TRUE: ��ǰ�жϱ�־��״̬Ϊ1�����жϲ���
 *         FALSE:��ǰ�жϱ�־��״̬Ϊ0����û���жϲ���
 */
bool IR_IntFlagGet(void);

/**
 * @brief  ʹ��IR�ж�
 * @param  ��
 * @return ��
 */
void IR_InterruptEnable(void);

/**
 * @brief  ����IR�ж�
 * @param  ��
 * @return ��
 */
void IR_InterruptDisable(void);

/**
 * @brief  ���IR�жϱ�־
 * @param  ��
 * @return ��
 */
void IR_IntFlagClear(void);

/**
 * @brief  ʹ��IR��ϵͳ��DeepSleep״̬���ѵĹ��ܣ�
 * @param  ��
 * @return ��
 */
void IR_WakeupEnable(void);

/**
 * @brief  �ر�IR��ϵͳ��DeepSleep״̬���ѵĹ��ܣ�
 * @param  ��
 * @return ��
 */
void IR_WakeupDisable(void);

/**
 * @brief  IR���ܿ������ж�IR�Ƿ���յ����ݵ�״̬��־
 * @param  ��
 * @return TRUE: ��ǰ���յ����ݵ�״̬��־Ϊ1���н��յ�IR����
 *         FALSE:��ǰ���յ����ݵ�״̬��־Ϊ1��û�н��յ�IR����
 */
bool IR_CommandFlagGet(void);

/**
 * @brief  IR���ܿ��������CommandFlag�Ĵ�����RepeatCount�Ĵ���
 * @param  ��
 * @return ��
 */
void IR_CommandFlagClear(void);

/**
 * @brief  IR���ܿ����󣬻�ȡ�����յ���IR����
 * @param  ��
 * @return ����ȡ����IR����
 */
uint32_t IR_CommandDataGet(void);

/**
 * @brief  IR���ܿ����󣬻�ȡIR�źŵ�repeat����
 * @param  ��
 * @return IR�źŵ�repeat����
 */
uint8_t IR_RepeatCountGet(void);

/**
 * @brief  IR���ܿ���֮ǰ��Ҫ������
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

