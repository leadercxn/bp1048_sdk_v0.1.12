/**
 **************************************************************************************
 * @file    i2sin_mode.c
 * @brief   
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2019-1-4 17:29:47$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include "type.h"
#include "irqn.h"
#include "gpio.h"
#include "dma.h"
#include "rtos_api.h"
#include "app_message.h"
#include "app_config.h"
#include "debug.h"
#include "delay.h"
#include "audio_adc.h"
#include "dac.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "decoder_service.h"
#include "remind_sound_service.h"
#include "recorder_service.h"
#include "main_task.h"
#include "audio_effect.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "audio_vol.h"
#include "i2s.h"
#include "i2s_interface.h"
#include "ctrlvars.h"
#include "mcu_circular_buf.h"
#include "audio_adjust.h"

#ifdef CFG_APP_I2SIN_MODE_EN


#define I2SIN_PLAY_TASK_STACK_SIZE		512//1024
#define I2SIN_PLAY_TASK_PRIO			4//3
#define I2SIN_NUM_MESSAGE_QUEUE			10

#define I2SIN_SOURCE_NUM				APP_SOURCE_NUM

typedef struct _I2SInPlayContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;

	TaskState			state;

	uint32_t			*I2SFIFO1;			//I2S的DMA循环fifo
	uint16_t 			*Source1Buf_I2S;	//
	AudioCoreContext 	*AudioCoreI2SIn;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	uint16_t*			Source2Decoder;
	TaskState			DecoderSync;
#endif

	//play
	uint32_t 			SampleRate; //带提示音时，如果不重采样，要避免采样率配置冲突

#ifdef CFG_FUNC_RECORDER_EN
	TaskState			RecorderSync;
#endif	

#ifdef	CFG_FUNC_SOFT_ADJUST_IN
	int16_t* 				pBufTemp;//[128*4];//SRA转换前固定buf 128 samples
	MCU_CIRCULAR_CONTEXT 	CircularBufSRA;
	uint16_t				*SRAFifo;
#endif
}I2SInPlayContext;

/**根据appconfig缺省配置:DMA 8个通道配置**/
/*1、cec需PERIPHERAL_ID_TIMER3*/
/*2、SD卡录音需PERIPHERAL_ID_SDIO RX/TX*/
/*3、在线串口调音需PERIPHERAL_ID_UART1 RX/TX,建议使用USB HID，节省DMA资源*/
/*4、线路输入需PERIPHERAL_ID_AUDIO_ADC0_RX*/
/*5、Mic开启需PERIPHERAL_ID_AUDIO_ADC1_RX，mode之间通道必须一致*/
/*6、Dac0开启需PERIPHERAL_ID_AUDIO_DAC0_TX mode之间通道必须一致*/
/*7、DacX需开启PERIPHERAL_ID_AUDIO_DAC1_TX mode之间通道必须一致*/
/*注意DMA 8个通道配置冲突:*/
/*a、UART在线调音和DAC-X有冲突,默认在线调音使用USB HID*/
/*b、UART在线调音与HDMI/SPDIF模式冲突*/
static const uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,	//0
	255,//PERIPHERAL_ID_SPIS_TX,		//1
#ifdef CFG_APP_HDMIIN_MODE_EN
	5,//PERIPHERAL_ID_TIMER3,			//2
#else
	255,//PERIPHERAL_ID_TIMER3,			//2
#endif
	4,//PERIPHERAL_ID_SDIO_RX,			//3
	4,//PERIPHERAL_ID_SDIO_TX,			//4
	255,//PERIPHERAL_ID_UART0_RX,		//5
	255,//PERIPHERAL_ID_TIMER1,			//6
	255,//PERIPHERAL_ID_TIMER2,			//7
	255,//PERIPHERAL_ID_SDPIF_RX,		//8 SPDIF_RX /TX same chanell
	255,//PERIPHERAL_ID_SDPIF_TX,		//8 SPDIF_RX /TX same chanell
	255,//PERIPHERAL_ID_SPIM_RX,		//9
	255,//PERIPHERAL_ID_SPIM_TX,		//10
	255,//PERIPHERAL_ID_UART0_TX,		//11
	
#ifdef CFG_COMMUNICATION_BY_UART	
	7,//PERIPHERAL_ID_UART1_RX,			//12
	6,//PERIPHERAL_ID_UART1_TX,			//13
#else
	255,//PERIPHERAL_ID_UART1_RX,		//12
	255,//PERIPHERAL_ID_UART1_TX,		//13
#endif

	255,//PERIPHERAL_ID_TIMER4,			//14
	255,//PERIPHERAL_ID_TIMER5,			//15
	255,//PERIPHERAL_ID_TIMER6,			//16
	0,//PERIPHERAL_ID_AUDIO_ADC0_RX,	//17
	1,//PERIPHERAL_ID_AUDIO_ADC1_RX,	//18
	2,//PERIPHERAL_ID_AUDIO_DAC0_TX,	//19
	3,//PERIPHERAL_ID_AUDIO_DAC1_TX,	//20
