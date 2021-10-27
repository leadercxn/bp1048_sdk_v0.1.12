/**
 **************************************************************************************
 * @file    IR_NEC_Key.h
 * @brief   nec decoder
 *
 * @author  Cecilia Wang
 * @version V1.0.0
 *
 * $Created: 2015-08-25 13:25:30$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __IR_NEC_KEY_H__
#define __IR_NEC_KEY_H__
#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
#include "app_config.h"
#ifdef CFG_RES_IR_KEY_USE


/**
 * @brief      NECЭ��ĺ��ⰴ����ʼ��
 * @param[in]  ��
 * @return     ��
 */
void IRNecKeyInit(void);

void IRNecKeyReset(void);

/**
 * @brief      ��ȡNECЭ��ĺ��ⰴ����������
 * @param[in]  ��
 * @return     0xff -- ����ȷ��������������
 *             ���� -- ��������������
 */
uint32_t GetIRNecKeyIndex(void);

#endif


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__IR_NEC_KEY_H__

/**
 * @}
 * @}
 */
