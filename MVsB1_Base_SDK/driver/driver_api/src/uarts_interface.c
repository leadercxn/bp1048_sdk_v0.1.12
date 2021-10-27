/**
 **************************************************************************************
 * @file    uart_interface.c
 * @brief   uart interface
 *
 * @author  Messi
 * @version V1.0.0
 *
 * $Created: 2015-10-28 16:15:00$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
#include "type.h"
#include "uarts.h"
#include "gpio.h"
#include "dma.h"
#include "timeout.h"
#include "uarts_interface.h"

#ifdef FUNC_OS_EN
#include "rtos_api.h"

osMutexId UART0Mutex = NULL;
osMutexId UART1Mutex = NULL;
#endif
//#define OS_RELATIVE//only test

//串口驱动APIs的包装函数，用于OS加锁
bool UART0_InitWpr(uint32_t BaudRate, uint8_t DataBits, uint8_t Parity, uint8_t StopBits)
{
	return UART0_Init(BaudRate, DataBits, Parity, StopBits);
}

bool UART1_InitWpr(uint32_t BaudRate, uint8_t DataBits, uint8_t Parity, uint8_t StopBits)
{
	return UART1_Init(BaudRate, DataBits, Parity, StopBits);
}

uint32_t UART0_RecvWpr(uint8_t* RecvBuf, uint32_t BufLen,uint32_t TimeOut)
{
	return UART0_Recv(RecvBuf, BufLen,TimeOut);
}

uint32_t UART1_RecvWpr(uint8_t* RecvBuf, uint32_t BufLen,uint32_t TimeOut)
{
	return UART1_Recv(RecvBuf, BufLen,TimeOut);
}

bool UART0_RecvByteWpr(uint8_t* Val)
{
	return UART0_RecvByte(Val);
}

bool UART1_RecvByteWpr(uint8_t* Val)
{
	return UART1_RecvByte(Val);
}

uint32_t UART0_SendWpr(uint8_t* SendBuf, uint32_t BufLen,uint32_t TimeOut)
{
	uint32_t Len;
#ifdef FUNC_OS_EN
	osMutexLock(UART0Mutex);
#endif
	Len =  UART0_Send(SendBuf, BufLen,TimeOut);
#ifdef FUNC_OS_EN
	osMutexUnlock(UART0Mutex);
#endif
	return Len;
}

uint32_t UART1_SendWpr(uint8_t* SendBuf, uint32_t BufLen,uint32_t TimeOut)
{
	uint32_t Len;

#ifdef FUNC_OS_EN
	osMutexLock(UART1Mutex);
#endif

	Len =  UART1_Send(SendBuf, BufLen,TimeOut);

#ifdef FUNC_OS_EN
	osMutexUnlock(UART1Mutex);
#endif
	return Len;
}

void UART0_SendByteWpr(uint8_t SendByte)
{
#ifdef FUNC_OS_EN
	osMutexLock(UART0Mutex);
#endif

	UART0_SendByte(SendByte);

#ifdef FUNC_OS_EN
	osMutexUnlock(UART0Mutex);
#endif
}

void UART1_SendByteWpr(uint8_t SendByte)
{
#ifdef FUNC_OS_EN
	osMutexLock(UART1Mutex);
#endif

	UART1_SendByte(SendByte);

#ifdef FUNC_OS_EN
	osMutexUnlock(UART1Mutex);
#endif
}

void UART0_SendByte_In_InterruptWpr(uint8_t SendByte)
{
#ifdef FUNC_OS_EN
	osMutexLock(UART0Mutex);
#endif

	UART0_SendByte_In_Interrupt(SendByte);

#ifdef FUNC_OS_EN
	osMutexUnlock(UART0Mutex);
#endif
}

void UART1_SendByte_In_InterruptWpr(uint8_t SendByte)
{
#ifdef FUNC_OS_EN
	osMutexLock(UART1Mutex);
#endif

	UART1_SendByte_In_Interrupt(SendByte);

#ifdef FUNC_OS_EN
	osMutexUnlock(UART1Mutex);
#endif
}

int32_t UART0_IOCtlWpr(UART_IOCTL_CMD_T Cmd, uint32_t Arg)
{
	return UART0_IOCtl(Cmd, Arg);
}

int32_t UART1_IOCtlWpr(UART_IOCTL_CMD_T Cmd, uint32_t Arg)
{
	return UART1_IOCtl(Cmd, Arg);
}

//UART DMA方式的完成回调函数
//对于接收，数据的路径:UART---->UART Rx FIFO---->DMA Rx buffer 回调函数放在DMA接收缓冲端
//对于发送，数据的路径:DMA Tx buffer---->UART Tx FIFO---->UART 回调函数放在UART发送中断ISR端
static INT_FUNC DmaTxIntDoneFun[2] = {
	NULL,
	NULL,
};

static INT_FUNC DmaRxIntDoneFun[2] = {
	NULL,
	NULL,
};

//目前确认UART-DMA采用Circular方式发送和接收数据
//接收方式初始化
bool UART0_DMA_RxInit(void* RxBufAddr, uint16_t RxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack)
{
    if(DMA_CircularConfig(PERIPHERAL_ID_UART0_RX, 0, RxBufAddr, RxBufLen) != DMA_OK)
    {
        return FALSE;
    }
	
    DMA_CircularThresholdLenSet(PERIPHERAL_ID_UART0_RX, ThresholdLen);
	DMA_InterruptFunSet(PERIPHERAL_ID_UART0_RX, DMA_THRESHOLD_INT, CallBack);
	DMA_InterruptEnable(PERIPHERAL_ID_UART0_RX, DMA_THRESHOLD_INT, 1);
    DMA_ChannelEnable(PERIPHERAL_ID_UART0_RX);
	UART0_IOCtl(UART_IOCTL_DMA_RX_EN, 1);
    return TRUE;
}

bool UART1_DMA_RxInit(void* RxBufAddr, uint16_t RxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack)
{
    if(DMA_CircularConfig(PERIPHERAL_ID_UART1_RX, 0, RxBufAddr, RxBufLen) != DMA_OK)
    {
        return FALSE;
    }
    DMA_CircularThresholdLenSet(PERIPHERAL_ID_UART1_RX, ThresholdLen);
	DMA_InterruptFunSet(PERIPHERAL_ID_UART1_RX, DMA_THRESHOLD_INT, CallBack);
	DMA_InterruptEnable(PERIPHERAL_ID_UART1_RX, DMA_THRESHOLD_INT, 1);
    DMA_ChannelEnable(PERIPHERAL_ID_UART1_RX);
    UART1_IOCtl(UART_IOCTL_DMA_RX_EN, 1);
    return TRUE;
}

//block模式发送方式初始化
bool UART0_DMA_BlockTxInit(INT_FUNC CallBack)
{
	if(DMA_BlockConfig(PERIPHERAL_ID_UART0_TX)!=DMA_OK)
	{
		return FALSE;
	}
	DMA_InterruptFunSet(PERIPHERAL_ID_UART0_TX, DMA_DONE_INT, CallBack);
	DMA_InterruptEnable(PERIPHERAL_ID_UART0_TX, DMA_DONE_INT, 1);
    DMA_ChannelEnable(PERIPHERAL_ID_UART0_TX);
    UART0_IOCtl(UART_IOCTL_DMA_TX_EN, 1);
    return TRUE;
}

bool UART1_DMA_BlockTxInit(INT_FUNC CallBack)
{
	if(DMA_BlockConfig(PERIPHERAL_ID_UART1_TX)!=DMA_OK)
	{
		return FALSE;
	}
	DMA_InterruptFunSet(PERIPHERAL_ID_UART1_TX, DMA_DONE_INT, CallBack);
	DMA_InterruptEnable(PERIPHERAL_ID_UART1_TX, DMA_DONE_INT, 1);
    //DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX, 1);
    UART1_IOCtl(UART_IOCTL_DMA_TX_EN, 1);
    return TRUE;
}


uint32_t UART0_DMA_BlockSend(uint8_t* SendBuf, uint16_t BufLen)
{
	DMA_BlockBufSet(PERIPHERAL_ID_UART0_TX,SendBuf,BufLen);
	DMA_ChannelEnable(PERIPHERAL_ID_UART0_TX);
	return 0;
}

uint32_t UART1_DMA_BlockSend(uint8_t* SendBuf, uint16_t BufLen)
{
	DMA_BlockBufSet(PERIPHERAL_ID_UART1_TX,SendBuf,BufLen);
	DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);
	return 0;
}

//发送方式初始化
bool UART0_DMA_TxInit(void* TxBufAddr, uint16_t TxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack)
{
    if(DMA_CircularConfig(PERIPHERAL_ID_UART0_TX, 0, TxBufAddr, TxBufLen) != DMA_OK)
    {
        return FALSE;
    }
    DMA_CircularThresholdLenSet(PERIPHERAL_ID_UART0_TX, ThresholdLen);
    DMA_InterruptFunSet(PERIPHERAL_ID_UART0_TX, DMA_THRESHOLD_INT, CallBack);
	//DMA_InterruptEnable(PERIPHERAL_ID_UART0_TX, DMA_THRESHOLD_INT, 1);
    DMA_ChannelEnable(PERIPHERAL_ID_UART0_TX);
    UART0_IOCtl(UART_IOCTL_DMA_TX_EN, 1);
    return TRUE;
}

bool UART1_DMA_TxInit(void* TxBufAddr, uint16_t TxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack)
{
    if(DMA_CircularConfig(PERIPHERAL_ID_UART1_TX, 0, TxBufAddr, TxBufLen) != DMA_OK)
    {
        return FALSE;
    }
    DMA_CircularThresholdLenSet(PERIPHERAL_ID_UART1_TX, ThresholdLen);
    DMA_InterruptFunSet(PERIPHERAL_ID_UART1_TX, DMA_THRESHOLD_INT, CallBack);
	//DMA_InterruptEnable(PERIPHERAL_ID_UART1_TX, DMA_THRESHOLD_INT, 1);
    DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);
    UART1_IOCtl(UART_IOCTL_DMA_TX_EN, 1);
    return TRUE;
}

uint32_t UART0_DMA_Send(uint8_t* SendBuf, uint16_t BufLen, uint32_t TimeOut)
{
    TIMER TimerRx;
	TimeOutSet(&TimerRx, TimeOut);
    DMA_CircularFIFOClear(PERIPHERAL_ID_UART0_TX);//如上次发送数据没有完成，会在此处被清掉
    DMA_CircularDataPut(PERIPHERAL_ID_UART0_TX, SendBuf, BufLen);
    while(DMA_CircularDataLenGet(PERIPHERAL_ID_UART0_TX) && !IsTimeOut(&TimerRx));
    while(!UART0_IOCtl(UART_IOCTL_TX_FIFO_EMPTY, 0));
    return BufLen;
}

uint32_t UART1_DMA_Send(uint8_t* SendBuf, uint16_t BufLen, uint32_t TimeOut)
{
    TIMER TimerRx;
	TimeOutSet(&TimerRx, TimeOut);
    DMA_CircularFIFOClear(PERIPHERAL_ID_UART1_TX);
    DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, SendBuf, BufLen);
    while(DMA_CircularDataLenGet(PERIPHERAL_ID_UART1_TX) && !IsTimeOut(&TimerRx) );
    while(!UART1_IOCtl(UART_IOCTL_TX_FIFO_EMPTY, 0));
    return BufLen;
}

uint32_t UART0_DMA_Recv(uint8_t* RecvBuf, uint16_t BufLen, uint32_t TimeOut)
{
    TIMER TimerRx;
	TimeOutSet(&TimerRx, TimeOut);
    while(( DMA_CircularDataLenGet(PERIPHERAL_ID_UART0_RX) < BufLen) && !IsTimeOut(&TimerRx));
    return DMA_CircularDataGet(PERIPHERAL_ID_UART0_RX, RecvBuf, BufLen);
}

uint32_t UART1_DMA_Recv(uint8_t* RecvBuf, uint16_t BufLen, uint32_t TimeOut)
{
    TIMER TimerRx;
	TimeOutSet(&TimerRx, TimeOut);
    while(( DMA_CircularDataLenGet(PERIPHERAL_ID_UART1_RX) < BufLen) && !IsTimeOut(&TimerRx));
    return DMA_CircularDataGet(PERIPHERAL_ID_UART1_RX, RecvBuf, BufLen);
}

//非阻塞方式发送数据(查询是否完成的接口函数)
//准备数据，使能发送
void UART0_DMA_SendDataStart(uint8_t* SendBuf, uint16_t BufLen)
{
    DMA_CircularFIFOClear(PERIPHERAL_ID_UART0_TX);//如上次发送数据没有完成，会在此处被清掉
    DMA_CircularDataPut(PERIPHERAL_ID_UART0_TX, SendBuf, BufLen);
}

void UART1_DMA_SendDataStart(uint8_t* SendBuf, uint16_t BufLen)
{
    DMA_CircularFIFOClear(PERIPHERAL_ID_UART1_TX);//如上次发送数据没有完成，会在此处被清掉
    DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, SendBuf, BufLen);
}

//判断是否传输完成
bool UART0_DMA_TxIsTransferDone(void)
{
	/*DMA BUFFER--->UART FIFO--->*/
	if(DMA_CircularDataLenGet(PERIPHERAL_ID_UART0_TX) == 0 && UART0_IOCtl(UART_IOCTL_TX_FIFO_EMPTY, 0))
		return TRUE;
	else
		return FALSE;
}