#if (CFG_RES_I2S_PORT==0)
	6,//PERIPHERAL_ID_I2S0_RX,		//21
	7,//PERIPHERAL_ID_I2S0_TX,		//22
	255,//PERIPHERAL_ID_I2S1_RX,		//23
	255,//PERIPHERAL_ID_I2S1_TX,		//24
#else
	255,//PERIPHERAL_ID_I2S0_RX,		//21
	255,//PERIPHERAL_ID_I2S0_TX,		//22
	6,//PERIPHERAL_ID_I2S1_RX,		//23
	7,//PERIPHERAL_ID_I2S1_TX,		//24
#endif

	255,//PERIPHERAL_ID_PPWM,			//25
	255,//PERIPHERAL_ID_ADC,     		//26
	255,//PERIPHERAL_ID_SOFTWARE,		//27
};

static  I2SInPlayContext*		I2SInPlayCt;


static void I2SInPlayModeCreating(uint16_t msgId);
static void I2SInPlayModeStarting(uint16_t msgId);
static void I2SInPlayModeStopping(uint16_t msgId);
static void I2SInPlayModeStopped(void);
static void I2SInPlayRunning(uint16_t msgId);
uint8_t I2SInDecoderSourceNum(void);

#ifdef CFG_FUNC_SOFT_ADJUST_IN_I2SIN

uint16_t I2S_DataGet(void* Buf, uint16_t Len)
{
	uint16_t DaneLen;
	DaneLen = MCUCircular_GetData(&I2SInPlayCt->CircularBufSRA, (void *)Buf, Len * 4);
	return DaneLen / 4;
}

uint16_t I2S_DataLenGet(void)
{
	return MCUCircular_GetDataLen(&I2SInPlayCt->CircularBufSRA) / 4;
}

uint16_t I2S_DataSpaceLenGet(void)
{
	return MCUCircular_GetSpaceLen(&I2SInPlayCt->CircularBufSRA) / 4;
}

void I2SINDataProcess(void)
{
	uint16_t SRADoneLen;

#if (CFG_RES_I2S_IO_PORT==0)
	if((AudioI2S0_DataLenGet() >= 128)
	&& SoftFlagGet(SoftFlagBtSra)
	&& (I2SInPlayCt->state == TaskStateRunning))
	{
		if(MCUCircular_GetSpaceLen(&I2SInPlayCt->CircularBufSRA) / 4 - 1 > 128)
		{
			SRADoneLen = AudioI2S0_DataGet(I2SInPlayCt->pBufTemp, 128);
			SRADoneLen = AudioSourceSRAProcess(I2SInPlayCt->pBufTemp, 128);
			MCUCircular_PutData(&I2SInPlayCt->CircularBufSRA, (uint8_t*)audioSourceAdjustCt->SraPcmOutnBuf, SRADoneLen*2*2);
		}
	}
#else
	if((AudioI2S1_DataLenGet() >= 128)
	&& SoftFlagGet(SoftFlagBtSra)
	&& (I2SInPlayCt->state == TaskStateRunning))
	{
		if(MCUCircular_GetSpaceLen(&I2SInPlayCt->CircularBufSRA) / 4 - 1 > 128)
		{
			SRADoneLen = AudioI2S1_DataGet(I2SInPlayCt->pBufTemp, 128);
			SRADoneLen = AudioSourceSRAProcess(I2SInPlayCt->pBufTemp, 128);
			MCUCircular_PutData(&I2SInPlayCt->CircularBufSRA, (uint8_t*)audioSourceAdjustCt->SraPcmOutnBuf, SRADoneLen*2*2);
		}
	}
#endif
}

#endif

