/**
 **************************************************************************************
 * @file    i2c_interface.h
 * @brief   i2c interface
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2016-10-28 19:15:00$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
#ifndef __I2C_INTERFACE_H__
#define __I2C_INTERFACE_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include "type.h"
#include "i2c.h"

/**
 * @brief     I2C主模式下发送数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_MasterSendBuffer(uint8_t SlaveAddr, void* SendBuf, uint32_t BufLen, uint32_t timeout);

/**
 * @brief     I2C主模式下发送数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] RegAddr   从设备寄存器地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_MasterSendData(uint8_t SlaveAddr, uint8_t RegAddr, void* SendBuf, uint32_t BufLen, uint32_t timeout);

/**
 * @brief     I2C主模式下接收数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_MasterReceiveBuffer(uint8_t SlaveAddr, void* RecvBuf, uint32_t BufLen, uint32_t timeout);

/**
 * @brief     I2C主模式下接收数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] RegAddr   从设备寄存器地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_MasterReceiveData(uint8_t SlaveAddr, uint8_t RegAddr, void* RecvBuf, uint32_t BufLen, uint32_t timeout);

/**
 * @brief     I2C从模式下发送数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_SlaveSendBuffer(uint8_t *SendBuf, uint32_t BufLen, uint32_t timeout);

/**
 * @brief     I2C从模式下接收数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_SlaveReceiveBuffer(uint8_t *RecvBuf, uint32_t BufLen, uint32_t timeout);

#ifdef __cplusplus
}
#endif // __cplusplus 


#endif//__I2C_INTERFACE_H__