bool UART1_DMA_TxIsTransferDone(void)
{
	/*DMA BUFFER--->UART FIFO--->*/
	if(DMA_CircularDataLenGet(PERIPHERAL_ID_UART1_TX) == 0 && UART1_IOCtl(UART_IOCTL_TX_FIFO_EMPTY, 0))
		return TRUE;
	else
		return FALSE;
}

//非阻塞方式接收数据(查询是否完成的接口函数)
//准备缓冲区
int32_t UART0_DMA_RecvDataStart(uint8_t* RecvBuf, uint16_t BufLen)
{
	return DMA_CircularDataGet(PERIPHERAL_ID_UART0_RX, RecvBuf, BufLen);
}

int32_t UART1_DMA_RecvDataStart(uint8_t* RecvBuf, uint16_t BufLen)
{
	return DMA_CircularDataGet(PERIPHERAL_ID_UART1_RX, RecvBuf, BufLen);
}

//判断是否传输完成
bool UART0_DMA_RxIsTransferDone(void)
{
	/*UART FIFO--->DMA BUFFER--->*/
	if(DMA_CircularDataLenGet(PERIPHERAL_ID_UART0_RX) == 0 && UART0_IOCtl(UART_IOCTL_RX_FIFO_EMPTY, 0))
		return TRUE;
	else
		return FALSE;
}