void I2SInPlayResRelease(void)
{
	uint32_t RxPeripheralID;
#if (CFG_RES_I2S_PORT == 0)
	RxPeripheralID=PERIPHERAL_ID_I2S0_RX;
	I2S_ModuleDisable(I2S0_MODULE);
#else
	RxPeripheralID=PERIPHERAL_ID_I2S1_RX;
	I2S_ModuleDisable(I2S1_MODULE);
#endif
	
	DMA_InterruptFlagClear(RxPeripheralID, DMA_DONE_INT);
	DMA_InterruptFlagClear(RxPeripheralID, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(RxPeripheralID, DMA_ERROR_INT);
	DMA_ChannelDisable(RxPeripheralID);

	if(I2SInPlayCt->I2SFIFO1 != NULL)
	{
		APP_DBG("I2SFIFO1\n");
		osPortFree(I2SInPlayCt->I2SFIFO1);
		I2SInPlayCt->I2SFIFO1 = NULL;
	}
	if(I2SInPlayCt->Source1Buf_I2S  != NULL)
	{
		APP_DBG("Source1Buf_I2S\n");
		osPortFree(I2SInPlayCt->Source1Buf_I2S );
		I2SInPlayCt->Source1Buf_I2S  = NULL;
	}
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(I2SInPlayCt->Source2Decoder != NULL)
	{
		APP_DBG("Source2Decoder\n");
		osPortFree(I2SInPlayCt->Source2Decoder);
		I2SInPlayCt->Source2Decoder = NULL;
	}
#endif

#ifdef CFG_FUNC_SOFT_ADJUST_IN_I2SIN
	if(I2SInPlayCt->SRAFifo != NULL)
	{
		APP_DBG("I2SInPlayCt.SRAFifo memory free!\n");
		osPortFree(I2SInPlayCt->SRAFifo);
		I2SInPlayCt->SRAFifo = NULL;
	}
	AudioCoreSourceFreqAdjustDisable();
#endif
}

bool I2SInPlayResMalloc(uint16_t SampleLen)
{
	//I2SIn  digital (DMA)
	I2SInPlayCt->I2SFIFO1 = (uint32_t*)osPortMallocFromEnd(SampleLen * 2 * 2 * 2);
	if(I2SInPlayCt->I2SFIFO1 == NULL)
	{
		return FALSE;
	}
	memset(I2SInPlayCt->I2SFIFO1, 0, SampleLen * 2 * 2 * 2);

	//InCore1 buf
	I2SInPlayCt->Source1Buf_I2S = (uint16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);//stereo one Frame
	if(I2SInPlayCt->Source1Buf_I2S == NULL)
	{
		return FALSE;
	}
	memset(I2SInPlayCt->Source1Buf_I2S, 0, SampleLen * 2 * 2);

#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	I2SInPlayCt->Source2Decoder = (uint16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);//One Frame
	if(I2SInPlayCt->Source2Decoder == NULL)
	{
		return FALSE;
	}
	memset(I2SInPlayCt->Source2Decoder, 0, SampleLen * 2 * 2);//2K
#endif

#ifdef CFG_FUNC_SOFT_ADJUST_IN_I2SIN
	uint32_t Len;
	if(SampleLen == 128)
	{
		Len = SampleLen * 2 * 2 * 2 + 4 * 2 + 128 * 4;//128帧长时需要增加一个128sample buf,否则软件微调会出问题
	}
	else
	{
		Len = SampleLen * 2 * 2 * 2 + 4 * 2;
	}
	I2SInPlayCt->SRAFifo = osPortMallocFromEnd(Len);
	if(I2SInPlayCt->SRAFifo == NULL)
	{
		APP_DBG("I2SInPlayCt->SRAFifo memory error\n");
		return FALSE;
	}
	memset(I2SInPlayCt->SRAFifo, 0, Len);
	MCUCircular_Config(&I2SInPlayCt->CircularBufSRA, I2SInPlayCt->SRAFifo, Len);
#endif

	return TRUE;
}

void I2SInPlayResInit(void)
{
	uint32_t RxPeripheralID;
#if (CFG_RES_I2S_PORT == 0)
	RxPeripheralID=PERIPHERAL_ID_I2S0_RX;
#else
	RxPeripheralID=PERIPHERAL_ID_I2S1_RX;
#endif

	if(I2SInPlayCt->Source1Buf_I2S != NULL)
	{
		I2SInPlayCt->AudioCoreI2SIn->AudioSource[I2SIN_SOURCE_NUM].PcmInBuf = (int16_t *)I2SInPlayCt->Source1Buf_I2S;
	}
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(I2SInPlayCt->Source2Decoder != NULL)
	{
		I2SInPlayCt->AudioCoreI2SIn->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)I2SInPlayCt->Source2Decoder;
	}
#endif

	//DMA_ChannelDisable(PERIPHERAL_ID_I2S1_RX);
	DMA_CircularConfig(RxPeripheralID, mainAppCt.SamplesPreFrame * 2 * 2, (void*)I2SInPlayCt->I2SFIFO1, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
    DMA_ChannelEnable(RxPeripheralID);
    //AudioADC_LREnable(ADC0_MODULE, 1, 1);
    //AudioADC_Enable(ADC0_MODULE);
#if (CFG_RES_I2S_PORT == 0)
	I2S_ModuleEnable(I2S0_MODULE);
#else
	I2S_ModuleEnable(I2S1_MODULE);
#endif
#ifdef CFG_FUNC_SOFT_ADJUST_IN_I2SIN
	AudioCoreSourceFreqAdjustEnable(I2SIN_SOURCE_NUM, 128, 128);
#endif
}

/**
 * @func        I2SInPlay_Init
 * @brief       I2SIn模式参数配置，资源初始化
 * @param       MessageHandle parentMsgHandle
 * @Output      None
 * @return      bool
 * @Others      任务块、I2S、Dac、AudioCore配置
 * @Others      数据流从I2S到audiocore配有函数指针，audioCore到Dac同理，由audiocoreService任务按需驱动
 * Record
 */
static bool I2SInPlay_Init(MessageHandle parentMsgHandle)
{
	I2SParamCt i2s_set;

//System config
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);//
//Task & App Config
	I2SInPlayCt = (I2SInPlayContext*)osPortMalloc(sizeof(I2SInPlayContext));
	if(I2SInPlayCt == NULL)
	{
		return FALSE;
	}
	memset(I2SInPlayCt, 0, sizeof(I2SInPlayContext));
	I2SInPlayCt->msgHandle = MessageRegister(I2SIN_NUM_MESSAGE_QUEUE);
	if(I2SInPlayCt->msgHandle == NULL)
	{
		return FALSE;
	}
	I2SInPlayCt->parentMsgHandle = parentMsgHandle;
	I2SInPlayCt->state = TaskStateCreating;
	I2SInPlayCt->SampleRate = CFG_PARA_SAMPLE_RATE;
	//Core Source1 para
	I2SInPlayCt->AudioCoreI2SIn = (AudioCoreContext*)&AudioCore;

//	//I2SIn  digital (DMA)
//	I2SInPlayCt->I2SFIFO1 = (uint32_t*)osPortMalloc(I2SIN_FIFO_LEN);
//	if(I2SInPlayCt->I2SFIFO1 == NULL)
//	{
//		return FALSE;
//	}
//	memset(I2SInPlayCt->I2SFIFO1, 0, I2SIN_FIFO_LEN);
#ifdef CFG_FUNC_SOFT_ADJUST_IN_I2SIN
	I2SInPlayCt->pBufTemp = osPortMalloc(128 * 4);
	if(I2SInPlayCt->pBufTemp == NULL)
	{
		APP_DBG("I2SInPlayCt->pBufTemp memory error\n");
		return FALSE;
	}
	memset(I2SInPlayCt->pBufTemp, 0, 128 * 4);
#endif

	if(!I2SInPlayResMalloc(mainAppCt.SamplesPreFrame))
	{
		APP_DBG("I2SInPlay Res Error!\n");
		return FALSE;
	}

	i2s_set.IsMasterMode=CFG_RES_I2S_MODE;// 0:master 1:slave
	i2s_set.SampleRate=I2SInPlayCt->SampleRate;
	i2s_set.I2sFormat=I2S_FORMAT_I2S;
	i2s_set.I2sBits=I2S_LENGTH_16BITS;
	i2s_set.I2sTxRxEnable=2;
#if (CFG_RES_I2S_PORT == 0)
	i2s_set.RxPeripheralID=PERIPHERAL_ID_I2S0_RX;
#else
	i2s_set.RxPeripheralID=PERIPHERAL_ID_I2S1_RX;
#endif
	i2s_set.RxBuf=I2SInPlayCt->I2SFIFO1;
	i2s_set.RxLen=mainAppCt.SamplesPreFrame * 2 * 2 * 2 ;//I2SIN_FIFO_LEN;

#if (CFG_RES_I2S_IO_PORT==0)
//i2s0  group_gpio0
	GPIO_PortAModeSet(GPIOA0, 9);// mclk out
	GPIO_PortAModeSet(GPIOA1, 6);// lrclk
	GPIO_PortAModeSet(GPIOA2, 5);// bclk
	GPIO_PortAModeSet(GPIOA3, 7);// dout
	GPIO_PortAModeSet(GPIOA4, 1);// din
//i2s0  group_gpio0
#else //lif (CFG_RES_I2S_IO_PORT==1)
//i2s1  group_gpio1 
	GPIO_PortAModeSet(GPIOA7, 5);//mclk out
	GPIO_PortAModeSet(GPIOA8, 1);//lrclk
	GPIO_PortAModeSet(GPIOA9, 2);//bclk
	GPIO_PortAModeSet(GPIOA10, 4);//do
	GPIO_PortAModeSet(GPIOA11, 2);//di
//i2s1  group_gpio1 
#endif

#if (CFG_RES_I2S_PORT == 0)
	I2S_ModuleDisable(I2S0_MODULE);
	AudioI2S_Init(I2S0_MODULE,&i2s_set);
	//AudioI2S_DMAInit(PERIPHERAL_ID_I2S0_RX, (void*)I2SInPlayCt->I2SFIFO1, I2SIN_FIFO_LEN);
#else
	I2S_ModuleDisable(I2S1_MODULE);
	AudioI2S_Init(I2S1_MODULE, &i2s_set);
	//AudioI2S_DMAInit(PERIPHERAL_ID_I2S1_RX, (void*)I2SInPlayCt->I2SFIFO1, I2SIN_FIFO_LEN);
#endif
//	//note Soure0.和sink0已经在main app中配置，不要随意配置
	//Core Soure1.Para
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[I2SIN_SOURCE_NUM].Enable = 0;
#ifndef CFG_FUNC_SOFT_ADJUST_IN_I2SIN
#if (CFG_RES_I2S_IO_PORT==0)
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[I2SIN_SOURCE_NUM].FuncDataGet = AudioI2S0_DataGet ;
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[I2SIN_SOURCE_NUM].FuncDataGetLen = AudioI2S0_DataLenGet;
#else
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[I2SIN_SOURCE_NUM].FuncDataGet = AudioI2S1_DataGet ;
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[I2SIN_SOURCE_NUM].FuncDataGetLen = AudioI2S1_DataLenGet;
#endif
#else
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[I2SIN_SOURCE_NUM].FuncDataGet = I2S_DataGet ;
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[I2SIN_SOURCE_NUM].FuncDataGetLen = I2S_DataLenGet;
#endif
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[I2SIN_SOURCE_NUM].IsSreamData = FALSE;//TRUE;
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[I2SIN_SOURCE_NUM].PcmFormat = 2;//stereo
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[I2SIN_SOURCE_NUM].PcmInBuf = (int16_t *)I2SInPlayCt->Source1Buf_I2S;

#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	//Core Soure2 Para
	DecoderSourceNumSet(REMIND_SOURCE_NUM);
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[REMIND_SOURCE_NUM].Enable = 0;
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 2; //stereo
	I2SInPlayCt->AudioCoreI2SIn->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)I2SInPlayCt->Source2Decoder;
