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
 * @brief  读取Efuse中指定地址的数据
 * @param  Addr为Efuse中指定的地址，十六进制表示
 *         可读的地址为  0~58
 * @return 读取到指定地址的数据
 */
uint8_t Efuse_ReadData(uint8_t Addr);

/**
 * @brief  禁止读取Efuse中所有地址的数据
 * @param  空
 * @return 空
 */
void Efuse_ReadDataDisable(void);

/**
 * @brief  允许读取Efuse中的数据
 * @param  空
 * @return 空
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