bool UART1_DMA_RxIsTransferDone(void)
{
	/*UART FIFO--->DMA BUFFER--->*/
	if(DMA_CircularDataLenGet(PERIPHERAL_ID_UART1_RX) == 0 && UART1_IOCtl(UART_IOCTL_RX_FIFO_EMPTY, 0))
		return TRUE;
	else
		return FALSE;
}

//检查缓冲区数据长度
int32_t UART0_DMA_RxDataLen(void)
{
	return DMA_CircularDataLenGet(PERIPHERAL_ID_UART0_RX);
}

int32_t UART1_DMA_RxDataLen(void)
{
	return DMA_CircularDataLenGet(PERIPHERAL_ID_UART1_RX);
}

int32_t UART0_DMA_TxDataLen(void)
{
	return DMA_CircularDataLenGet(PERIPHERAL_ID_UART0_TX);
}

int32_t UART1_DMA_TxDataLen(void)
{
	return DMA_CircularDataLenGet(PERIPHERAL_ID_UART1_TX);
}

//DMA中断方式回调注册及关闭
void UART0_DMA_RxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack)
{
	if(CallBack)
	{
		DMA_InterruptFunSet(PERIPHERAL_ID_UART0_RX, IntType, CallBack);
		DMA_InterruptEnable(PERIPHERAL_ID_UART0_RX, IntType, 1);
	}
	else
	{
		DMA_InterruptEnable(PERIPHERAL_ID_UART0_RX, IntType, 0);
	}
}