#endif

	//Core Process	
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	#ifdef CFG_FUNC_MIC_KARAOKE_EN
	I2SInPlayCt->AudioCoreI2SIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioEffectProcess;
	#else
	I2SInPlayCt->AudioCoreI2SIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
	#endif
#else
	I2SInPlayCt->AudioCoreI2SIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
#endif

#ifdef CFG_FUNC_RECORDER_EN
	I2SInPlayCt->RecorderSync = TaskStateNone;
#endif

#ifdef CFG_FUNC_SOFT_ADJUST_IN_I2SIN
	AudioCoreSourceSRAResMalloc();
	SoftFlagRegister(SoftFlagBtSra);
#endif
	return TRUE;
}

//创建从属services
static void I2SInPlayModeCreate(void)
{
	bool NoService = TRUE;
	
	// Create service task
#ifdef CFG_FUNC_REMIND_SBC
	DecoderServiceCreate(I2SInPlayCt->msgHandle, DECODER_BUF_SIZE_SBC, DECODER_FIFO_SIZE_FOR_SBC);//提示音格式决定解码器内存消耗
	NoService = FALSE;
#elif defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceCreate(I2SInPlayCt->msgHandle, DECODER_BUF_SIZE_MP3, DECODER_FIFO_SIZE_FOR_MP3);
	NoService = FALSE;
#endif
	if(NoService)
	{
		I2SInPlayModeCreating(MSG_NONE);
	}
}

