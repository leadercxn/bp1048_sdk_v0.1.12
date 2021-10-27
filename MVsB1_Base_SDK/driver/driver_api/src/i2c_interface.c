/**
 **************************************************************************************
 * @file    i2c_interface.c
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
 
#include "type.h"
//#include "app_config.h"
#include "uarts.h"
#include "gpio.h"
#include "timeout.h"
#include "i2c_interface.h"


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
I2C_ErrorState I2C_MasterSendBuffer(uint8_t SlaveAddr, void* SendBuf, uint32_t BufLen, uint32_t timeout)
{
	TIMER I2CTimer;
	uint32_t i;
	
	TimeOutSet(&I2CTimer, timeout);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer))
		{
			return ERROR_BUSY;
		}
	}

	I2C_MasterConfig();

	//send slave address
	I2C_SendByte(SlaveAddr<<1);
	while(!I2C_GetIntFlag());
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		return ERROR_NOACK;
	}
	
	//send data
	for(i=0; i<BufLen; i++)
	{
		I2C_SendByte(((uint8_t *)SendBuf)[i]);
		while(!I2C_GetIntFlag());
		I2C_IntClr();
		if(I2C_ReceiveAcknowledge())
		{
			I2C_SendStop();
			return ERROR_NOACK;
		}
	}
	I2C_SendStop();
	return ERROR_OK;
}

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
I2C_ErrorState I2C_MasterSendData(uint8_t SlaveAddr, uint8_t RegAddr, void* SendBuf, uint32_t BufLen, uint32_t timeout)
{
	TIMER I2CTimer;
	uint32_t i;

	TimeOutSet(&I2CTimer, timeout);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer))
		{
			return ERROR_BUSY;
		}
	}

	I2C_MasterConfig();

	//send slave address
	I2C_SendByte(SlaveAddr<<1);
	while(!I2C_GetIntFlag());
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		return ERROR_NOACK;
	}
	
	//send register address
	I2C_SendByte(RegAddr);
	while(!I2C_GetIntFlag());
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		return ERROR_NOACK;
	}

	//send data
	for(i=0; i<BufLen; i++)
	{
		I2C_SendByte(((uint8_t *)SendBuf)[i]);
		while(!I2C_GetIntFlag());
		I2C_IntClr();
		if(I2C_ReceiveAcknowledge())
		{
			I2C_SendStop();
			return ERROR_NOACK;
		}
	}
	I2C_SendStop();
	return ERROR_OK;
}

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
I2C_ErrorState I2C_MasterReceiveBuffer(uint8_t SlaveAddr, void* RecvBuf, uint32_t BufLen, uint32_t timeout)
{
	TIMER I2CTimer;
	uint32_t i;
	
	TimeOutSet(&I2CTimer, timeout);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer))
		{
			return ERROR_BUSY;
		}
	}

	I2C_MasterConfig();

	//send slave address(read)
	I2C_SendByte((SlaveAddr<<1) | I2C_READ);
	while(!I2C_GetIntFlag());
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		return ERROR_NOACK;
	}
	
	I2C_SendAck();
	I2C_ReceiveStart();

	((uint8_t *)RecvBuf)[0] = I2C_ReceiveByte();//dummy read for trigger recv

	for(i=0; i<BufLen-1; i++)
	{
		I2C_SendAck();
		while(!I2C_GetIntFlag());
		I2C_IntClr();
		((uint8_t *)RecvBuf)[i] = I2C_ReceiveByte();
	}
	
	I2C_SendNoAck();
	while(!I2C_GetIntFlag());
	I2C_IntClr();
	I2C_SendStop();
	((uint8_t *)RecvBuf)[i] = I2C_ReceiveByte();
	return ERROR_OK;
}

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
I2C_ErrorState I2C_MasterReceiveData(uint8_t SlaveAddr, uint8_t RegAddr, void* RecvBuf, uint32_t BufLen, uint32_t timeout)
{
	TIMER I2CTimer;
	uint32_t i;
	
	TimeOutSet(&I2CTimer, timeout);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer))
		{
			return ERROR_BUSY;
		}
	}

	I2C_MasterConfig();

	//send slave address(write)
	I2C_SendByte(SlaveAddr<<1);
	while(!I2C_GetIntFlag());
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		return ERROR_NOACK;
	}
	
	//send register address
	I2C_SendByte(RegAddr);
	while(!I2C_GetIntFlag());
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		return ERROR_NOACK;
	}

	//send restart
	I2C_SendReStart();
	
	//send slave address(read)
	I2C_SendByte((SlaveAddr<<1) | I2C_READ);
	while(!I2C_GetIntFlag());
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		return ERROR_NOACK;
	}

	I2C_SendAck();
	I2C_ReceiveStart();
	((uint8_t *)RecvBuf)[0] = I2C_ReceiveByte();//dummy read for trigger recv

	for(i=0; i<BufLen-1; i++)
	{
		I2C_SendAck();
		while(!I2C_GetIntFlag());
		I2C_IntClr();
		((uint8_t *)RecvBuf)[i] = I2C_ReceiveByte();
	}
	
	I2C_SendNoAck();
	while(!I2C_GetIntFlag());
	I2C_IntClr();
	I2C_SendStop();
	((uint8_t *)RecvBuf)[i] = I2C_ReceiveByte();
	return ERROR_OK;
}

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
I2C_ErrorState I2C_SlaveSendBuffer(uint8_t *SendBuf, uint32_t BufLen, uint32_t timeout)
{
	uint32_t i;
	TIMER I2CTimer;
	I2C_ErrorState ret;
	TimeOutSet(&I2CTimer, timeout);

	I2C_SlaveConfig();

	ret = I2C_SlaveRequestWrite();
	if(ret)
	{
		return ret;
	}
	
	I2C_SendStart();
	for(i = 0;i < BufLen; i++)
	{
		I2C_SendByte(SendBuf[i]);
		while(!I2C_GetIntFlag());
		I2C_IntClr();
		if(I2C_ReceiveAcknowledge())
		{
			break;
		}
	}

	I2C_SlaveReleaseI2C();
	while(I2C_IsBusy());

	return ERROR_OK;
}

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
I2C_ErrorState I2C_SlaveReceiveBuffer(uint8_t *RecvBuf, uint32_t BufLen, uint32_t timeout)
{
	uint32_t i;
	TIMER I2CTimer;
	I2C_ErrorState ret;
	TimeOutSet(&I2CTimer, timeout);	

	I2C_SlaveConfig();

	ret = I2C_SlaveRequestRead();
	if(ret)
	{
		return ret;
	}
	
	RecvBuf[0] = I2C_ReceiveByte();//dummy read for trigger recv
	for(i = 0; i < BufLen; i++)
	{
		while(!I2C_GetIntFlag());
		I2C_IntClr();
		RecvBuf[i] = I2C_ReceiveByte();
		if(!I2C_IsBusy())
		{	//end receive data
			break;
		}
	}
	return ERROR_OK;
}