void UART1_DMA_RxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack)
{
	if(CallBack)
	{
		DMA_InterruptFunSet(PERIPHERAL_ID_UART1_RX, IntType, CallBack);
		DMA_InterruptEnable(PERIPHERAL_ID_UART1_RX, IntType, 1);
	}
	else
	{
		DMA_InterruptEnable(PERIPHERAL_ID_UART1_RX, IntType, 0);
	}
}

void UART0_DMA_TxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack)
{
	if(CallBack)
	{
		DMA_InterruptFunSet(PERIPHERAL_ID_UART0_TX, IntType, CallBack);
		DMA_InterruptEnable(PERIPHERAL_ID_UART0_TX, IntType, 1);
	}
	else
	{
		DMA_InterruptEnable(PERIPHERAL_ID_UART0_TX, IntType, 0);
	}
}

void UART1_DMA_TxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack)
{
	if(CallBack)
	{
		DMA_InterruptFunSet(PERIPHERAL_ID_UART1_TX, IntType, CallBack);
		DMA_InterruptEnable(PERIPHERAL_ID_UART1_TX, IntType, 1);
	}
	else
	{
		DMA_InterruptEnable(PERIPHERAL_ID_UART1_TX, IntType, 0);
	}
}

//完成函数回调注册
//Tx
void UART0_DMA_TxSetTransmitDoneFun(INT_FUNC CallBack)
{
	DmaTxIntDoneFun[UART_PORT0] = CallBack;
}