//All of services is created
//Send CREATED message to parent
static void I2SInPlayModeCreating(uint16_t msgId)
{
	MessageContext		msgSend;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_CREATED)
	{
		I2SInPlayCt->DecoderSync = TaskStateReady;
	}
	if(I2SInPlayCt->DecoderSync == TaskStateReady)
#endif
	{
		msgSend.msgId		= MSG_I2SIN_AUDIO_MODE_CREATED;//bkd
		MessageSend(I2SInPlayCt->parentMsgHandle, &msgSend);
		I2SInPlayCt->state = TaskStateReady;
	}
}

static void I2SInPlayModeStart(void)
{
	bool NoService = TRUE;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceStart();
	I2SInPlayCt->DecoderSync = TaskStateStarting;
	NoService = FALSE;
#endif
	if(NoService)
	{
		I2SInPlayModeStarting(MSG_NONE);
	}
	else
	{
		I2SInPlayCt->state = TaskStateStarting;
	}
}

static void I2SInPlayModeStarting(uint16_t msgId)
{
	MessageContext		msgSend;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_STARTED)
	{
		I2SInPlayCt->DecoderSync = TaskStateRunning;
	}
	if(I2SInPlayCt->DecoderSync == TaskStateRunning)
#endif
	{
		msgSend.msgId		= MSG_I2SIN_AUDIO_MODE_STARTED;//bkd
		MessageSend(I2SInPlayCt->parentMsgHandle, &msgSend);

		AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
		vTaskDelay(20);
		AudioCoreSourceEnable(I2SIN_SOURCE_NUM);
		I2SInPlayCt->state = TaskStateRunning;

#ifdef CFG_FUNC_SOFT_ADJUST_IN_I2SIN
		AudioCoreSourceFreqAdjustEnable(I2SIN_SOURCE_NUM, 128, 128);
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
		if(!SoftFlagGet(SoftFlagRemindMask))
		{
			if(RemindSoundServiceItemRequest(SOUND_REMIND_I2SMODE, FALSE))//插播提示音：模式line启动
			{
				AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
			}
		}
		else
		{
			SoftFlagDeregister(SoftFlagRemindMask);
			if(SoftFlagGet(SoftFlagDiscDelayMask))
			{
				SoftFlagDeregister(SoftFlagDiscDelayMask);
				RemindSoundServiceItemRequest(SOUND_REMIND_DISCONNE, FALSE);
			}
		}
#endif
	}
}

static void I2SInPlayModeStop(void)
{
	bool NoService = TRUE;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(I2SInPlayCt->DecoderSync != TaskStateNone && I2SInPlayCt->DecoderSync != TaskStateStopped)
	{//解码器是 随app kill
		DecoderServiceStop();
		I2SInPlayCt->DecoderSync = TaskStateStopping;
		NoService = FALSE;
	}
#endif
#ifdef CFG_FUNC_RECORDER_EN
	if(I2SInPlayCt->RecorderSync != TaskStateNone)
	{//此service 随用随Kill
		MediaRecorderServiceStop();
		I2SInPlayCt->RecorderSync = TaskStateStopping;
		NoService = FALSE;
	}
#endif
	I2SInPlayCt->state = TaskStateStopping;
	if(NoService)
	{
		I2SInPlayModeStopped();
	}
}

