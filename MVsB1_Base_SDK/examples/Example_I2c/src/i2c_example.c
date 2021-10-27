/**
 **************************************************************************************
 * @file   I2c_example.c
 * @brief   i2c
 * 
 * @author  messi
 * @version V1.0.0 	initial release
 * 
 * $Id$
 * $Created: 2017-10-29 17:31:10$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "pwm.h"
#include "delay.h"
#include "rtc.h"
#include "watchdog.h"
#include "irqn.h"
#include "spi_flash.h"
#include "sys.h"
#include "remap.h"
#include "i2c.h"
#include "i2c_interface.h"
#include "chip_info.h"


#define MASTER_SLAVE	1//0:SLAVE_MODE 1:MASTER_MODE
#define SLAVE_INT		1//0:no int  1��int

/*ע���ַ�ĸ��ģ�	B1X_ADDRΪ������ַ���������ӣ�
					PCI_ADDRΪ�ⲿ�����ַ���������ӣ�
					��ʵ��Ӧ���������ʵ��Ӧ�ø��ģ�����ַ��Ӧ��
*/
#define B1X_ADDR		0x12
#define PCI_ADDR		0x12

#define PACKET_LEN		255
#define DATA_LEN		255

uint8_t SendBuf[DATA_LEN];
uint8_t RecvBuf[DATA_LEN];
uint8_t RX_OVER = 0;

__attribute__((section(".driver.isr")))
void I2C_Interrupt(void)
{
	static uint8_t RW_FLAG  = 0;
	static uint8_t RX_count = 0;
	static uint8_t TX_count = 0;
	if(I2C_IsAddressMatched())//�жϸ��ж��Ƿ�Ϊ��ַƥ���ж�
	{
		 if(I2C_SlaveReadWriteGet())//�ж϶�д��־
		 {
			I2C_IntEn(1);//дһ�¼Ĵ��������ﶨΪд�ж�ʹ��λ�������ַƥ���ʶ���Է��������ַһ���������󴥷���ַƥ���ж�
			I2C_IntClr();//����жϱ�־λ
			RW_FLAG=1;
			I2C_SendStart();//����Ϊ����ģʽ
			TX_count=0;
			I2C_SendByte(SendBuf[TX_count]);//���͵�һ������
		 }else
		 {
			I2C_IntEn(1);//дһ�¼Ĵ��������ﶨΪд�ж�ʹ��λ�������ַƥ���ʶ���Է��������ַһ���������󴥷���ַƥ���ж�
			I2C_IntClr();//����жϱ�־λ
			I2C_ReceiveByte();//���FIFO
			I2C_ReceiveStart();//����Ϊ����ģʽ
			RX_count =0 ;
			RW_FLAG = 0;
		 }
	}else
	{//����֮ǰ�Ķ�д��־��������Ӧ�Ĵ���
		if(RW_FLAG)
		{
			if(I2C_ReceiveAcknowledge())//�ж��Ƿ��յ�ACK
			{
				TX_count++;
				I2C_SlaveReleaseI2C();//�ͷ�I2C����
				I2C_ReceiveByte();//���FIFO
			}else
			{
				TX_count++;
				I2C_SendByte(SendBuf[TX_count]);
			}
			I2C_IntClr();//����жϱ�־λ
		}else
		{
			RecvBuf[RX_count++] = I2C_ReceiveByte();
			if(RX_count == DATA_LEN)
			{
				RX_count = 0;
				RX_OVER=1;
			}
			I2C_IntClr();//����жϱ�־λ
		}
	}
}


static int32_t WaitDatum1Ever(void)//�Ӵ��ڵȴ�����һ���ֽ�
{
	uint8_t Data;
	do{
		if(0 <= UART1_Recv(&Data, 1,5000))
        {
			break;
		}
	}while(1);
	return Data-0x30;
}