void UART1_DMA_TxSetTransmitDoneFun(INT_FUNC CallBack)
{
	DmaTxIntDoneFun[UART_PORT1] = CallBack;
}

//Rx
void UART0_DMA_RxSetTransmitDoneFun(INT_FUNC CallBack)
{
	DmaRxIntDoneFun[UART_PORT0] = CallBack;
}

void UART1_DMA_RxSetTransmitDoneFun(INT_FUNC CallBack)
{
	DmaRxIntDoneFun[UART_PORT1] = CallBack;
}


//DMA 通道使能，设置DMA开始/停止工作
void UART0_DMA_RxChannelEn(void)
{
	DMA_ChannelEnable(PERIPHERAL_ID_UART0_RX);
}
void UART1_DMA_RxChannelEn(void)
{
	DMA_ChannelEnable(PERIPHERAL_ID_UART1_RX);
}

void UART0_DMA_TxChannelEn(void)
{
	DMA_ChannelEnable(PERIPHERAL_ID_UART0_TX);
}

void UART1_DMA_TxChannelEn(void)
{
	DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);
}

void UART0_DMA_RxChannelDisable(void)
{
	DMA_ChannelDisable(PERIPHERAL_ID_UART0_RX);
}

void UART1_DMA_RxChannelDisable(void)
{
	DMA_ChannelDisable(PERIPHERAL_ID_UART1_RX);
}


void UART0_DMA_TxChannelDisable(void)
{
	DMA_ChannelDisable(PERIPHERAL_ID_UART0_TX);
}

void UART1_DMA_TxChannelDisable(void)
{
	DMA_ChannelDisable(PERIPHERAL_ID_UART1_TX);
}




//DMA 中断标志位清除
//TX flag clear
int32_t UART0_DMA_TxIntFlgClr(DMA_INT_TYPE IntType)
{
	return DMA_InterruptFlagClear(PERIPHERAL_ID_UART0_TX, IntType);
}

int32_t UART1_DMA_TxIntFlgClr(DMA_INT_TYPE IntType)
{
	return DMA_InterruptFlagClear(PERIPHERAL_ID_UART1_TX, IntType);
}

//RX flag clear
int32_t UART0_DMA_RxIntFlgClr(DMA_INT_TYPE IntType)
{
	return DMA_InterruptFlagClear(PERIPHERAL_ID_UART0_RX, IntType);
}

int32_t UART1_DMA_RxIntFlgClr(DMA_INT_TYPE IntType)
{
	return DMA_InterruptFlagClear(PERIPHERAL_ID_UART1_RX, IntType);
}

/******************************************FUNCTION MAPS BEGIN*********************************************************************/
bool (*UARTS_Init_MAP[2])(uint32_t BaudRate, uint8_t DataBits, uint8_t Parity, uint8_t StopBits) = {
	UART0_InitWpr,
	UART1_InitWpr,
};

uint32_t (*UARTS_Recv_MAP[2])(uint8_t* RecvBuf, uint32_t BufLen,uint32_t TimeOut) = {
	UART0_RecvWpr,
	UART1_RecvWpr,
};

bool (*UARTS_RecvByte_MAP[2])(uint8_t* Val) = {
	UART0_RecvByteWpr,
	UART1_RecvByteWpr,
};

uint32_t (*UARTS_Send_MAP[2])(uint8_t* SendBuf, uint32_t BufLen, uint32_t TimeOut) = {
	UART0_SendWpr,
	UART1_SendWpr,
};



void (*UARTS_SendByte_MAP[2])(uint8_t SendByte) = {
	UART0_SendByteWpr,
	UART1_SendByteWpr,
};

void (*UARTS_SendByte_In_Interrupt_MAP[2])(uint8_t SendByte) = {
	UART0_SendByte_In_InterruptWpr,
	UART1_SendByte_In_InterruptWpr,
};


int32_t (*UARTS_IOCTL_MAP[2])(UART_IOCTL_CMD_T Cmd, uint32_t Arg) = {
	UART0_IOCtlWpr,
	UART1_IOCtlWpr,
};