static void I2SInPlayModeStopping(uint16_t msgId)//部分子service 随用随kill
{
#ifdef CFG_FUNC_RECORDER_EN
	if(msgId == MSG_MEDIA_RECORDER_SERVICE_STOPPED)
	{
		I2SInPlayCt->RecorderSync = TaskStateNone;
		APP_DBG("I2sin:RecorderKill");
		MediaRecorderServiceKill();
	}

#endif	
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_STOPPED)
	{
		I2SInPlayCt->DecoderSync = TaskStateNone;
	}
#endif
	if((I2SInPlayCt->state == TaskStateStopping)
#ifdef CFG_FUNC_RECORDER_EN
		&& (I2SInPlayCt->RecorderSync == TaskStateNone)
#endif
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
		&& (I2SInPlayCt->DecoderSync == TaskStateNone)
#endif
	)
	{
		I2SInPlayModeStopped();
	}
}

static void I2SInPlayModeStopped(void)
{
	MessageContext		msgSend;
	//Set para
	
	//clear msg
	MessageClear(I2SInPlayCt->msgHandle);
	
	//Set state
	I2SInPlayCt->state = TaskStateStopped;

	//reply
	msgSend.msgId		= MSG_I2SIN_AUDIO_MODE_STOPPED;//bkd 
	MessageSend(I2SInPlayCt->parentMsgHandle, &msgSend);
}

/**
 * @func        I2sInPlayEntrance
 * @brief       模式执行主体
 * @param       void * param  
 * @Output      None
 * @return      None
 * @Others      模式建立和结束过程
 * Record
 */
static void I2SInPlayEntrance(void * param)
{
	MessageContext		msgRecv;
	
	APP_DBG("I2SIn:App\n");
	// Create services
	I2SInPlayModeCreate();
#ifdef CFG_FUNC_AUDIO_EFFECT_EN	
	AudioEffectModeSel(mainAppCt.EffectMode, 2);//0=init hw,1=effect,2=hw+effect
#ifdef CFG_COMMUNICATION_BY_UART
	UART1_Communication_Init((void *)(&UartRxBuf[0]), 1024, (void *)(&UartTxBuf[0]), 1024);
#endif
#endif

#ifdef CFG_FUNC_BREAKPOINT_EN
	BackupInfoUpdata(BACKUP_SYS_INFO);
#endif

//	//如果进入模式AudioCore为静音状态，则unmute
//	if(IsAudioPlayerMute() == TRUE)
//	{
//		AudioPlayerMute();
//	}
#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceUnmute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif
	while(1)
	{
#ifndef CFG_FUNC_SOFT_ADJUST_IN_I2SIN
		MessageRecv(I2SInPlayCt->msgHandle, &msgRecv, MAX_RECV_MSG_TIMEOUT);
#else
		MessageRecv(I2SInPlayCt->msgHandle, &msgRecv, 1);
		I2SINDataProcess();
#endif
		switch(msgRecv.msgId)//警告：在此段代码，禁止新增提示音插播位置。
		{	
			case MSG_DECODER_SERVICE_CREATED:
				I2SInPlayModeCreating(msgRecv.msgId);
				break;
	
			case MSG_TASK_START:
				I2SInPlayModeStart();
				break;
			case MSG_DECODER_SERVICE_STARTED:
				//RemindSound request		
				I2SInPlayModeStarting(msgRecv.msgId);
				break;

			case MSG_TASK_RESUME:
				if(I2SInPlayCt->state == TaskStatePaused)
				{
					I2SInPlayCt->state = TaskStateRunning;
				}
				break;

			case MSG_TASK_STOP:
#ifdef CFG_FUNC_REMIND_SOUND_EN
				RemindSoundServiceReset();
#endif
#if 0//CFG_COMMUNICATION_BY_USB
				NVIC_DisableIRQ(Usb_IRQn);
				OTG_DeviceDisConnect();
#endif
				I2SInPlayModeStop();
				break;

			case MSG_DECODER_SERVICE_STOPPED:
			case MSG_MEDIA_RECORDER_SERVICE_STOPPED:
				I2SInPlayModeStopping(msgRecv.msgId);
				break;
			case MSG_APP_RES_RELEASE:
				I2SInPlayResRelease();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_RELEASE_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_MALLOC:
				I2SInPlayResMalloc(mainAppCt.SamplesPreFrame);
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_MALLOC_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_INIT:
				I2SInPlayResInit();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_INIT_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
				
			case MSG_REMIND_SOUND_PLAY_START:
				break;

			case MSG_REMIND_SOUND_PLAY_DONE://提示音播放结束
			case MSG_REMIND_SOUND_PLAY_REQUEST_FAIL:
				AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
				break;

			default:
				if(I2SInPlayCt->state == TaskStateRunning)
				{
					I2SInPlayRunning(msgRecv.msgId);
				}
				break;
		}
	}
}