void MasterExample(void)
{
	uint32_t i;
	uint8_t temp=0;
    I2C_ErrorState ret;
    DBG("**** I2C_MASTER test...\n");
	DelayMs(1);
	I2C_Init(0x28, I2C_PORT_B4_B5, B1X_ADDR);//ʱ�ӷ�Ƶ/GPIO����
	DelayMs(1000);

	 Master_start:
	 DBG("Input data 1 master mode to send data\n");
	 DBG("Input data 2 master mode to receive data\n");
	 DBG("/***** ************************************/\n");
	 temp = WaitDatum1Ever();
	 if(temp == 1)
	 {
		for(i=0;i<PACKET_LEN;i++)
		{
			SendBuf[i] = i;
		}
		//������������
		temp = 0;
		ret = I2C_MasterSendBuffer(PCI_ADDR,SendBuf, PACKET_LEN, 1000);
		while(ret!=ERROR_OK)
		{
			DBG("**** Write to slave Err = [%d]...\n", ret);
			DelayMs(2000);
			ret = I2C_MasterSendBuffer(PCI_ADDR,SendBuf, PACKET_LEN, 1000);
			temp++;
			if(temp>5)
			{
				temp =0;
				goto  Master_start;
			}
		}
		DBG("**** Write to slave OK\n");
	 }
	 else if(temp == 2)
	 {
		memset(RecvBuf, 0x00, PACKET_LEN);
		ret = I2C_MasterReceiveBuffer(PCI_ADDR, RecvBuf, PACKET_LEN, 5000);
		if(ret!=ERROR_OK)
		{
			DBG("**** Read from slave Err = [%d]...\n", ret);
		}
		else
		{
			for(i = 0; i < PACKET_LEN; i++)
			{
				if(RecvBuf[i] != i)
				{
					DBG("\n=============================================\n");
					DBG("%02X != %02X \r\n",RecvBuf[i], (uint8_t)i);
					DBG("\nErr\n");
					DBG("\n=============================================\n");
				}
				else
				{
					DBG("%02X is equal\n",RecvBuf[i]);
				}
			}
		}
	}
	goto  Master_start;
}


void SlaveExample(void)
{
	uint32_t i;
	DBG("**** I2C_SLAVE test(no int)...\n");
	DelayMs(1);
	I2C_Init(0x2f,I2C_PORT_B4_B5, B1X_ADDR);//ʱ�ӷ�Ƶ/GPIO����
	DelayMs(1000);
	memset(RecvBuf, 0x00, DATA_LEN);
	for(i=0;i<PACKET_LEN;i++)
	{
		SendBuf[i] = i;
	}
	while(1)
	{
		I2C_SlaveConfig();
		while(!I2C_GetIntFlag());
		I2C_IntClr();
		if(!I2C_IsAddressMatched())
		{
			DBG("Address Matched Error\n");
		}
		if(I2C_SlaveReadWriteGet())//��鷢�ͻ��߽���״̬
		{
			I2C_SendStart();
			for(i = 0;i < PACKET_LEN; i++)
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
		}
		else
		{
			RecvBuf[0] = I2C_ReceiveByte();//dummy read for trigger recv
			for(i = 0; i < DATA_LEN; i++)
			{
				while(!I2C_GetIntFlag());
				I2C_IntClr();
				RecvBuf[i] = I2C_ReceiveByte();
				if(!I2C_IsBusy())
				{
					break;
				}
			}
		}
	}
}



void SlaveIntExample(void)
{
	uint32_t i;
    DBG("**** I2C_SLAVE test(int)...\n");
	DelayMs(1);
	I2C_Init(0x2f,I2C_PORT_B4_B5, B1X_ADDR);//ʱ�ӷ�Ƶ/GPIO����
	NVIC_EnableIRQ(I2C_InIRQn);
	I2C_IntEn(1);
	DelayMs(1000);
	memset(RecvBuf, 0x00, DATA_LEN);
	for(i=0;i<PACKET_LEN;i++)
	{
		SendBuf[i] = i;
	}
	while(1)
	{
		if(RX_OVER)
		{
			RX_OVER = 0;
			for(i=0;i<DATA_LEN;i++)
			{
				DBG("N%d = %d\n",(int)i,(int)RecvBuf[i]);
			}
		}
	}
}







/**
 * @brief   ������
 * @return
 */
int main(void)
{
	WDG_Disable();

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_UARTClkSelect(PLL_CLK_MODE);

	GPIO_PortAModeSet(GPIOA24, 1);//Rx, A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx, A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);

	SysTickInit();

	Remap_InitTcm(0, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	GIE_ENABLE();

	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                     I2C Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

	//ע�⣺��Ҫ���ڲ��õ���I2C���IO������Ҳ����Ӳ�����ʱ����������
	GPIO_RegOneBitSet(GPIO_B_PU, GPIO_INDEX4);
	GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX4);
	GPIO_RegOneBitSet(GPIO_B_PU, GPIO_INDEX5);
	GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX5);
	if(MASTER_SLAVE==1)
	{
		MasterExample();
	}
	else
	{
		if(SLAVE_INT)
		{
			SlaveIntExample();
		}
		else
		{
			SlaveExample();
		}

	}
	DBG("The test is completed\n");
	while(1);
	return 0;
}








