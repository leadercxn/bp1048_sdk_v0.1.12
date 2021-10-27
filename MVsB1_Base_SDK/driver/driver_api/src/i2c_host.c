/**
 *************************************************************************************
 * @file	i2c_host.c
 * @author	
 * @version	v1.0.0
 * @date    2018/02/27
 * @brief	software I2C driver
 * @ maintainer: 
 * Copyright (C) Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *************************************************************************************
 */
#include <string.h>
#include "type.h"
#include "gpio.h"
#include "i2c_host.h"
#include "debug.h"
#include "delay.h"
#define MAX_I2C_MASTER   2

typedef struct
{
	uint32_t  SdaMask;
	uint32_t  SclMask;
	uint8_t   SdaPortIndex;
	uint8_t   SclPortIndex;

	bool   IsUsed;
} I2C_INFO, *I2C_HANDLE;

I2C_INFO I2cInfo[MAX_I2C_MASTER];


#define		IIC_GPIO_IN		(0x00 + 0)	/**<GPIOA input data register macro*/
#define		IIC_GPIO_OUT	(0x01 + 0)	/**<GPIOA output data register macro*/
#define     IIC_GPIO_SET	(0x02 + 0)
#define     IIC_GPIO_CLR	(0x03 + 0)
#define     IIC_GPIO_TGL	(0x04 + 0)
#define		IIC_GPIO_IE		(0x05 + 0)	/**<GPIOA input enable register macro.0: disable, 1: enable*/
#define		IIC_GPIO_OE		(0x06 + 0)	/**<GPIOA output enable register macro.0: disable, 1: enable*/
#define		IIC_GPIO_DS		(0x07 + 0)	/**<GPIOA pull-up/pull-down drive strength(when GPIOA input or output) register macro. 0: weak pull(20uA), 1: strong pull(70uA)*/
#define		IIC_GPIO_PU		(0x08 + 0)	/**<GPIOA pull up register macro*/
#define		IIC_GPIO_PD		(0x09 + 0)	/**<GPIOA pull down register macro.{pu,pd} 1'b1: pull-up, 1'b1: pull-down, 1'b0: no pull-up, no pull-down, */

/**
 * I2C Read/Write Flag
 */
#define	IIC_READ	0x01  /*!< Read  flag in I2C slave address. */
#define	IIC_WRITE	0x00  /*!< Write flag in I2C slave address. */


#define	I2C_Delay()		__udelay(5) /**< SCL/SDA hold time*/

/**
 * SCL Manipulation
 */
void SetOutputSCL(void* I2cMasterHandle)
{
	I2C_HANDLE I2cHandle = (I2C_HANDLE)I2cMasterHandle;
	GPIO_RegOneBitSet(I2cHandle->SclPortIndex + IIC_GPIO_OE, I2cHandle->SclMask);
	GPIO_RegOneBitClear(I2cHandle->SclPortIndex + IIC_GPIO_IE, I2cHandle->SclMask);
}

void SetSCL(void* I2cMasterHandle)
{
	I2C_HANDLE I2cHandle = (I2C_HANDLE)I2cMasterHandle;
	GPIO_RegOneBitSet(I2cHandle->SclPortIndex + IIC_GPIO_OUT, I2cHandle->SclMask);
	I2C_Delay();
}

void ClrSCL(void* I2cMasterHandle)
{
	I2C_HANDLE I2cHandle = (I2C_HANDLE)I2cMasterHandle;
	GPIO_RegOneBitClear(I2cHandle->SclPortIndex + IIC_GPIO_OUT, I2cHandle->SclMask);
	I2C_Delay();
}


/**
 * SDA Manipulation
 */
void SetOutputSDA(void* I2cMasterHandle)
{
	I2C_HANDLE I2cHandle = (I2C_HANDLE)I2cMasterHandle;
	GPIO_RegOneBitSet(I2cHandle->SdaPortIndex + IIC_GPIO_OE, I2cHandle->SdaMask);
	GPIO_RegOneBitClear(I2cHandle->SdaPortIndex + IIC_GPIO_IE, I2cHandle->SdaMask);
}