static void I2SInPlayRunning(uint16_t msgId)
{
	switch(msgId)
	{
#ifdef	CFG_FUNC_POWERKEY_EN
		case MSG_TASK_POWERDOWN:
			APP_DBG("MSG receive PowerDown, Please breakpoint\n");
			SystemPowerDown();
			break;
#endif

		case MSG_DECODER_STOPPED:
#if defined(CFG_FUNC_REMIND_SOUND_EN)
		if(SoftFlagGet(SoftFlagDecoderRemind))
		{
			MessageContext		msgSend;
			msgSend.msgId = msgId;
			MessageSend(GetRemindSoundServiceMessageHandle(), &msgSend);//提示音期间转发解码器消息。
		}
#endif
			break;

		case MSG_DEVICE_SERVICE_BATTERY_LOW:
#ifdef CFG_FUNC_REMIND_SOUND_EN
			//RemindSound request
			//RemindSoundServiceItemRequest(SOUND_REMIND_DLGUODI);
#endif
			break;

#ifdef CFG_FUNC_RECORDER_EN	
		case MSG_REC:
#ifdef	CFG_FUNC_RECORD_SD_UDISK
			if(ResourceValue(AppResourceCard) || ResourceValue(AppResourceUDisk))
			{
				if(I2SInPlayCt->RecorderSync == TaskStateNone)
				{
					if(!MediaRecordHeapEnough())
					{
						break;
					}
					MediaRecorderServiceCreate(I2SInPlayCt->msgHandle);
					I2SInPlayCt->RecorderSync = TaskStateCreating;
				}
				else if(I2SInPlayCt->RecorderSync == TaskStateRunning)//再按录音键 停止
				{
					MediaRecorderStop();
					MediaRecorderServiceStop();
					I2SInPlayCt->RecorderSync = TaskStateStopping;
				}
				
			}
			else
			{
				APP_DBG("error, no disk!!!\n");
			}
#elif defined(CFG_FUNC_RECORD_FLASHFS)
			if(I2SInPlayCt->RecorderSync == TaskStateNone)
			{
				MediaRecorderServiceCreate(I2SInPlayCt->msgHandle);
				I2SInPlayCt->RecorderSync = TaskStateCreating;
				break;
			} 
			else if(I2SInPlayCt->RecorderSync == TaskStateRunning)//再按录音键 停止
			{
				MediaRecorderStop();
				MediaRecorderServiceStop();
				I2SInPlayCt->RecorderSync = TaskStateStopping;
			}
//				APP_DBG("Recorder service error.\n");

#endif
			break;
		
		case MSG_MEDIA_RECORDER_SERVICE_CREATED:
#ifdef CFG_FUNC_REMIND_SOUND_EN
			//RemindSound request
			//录音事件提示音，规避录音文件携带本提示音，使用阻塞延时
			RemindSoundServiceItemRequest(SOUND_REMIND_LUYIN, FALSE);
			osTaskDelay(350);//即“录音”提示音本身时长
#endif			
			I2SInPlayCt->RecorderSync = TaskStateStarting;
			MediaRecorderServiceStart();
			break;

		case MSG_MEDIA_RECORDER_SERVICE_STARTED:
			MediaRecorderRun();
			I2SInPlayCt->RecorderSync = TaskStateRunning;
			break;

		case MSG_MEDIA_RECORDER_STOPPED:
#ifdef CFG_FUNC_REMIND_SOUND_EN
			RemindSoundServiceItemRequest(SOUND_REMIND_STOPREC, FALSE);
#endif			
			MediaRecorderServiceStop();
			I2SInPlayCt->RecorderSync = TaskStateStopping;
			break;

		case MSG_MEDIA_RECORDER_ERROR:
			if(I2SInPlayCt->RecorderSync == TaskStateRunning)
			{
				MediaRecorderStop();
				MediaRecorderServiceStop();
				I2SInPlayCt->RecorderSync = TaskStateStopping;
			}
			break;
#endif //录音
		
		default:
			CommonMsgProccess(msgId);
			break;
	}
}

/***************************************************************************************
 *
 * APIs
 *
 */
