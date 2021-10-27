/**
 **************************************************************************************
 * @file    uart_example.c
 * @brief   uart
 * 
 * @author  Ben
 * @version V1.0.1
 * 
 * $Id$
 * $Created: 2019-05-28
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
#include "spi_flash.h"
#include "timeout.h"
#include "clk.h"
#include "pwm.h"
#include "delay.h"
#include "rtc.h"
#include "spis.h"
#include "watchdog.h"
#include "irqn.h"
#include "spi_flash.h"
#include "remap.h"
#include "bmd.h"
#include "chip_info.h"
#include "sw_uart.h"
#include "core_d1088.h"

#define NEW_EXAMPLE
extern void SysTickInit(void);

#define DMA_TST_BUF_LEN 10
static uint8_t DmaTxFifo[512];
static uint8_t DmaTxBuf[512];
static uint8_t DmaTxBuf1[512];
static uint8_t DmaRxBuf[1024];
static uint8_t DmaTempBuf[512];
static uint8_t DmaTempBuf1[512];
uint8_t BuartRGBuf[1024];
static uint8_t crc_check_buf[DMA_TST_BUF_LEN+1];
BUFFER_INFO g_dma_ring_buf = {0};
static uint16_t dLenInDma = 0;

static uint8_t DmaChannelMap[28] = {
		255,//PERIPHERAL_ID_SPIS_RX = 0,		//0
		255,//PERIPHERAL_ID_SPIS_TX,			//1
		255,//PERIPHERAL_ID_TIMER3,				//2
		255,//PERIPHERAL_ID_SDIO0_RX,			//3
		255,//PERIPHERAL_ID_SDIO0_TX,			//4
		2,//PERIPHERAL_ID_UART0_RX,				//5
		255,//PERIPHERAL_ID_SDIO1_RX,			//6
		255,//PERIPHERAL_ID_SDIO1_TX,			//7
		255,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TX��ʹ��ͬһͨ��
		255,//PERIPHERAL_ID_SDPIF_TX,			//9
		255,//PERIPHERAL_ID_SPIM_RX,			//10
		255,//PERIPHERAL_ID_SPIM_TX,			//11
		3,//PERIPHERAL_ID_UART0_TX,				//12
		1,//PERIPHERAL_ID_UART1_RX,				//13
		4,//PERIPHERAL_ID_UART1_TX,				//14
		255,//PERIPHERAL_ID_PSRAM_CONTROL_TX,	//15
		255,//PERIPHERAL_ID_PSRAM_CONTROL_RX,	//16
		255,//PERIPHERAL_ID_TIMER4,				//17
		255,//PERIPHERAL_ID_AUDIO_ADC0_RX,		//18
		255,//PERIPHERAL_ID_AUDIO_ADC1_RX,		//19
		255,//PERIPHERAL_ID_AUDIO_DAC0_TX,		//20
		255,//PERIPHERAL_ID_AUDIO_DAC1_TX,		//21
		255,//PERIPHERAL_ID_I2S0_TX,			//22
		255,//PERIPHERAL_ID_I2S0_RX,			//23
		255,//PERIPHERAL_ID_I2S1_TX,			//24
		255,//PERIPHERAL_ID_I2S1_RX,			//25
		255,//PERIPHERAL_ID_ADC,     			//26
		255,//PERIPHERAL_ID_SOFTWARE,			//27
};
static uint8_t rxdata;
int flag=0;
__attribute__((section(".driver.isr")))
void UART1_Interrupt(void)
{
    if(UART1_IOCtl(UART_IOCTL_RXSTAT_GET, 1) & 0x01)
    {
    	UART1_RecvByte(&rxdata);
        UART1_IOCtl(UART_IOCTL_RXINT_CLR, 1);
        flag = rxdata;

        //��ʾ�����յ��ķ���
        DBG("\n[Recieved data]:\n");
        UART1_SendByte(rxdata);//DBG("%c",rxdata);

		//ʵ��ʹ���У�Ҫ���жϴ��������洦�����ݣ��������Ӱ��Ч�ʡ�
		//��ע��:RX FIFOΪ4Bytes������ģʽ�£�����Cache�Ĵ��ڣ���һ��ִ���ж���ں����еĴ���Ч�ʻ�Ⱥ����
	}

	if( UART1_IOCtl(UART_IOCTL_RX_ERR_INT_GET, 0)
		&& (UART1_IOCtl(UART_IOCTL_RXSTAT_GET, 0) & 0x1C))//ERROR INT
	{
		UART1_IOCtl(UART_IOCTL_RX_ERR_CLR, 1);
		UART1_IOCtl(UART_IOCTL_TXFIFO_CLR, 1);
		UART1_IOCtl(UART_IOCTL_RXFIFO_CLR, 1);
		DBG("\nUART_IOCTL_RX_ERR_INT_GET\n");
    }

    if(UART1_IOCtl(UART_IOCTL_TXSTAT_GET, 0)) { //TX Finished INT
    	UART1_IOCtl(UART_IOCTL_TXINT_CLR, 1);
    	//DBG("\nUART_IOCTL_TXSTAT_GET\n");
    }

    if(UART1_IOCtl(UART_IOCTL_RXSTAT_GET, 0) & 0x02) { //OVERTIME INT
    	UART1_IOCtl(UART_IOCTL_OVERTIME_CLR, 1);
    	DBG("\nUART_IOCTL_RXSTAT_GET\n");
    }
}
static void DmaInterruptUart0Tx(void)
{

    UARTS_DMA_TxIntFlgClr(UART_PORT0, DMA_DONE_INT);
    UARTS_DMA_TxIntFlgClr(UART_PORT0, DMA_THRESHOLD_INT);
    UARTS_DMA_TxIntFlgClr(UART_PORT0, DMA_ERROR_INT);
}
static void DmaInterruptUart1Tx(void)
{
	static int Flag = 0;
	if(DMA_InterruptFlagGet(PERIPHERAL_ID_UART1_TX, DMA_THRESHOLD_INT))
	{
		if(Flag==0)
		{
			DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, DmaTempBuf, sizeof(DmaTempBuf)/2);
			Flag = 1;
		}else
		{
			DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, DmaTempBuf1, sizeof(DmaTempBuf1)/2);
			Flag = 0;
		}

		UARTS_DMA_TxIntFlgClr(UART_PORT1, DMA_THRESHOLD_INT);
		//DBG("\nUART1 TX DMA_THRESHOLD_INT\n");
	}
	if(DMA_InterruptFlagGet(PERIPHERAL_ID_UART1_TX, DMA_DONE_INT))
	{
		UARTS_DMA_TxIntFlgClr(UART_PORT1, DMA_DONE_INT);
		DBG("\nUART1 TX DMA_DONE_INT\n");
	}
	if(DMA_InterruptFlagGet(PERIPHERAL_ID_UART1_TX, DMA_ERROR_INT))
	{
		UARTS_DMA_TxIntFlgClr(UART_PORT1, DMA_ERROR_INT);
		DBG("\nUART1 TX DMA_ERROR_INT\n");
	}
}

static void DmaBlockInterruptUart1Tx(void)
{

	if(DMA_InterruptFlagGet(PERIPHERAL_ID_UART1_TX, DMA_DONE_INT))
	{
		//block send step 6:����������жϺ���
		UARTS_DMA_TxIntFlgClr(UART_PORT1, DMA_DONE_INT);//��DMA_DONE_INT�ж�
		DMA_ChannelDisable(PERIPHERAL_ID_UART1_TX);//�ر�DmaChannel���õ�ʱ���ٴ�
		UARTS_IOCTL(UART_PORT1,UART_IOCTL_DMA_TX_EN, 0);//�ر�DMA_TX,�õ�ʱ���ٴ�
		//DBG("\nUART1 TX DMA_DONE_INT\n");
	}
	if(DMA_InterruptFlagGet(PERIPHERAL_ID_UART1_TX, DMA_THRESHOLD_INT))
	{
		UARTS_DMA_TxIntFlgClr(UART_PORT1, DMA_THRESHOLD_INT);
//		DMA_ChannelDisable(PERIPHERAL_ID_UART1_TX);//�ر�DmaChannel���õ�ʱ���ٴ�
//		UARTS_IOCTL(UART_PORT1,UART_IOCTL_DMA_TX_EN, 0);//�ر�DMA_TX,�õ�ʱ���ٴ�
		DBG("\nUART1 TX DMA_THRESHOLD_INT\n");
	}
	if(DMA_InterruptFlagGet(PERIPHERAL_ID_UART1_TX, DMA_ERROR_INT))
	{
		UARTS_DMA_TxIntFlgClr(UART_PORT1, DMA_ERROR_INT);
		DBG("\nUART1 TX DMA_ERROR_INT\n");
	}
}
static void DmaInterruptUart0Rx(void)
{
    int32_t Echo, _result;
    Echo = UARTS_DMA_RecvDataStart(UART_PORT0, DmaTempBuf, DMA_TST_BUF_LEN);
    Buf_GetRoomLeft(&g_dma_ring_buf, _result);
    if(_result > Echo) {
        Buf_Push_Multi(&g_dma_ring_buf, DmaTempBuf, Echo, 0);
		dLenInDma += Echo;
    } else {
        DBG("uart0 ring buffer overflow\n");
    	;
    }
    UARTS_DMA_RxIntFlgClr(UART_PORT0, DMA_DONE_INT);
    UARTS_DMA_RxIntFlgClr(UART_PORT0, DMA_THRESHOLD_INT);
    UARTS_DMA_RxIntFlgClr(UART_PORT0, DMA_ERROR_INT);
}

static void DmaInterruptUart1Rx(void)
{
    int32_t Echo, _result;
    Echo = UARTS_DMA_RecvDataStart(UART_PORT1, DmaTempBuf, DMA_TST_BUF_LEN);
    Buf_GetRoomLeft(&g_dma_ring_buf, _result);
    if(_result > Echo) {
        Buf_Push_Multi(&g_dma_ring_buf, DmaTempBuf, Echo, 0);
    } else {
        DBG("uart1 ring buffer overflow\n");
    }

	if(DMA_InterruptFlagGet(PERIPHERAL_ID_UART1_RX, DMA_DONE_INT))
	{
		UARTS_DMA_RxIntFlgClr(UART_PORT1, DMA_DONE_INT);
		DBG("UART1 RX DMA_DONE_INT\n");
	}
	if(DMA_InterruptFlagGet(PERIPHERAL_ID_UART1_RX, DMA_THRESHOLD_INT))
	{
		UARTS_DMA_RxIntFlgClr(UART_PORT1, DMA_THRESHOLD_INT);//��ʱ��ˮλ�ж�
		//DBG("\n\t\t\tUART1 RX DMA_THRESHOLD_INT\n");
	}
	if(DMA_InterruptFlagGet(PERIPHERAL_ID_UART1_RX, DMA_ERROR_INT))
	{
		UARTS_DMA_RxIntFlgClr(UART_PORT1, DMA_ERROR_INT);
		DBG("UART1 RX DMA_ERROR_INT\n");
	}
}

INT_FUNC DmaIntTxFunArry[2] = {
    DmaInterruptUart0Tx,
    DmaInterruptUart1Tx,
};

INT_FUNC DmaIntRxFunArry[2] = {
    DmaInterruptUart0Rx,
    DmaInterruptUart1Rx,
};
//UART RX ����
void TGPIO_UartRxIoConfig(int8_t Which, uint8_t ModeSel)
{
    switch(Which)
    {
        case	0:
        		if(ModeSel == 0)
        	    {
        	         GPIO_PortAModeSet(GPIOA0, 0x2);//A0 UART RX
        	    }
        	    else if(ModeSel == 1)
        	    {
        	         GPIO_PortAModeSet(GPIOA5, 0x1); //A5 UART RX
        	    }
        	    else if(ModeSel == 2)
        	    {
        	         GPIO_PortAModeSet(GPIOA1, 0x2); //A1 UART RX
        	    }
                break;

        case	1:
    		if(ModeSel == 0)
    	    {
    	         GPIO_PortAModeSet(GPIOA9, 0x1);//A9 UART RX
    	    }
    	    else if(ModeSel == 1)
    	    {
    	         GPIO_PortAModeSet(GPIOA18, 0x1); //A1 UART RX
    	    }
    	    else if(ModeSel == 2)
    	    {
    	         GPIO_PortAModeSet(GPIOA24, 0x1); //A1 UART RX
    	    }

                break;
        default:
                break;
    }
}

//UART TX ����
void TGPIO_UartTxIoConfig(int8_t Which, uint8_t ModeSel)
{
    switch(Which)
    {
        case	0:
        		if(ModeSel == 0)
        	    {
        			GPIO_PortAModeSet(GPIOA1, 0xC);//A1 UART TX
        	    }
        	    else if(ModeSel == 1)
        	    {
        	        GPIO_PortAModeSet(GPIOA6, 0x5); //A6 UART TX
        	    }
        	    else if(ModeSel == 2)
        	    {
        	        GPIO_PortAModeSet(GPIOA0, 0x8); //A0 UART TX
        	    }
                break;

        case	1:
                if(ModeSel == 0)
                {
                	GPIO_PortAModeSet(GPIOA10, 0x3); //A10 UART TX
                }
                else if(ModeSel == 1)
                {
                	GPIO_PortAModeSet(GPIOA19, 0x3); //A19 UART TX
                }
                else if(ModeSel == 2)
                {
                	GPIO_PortAModeSet(GPIOA25, 0x6); //A25 UART TX
                }
                break;
        default:
                break;
    }
}

static int32_t WaitDatum1Ever(void)//�Ӵ��ڵȴ�����һ���ֽ�
{
	uint8_t Data;
	DBG("*******************Please input a num (0~9)*********************\n");
	do{
		if(0 < UART1_Recv(&Data, 1,10))
        {
			break;
		}
	}while(1);
	return Data-0x30;
}

void clean_ring_buffer(BUFFER_INFO *buf)
{
    Buf_Flush(buf);
    memset(buf->CharBuffer, 0x00, buf->Length);

}

void Example_UART_MCU_NonInt(void)
{
	uint8_t Buf[100];
	uint8_t len = 0;
	uint32_t cnt = 0;
	//˵����Example Ĭ��ʹ��UART1��PORT 0����Ϊʾ��;
	//     UART0: PORT0(RX-A0 TX-A1) PORT1(RX-A5 TX-A6) PORT2(RX-A1 TX-A0)
	//     UART1 PORT0(RX-A9 TX-A10) PORT1(RX-A18 TX-A19) PORT2(RX-A24 TX-A25)
	//step 0:ȷ��UART��ʱ���Ƿ���
	//step 1:����GPIO����
	TGPIO_UartRxIoConfig(UART_PORT1,0);//RX A9
	TGPIO_UartTxIoConfig(UART_PORT1,0);//TX A10

	//step 2:����UART����
	UART1_Init(115200, 8, 0, 1);//UART0_Init(115200, 8, 0, 1);

	//step 3:���ڽ���UART����
	DBG("\n***** you will receive what you have sent *****\n");
	while(1)
	{
		if((UART1_IOCtl(UART_IOCTL_RXSTAT_GET, 1) & 0x01)&&(cnt%100==0))//���յ�Ƶ�α�ϡ������ܻ�������
		{
		     len = UART1_Recv( Buf,10,100);//TimeOut����һ������յĳ�����䳤
		     UART1_IOCtl( UART_IOCTL_RXINT_CLR, 1);
		     if(len > 0)//������յ�����
		     {
		    	 UART1_Send(Buf,len,20);//�ѽ��յ������ݴ�ӡ����
		     }
		}
		cnt++;
	}
}

void Example_UART_MCU_Int(void)
{
	//˵����Example Ĭ��ʹ��UART1��PORT 0����Ϊʾ��;
	//     UART0: PORT0(RX-A0 TX-A1) PORT1(RX-A5 TX-A6) PORT2(RX-A1 TX-A0)
	//     UART1 PORT0(RX-A9 TX-A10) PORT1(RX-A18 TX-A19) PORT2(RX-A24 TX-A25)

	//step 0:ȷ��UART��ʱ���Ƿ���
	//step 1:����GPIO����
	TGPIO_UartRxIoConfig(UART_PORT1,0);//RX A9
	TGPIO_UartTxIoConfig(UART_PORT1,0);//TX A10

	//step 2:����UART����
	UART1_Init(115200, 8, 0, 1);//UART0_Init(115200, 8, 0, 1);

	//step 3:�����ж�
	//  ע���ж���ں����Ƿ�����,���û�����ã�ʹ��Ĭ�����UART1_Interrupt
	NVIC_EnableIRQ(UART1_IRQn);
	UART1_IOCtl(UART_IOCTL_RXINT_SET, 1);
	UART1_IOCtl(UART_IOCTL_RXINT_CLR, 1);
	UART1_IOCtl(UART_IOCTL_RXFIFO_CLR, 1);

	DBG("\n***** you will receive what you have sent *****\n");
	while(1);;;
}
void Example_UART_DMA_Block_Send(void)
{
	uint8_t buf[] = "Buf0 data has been send\n";
	uint8_t buf1[] = "Buf1 data has been send\n";
	memset(DmaTxBuf,0x38,512);
	memset(DmaTxBuf1,0x39,512);
	DBG("\n***** Select 0 1 2 3  *****\n");
	DBG("***** to choose different sendbuf *****\n");

	//˵����Example Ĭ��ʹ��UART1��PORT 0����Ϊʾ��;
		//     UART0: PORT0(RX-A0 TX-A1) PORT1(RX-A5 TX-A6) PORT2(RX-A1 TX-A0)
		//     UART1 PORT0(RX-A9 TX-A10) PORT1(RX-A18 TX-A19) PORT2(RX-A24 TX-A25)

	//block send step 0:ȷ��UART��ʱ���Ƿ���,��mian()
	//block send step 1:����GPIO����
	TGPIO_UartRxIoConfig(UART_PORT1,0);//RX A9
	TGPIO_UartTxIoConfig(UART_PORT1,0);//TX A10

	//block send step 2:����UART����
	UART1_Init(115200,8,0,1);

	//block send step 3:����DMA
	DMA_ChannelAllocTableSet(DmaChannelMap);//����DmaChannelMap����Ŀ��������UART RX��UART TX�ֱ�ʹ����һ·DMA
	if(DMA_BlockConfig(PERIPHERAL_ID_UART1_TX)!= DMA_OK)//����DMA Blockģʽ
	{
	   DBG("\nDma Block Mode Set Failure!\n");
	   return;
	}
	DMA_BlockBufSet(PERIPHERAL_ID_UART1_TX,(void*)DmaTxFifo,24);//����ҪBlockģʽ�õ�FIFO
	DMA_InterruptFunSet(PERIPHERAL_ID_UART1_TX, DMA_DONE_INT, DmaBlockInterruptUart1Tx);//�����жϺ������
	DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);//ʹ������UART1_TX��DMAͨ������TX��DmaChannel����������Ӧʹ����һ·Ӳ��DMA��DmaChannelMap�������úã�
	DMA_InterruptEnable(PERIPHERAL_ID_UART1_TX,DMA_DONE_INT,1);//������Ҫ�õ���DMA�жϣ�Blockģʽʹ�õ���DMA_DONE_INT�ж�
	DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, buf, 24);

	UART1_IOCtl( UART_IOCTL_DMA_TX_EN, 1);//ʹ��UART TX

	//block send step 4:��ʼ��������
	//UART1_DMA_SendDataStart(DmaTxBuf,0);//������Blockģʽ��ˮλ�жϵķ�ֵ����Ҫ���ã���0����

	//�����ã�����UART RX MCU�ж�ģʽ
	NVIC_EnableIRQ(UART1_IRQn);//ע���ж���ں����Ƿ�����,���û�����ã�ʹ��Ĭ�����UART1_Interrupt
	UART1_IOCtl(UART_IOCTL_RXINT_SET, 1);
	UART1_IOCtl(UART_IOCTL_RXINT_CLR, 1);
	UART1_IOCtl(UART_IOCTL_RXFIFO_CLR, 1);
    while(1)
    {
    	//Block���͵Ķ�BUF�л�ʾ��
    	switch(flag)
    	{
    	case '0':
    		DBG("\n\n[Select send buf ]:\n\t");
    		DMA_BlockBufSet(PERIPHERAL_ID_UART1_TX,(void*)buf,24);//�������÷���Buf
    		DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);//ʹ��DMA Channel
    		UART1_IOCtl( UART_IOCTL_DMA_TX_EN, 1);//ʹ��TX
    		DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, buf, 0);//��ʼBlock ��������
    		flag = 0;
    		break;
    	case '1':
    		DBG("\n\n[Select send buf2]:\n\t");
    		DMA_BlockBufSet(PERIPHERAL_ID_UART1_TX,(void*)buf1,24);
    		DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);
    		UART1_IOCtl( UART_IOCTL_DMA_TX_EN, 1);
    		DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, buf1, 0);//��ʼBlock ��������
    		flag = 0;
    		break;
    	case '2':
    		DBG("\n\n[Select send DmaTxBuf]:\n\t");
    		DMA_BlockBufSet(PERIPHERAL_ID_UART1_TX,(void*)DmaTxBuf,200);
    		DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);
    		UART1_IOCtl( UART_IOCTL_DMA_TX_EN, 1);
    		DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, DmaTxBuf, 0);//��ʼBlock ��������
    		flag = 0;
    		break;
    	case '3':
    		DBG("\n\n[Select send DmaTxBuf1]:\n\t");
    		DMA_BlockBufSet(PERIPHERAL_ID_UART1_TX,(void*)DmaTxBuf1,512);
    		DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);
    		UART1_IOCtl( UART_IOCTL_DMA_TX_EN, 1);
    		DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, DmaTxBuf1, 0);//��ʼBlock ��������
    		flag = 0;
    		break;
    	}
    }
}

void Example_UART_DMA_Circular_Int_Recv(void)
{
	int echo;
	//˵����Example Ĭ��ʹ��UART1��PORT 0����Ϊʾ��;
	//     UART0: PORT0(RX-A0 TX-A1) PORT1(RX-A5 TX-A6) PORT2(RX-A1 TX-A0)
	//     UART1 PORT0(RX-A9 TX-A10) PORT1(RX-A18 TX-A19) PORT2(RX-A24 TX-A25)

	//step 0:ȷ��UART��ʱ���Ƿ���
	//step 1:����GPIO����
	TGPIO_UartRxIoConfig(UART_PORT1,0);//RX A9
	TGPIO_UartTxIoConfig(UART_PORT1,0);//TX A10

	//step 2:����UART����
	UART1_Init(115200,8,0,1);//UARTģ���ʼ��

	//step 2:����DMA����
	DMA_ChannelAllocTableSet(DmaChannelMap);//����DmaChannelMap����Ŀ��������UART RX��UART TX�ֱ�ʹ����һ·DMA
	if(DMA_CircularConfig(PERIPHERAL_ID_UART1_RX, DMA_TST_BUF_LEN, (void*)DmaRxBuf, sizeof(DmaRxBuf)) != DMA_OK)//����Circularģʽ
	{
		return;
	}
	//DMA_CircularThresholdLenSet(PERIPHERAL_ID_UART1_RX, DMA_TST_BUF_LEN);//���������������ˮλ
	DMA_InterruptFunSet(PERIPHERAL_ID_UART1_RX, DMA_THRESHOLD_INT, DmaInterruptUart1Rx);//����ˮλ�жϺ������
	DMA_InterruptEnable(PERIPHERAL_ID_UART1_RX, DMA_THRESHOLD_INT, 1);//ʹ��ˮλ�ж�
	DMA_ChannelEnable(PERIPHERAL_ID_UART1_RX);//ʹ��DMAͨ��

	//step 3:ʹ��DMA_RX
	UART1_IOCtl(UART_IOCTL_DMA_RX_EN, 1);//ʹ��DMA RX

	//�����ã� ��ʼ��ѭ��buf
	Buf_init(&g_dma_ring_buf, BuartRGBuf, sizeof(BuartRGBuf));
	clean_ring_buffer(&g_dma_ring_buf);

	DBG("\n***** you will receive what you have sent *****\n");
	while(1)
	{
		echo=0;
		Buf_GetBytesAvail(&g_dma_ring_buf, echo);
		Buf_Pop_Multi(&g_dma_ring_buf, crc_check_buf, echo, 0);
		if(echo>0)
		{
			UART1_Send(crc_check_buf,echo,100);//���յ������ݷ���ѭ�����֣��������ٷ��ͳ���
		}
	}
}

void Example_UART_DMA_Circular_NoInt_Recv(void)
{
	int i,Echo;

	//˵����Example Ĭ��ʹ��UART1��PORT 0����Ϊʾ��;
	//     UART0: PORT0(RX-A0 TX-A1) PORT1(RX-A5 TX-A6) PORT2(RX-A1 TX-A0)
	//     UART1 PORT0(RX-A9 TX-A10) PORT1(RX-A18 TX-A19) PORT2(RX-A24 TX-A25)

	//step 0:ȷ��UART��ʱ���Ƿ���
	//step 1:����GPIO����
	TGPIO_UartRxIoConfig(UART_PORT1,0);//RX A9
	TGPIO_UartTxIoConfig(UART_PORT1,0);//TX A10

	//step 2:����UART����
	UART1_Init(115200,8,0,1);//UARTģ���ʼ��

	//step 3:����DMA����
	DMA_ChannelAllocTableSet(DmaChannelMap);//����DmaChannelMap����Ŀ��������UART RX��UART TX�ֱ�ʹ����һ·DMA
	DMA_ChannelDisable(PERIPHERAL_ID_UART1_RX);
	DMA_CircularConfig(PERIPHERAL_ID_UART1_RX,sizeof(DmaRxBuf)/2,DmaRxBuf,sizeof(DmaRxBuf));
	DMA_ChannelEnable(PERIPHERAL_ID_UART1_RX);

	//step 4:ʹ��RX
	UART1_IOCtl(UART_IOCTL_DMA_RX_EN, 1);

	DBG("\n***** you will receive what you have sent *****\n");
	while(1)
	{
		//���ݴ���ʾ����
		Echo = DMA_CircularDataLenGet(PERIPHERAL_ID_UART1_RX);
		if(Echo>0)
		{
			DMA_CircularDataGet(PERIPHERAL_ID_UART1_RX,DmaRxBuf,Echo);
			for(i=0;i<Echo;i++)
			{
				DBG("%c",DmaRxBuf[i]);//չʾ�˴��յ�������
			}
		}
	}
}
void Example_UART_DMA_Circular_Noint_Send(void)
{
	uint8_t buf[] = "0123456789abcdef";
	memset(DmaTxBuf,0x38,512);
	DBG("\n***** Select 0 1 2  *****\n");
	DBG("\n***** to choose different send data length *****\n");
	//˵����Example Ĭ��ʹ��UART1��PORT 0����Ϊʾ��;
	//     UART0: PORT0(RX-A0 TX-A1) PORT1(RX-A5 TX-A6) PORT2(RX-A1 TX-A0)
	//     UART1 PORT0(RX-A9 TX-A10) PORT1(RX-A18 TX-A19) PORT2(RX-A24 TX-A25)

	//step 0:ȷ��UART��ʱ���Ƿ���
	//step 1:����GPIO����
	TGPIO_UartTxIoConfig(UART_PORT1,0);//TX A10

	//step 2:����UART����
	UARTS_Init(UART_PORT1,115200,8,0,1);//UARTģ���ʼ��

	//step 3:����DMA����
	DMA_ChannelAllocTableSet(DmaChannelMap);//����DmaChannelMap����Ŀ��������UART RX��UART TX�ֱ�ʹ����һ·DMA
	UARTS_DMA_TxInit(UART_PORT1, (void*)DmaTxBuf, sizeof(DmaTxBuf), DMA_TST_BUF_LEN, DmaIntTxFunArry[UART_PORT1]);//����

	//step 4:��ʼ��������
	UARTS_DMA_SendDataStart(UART_PORT1,DmaTxBuf,256);

	//����UART RX��������ʾ��
	TGPIO_UartRxIoConfig(UART_PORT1,0);//A9
	NVIC_EnableIRQ(UART1_IRQn);
	UART1_IOCtl(UART_IOCTL_RXINT_SET, 1);
	UART1_IOCtl(UART_IOCTL_RXINT_CLR, 1);
	UART1_IOCtl(UART_IOCTL_RXFIFO_CLR, 1);

	while(1)
	{
		//���ݴ���ʾ�����任ˮλ����  �л�����Buf
		switch(flag)
		{
		case '0':
			DBG("\n\n[Select send buf]:\n\t");
			DMA_CircularConfig(PERIPHERAL_ID_UART1_TX,0,(void*)buf,16);//�������÷���Buf
			DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);//ʹ��DMA Channel
			UART1_IOCtl( UART_IOCTL_DMA_TX_EN, 1);//ʹ��TX
			//UARTS_DMA_SendDataStart(UART_PORT1,buf,32);//��ʼ��������  ���ͳ��ȳ���BufLen(ʾ������24�������Ͳ��ᳬ��24
			DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, (void*)buf, 32);
			flag = 0;
			break;
		case '1':
			DBG("\n\n[Select send buf]:\n\t");
			DMA_CircularConfig(PERIPHERAL_ID_UART1_TX,0,(void*)buf,16);//�������÷���Buf
			DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);//ʹ��DMA Channel
			UART1_IOCtl( UART_IOCTL_DMA_TX_EN, 1);//ʹ��TX
			//UARTS_DMA_SendDataStart(UART_PORT1,buf,10);//��ʼ��������  ���ͳ���С��BufLen(ʾ������10��
			DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, (void*)buf, 10);
			flag = 0;
			break;
		case '2':
			DBG("\n\n[Select send buf]:\n\t");
			DMA_CircularConfig(PERIPHERAL_ID_UART1_TX,0,(void*)buf,16);//�������÷���Buf
			DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);//ʹ��DMA Channel
			UART1_IOCtl( UART_IOCTL_DMA_TX_EN, 1);//ʹ��TX
			//UARTS_DMA_SendDataStart(UART_PORT1,buf,3);//��ʼ��������  ���ͳ���С��BufLen(ʾ������3��
			DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, (void*)buf, 3);
			flag = 0;
			break;
		}
	}
}

void Example_UART_DMA_Circular_Int_Send(void)
{
	memset(DmaTempBuf,0x30,512);
	memset(DmaTempBuf1,0x31,512);
	//����UART RX��������ʾ��
//	TGPIO_UartRxIoConfig(UART_PORT1,0);//A9
//	NVIC_EnableIRQ(UART1_IRQn);
//	UART1_IOCtl(UART_IOCTL_RXINT_SET, 1);
//	UART1_IOCtl(UART_IOCTL_RXINT_CLR, 1);
//	UART1_IOCtl(UART_IOCTL_RXFIFO_CLR, 1);

	//˵����Example Ĭ��ʹ��UART1��PORT 0����Ϊʾ��;
	//     UART0: PORT0(RX-A0 TX-A1) PORT1(RX-A5 TX-A6) PORT2(RX-A1 TX-A0)
	//     UART1 PORT0(RX-A9 TX-A10) PORT1(RX-A18 TX-A19) PORT2(RX-A24 TX-A25)

	//step 0:ȷ��UART��ʱ���Ƿ���
	//step 1:����GPIO����
	TGPIO_UartTxIoConfig(UART_PORT1,0);//TX A10

	//step 2:����UART����
	UARTS_Init(UART_PORT1,115200,8,0,1);//UARTģ���ʼ��

	//step 3:����DMA����
	DMA_ChannelDisable(PERIPHERAL_ID_UART1_TX);
	DMA_ChannelAllocTableSet(DmaChannelMap);//����DmaChannelMap����Ŀ��������UART RX��UART TX�ֱ�ʹ����һ·DMA
	if(DMA_CircularConfig(PERIPHERAL_ID_UART1_TX, 0, DmaTxFifo, sizeof(DmaTxFifo)) != DMA_OK)
	{
		return;
	}
	DMA_CircularThresholdLenSet(PERIPHERAL_ID_UART1_TX, 256);
	DMA_InterruptFunSet(PERIPHERAL_ID_UART1_TX, DMA_THRESHOLD_INT, DmaInterruptUart1Tx);
	DMA_InterruptEnable(PERIPHERAL_ID_UART1_TX, DMA_THRESHOLD_INT, 1);
	DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, DmaTempBuf, sizeof(DmaTempBuf)/2);//ע�ⷢ��BufҪ��FiFo��������

	DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);
	//step 4:ʹ��DMA_TX
	UART1_IOCtl(UART_IOCTL_DMA_TX_EN, 1);

	//���ݴ���ʾ����
	//���жϴ�����DmaInterruptUart1Tx
	while(1);
}

int main(void)
{
  	uint8_t Key;

	WDG_Disable();
	GIE_ENABLE();

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);//ALL_MODULE1_CLK_SWITCHʱ�Ӷ���
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_APllLock(240000);
	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_UARTClkSelect(APLL_CLK_MODE);
	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_UARTClkSelect(APLL_CLK_MODE);
	GPIO_PortAModeSet(GPIOA24, 1);//Rx,A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx,A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);//ʹ��UART1����ӡ
	SysTickInit();

	DBG("****************************************************************\n");
	DBG("               UART Example 2019 MVSilicon  \n");
	DBG("****************************************************************\n");

	DBG("Select an example:\n");
	DBG("0:: MCU with no interrupt\n");
	DBG("1:: MCU with interrupt\n");
	DBG("2:: DMA Circular send with interrupt\n");
	DBG("3:: DMA Circular send with no interrupt\n");
	DBG("4:: DMA Circular receive with interrupt\n");
	DBG("5:: DMA Circular receive with no interrupt\n");
	DBG("6:: DMA Block send\n");
	Key = WaitDatum1Ever();//Key = 1;

	switch(Key)
	{
	case 0:
		//MCU mode with no interrupt receive
		DBG("Selected: MCU with no interrupt\n");
		Example_UART_MCU_NonInt();
		break;
	case 1:
		// MCU mode with interrupt receive
		DBG("Selected:: MCU with interrupt\n");
		Example_UART_MCU_Int();
		break;
	case 2:
		// DMA circular mode send with interrupt
		DBG("DMA Circular send with interrupt example\n");
		Example_UART_DMA_Circular_Int_Send();
		break;
	case 3:
		// DMA circular mode send with no interrupt
		DBG("DMA Circular send with no interrupt example\n");
		Example_UART_DMA_Circular_Noint_Send();
		break;
	case 4:
		// DMA circular mode receive with interrupt
		DBG("DMA Circular receive with interrupt example\n");
		Example_UART_DMA_Circular_Int_Recv();
		break;
	case 5:
		// DMA circular mode receive with no interrupt
		DBG("DMA Circular receive with no interrupt example\n");
		Example_UART_DMA_Circular_NoInt_Recv();
		break;
	case 6:
		//  DMA block mode send
		DBG("DMA Block send example\n");
		Example_UART_DMA_Block_Send();
		break;

	default:
		break;
	}

	return -1;
}