void SetSDA(void* I2cMasterHandle)
{
	I2C_HANDLE I2cHandle = (I2C_HANDLE)I2cMasterHandle;
	GPIO_RegOneBitSet(I2cHandle->SdaPortIndex + IIC_GPIO_OUT, I2cHandle->SdaMask);
	I2C_Delay();
}

void ClrSDA(void* I2cMasterHandle)
{
	I2C_HANDLE I2cHandle = (I2C_HANDLE)I2cMasterHandle;
	GPIO_RegOneBitClear(I2cHandle->SdaPortIndex + IIC_GPIO_OUT, I2cHandle->SdaMask);
	I2C_Delay();
}

void SetInputSDA(void* I2cMasterHandle)
{
	I2C_HANDLE I2cHandle = (I2C_HANDLE)I2cMasterHandle;
	
	GPIO_RegOneBitSet(I2cHandle->SdaPortIndex + IIC_GPIO_IE, I2cHandle->SdaMask);
	GPIO_RegOneBitClear(I2cHandle->SdaPortIndex + IIC_GPIO_OE, I2cHandle->SdaMask);
}

uint8_t GetSDA(void* I2cMasterHandle)
{
	I2C_HANDLE I2cHandle = (I2C_HANDLE)I2cMasterHandle;
	uint32_t Val = GPIO_RegOneBitGet(I2cHandle->SdaPortIndex,I2cHandle->SdaMask);
	if(Val)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static uint8_t I2cInfoInit(void)
{
	static uint8_t I2cInfoInitFlag = FALSE;

	if(!I2cInfoInitFlag)
	{
		memset(I2cInfo, 0, sizeof(I2cInfo));
		I2cInfoInitFlag = TRUE;
	}
	return I2cInfoInitFlag;
}

/**
  * @brief  This function create an I2C object.
  * @param  None
  * @return None
  */
void* I2cMasterCreate(PORT_SEL SclPortSel, uint8_t SclIndex, PORT_SEL SdaPortSel, uint8_t SdaIndex)
{
	uint8_t i;

	I2cInfoInit();

	for(i = 0; i < MAX_I2C_MASTER; i++)
	{
		if(I2cInfo[i].IsUsed == FALSE)
		{
			switch(SdaPortSel)
			{
				case PORT_A:
					I2cInfo[i].SdaPortIndex = GPIO_A_IN;
					break;
				case PORT_B:
					I2cInfo[i].SdaPortIndex = GPIO_B_IN;
					break;
				default:
					return NULL;
			}
			switch(SclPortSel)
			{
				case PORT_A:
					I2cInfo[i].SclPortIndex = GPIO_A_IN;
					break;
				case PORT_B:
					I2cInfo[i].SclPortIndex = GPIO_B_IN;
					break;
				default:
					return NULL;
			}
			I2cInfo[i].SdaMask = 1 << SdaIndex;
			I2cInfo[i].SclMask = 1 << SclIndex;
			I2cInfo[i].IsUsed = TRUE;
            //-----------SDA PULL-----------------------------------//
			GPIO_RegOneBitClear(I2cInfo[i].SdaPortIndex + IIC_GPIO_PU, I2cInfo[i].SdaMask);
			GPIO_RegOneBitClear(I2cInfo[i].SdaPortIndex + IIC_GPIO_PD, I2cInfo[i].SdaMask);
            //-----------CLK PULL-----------------------------------//
			GPIO_RegOneBitClear(I2cInfo[i].SclPortIndex + IIC_GPIO_PU, I2cInfo[i].SclMask);
			GPIO_RegOneBitClear(I2cInfo[i].SclPortIndex + IIC_GPIO_PD, I2cInfo[i].SclMask);

			return (void*)&I2cInfo[i];
		}
	}

	return NULL;
}

/**
  * @brief  This function validate an I2C object.
  * @param  None
  * @return None
  */
bool ValidateI2cHandle(void* I2cMasterHandle)
{
	uint8_t i;

	for(i = 0; i < MAX_I2C_MASTER; i++)
	{
		void* h = (void*)&I2cInfo[i];
		if(h == I2cMasterHandle)
		{
			return TRUE;
		}
	}
	return FALSE;
}


/**
  * @brief  This function generates I2C start timing.
  * @param  None
  * @return None
  */
void I2cStart(void* I2cMasterHandle)
{
	SetSDA(I2cMasterHandle);
	SetSCL(I2cMasterHandle);
	SetOutputSCL(I2cMasterHandle);
	SetOutputSDA(I2cMasterHandle);

	ClrSDA(I2cMasterHandle);
	ClrSCL(I2cMasterHandle);
}

/**
  * @brief  This function generates I2C stop timing.
  * @param  None
  * @return None
  */
void I2cStop(void* I2cMasterHandle)
{
	SetOutputSCL(I2cMasterHandle);
	SetOutputSDA(I2cMasterHandle);

	ClrSDA(I2cMasterHandle);
	SetSCL(I2cMasterHandle);
	SetSDA(I2cMasterHandle);

	SetInputSDA(I2cMasterHandle);
}

/**
  * @brief  This function send ACK to I2C slave.
  * @param  None
  * @return None
  */
void I2cSendAck(void* I2cMasterHandle)
{
	ClrSDA(I2cMasterHandle);
	SetSCL(I2cMasterHandle);
	ClrSCL(I2cMasterHandle);
}

/**
  * @brief  This function send NACK to I2C slave.
  * @param  None
  * @return None
  */
void I2cSendNoAck(void* I2cMasterHandle)
{
	SetSDA(I2cMasterHandle);
	SetSCL(I2cMasterHandle);
	ClrSCL(I2cMasterHandle);
}

/**
  * @brief  This function checks ACK/NACK from I2C slave.
  * @param  None
  * @return None
  */
bool I2cChkAck(void* I2cMasterHandle)
{
	bool	Ack;

	SetInputSDA(I2cMasterHandle);//Allow slave to send ACK
	ClrSCL(I2cMasterHandle);//slave send ACK
	SetSCL(I2cMasterHandle);
	Ack = !GetSDA(I2cMasterHandle);//Get ACK from slave
	ClrSCL(I2cMasterHandle);
	return Ack;
}

/**
  * @brief  This function send one byte to I2C slave.
  * @param  Val: data to be sent
  * @return
  *     @arg    True:  Receive ACK from I2C slave
  *     @arg    False: Not receive ACK from I2C slave
  */
bool I2cWriteByte(void* I2cMasterHandle, uint8_t Val)
{
	uint8_t i = 8;

	SetOutputSDA(I2cMasterHandle);
	ClrSCL(I2cMasterHandle);
	while(i--)
	{
		if(Val & 0x80)	/* MSB output first */
		{
			SetSDA(I2cMasterHandle);
		}
		else
		{
			ClrSDA(I2cMasterHandle);
		}
		Val <<= 1;
		SetSCL(I2cMasterHandle);
		ClrSCL(I2cMasterHandle);
	}
	return I2cChkAck(I2cMasterHandle);
}

/**
  * @brief  This function recieve one byte from I2C slave.
  * @param  none
  * @return Data received from I2C slave
  */
uint8_t I2cReadByte(void* I2cMasterHandle)
{
	uint8_t i = 8;
	uint8_t Dat = 0;

	SetInputSDA(I2cMasterHandle);
	I2C_Delay();
	while(i--)
	{
		SetSCL(I2cMasterHandle);
		Dat <<= 1;
		if(GetSDA(I2cMasterHandle))
		{
			Dat |= 0x01;
		}
		ClrSCL(I2cMasterHandle);
	}
	SetOutputSDA(I2cMasterHandle);

	return Dat;
}


/**
  * @brief  This function send multiple bytes to I2C slave.
  * @param  Buf: Buffer pointer to the data to be send
  * @param  Len: Data Length to be send in byte uint
  * @return
  *     @arg TRUE:  success
  *     @arg FALSE: fail
  */
bool I2cWriteBytes(void* I2cMasterHandle, uint8_t* Buf, uint8_t Len)
{
	while(Len--)
	{
		if(!I2cWriteByte(I2cMasterHandle, *(Buf++)))
		{
			DBG("write data err\n");
			return FALSE;
		}
	}

	return TRUE;
}

/**
  * @brief  This function receive multiple bytes to I2C slave.
  * @param  Buf: Buffer pointer to hold the received data.
  * @param  Len: Data Length to be received
  * @return None
  */
void I2cReadBytes(void* I2cMasterHandle, uint8_t* Buf, uint8_t Len)
{
	while(Len--)
	{
		*(Buf++) = I2cReadByte(I2cMasterHandle);
		if(Len == 0)
		{
			I2cSendNoAck(I2cMasterHandle);
		}
		else
		{
			I2cSendAck(I2cMasterHandle);
		}
	}
}

/**
  * @brief  This function send device address and register address to I2C slave.
  * @param  DeviceAddr: I2C slave address
  * @param  RegAddr: slave device's register address, usually before read/write slave device's register,
  *         i2c master should first send it.
  * @return
  *     @arg TRUE: success
  *     @arg FALSE:fail
  */
bool I2cSendAddr(void* I2cMasterHandle, uint8_t SlaveAddr, uint8_t RegAddr, uint8_t RwFlag)
{
	I2cStart(I2cMasterHandle);
	if(!I2cWriteByte(I2cMasterHandle, SlaveAddr))
	{
		/* Retry */
		I2cStart(I2cMasterHandle);
		if(!I2cWriteByte(I2cMasterHandle, SlaveAddr))
		{
			I2cStop(I2cMasterHandle);
			return FALSE;
		}
	}

	if(!I2cWriteByte(I2cMasterHandle, RegAddr))
	{
		I2cStop(I2cMasterHandle);
		return FALSE;
	}

	if(RwFlag == IIC_READ)
	{
		I2cStart(I2cMasterHandle);
		if(!I2cWriteByte(I2cMasterHandle, SlaveAddr | IIC_READ))
		{
			I2cStop(I2cMasterHandle);
			return FALSE;
		}
	}

	return TRUE;
}


/**
  * @brief  This function send multiple bytes to I2C slave with address
  * @param  Buf: Buffer pointer to the data to be send
  * @param  Len: Data Length to be send in byte uint
  * @return
  *     @arg TRUE: success
  *     @arg FALSE:fail
  */
bool I2cWriteNByte(void* I2cMasterHandle, uint8_t SlaveAddr, uint8_t RegAddr, uint8_t* Buf, uint8_t Len)
{
	if(I2cSendAddr(I2cMasterHandle, SlaveAddr, RegAddr, IIC_WRITE))
	{
		if(I2cWriteBytes(I2cMasterHandle, Buf, Len))
		{
			I2cStop(I2cMasterHandle);
			return TRUE;
		}
	}
	I2cStop(I2cMasterHandle);
	return FALSE;
}


/**
  * @brief  This function read multiple bytes to I2C slave with address
  * @param  Buf: Buffer pointer to the data to be send
  * @param  Len: Data Length to be send in byte uint
  * @return
  *     @arg TRUE:  success
  *     @arg FALSE: fail
  */
bool I2cReadNByte(void* I2cMasterHandle, uint8_t SlaveAddr, uint8_t RegAddr, uint8_t* Buf, uint8_t Len)
{
	if(I2cSendAddr(I2cMasterHandle, SlaveAddr, RegAddr, IIC_READ))
	{
		I2cReadBytes(I2cMasterHandle, Buf, Len);
		I2cStop(I2cMasterHandle);
		return TRUE;
	}
	I2cStop(I2cMasterHandle);
	return FALSE;
}