/*UART DMA INTERFACE*/
bool (*UARTS_DMA_RxInit_MAP[2])(void* RxBufAddr, uint16_t RxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack) = {
	UART0_DMA_RxInit,
	UART1_DMA_RxInit,
};

bool (*UARTS_DMA_TxInit_MAP[2])(void* TxBufAddr, uint16_t TxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack) = {
	UART0_DMA_TxInit,
	UART1_DMA_TxInit,
};

uint32_t (*UARTS_DMA_Send_MAP[2])(uint8_t* SendBuf, uint16_t BufLen, uint32_t TimeOut) = {
	UART0_DMA_Send,
	UART1_DMA_Send,
};

uint32_t (*UARTS_DMA_Recv_MAP[2])(uint8_t* RecvBuf, uint16_t BufLen, uint32_t TimeOut) = {
	UART0_DMA_Recv,
	UART1_DMA_Recv,
};

void (*UARTS_DMA_SendDataStart_MAP[2])(uint8_t* SendBuf, uint16_t BufLen) = {
	UART0_DMA_SendDataStart,
	UART1_DMA_SendDataStart,
};

bool (*UARTS_DMA_TxIsTransferDone_MAP[2])(void) = {
	UART0_DMA_TxIsTransferDone,
	UART1_DMA_TxIsTransferDone,
};

int32_t (*UARTS_DMA_RecvDataStart_MAP[2])(uint8_t* RecvBuf, uint16_t BufLen) = {
	UART0_DMA_RecvDataStart,
	UART1_DMA_RecvDataStart,
};

bool (*UARTS_DMA_RxIsTransferDone_MAP[2])(void) = {
	UART0_DMA_RxIsTransferDone,
	UART1_DMA_RxIsTransferDone,
};

int32_t (*UARTS_DMA_RxDataLen_MAP[2])(void) = {
	UART0_DMA_RxDataLen,
	UART1_DMA_RxDataLen,
};

int32_t (*UARTS_DMA_TxDataLen_MAP[2])(void) = {
	UART0_DMA_TxDataLen,
	UART1_DMA_TxDataLen,
};

void (*UARTS_DMA_RxSetInterruptFun_MAP[2])(DMA_INT_TYPE IntType, INT_FUNC CallBack) = {
	UART0_DMA_RxSetInterruptFun,
	UART1_DMA_RxSetInterruptFun,
};

void (*UARTS_DMA_TxSetInterruptFun_MAP[2])(DMA_INT_TYPE IntType, INT_FUNC CallBack) = {
	UART0_DMA_TxSetInterruptFun,
	UART1_DMA_TxSetInterruptFun,
};

void (*UARTS_DMA_RxChannelEn_MAP[2])(void) = {
	UART0_DMA_RxChannelEn,
	UART1_DMA_RxChannelEn,
};

void (*UARTS_DMA_RxChannelDisable_MAP[2])(void) = {
	UART0_DMA_RxChannelDisable,
	UART1_DMA_RxChannelDisable,
};

void (*UARTS_DMA_TxChannelEn_MAP[2])(void) = {
	UART0_DMA_TxChannelEn,
	UART1_DMA_TxChannelEn,
};

void (*UARTS_DMA_TxChannelDisable_MAP[2])(void) = {
	UART0_DMA_TxChannelDisable,
	UART1_DMA_TxChannelDisable,
};

int32_t (*UARTS_DMA_TxIntFlgClr_MAP[2])(DMA_INT_TYPE IntType) = {
	UART0_DMA_TxIntFlgClr,
	UART1_DMA_TxIntFlgClr,
};

int32_t (*UARTS_DMA_RxIntFlgClr_MAP[2])(DMA_INT_TYPE IntType) = {
	UART0_DMA_RxIntFlgClr,
	UART1_DMA_RxIntFlgClr,
};

void (*UARTS_DMA_TxSetTransmitDoneFun_MAP[2])(INT_FUNC CallBack) = {
	UART0_DMA_TxSetTransmitDoneFun,
	UART1_DMA_TxSetTransmitDoneFun,
};

void (*UARTS_DMA_RxSetTransmitDoneFun_MAP[2])(INT_FUNC CallBack) = {
	UART0_DMA_RxSetTransmitDoneFun,
	UART1_DMA_RxSetTransmitDoneFun,
};

/******************************************FUNCTION MAPS ENDS****************************************************************/




