/*
 * efuse.h
 *
 *  Created on: May 15, 2019
 *      Author: jerry_rao
 */
/**
 * @addtogroup EFUSE
 * @{
 * @defgroup efuse efuse.h
 * @{
 */

#ifndef __EFUSE_H__
#define __EFUSE_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

/**
 * @addtogroup EFUSE
 * @{
 * @defgroup efuse efuse.h
 * @{
 */
 
#include "type.h"

/**
 * @brief  ��ȡEfuse��ָ����ַ������
 * @param  AddrΪEfuse��ָ���ĵ�ַ��ʮ�����Ʊ�ʾ
 *         �ɶ��ĵ�ַΪ  0~58
 * @return ��ȡ��ָ����ַ������
 */
uint8_t Efuse_ReadData(uint8_t Addr);

/**
 * @brief  ��ֹ��ȡEfuse�����е�ַ������
 * @param  ��
 * @return ��
 */
void Efuse_ReadDataDisable(void);

/**
 * @brief  �����ȡEfuse�е�����
 * @param  ��
 * @return ��
 */
void Efuse_ReadDataEnable(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __EFUSE_H__ 
/**
 * @}
 * @}
 */