bool I2SInPlayCreate(MessageHandle parentMsgHandle)
{
	bool		ret = TRUE;

	ret = I2SInPlay_Init(parentMsgHandle);
	if(ret)
	{
		I2SInPlayCt->taskHandle = NULL;
		xTaskCreate(I2SInPlayEntrance,
					"I2SInPlay",
					I2SIN_PLAY_TASK_STACK_SIZE,
					NULL, I2SIN_PLAY_TASK_PRIO,
					&I2SInPlayCt->taskHandle);
		if(I2SInPlayCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
	}
	else
	{
		APP_DBG("I2sInPlay app create fail!\n");
	}
	return ret;
}

bool I2SInPlayStart(void)
{
	MessageContext		msgSend;

	if(I2SInPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_START;
	return MessageSend(I2SInPlayCt->msgHandle, &msgSend);
}

bool I2SInPlayPause(void)
{
	MessageContext		msgSend;
	if(I2SInPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_PAUSE;
	return MessageSend(I2SInPlayCt->msgHandle, &msgSend);
}

bool I2SInPlayResume(void)
{
	MessageContext		msgSend;
	if(I2SInPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_RESUME;
	return MessageSend(I2SInPlayCt->msgHandle, &msgSend);
}

bool I2SInPlayStop(void)
{
	MessageContext		msgSend;
	if(I2SInPlayCt == NULL)
	{
		return FALSE;
	}

	AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceMute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	AudioCoreSourceMute(REMIND_SOURCE_NUM, TRUE, TRUE);
#endif
	vTaskDelay(30);

	msgSend.msgId		= MSG_TASK_STOP;
	return MessageSend(I2SInPlayCt->msgHandle, &msgSend);
}

bool I2SInPlayKill(void)
{
	if(I2SInPlayCt == NULL)
	{
		return FALSE;
	}

	//Kill used services
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
	AudioCoreSourceDisable(PLAYBACK_SOURCE_NUM);
	DecoderServiceKill();
#endif
#ifdef CFG_FUNC_RECORDER_EN
	if(I2SInPlayCt->RecorderSync != TaskStateNone)//当录音创建失败时，需要强行回收
	{
		MediaRecorderServiceKill();
		I2SInPlayCt->RecorderSync = TaskStateNone;
	}
#endif
	//注意：AudioCore父任务调整到mainApp下，此处只关闭AudioCore通道，不关闭任务
	AudioCoreProcessConfig((void*)AudioNoAppProcess);
	AudioCoreSourceDisable(I2SIN_SOURCE_NUM);
	//AudioADC_DeInit(ADC0_MODULE);

#ifdef CFG_FUNC_SOFT_ADJUST_IN_I2SIN
	AudioCoreSourceFreqAdjustDisable();
	SoftFlagDeregister(SoftFlagBtSra);
	AudioCoreSourceSRAResRelease();
#endif
	
#if (CFG_RES_I2S_PORT == 0)
	I2S_ModuleRxDisable(I2S0_MODULE);
#ifndef CFG_RES_AUDIO_I2SOUT_EN
	I2S_ModuleDisable(I2S0_MODULE);
	DMA_ChannelDisable(PERIPHERAL_ID_I2S0_RX);
#endif
#else
	I2S_ModuleRxDisable(I2S1_MODULE);
#ifndef CFG_RES_AUDIO_I2SOUT_EN
	I2S_ModuleDisable(I2S1_MODULE);
	DMA_ChannelDisable(PERIPHERAL_ID_I2S1_RX);
#endif	
#endif	

	//task
	if(I2SInPlayCt->taskHandle != NULL)
	{
		vTaskDelete(I2SInPlayCt->taskHandle);
		I2SInPlayCt->taskHandle = NULL;
	}

	//Msgbox
	if(I2SInPlayCt->msgHandle)
	{
		MessageDeregister(I2SInPlayCt->msgHandle);
		I2SInPlayCt->msgHandle = NULL;
	}

#ifdef CFG_FUNC_SOFT_ADJUST_IN_I2SIN
	if(I2SInPlayCt->pBufTemp != NULL)
	{
		osPortFree(I2SInPlayCt->pBufTemp);
		I2SInPlayCt->pBufTemp = NULL;
	}
	if(I2SInPlayCt->SRAFifo != NULL)
	{
		APP_DBG("I2SInPlayCt.SRAFifo memory free!\n");
		osPortFree(I2SInPlayCt->SRAFifo);
		I2SInPlayCt->SRAFifo = NULL;
	}
#endif

	//PortFree
	I2SInPlayCt->AudioCoreI2SIn = NULL;
	if(I2SInPlayCt->Source1Buf_I2S != NULL)
	{
		osPortFree(I2SInPlayCt->Source1Buf_I2S);
		I2SInPlayCt->Source1Buf_I2S = NULL;
	}
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(I2SInPlayCt->Source2Decoder != NULL)
	{
		osPortFree(I2SInPlayCt->Source2Decoder);
		I2SInPlayCt->Source2Decoder = NULL;
	}
#endif

	if(I2SInPlayCt->I2SFIFO1 != NULL)
	{
		osPortFree(I2SInPlayCt->I2SFIFO1);
		I2SInPlayCt->I2SFIFO1 = NULL;
	}

	osPortFree(I2SInPlayCt);
	I2SInPlayCt = NULL;
	APP_DBG("I2s:Kill Ct\n");

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioEffectsDeInit();
#endif

	return TRUE;
}

MessageHandle GetI2SInMessageHandle(void)
{
	if(I2SInPlayCt != NULL)
	{
		return I2SInPlayCt->msgHandle;
	}
	else
	{
		return NULL;
	}
}

#endif//#ifdef CFG_APP_I2SIN_MODE_EN

