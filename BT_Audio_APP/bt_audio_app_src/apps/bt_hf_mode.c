/**
 **************************************************************************************
 * @file    bt_hf_mode.c
 * @brief   蓝牙通话模式
 *
 * @author  KK
 * @version V1.0.1
 *
 * $Created: 2019-3-28 18:00:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "type.h"
#include "app_config.h"
#include "app_message.h"
#include "mvintrinsics.h"
//driver
#include "chip_info.h"
#include "dac.h"
#include "gpio.h"
#include "dma.h"
#include "dac.h"
#include "audio_adc.h"
#include "debug.h"
//middleware
#include "main_task.h"
#include "audio_vol.h"
#include "rtos_api.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "bt_manager.h"
#include "resampler.h"
#include "mcu_circular_buf.h"
#include "audio_core_api.h"
#include "audio_decoder_api.h"
#include "sbcenc_api.h"
#include "bt_config.h"
#include "cvsd_plc.h"
#include "ctrlvars.h"
//application
#include "bt_hf_mode.h"
#include "decoder_service.h"
#include "audio_core_service.h"
#include "mode_switch_api.h"
#include "audio_core_api.h"
#include "bt_hf_api.h"
#include "bt_app_interface.h"
#include "recorder_service.h"

#ifdef CFG_FUNC_REMIND_SOUND_EN
#include "remind_sound_service.h"
#endif
#include "ctrlvars.h"
#include "reset.h"
#include "clk.h"

extern void ResumeAudioCoreMicSource(void);
extern uint32_t gHfFuncState;

#ifdef CFG_APP_BT_MODE_EN

#if (BT_HFP_SUPPORT == ENABLE)

#ifdef BT_RECORD_FUNC_ENABLE
extern uint32_t gSysRecordMode2HfMode;
#endif

static void BtHfModeStarted(void);
static void BtHfModeCreated(void);
static void BtHfModeStopping(uint16_t msgId);
static void BtHfModeStopped(void);

#define BT_HF_TASK_STACK_SIZE		384//512//256
#define BT_HF_TASK_PRIO				4
#define BT_HF_NUM_MESSAGE_QUEUE		10

//msbc encoder
#define MSBC_CHANNE_MODE	1 		// mono
#define MSBC_SAMPLE_REATE	16000	// 16kHz
#define MSBC_BLOCK_LENGTH	15

static void BtHfMsbcKill(void);


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
	255,//PERIPHERAL_ID_I2S0_RX,		//21
#if	(defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S_PORT==0))
	7,//PERIPHERAL_ID_I2S0_TX,			//22
#else	
	255,//PERIPHERAL_ID_I2S0_TX,		//22
#endif	
	255,//PERIPHERAL_ID_I2S1_RX,		//23
#if	(defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S_PORT==1))
	7,	//PERIPHERAL_ID_I2S1_TX,		//24
#else
	255,//PERIPHERAL_ID_I2S1_TX,		//24
#endif
	255,//PERIPHERAL_ID_PPWM,			//25
	255,//PERIPHERAL_ID_ADC,     		//26
	255,//PERIPHERAL_ID_SOFTWARE,		//27
};

BtHfContext	*BtHfCt = NULL;

uint32_t hfModeRestart = 0;
uint32_t hfModeSuspend = 0;

//进入、退出通话模式保护的三个全局变量
static uint32_t BtHfModeEnterFlag = 0;
uint32_t BtHfModeExitFlag = 0;
static uint32_t BtHfModeEixtList = 0; //插入消息队列，在模式进入完成后再调用退出模式

static void BtHfModeCreate(void);


/*******************************************************************************
 * TIMER2定时器进行通话时长的计时
 * 1MS计算1次
 ******************************************************************************/
void BtHf_Timer1msCheck(void)
{
#ifdef BT_HFP_CALL_DURATION_DISP
	if(BtHfCt == NULL)
		return;
	
	//if(GetHfpState() == BT_HFP_STATE_ACTIVE)
	if(GetHfpState() >= BT_HFP_STATE_ACTIVE)
	{
		BtHfCt->BtHfTimerMsCount++;
		if(BtHfCt->BtHfTimerMsCount>=1000) //1s
		{
			BtHfCt->BtHfTimerMsCount = 0;
			BtHfCt->BtHfTimeUpdate = 1;
			BtHfCt->BtHfActiveTime++;
		}
	}
#endif
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtHfResRelease(void)
{
//	AudioADC_Disable(ADC0_MODULE);
	//DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
//
//	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_DONE_INT);
//	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_THRESHOLD_INT);
//	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_ERROR_INT);
//	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);

//	if(BtHfCt->ADCFIFO != NULL)
//	{
//		APP_DBG("ADCFIFO\n");
//		osPortFree(BtHfCt->ADCFIFO);
//		BtHfCt->ADCFIFO = NULL;
//	}
//	if(BtHfCt->Source1Buf_ScoData != NULL)
//	{
//		APP_DBG("Source1Buf_ScoData\n");
//		osPortFree(BtHfCt->Source1Buf_ScoData);
//		BtHfCt->Source1Buf_ScoData = NULL;
//	}
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(BtHfCt->SourceDecoder != NULL)
	{
		APP_DBG("SourceDecoder\n");
		osPortFree(BtHfCt->SourceDecoder);
		BtHfCt->SourceDecoder = NULL;
	}
#endif
}

bool BtHfResMalloc(uint16_t SampleLen)
{
	//LineIn5  digital (DMA)
	//BtHfCt->ADCFIFO = (uint32_t*)osPortMalloc(ADC_FIFO_LEN);
//	BtHfCt->ADCFIFO = (uint32_t*)osPortMallocFromEnd(SampleLen * 2 * 2 * 2);
//	if(BtHfCt->ADCFIFO == NULL)
//	{
//		return FALSE;
//	}
//	memset(BtHfCt->ADCFIFO, 0, SampleLen * 2 * 2 * 2);
//
//	//InCore1 buf
//	BtHfCt->Source1Buf_ScoData = (uint16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);//stereo one Frame
//	if(BtHfCt->Source1Buf_ScoData == NULL)
//	{
//		return FALSE;
//	}
//	memset(BtHfCt->Source1Buf_ScoData, 0, SampleLen * 2 * 2);
	
	//InCore buf
#ifdef CFG_FUNC_REMIND_SOUND_EN
	BtHfCt->SourceDecoder = (uint16_t*)osPortMallocFromEnd(mainAppCt.SamplesPreFrame * 2 * 2 * 2);//One Frame
	if(BtHfCt->SourceDecoder == NULL)
	{
		return FALSE;
	}
	memset(BtHfCt->SourceDecoder, 0, mainAppCt.SamplesPreFrame * 2 * 2 * 2);//2K
#endif
	return TRUE;
}

void BtHfResInit(void)
{
//	if(BtHfCt->Source1Buf_ScoData != NULL)
//	{
//		BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].PcmInBuf = (int16_t *)BtHfCt->Source1Buf_ScoData;
//	}
//
//	//DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
//	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_ADC0_RX, mainAppCt.SamplesPreFrame * 2 * 2, (void*)BtHfCt->ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
//    DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC0_RX);
//    AudioADC_LREnable(ADC0_MODULE, 1, 1);
//    AudioADC_Enable(ADC0_MODULE);
}

/***********************************************************************************
 **********************************************************************************/
bool BtHf_CvsdInit(void)
{
	if(BtHfCt->CvsdInitFlag)
		return FALSE;
	
//CVSD 转采样配置 8K->16K       16K->8K
	//CVSD接收数据转采样 8K->16K
	BtHfCt->ScoInResamplerCt = (ResamplerPolyphaseContext*)osPortMalloc(sizeof(ResamplerPolyphaseContext));
	if(BtHfCt->ScoInResamplerCt == NULL)
	{
		return FALSE;
	}
	memset(BtHfCt->ScoInResamplerCt, 0, sizeof(ResamplerPolyphaseContext));
	BtHfCt->ScoInSrcOutBuf = (int16_t*)osPortMalloc(BTHF_RESAMPLE_FIFO_LEN);
	if(BtHfCt->ScoInSrcOutBuf == NULL)
	{
		return FALSE;
	}
	memset(BtHfCt->ScoInSrcOutBuf, 0, sizeof(BTHF_RESAMPLE_FIFO_LEN));
	//resampler_init(BtHfCt->ScoInResamplerCt, 1, BTHF_CVSD_SAMPLE_RATE, CFG_BTHF_PARA_SAMPLE_RATE, 0, 0);
	resampler_polyphase_init(BtHfCt->ScoInResamplerCt, 1, RESAMPLER_POLYPHASE_SRC_RATIO_2_1);
	
	//CVSD发送数据转采样 16K->8K
	BtHfCt->ScoOutResamplerCt = (ResamplerPolyphaseContext*)osPortMalloc(sizeof(ResamplerPolyphaseContext));
	if(BtHfCt->ScoOutResamplerCt == NULL)
	{
		return FALSE;
	}
	memset(BtHfCt->ScoOutResamplerCt, 0, sizeof(ResamplerPolyphaseContext));
	BtHfCt->ScoOutSrcOutBuf = (int16_t*)osPortMalloc(BTHF_RESAMPLE_FIFO_LEN);
	if(BtHfCt->ScoOutSrcOutBuf == NULL)
	{
		return FALSE;
	}
	memset(BtHfCt->ScoOutSrcOutBuf, 0, sizeof(BTHF_RESAMPLE_FIFO_LEN));
	//resampler_init(BtHfCt->ScoOutResamplerCt, 1, CFG_BTHF_PARA_SAMPLE_RATE, BTHF_CVSD_SAMPLE_RATE, 0, 0);
	resampler_polyphase_init(BtHfCt->ScoOutResamplerCt, 1, RESAMPLER_POLYPHASE_SRC_RATIO_1_2);

	//cvsd plc config
	BtHfCt->cvsdPlcState = osPortMalloc(sizeof(CVSD_PLC_State));
	if(BtHfCt->cvsdPlcState == NULL)
	{
		APP_DBG("CVSD PLC error\n");
		return FALSE;
	}
	memset(BtHfCt->cvsdPlcState, 0, sizeof(CVSD_PLC_State));
	cvsd_plc_init(BtHfCt->cvsdPlcState);

	BtHfCt->CvsdInitFlag = 1;
	return TRUE;
}

/***********************************************************************************
 **********************************************************************************/
bool BtHf_MsbcInit(void)
{
	if(BtHfCt->MsbcInitFlag)
		return FALSE;
	
	//msbc decoder init
	if(BtHf_MsbcDecoderInit() != 0)
	{
		APP_DBG("msbc decoder error\n");
		return FALSE;
	}

	//encoder init
	BtHf_MsbcEncoderInit();

	BtHfCt->MsbcInitFlag = 1;
	return TRUE;
}

/***********************************************************************************
 **********************************************************************************/
void BtHfCodecTypeUpdate(uint8_t codecType)
{
	if(BtHfCt == NULL)
	{
		APP_DBG("BtHfCodecTypeUpdate Error\n");
		return;
	}
	if(BtHfCt->codecType != codecType)
	{
		MessageContext		msgSend;
		APP_DBG("codec type %d -> %d\n", BtHfCt->codecType, codecType);
		
		BtHfCt->codecType = codecType;
		
		//reply
		msgSend.msgId		= MSG_BT_HF_MODE_CODEC_UPDATE;
		MessageSend(BtHfCt->msgHandle, &msgSend);
	}
}

/***********************************************************************************
 **********************************************************************************/
void BtHfModeRunningConfig(void)
{
	{
		if(BtHfCt->hfModeAudioResInitOk)
		{
			if((!BtHfCt->CvsdInitFlag)&&(!BtHfCt->MsbcInitFlag))
			{
				APP_DBG("calling start...\n");

#ifdef CFG_FUNC_REMIND_SOUND_EN
				if(GetHfpState() == BT_HFP_STATE_INCOMING)
				{
					DecoderSourceNumSet(REMIND_SOURCE_NUM);
					BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].Enable = 0;
					BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
					BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
					BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
					BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 1;//2; //stereo
					BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)BtHfCt->SourceDecoder;
				}
				else
#endif
				DecoderSourceNumSet(BT_HF_SOURCE_NUM);

				//call resume 
				if(GetHfpScoAudioCodecType())
					BtHfCt->codecType = HFP_AUDIO_DATA_mSBC;
				else
					BtHfCt->codecType = HFP_AUDIO_DATA_PCM;
				
#ifdef CFG_FUNC_REMIND_SOUND_EN
				//如有提示音，则初始化DECODER
				BtHf_MsbcInit();
				BtHfModeCreate();
#else
				//无提示音，则只有当数据格式为msbc时才初始化DECODER
				if(BtHfCt->codecType == HFP_AUDIO_DATA_mSBC)
				{
					BtHf_MsbcInit();
					BtHfModeCreate();
				}
#endif

				if(BtHfCt->codecType == HFP_AUDIO_DATA_PCM)
				{
					BtHf_CvsdInit();
				}

				//sink1
				if(BtHfCt->codecType == HFP_AUDIO_DATA_mSBC)
				{
					//Audio init
					//Soure1.
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].Enable = 0;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].FuncDataGetLen = DecodedPcmDataLenGet;//NULL;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].IsSreamData = FALSE;//Decoder
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].PcmFormat = 1; //mono
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].PcmInBuf = (int16_t*)BtHfCt->Source1Buf_ScoData;
					
				}
				else
				{
					//Audio init
					//Soure1.
					BtHfCt->DecoderSourceNum = BT_HF_SOURCE_NUM;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].Enable = 0;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].FuncDataGet = BtHf_ScoDataGet;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].FuncDataGetLen = BtHf_ScoDataLenGet;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].IsSreamData = FALSE;//Decoder
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].PcmFormat = 1; //mono
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].PcmInBuf = (int16_t*)BtHfCt->Source1Buf_ScoData;
				}
				
				//adc
				AudioADC1ParamsSet(BtHfCt->SampleRate, BT_HFP_MIC_PGA_GAIN, 2);

				//sink2 channel
				memset(BtHfCt->Sink2ScoFifo, 0, BT_SCO_FIFO_LEN);
				MCUCircular_Config(&BtHfCt->Sink2ScoFifoCircular, BtHfCt->Sink2ScoFifo, BT_SCO_FIFO_LEN);

				BtHfCt->callingState = BT_HF_CALLING_ACTIVE;
				
				if(BtHfCt->codecType != HFP_AUDIO_DATA_mSBC)
				{
					bool flag = 0;
					BtAppiFunc_SaveScoData(BtHf_SaveScoData);
					AudioCoreSourceEnable(MIC_SOURCE_NUM);
					AudioCoreSourceUnmute(MIC_SOURCE_NUM, TRUE, TRUE);
					//AudioCoreSourceEnable(BT_HF_SOURCE_NUM);
					//AudioCoreSourceUnmute(BT_HF_SOURCE_NUM, TRUE, TRUE);
					AudioCoreSinkEnable(AUDIO_DAC0_SINK_NUM);
					AudioCoreSinkEnable(AUDIO_HF_SCO_SINK_NUM);

					BtHfCt->btHfScoSendReady = 1;
					BtHfCt->btHfScoSendStart = 0;

					GetBtHfpVoiceRecognition(&flag);
					if(flag)
					{
						SetHfpState(BT_HFP_STATE_ACTIVE);
					}
				#ifndef CFG_FUNC_REMIND_SOUND_EN
					AudioCoreSourceEnable(BT_HF_SOURCE_NUM);
					AudioCoreSourceUnmute(BT_HF_SOURCE_NUM, TRUE, TRUE);
				#endif
				}
			}
			/*else
			{
				BtHfTaskResume();
			}*/
		}
	}
}

void BtHfModeRunningResume(void)
{
	if(BtHfCt == NULL)
		return;

	if(BtHfCt->state == TaskStatePaused)
	{
		BtHfTaskResume();
	}
}

/***********************************************************************************
 **********************************************************************************/
/**
 * @func        BtHfInit
 * @brief       BtHf模式参数配置，资源初始化
 * @param       MessageHandle parentMsgHandle  
 * @Output      None
 * @return      int32_t
 * @Others      任务块、Dac、AudioCore配置，数据源自DecoderService
 * @Others      数据流从Decoder到audiocore配有函数指针，audioCore到Dac同理，由audiocoreService任务按需驱动
 * @Others      初始化通话模式所必须的模块,待模式启动后,根据SCO链路建立交互的数据格式对应的创建其他的任务；
 * Record
 */
static bool BtHfInit(MessageHandle parentMsgHandle)
{
	//DMA channel
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);

	//DecoderSourceNumSet(BT_HF_SOURCE_NUM);
	
	//Task & App Config
	BtHfCt = (BtHfContext*)osPortMalloc(sizeof(BtHfContext));
	if(BtHfCt == NULL)
	{
		return FALSE;
	}
	memset(BtHfCt, 0, sizeof(BtHfContext));
	
	BtHfCt->msgHandle = MessageRegister(BT_HF_NUM_MESSAGE_QUEUE);
	if(BtHfCt->msgHandle == NULL)
	{
		return FALSE;
	}
	BtHfCt->state = TaskStateCreating;
	BtHfCt->parentMsgHandle = parentMsgHandle;
	
	/* Create media audio services */
	BtHfCt->SampleRate = CFG_BTHF_PARA_SAMPLE_RATE;//蓝牙模式下采样率为16K
	
	if(!BtHfResMalloc(mainAppCt.SamplesPreFrame))
	{
		APP_DBG("BtHf Res Error!\n");
		return FALSE;
	}

	BtHfCt->AudioCoreBtHf = (AudioCoreContext*)&AudioCore;

	//MIC: ADC1 参数配置
	AudioADC1ParamsSet(BtHfCt->SampleRate, BT_HFP_MIC_PGA_GAIN, 2);

	//Soure0.
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].Enable = 0;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].FuncDataGet = AudioADC1DataGet;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].FuncDataGetLen = AudioADC1DataLenGet;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].IsSreamData = TRUE;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].PcmFormat = 2;//test mic audio effect
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf = (int16_t *)mainAppCt.Source0Buf_ADC1;
	//AudioCoreSourceEnable(MIC_SOURCE_NUM);

#ifdef CFG_FUNC_REMIND_SOUND_EN
	BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].Enable = 0;
	BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
	BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
	BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
	BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 1;//2; //stereo
	BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)BtHfCt->SourceDecoder;
#ifdef CFG_BT_NUMBER_REMIND
	BtHfCt->RemindCnt = 0;
	memset(BtHfCt->PhoneNumber, 0, 20);
	BtHfCt->PhoneNumberLen = 0;
	BtHfCt->PhoneNumberPos = 0;
#endif
#endif
	//sco input/output 参数配置
	BtHfCt->ScoInputFifo = osPortMalloc(SCO_INPUT_FIFO_SIZE);
	if(BtHfCt->ScoInputFifo == NULL)
	{
		return FALSE;
	}
	memset(BtHfCt->ScoInputFifo, 0, SCO_INPUT_FIFO_SIZE);
	
	BtHfCt->ScoOutputFifo = osPortMalloc(SCO_OUTPUT_FIFO_SIZE);
	if(BtHfCt->ScoOutputFifo == NULL)
	{
		return FALSE;
	}
	memset(BtHfCt->ScoOutputFifo, 0, SCO_OUTPUT_FIFO_SIZE);
	
	//call resume 
	if(GetHfpScoAudioCodecType())
		BtHfCt->codecType = HFP_AUDIO_DATA_mSBC;
	else
		BtHfCt->codecType = HFP_AUDIO_DATA_PCM;
	
	//if(BtHfCt->codecType == HFP_AUDIO_DATA_PCM)
	{
		//sco 接收缓存 fifo
		BtHfCt->ScoBuffer = (int8_t*)osPortMalloc(BT_SCO_FIFO_LEN);
		if(BtHfCt->ScoBuffer == NULL)
		{
			return FALSE;
		}
		MCUCircular_Config(&BtHfCt->BtScoCircularBuf, BtHfCt->ScoBuffer, BT_SCO_FIFO_LEN);
		memset(BtHfCt->ScoBuffer, 0, BT_SCO_FIFO_LEN);
	}

//AudioCore通路配置
	//source in buf
	BtHfCt->Source1Buf_ScoData = (uint16_t*)osPortMalloc(CFG_BTHF_PARA_SAMPLES_PER_FRAME * 2);
	if(BtHfCt->Source1Buf_ScoData == NULL)
	{
		return FALSE;
	}
	memset(BtHfCt->Source1Buf_ScoData, 0, CFG_BTHF_PARA_SAMPLES_PER_FRAME * 2);

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	BtHfCt->AudioCoreBtHf->AudioEffectProcess = (AudioCoreProcessFunc)AudioEffectProcessBTHF;
#else
	BtHfCt->AudioCoreBtHf->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
#endif

//DAC0/DACX
#ifdef CFG_RES_AUDIO_DACX_EN
	AudioDAC_DigitalMute(DAC1, TRUE, TRUE);
	vTaskDelay(20);
	AudioDAC_Pause(DAC1);
	AudioDAC_Disable(DAC1);
	AudioDAC_Reset(DAC1);
	AudioDAC_SampleRateChange(DAC1, BTHF_DAC_SAMPLE_RATE);//16K
	AudioDAC_Enable(DAC1);
	AudioDAC_Run(DAC1);
	AudioDAC_DigitalMute(DAC1, FALSE, FALSE);
#endif

#ifdef CFG_RES_AUDIO_DAC0_EN
	AudioDAC_DigitalMute(DAC0, TRUE, TRUE);
	vTaskDelay(20);
	AudioDAC_Pause(DAC0);
	AudioDAC_Disable(DAC0);
	AudioDAC_Reset(DAC0);
	AudioDAC_SampleRateChange(DAC0, BTHF_DAC_SAMPLE_RATE);//16K
	AudioDAC_Enable(DAC0);
	AudioDAC_Run(DAC0);
	AudioDAC_DigitalMute(DAC0, FALSE, FALSE);
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
{
	#include "i2s_interface.h"
	I2SParamCt i2s_set;
	i2s_set.IsMasterMode=CFG_RES_I2S_MODE;// 0:master 1:slave
	i2s_set.SampleRate=BTHF_DAC_SAMPLE_RATE;
	i2s_set.I2sFormat=I2S_FORMAT_I2S;
	i2s_set.I2sBits=I2S_LENGTH_16BITS;
	i2s_set.I2sTxRxEnable=1;
#if (CFG_RES_I2S_PORT == 0)
	i2s_set.TxPeripheralID=PERIPHERAL_ID_I2S0_TX;
#else
	i2s_set.TxPeripheralID=PERIPHERAL_ID_I2S1_TX;
#endif
	i2s_set.TxBuf=(void*)mainAppCt.I2SFIFO;
	i2s_set.TxLen=mainAppCt.SamplesPreFrame * 2 * 2 * 2;
	AudioI2S_DeInit(I2S0_MODULE);
	AudioI2S_DeInit(I2S1_MODULE);
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
	AudioI2S_Init(I2S0_MODULE, &i2s_set);
#else
	AudioI2S_Init(I2S1_MODULE, &i2s_set);
#endif
}

#endif

	//sink2
	BtHfCt->Sink2PcmFifo = (int16_t*)osPortMalloc(CFG_BTHF_PARA_SAMPLES_PER_FRAME * 4);
	if(BtHfCt->Sink2PcmFifo == NULL)
	{
		return FALSE;
	}

	BtHfCt->Sink2ScoFifo = (int16_t*)osPortMalloc(BT_SCO_FIFO_LEN);
	if(BtHfCt->Sink2ScoFifo == NULL) 
	{
		return FALSE;
	}
	MCUCircular_Config(&BtHfCt->Sink2ScoFifoCircular, BtHfCt->Sink2ScoFifo, BT_SCO_FIFO_LEN);

//AudioCore sink2 config -- 用于HFP SCO发送数据
	BtHfCt->AudioCoreSinkHfSco = &AudioCore.AudioSink[AUDIO_HF_SCO_SINK_NUM];
	BtHfCt->AudioCoreSinkHfSco->Enable = 0;
	BtHfCt->AudioCoreSinkHfSco->PcmFormat = 1;//nomo
	BtHfCt->AudioCoreSinkHfSco->FuncDataSet = BtHf_SinkScoDataSet;
	BtHfCt->AudioCoreSinkHfSco->FuncDataSpaceLenGet = BtHf_SinkScoDataSpaceLenGet;
	BtHfCt->AudioCoreSinkHfSco->PcmOutBuf = (int16_t *)BtHfCt->Sink2PcmFifo;

	AudioCoreSinkEnable(AUDIO_HF_SCO_SINK_NUM);

	if((BtHfCt->Sink2ScoFifoMutex = xSemaphoreCreateMutex()) == NULL)
	{
		return FALSE;
	}

	//需要发送数据缓存
	BtHfCt->BtScoSendFifo = (uint8_t*)osPortMalloc(BT_SCO_FIFO_LEN);
	if(BtHfCt->BtScoSendFifo == NULL)
	{
		return FALSE;
	}
	MCUCircular_Config(&BtHfCt->BtScoSendCircularBuf, BtHfCt->BtScoSendFifo, BT_SCO_FIFO_LEN);
	memset(BtHfCt->BtScoSendFifo, 0, BT_SCO_FIFO_LEN);

	//AEC
	BtHf_AECInit();

	//SBC receivr fifo
	BtHfCt->msbcRecvFifo = (uint8_t*)osPortMalloc(BT_SBC_RECV_FIFO_SIZE);
	if(BtHfCt->msbcRecvFifo == NULL) 
	{
		return FALSE;
	}
	MCUCircular_Config(&BtHfCt->msbcRecvFifoCircular, BtHfCt->msbcRecvFifo, BT_SBC_RECV_FIFO_SIZE);
	return TRUE;
}


/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtHfMsgToBtHfMode(uint16_t id)
{
	MessageContext		msgSend;

	if(BtHfCt == NULL)
		return;
	
	// Send message to bt hf mode
	msgSend.msgId		= id;
	MessageSend(GetBtHfMessageHandle(), &msgSend);
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtHfMsgToParent(uint16_t id)
{
	MessageContext		msgSend;

	// Send message to main app
	msgSend.msgId		= id;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

uint8_t BtHfDecoderSourceNum(void)
{
	return BtHfCt->DecoderSourceNum;
}

static void BtHfDeinitialize(void)
{
	MessageContext		msgSend;

	BtHfCt->state = TaskStateNone;
	// Send message to main app
	msgSend.msgId		= MSG_BT_HF_MODE_STOPPED;
	MessageSend(BtHfCt->parentMsgHandle, &msgSend);
}

/*****************************************************************************************
 * 任务创建
 ****************************************************************************************/
//MSBC:先创建模式任务，然后等待Decoder服务创建，才判断为HF模式创建完成
static void BtHfModeCreate(void)
{
#if defined(CFG_FUNC_REMIND_SOUND_EN) && !defined(CFG_FUNC_REMIND_SBC)
	DecoderServiceCreate(BtHfCt->msgHandle, DECODER_BUF_SIZE_MP3, DECODER_FIFO_SIZE_FOR_MP3);
#else
	DecoderServiceCreate(BtHfCt->msgHandle, DECODER_BUF_SIZE_SBC, DECODER_FIFO_SIZE_FOR_SBC);
#endif
	BtHfCt->DecoderSync = TaskStateCreating;
}

//CVSD:直接开启HF模式，不需要Decoder
static void BtHfModeCreated(void)
{
	MessageContext		msgSend;
	
	APP_DBG("Bt Handfree Mode Created\n");
	
	BtHfCt->state = TaskStateReady;
	msgSend.msgId		= MSG_BT_HF_MODE_CREATED;
	MessageSend(BtHfCt->parentMsgHandle, &msgSend);
}

/*****************************************************************************************
 * 任务启动
 ****************************************************************************************/
static void BtHfModeStarted(void)
{
	MessageContext		msgSend;
	
	APP_DBG("Bt Handfree Mode Started\n");
	
	BtHfCt->state = TaskStateRunning;
	msgSend.msgId		= MSG_BT_HF_MODE_STARTED;
	MessageSend(BtHfCt->parentMsgHandle, &msgSend);
}

/*****************************************************************************************
 * 任务停止
 ****************************************************************************************/
//MSBC:
static void BtHfModeStop(void)
{
	bool NoService = TRUE;
	if((BtHfCt->DecoderSync != TaskStateStopped) && (BtHfCt->DecoderSync != TaskStateNone))
	{
		//先decoder stop
		DecoderServiceStop();
		NoService = FALSE;
		BtHfCt->DecoderSync = TaskStateStopping;
	}
#ifdef CFG_FUNC_RECORDER_EN
	if(BtHfCt->RecorderSync != TaskStateNone)
	{//此service 随用随Kill
		MediaRecorderServiceStop();
		BtHfCt->RecorderSync = TaskStateStopping;
		NoService = FALSE;
	}
#endif
	BtHfCt->state = TaskStateStopping;
	if(NoService)
	{
		BtHfModeStopping(MSG_NONE);
	}
}

static void BtHfModeStopping(uint16_t msgId)
{
	if(msgId == MSG_DECODER_SERVICE_STOPPED)
	{
		APP_DBG("Decoder service Stopped\n");
		BtHfCt->DecoderSync = TaskStateStopped;
	}
#ifdef CFG_FUNC_RECORDER_EN
	else if(msgId == MSG_MEDIA_RECORDER_SERVICE_STOPPED)
	{
		APP_DBG("RecorderKill");
		MediaRecorderServiceKill();
		BtHfCt->RecorderSync = TaskStateNone;
	}
#endif	
	if((BtHfCt->state = TaskStateStopping)
#ifdef CFG_FUNC_RECORDER_EN
		&& (BtHfCt->RecorderSync == TaskStateNone)
#endif
		&& (BtHfCt->DecoderSync == TaskStateNone || BtHfCt->DecoderSync == TaskStateStopped)
		)
	{
		BtHfModeStopped();
	}
}

//CVSD
static void BtHfModeStopped(void)
{
	MessageContext		msgSend;
	
	APP_DBG("Bt Handfree Mode Stopped\n");
	//Set para
	
	//clear msg
	MessageClear(BtHfCt->msgHandle);

	//Set state
	BtHfCt->state = TaskStateStopped;

	//reply
	msgSend.msgId		= MSG_BT_HF_MODE_STOPPED;
	MessageSend(BtHfCt->parentMsgHandle, &msgSend);
}


/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtHfTaskPause(void)
{
	MessageContext		msgSend;
	if(!BtHfCt)
		return;

	if(BtHfCt->ModeKillFlag)
		return;
	
	APP_DBG("Bt Handfree Mode Pause\n");
	
#ifdef CFG_FUNC_FREQ_ADJUST
#ifndef CFG_FUNC_SOFT_ADJUST_IN
	AudioCoreSourceFreqAdjustDisable();//bkd
#endif
#endif
	//Set state
	BtHfCt->state = TaskStatePausing;

	//reply
	msgSend.msgId		= MSG_TASK_PAUSE;
	MessageSend(BtHfCt->msgHandle, &msgSend);
}

void BtHfTaskResume(void)
{
	MessageContext		msgSend;
	if(!BtHfCt)
		return;
	
	if(BtHfCt->ModeKillFlag)
		return;
	
	APP_DBG("Bt Handfree Mode Resume\n");
	
#ifdef CFG_FUNC_FREQ_ADJUST
#ifndef CFG_FUNC_SOFT_ADJUST_IN
	AudioCoreSourceFreqAdjustEnable(MIC_SOURCE_NUM, BT_SCO_SEND_BUFFER_LOW_LEVEL, BT_SCO_SEND_BUFFER_HIGH_LEVEL);
#endif
#endif
	//Set state
	BtHfCt->state = TaskStateResuming;

	//reply
	msgSend.msgId		= MSG_TASK_RESUME;
	MessageSend(BtHfCt->msgHandle, &msgSend);
}
/*****************************************************************************************
 * 
 ****************************************************************************************/
#ifdef CFG_BT_NUMBER_REMIND
void BtHfRemindNumberStop(void)
{
	BtHfCt->PhoneNumberRemindStart = 0;
	BtHfCt->RemindCnt = 0;
	memset(BtHfCt->PhoneNumber, 0, 20);
	BtHfCt->PhoneNumberLen = 0;
	BtHfCt->PhoneNumberPos = 0;
	
	RemindSoundServiceReset();
}

static void BtHfRemindPlayClear(void)
{
	BtHfCt->PhoneNumberRemindStart = 0;
	BtHfCt->RemindCnt = 0;
	memset(BtHfCt->PhoneNumber, 0, 20);
	BtHfCt->PhoneNumberLen = 0;
	BtHfCt->PhoneNumberPos = 0;
}

//20ms
static void BtHfRemindNumberRunning(void)
{
	char *SoundItem = NULL;
	if(BtHfCt->PhoneNumberRemindStart)
	{
		BtHfCt->RemindCnt++;
		if(BtHfCt->RemindCnt>=10)
		{
			BtHfCt->RemindCnt=0;
			if(GetHfpState() == BT_HFP_STATE_INCOMING)
			{
				while((BtHfCt->PhoneNumberPos < BtHfCt->PhoneNumberLen))
				{
					switch(BtHfCt->PhoneNumber[BtHfCt->PhoneNumberPos])
					{
						case '0':
							SoundItem = SOUND_REMIND_0;
							break;

						case '1':
							SoundItem = SOUND_REMIND_1;
							break;

						case '2':
							SoundItem = SOUND_REMIND_2;
							break;

						case '3':
							SoundItem = SOUND_REMIND_3;
							break;

						case '4':
							SoundItem = SOUND_REMIND_4;
							break;

						case '5':
							SoundItem = SOUND_REMIND_5;
							break;

						case '6':
							SoundItem = SOUND_REMIND_6;
							break;

						case '7':
							SoundItem = SOUND_REMIND_7;
							break;

						case '8':
							SoundItem = SOUND_REMIND_8;
							break;

						case '9':
							SoundItem = SOUND_REMIND_9;
							break;

						default:
							SoundItem = NULL;
							break;
					}
					if(SoundItem)
					{
						if(RemindSoundServiceItemRequest(SoundItem, TRUE))
						{
							APP_DBG("Pos:%d\n", BtHfCt->PhoneNumberPos);
							BtHfCt->PhoneNumberPos++;
						}
						else
							break;
					}
				}
			}
		}
	}
}
#endif
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
extern bool IsEffectBufSampleRateEnalbe;
#endif
/*****************************************************************************************
 * 
 ****************************************************************************************/
static void BtHfEntrance(void * param)
{
	TIMER CallRingTmr;
	MessageContext		msgRecv;

	BtHfModeEixtList = 0;	
	BtHfCt->hfModeAudioResInitOk = 0;

	BtHfCt->SystemEffectMode = mainAppCt.EffectMode;
	mainAppCt.EffectMode = EFFECT_MODE_HFP_AEC;
	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	IsEffectBufSampleRateEnalbe = 1;
	#endif
	APP_DBG("Bt Handfree Call mode\n");
	BtHfModeCreated();

#ifdef BT_HFP_CALL_DURATION_DISP
	BtHfCt->BtHfActiveTime = 0;
	BtHfCt->BtHfTimerMsCount = 0;
	BtHfCt->BtHfTimeUpdate = 0;
#endif

#ifdef CFG_FUNC_AUDIO_EFFECT_EN	
	Communication_Effect_Init_BTHFP();
	APP_DBG("mainAppCt.EffectMode = %d\n", mainAppCt.EffectMode);
	AudioEffectModeSel(mainAppCt.EffectMode, 2);//0=init hw,1=effect,2=hw+effect
#ifdef CFG_COMMUNICATION_BY_UART
	UART1_Communication_Init((void *)(&UartRxBuf[0]), 1024, (void *)(&UartTxBuf[0]), 1024);
#endif
#endif

	SetBtHfSyncVolume(mainAppCt.HfVolume);
	AudioHfVolSet(mainAppCt.HfVolume);

	
#ifdef CFG_FUNC_FREQ_ADJUST
#ifndef CFG_FUNC_SOFT_ADJUST_IN
	AudioCoreSourceFreqAdjustEnable(MIC_SOURCE_NUM, BT_SCO_SEND_BUFFER_LOW_LEVEL, BT_SCO_SEND_BUFFER_HIGH_LEVEL);
#endif
#endif
	while(1)
	{
		MessageRecv(BtHfCt->msgHandle, &msgRecv, 1);

		switch(msgRecv.msgId)
		{
			//decoder service event
			case MSG_DECODER_SERVICE_CREATED:
#ifdef BT_RECORD_FUNC_ENABLE
				gSysRecordMode2HfMode = 0;
#endif
				if(BtHfCt->state == TaskStateRunning)
				{
					BtHfCt->DecoderSync = TaskStateReady;
					DecoderServiceStart();
					BtHfCt->DecoderSync = TaskStateStarting;
				}
				break;
				
			//decoder service event
			case MSG_DECODER_SERVICE_STARTED:
				if(BtHfCt->state == TaskStateRunning)
				{
					bool flag = 0;
					BtHfCt->DecoderSync = TaskStateRunning;
					APP_DBG("MSG_DECODER_SERVICE_STARTED\n");
					
					BtAppiFunc_SaveScoData(BtHf_SaveScoData);
					AudioCoreSourceEnable(MIC_SOURCE_NUM);
					AudioCoreSourceUnmute(MIC_SOURCE_NUM, TRUE, TRUE);
				//	AudioCoreSourceEnable(BT_HF_SOURCE_NUM);
				//	AudioCoreSourceUnmute(BT_HF_SOURCE_NUM, TRUE, TRUE);
					AudioCoreSinkEnable(AUDIO_HF_SCO_SINK_NUM);
					
					BtHfCt->callingState = BT_HF_CALLING_ACTIVE;
					BtHfCt->btHfScoSendReady = 1;

					
					GetBtHfpVoiceRecognition(&flag);
					if(flag)
					{
						SetHfpState(BT_HFP_STATE_ACTIVE);
					}
					
					//等待FRAME切换完成后再开启提示音播放
/*#ifdef CFG_FUNC_REMIND_SOUND_EN
					//提示音开始
					if(GetHfpState() == BT_HFP_STATE_INCOMING && (!SoftFlagGet(SoftFlagNoRemind) && SoftFlagGet(SoftFlagDecoderRun)))
					{
						AudioCoreSourceDisable(BT_HF_SOURCE_NUM);
						RemindSoundServiceItemRequest(SOUND_REMIND_CALLRING, TRUE);//test？
					}
					else
					{
						DecoderSourceNumSet(BT_HF_SOURCE_NUM);
						AudioCoreSourceEnable(BT_HF_SOURCE_NUM);
						AudioCoreSourceUnmute(BT_HF_SOURCE_NUM, TRUE, TRUE);
					}
#else
					AudioCoreSourceEnable(BT_HF_SOURCE_NUM);
					AudioCoreSourceUnmute(BT_HF_SOURCE_NUM, TRUE, TRUE);
#endif*/
					DecoderSourceNumSet(BT_HF_SOURCE_NUM);
					AudioCoreSourceEnable(BT_HF_SOURCE_NUM);
					AudioCoreSourceUnmute(BT_HF_SOURCE_NUM, TRUE, TRUE);
					
#if 0//defined(CFG_BT_RING_REMIND) && defined(CFG_BT_RING_LOCAL) && !defined(CFG_BT_NUMBER_REMIND) && defined(CFG_FUNC_REMIND_SOUND_EN)
					if(!GetScoConnectFlag())	// 如果SCO没有建立就提前播放本地铃声
					{
						AudioCoreSourceDisable(BT_HF_SOURCE_NUM);
						if(!RemindSoundServiceItemRequest(SOUND_REMIND_CALLRING, FALSE))
						{
							APP_DBG("Play Local Ring Error!!!\n");
						}
					}
#endif
				}
				break;
			
			case MSG_TASK_START:
				//以下代码effect 流程整理之后需要删掉
				#ifdef CFG_FUNC_AUDIO_EFFECT_EN
				gCtrlVars.SamplesPerFrame = CFG_BTHF_PARA_SAMPLES_PER_FRAME;
				if((gCtrlVars.SamplesPerFrame != mainAppCt.SamplesPreFrame) && SoftFlagGet(SoftFlagFrameSizeChange))
				{
					extern bool IsEffectSampleLenChange;
					IsEffectSampleLenChange = 1;
					APP_DBG("MSG_TASK_START IsEffectSampleLenChange == 1\n");
				}
				else
				{
					gCtrlVars.audio_effect_init_flag = 1;
					BtHf_AECEffectInit();
					BtHf_PitchShifterEffectInit();
					gCtrlVars.audio_effect_init_flag = 0;
					
					BtHfCt->hfModeAudioResInitOk = 1;
					BtHfMsgToBtHfMode(MSG_BT_HF_MODE_RUN_CONFIG);
				}
				#else
				{
					BtHfCt->hfModeAudioResInitOk = 1;
					BtHfMsgToBtHfMode(MSG_BT_HF_MODE_RUN_CONFIG);
				}
				#endif
				BtHfModeStarted();
				break;

			case MSG_TASK_PAUSE:
				APP_DBG("MSG_TASK_PAUSED\n");
				if((BtHfCt->callingState == BT_HF_CALLING_ACTIVE) && (hfModeSuspend))
				{
#ifdef CFG_RES_AUDIO_DAC0_EN
					AudioDAC_DigitalMute(DAC0, TRUE, TRUE);// 先mute输出再mute通道
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
					AudioDAC_DigitalMute(DAC1, TRUE, TRUE);

#endif	
					vTaskDelay(10);
					AudioCoreSourceMute(MIC_SOURCE_NUM, TRUE, TRUE);
					AudioCoreSourceMute(BT_HF_SOURCE_NUM, TRUE, TRUE);
					//暂停source和sink服务
					BtAppiFunc_SaveScoData(NULL);
					
					//通话挂起
					BtHfCt->state = TaskStatePaused;
					BtHfCt->callingState = BT_HF_CALLING_SUSPEND;

					//clear sco data
					//msbc
					BtHf_MsbcMemoryReset();
					//cvsd
					if(BtHfCt->ScoBuffer)
					{
						memset(BtHfCt->ScoBuffer, 0, BT_SCO_FIFO_LEN);
					}
					
				//	BtHfCt->btHfScoSendStart = 0;
					BtHf_AECReset();
				}
				break;

			case MSG_TASK_RESUME:
				//通话恢复
				if(GetHfpScoAudioCodecType() != (BtHfCt->codecType - 1))
				{
					APP_DBG("!!!SCO Audio Codec Type is changed %d -> %d", (BtHfCt->codecType-1), GetHfpScoAudioCodecType());
				}
				else
				{
					APP_DBG("sco audio resume...\n");
					BtHfCt->btHfScoSendReady = 1;
					BtHfCt->btHfScoSendStart = 0;
					
					BtAppiFunc_SaveScoData(BtHf_SaveScoData);
					
					AudioCoreSourceEnable(MIC_SOURCE_NUM);
					AudioCoreSourceUnmute(MIC_SOURCE_NUM, TRUE, TRUE);
					
					DecoderSourceNumSet(BT_HF_SOURCE_NUM);
					AudioCoreSourceDisable(BT_HF_SOURCE_NUM);
					AudioCoreSourceMute(BT_HF_SOURCE_NUM, TRUE, TRUE);

#ifdef CFG_RES_AUDIO_DAC0_EN
					AudioDAC_DigitalMute(DAC0, FALSE, FALSE);// 先打开通道再打开DAC
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
					AudioDAC_DigitalMute(DAC1, FALSE, FALSE);

#endif	
					BtHfCt->btHfResumeCnt = 1;
				}
				
				BtHfCt->state = TaskStateRunning;
				BtHfCt->callingState = BT_HF_CALLING_ACTIVE;
				break;
			
			case MSG_TASK_STOP://此处响应Created时exit
				if(BtHfCt->codecType == HFP_AUDIO_DATA_mSBC)
					BtHfModeStop();
				else
					BtHfModeStopped();
				break;

			case MSG_MEDIA_RECORDER_SERVICE_STOPPED:
			case MSG_DECODER_SERVICE_STOPPED:
				if(BtHfCt->ModeKillFlag)
				{
					BtHfModeStopping(msgRecv.msgId);
				}
				else
				{	
					BtHfCt->DecoderSync = TaskStateStopped;
					APP_DBG("msg:BtHf:Decoder service Stopped\n");
					BtHfMsbcKill();
				}
				break;

			case MSG_APP_RES_RELEASE:
				BtHfResRelease();
				mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf = NULL;//系统会释放空间
				BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = NULL;
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_RELEASE_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;

			case MSG_APP_RES_MALLOC:
				BtHfResMalloc(mainAppCt.SamplesPreFrame);
				mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf = (int16_t *)mainAppCt.Source0Buf_ADC1;//注意配合，要等maintask申请完毕。
#ifdef CFG_FUNC_REMIND_SOUND_EN
				BtHfCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)BtHfCt->SourceDecoder;
#endif
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_MALLOC_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;

			case MSG_APP_RES_INIT:
				BtHfResInit();
				gCtrlVars.audio_effect_init_flag = 1;
				BtHf_AECEffectInit();
				BtHf_PitchShifterEffectInit();
				gCtrlVars.audio_effect_init_flag = 0;
				
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_INIT_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}

				BtHfCt->hfModeAudioResInitOk = 1;
				BtHfMsgToBtHfMode(MSG_BT_HF_MODE_RUN_CONFIG);
				break;
				
			case MSG_BT_HF_MODE_RUN_CONFIG:
				//音效切换流程完成，通话模式的前期初始化工作已经完成
				BtHfModeEnterFlag = 0;
				
				if(BtHfModeEixtList)
				{
					//退出队列有消息，则不进行decoder等处理，直接退出通话模式；
					BtHfModeEixtList = 0;
					ResourceDeregister(AppResourceBtHf);
					BtHfMsgToParent(MSG_DEVICE_SERVICE_BTHF_OUT);

					//在退出通话模式前,先关闭sink2通路,避免AudioCore在轮转时,使用到sink相关的内存,和Hf模式kill流程相冲突,导致野指针问题
					AudioCoreSinkDisable(AUDIO_HF_SCO_SINK_NUM);

					BtHfCt->ModeKillFlag=1;
					
					//通话模式退出标志置起来
					BtHfModeExitFlag = 1;
				}
				else
				{
					//无退出消息，则直接初始化最后的部分，如解码器，编码器等
					BtHfModeRunningConfig();
				}
				break;

			case MSG_BT_HF_MODE_CODEC_UPDATE:
				//未完成初始化流程，则不操作解码类型的更新
				if(BtHfCt->hfModeAudioResInitOk == 0)
				  break;

				if(BtHfCt->codecType == HFP_AUDIO_DATA_mSBC)
				{
					if(!BtHfCt->MsbcInitFlag)
					{
						BtHf_MsbcInit();
						BtHfModeCreate();
					}
						
					//Audio init
					//Soure1.
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].Enable = 0;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].FuncDataGetLen = DecodedPcmDataLenGet;//NULL;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].IsSreamData = FALSE;//Decoder
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].PcmFormat = 1; //mono
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].PcmInBuf = (int16_t*)BtHfCt->Source1Buf_ScoData;
				}
				else
				{
					BtHf_CvsdInit();
					
					//Audio init
					//Soure1.
					//BtHfCt->DecoderSourceNum = BT_HF_SOURCE_NUM;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].Enable = 0;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].FuncDataGet = BtHf_ScoDataGet;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].FuncDataGetLen = BtHf_ScoDataLenGet;
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].IsSreamData = FALSE;//Decoder
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].PcmFormat = 1; //mono
					BtHfCt->AudioCoreBtHf->AudioSource[BT_HF_SOURCE_NUM].PcmInBuf = (int16_t*)BtHfCt->Source1Buf_ScoData;
				}
				break;

		#if ((CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN)&&defined(BT_HFP_MIC_PITCH_SHIFTER_FUNC))
			case MSG_PITCH_UP:
				if(BtHfCt->PitchShifterStep < (MAX_PITCH_SHIFTER_STEP-1))
					BtHfCt->PitchShifterStep++;
				else 
					BtHfCt->PitchShifterStep = 0;

				APP_DBG("pitch shifter:%d\n", BtHfCt->PitchShifterStep);
				BtHf_PitchShifterEffectConfig(BtHfCt->PitchShifterStep);
				break;
		#endif
			///////////////////////////////////////////////////////
#ifdef CFG_FUNC_REMIND_SOUND_EN
#if defined(CFG_BT_NUMBER_REMIND) || defined(CFG_BT_RING_REMIND)
			case MSG_BT_HF_MODE_REMIND_PLAY:
				if((GetHfpState() == BT_HFP_STATE_INCOMING) && (!SoftFlagGet(SoftFlagDecoderRemind)) && !SoftFlagGet(SoftFlagNoRemind)
					&& ((BtHfCt->CvsdInitFlag) || (BtHfCt->MsbcInitFlag)) /*&& (BtHfCt->btPhoneCallRing == 0)*/)
				{
#if defined(CFG_BT_RING_REMIND) && defined(CFG_BT_RING_LOCAL) && !defined(CFG_BT_NUMBER_REMIND)
					if(!GetScoConnectFlag() && IsTimeOut(&CallRingTmr))
					{
						AudioCoreSourceDisable(BT_HF_SOURCE_NUM);
						APP_DBG("ring remind...\n");
						btManager.localringState = 0;
						TimeOutSet(&CallRingTmr, CFG_BT_RING_TIME);
						if(!RemindSoundServiceItemRequest(SOUND_REMIND_CALLRING, FALSE))
						{
							APP_DBG("Play Local Ring Error!!!\n");
						}
					}
#endif
#ifdef CFG_BT_NUMBER_REMIND
					BtHfCt->PhoneNumberLen = GetBtCallInPhoneNumber(BtHfCt->PhoneNumber);
					if(BtHfCt->PhoneNumberLen)
						BtHfCt->PhoneNumberRemindStart = 1;
					
					APP_DBG("phone number len:%d\n", BtHfCt->PhoneNumberLen);
#endif
				}
				break;
#endif
			case MSG_DECODER_RESET://解码器出让和回收，临界消息。
				if(SoftFlagGet(SoftFlagDecoderSwitch))
				{
					APP_DBG("BtHf:Switch out\n");
					AudioCoreSourceDisable(BT_HF_SOURCE_NUM);
/*#ifdef CFG_FUNC_FREQ_ADJUST
					AudioCoreSourceFreqAdjustDisable();
#endif*/
					SoftFlagRegister(SoftFlagDecoderRemind);//出让解码器
					if(BtHf_MsbcDecoderIsInitialized())
					{
						BtHf_MsbcDecoderStartedSet(FALSE);
					}
					RemindSoundServicePlay();
				}
				else//非app使用解码器时 回收
				{
					APP_DBG("BtHf:Switch in\n");
					
					DecoderSourceNumSet(BT_HF_SOURCE_NUM);
				//	AudioCoreSourceEnable(BT_HF_SOURCE_NUM);
					SoftFlagDeregister(SoftFlagDecoderRemind);//回收解码器
					AudioCoreSourceUnmute(BT_HF_SOURCE_NUM, TRUE, TRUE);
#ifdef CFG_BT_NUMBER_REMIND
					if(BtHfCt->PhoneNumberRemindStart)
					{
						BtHfRemindPlayClear();//清理
					}
#endif
				}
				SoftFlagDeregister(SoftFlagDecoderSwitch);
				break;
	
	
			case MSG_DECODER_STOPPED:
				if(SoftFlagGet(SoftFlagDecoderRemind))
				{
					MessageContext		msgSend;
					msgSend.msgId = MSG_DECODER_STOPPED;
					MessageSend(GetRemindSoundServiceMessageHandle(), &msgSend);
				}
				/*else if(DecoderSourceNumGet() != BT_HF_SOURCE_NUM)
				{
					RemindSoundServicePlayEnd();
				}*/
				break;
	
			case MSG_REMIND_SOUND_NEED_DECODER:
				if(!SoftFlagGet(SoftFlagDecoderMask & ~SoftFlagDecoderApp))
				{
					AudioCoreSourceMute(BT_HF_SOURCE_NUM, TRUE, TRUE);
					SoftFlagRegister(SoftFlagDecoderSwitch);
					DecoderReset();//发起解码器复位，准备出让。
				}
				break;
				
			case MSG_REMIND_SOUND_PLAY_REQUEST_FAIL:
				break;
			
			case MSG_REMIND_SOUND_PLAY_RENEW:
				break;
#endif

			//HFP control
			case MSG_PLAY_PAUSE:
				switch(GetHfpState())
				{
					case BT_HFP_STATE_CONNECTED:
					case BT_HFP_STATE_INCOMING:
						APP_DBG("call answer\n");
						HfpAnswerCall();
						break;

					case BT_HFP_STATE_OUTGOING:
					case BT_HFP_STATE_ACTIVE:
						{
							bool flag = 0;
							GetBtHfpVoiceRecognition(&flag);
							if(flag)
							{
								APP_DBG("close voice recognition\n");
								HfpVoiceRecognition(0);
							}
							else
							{
								APP_DBG("call hangup\n");
								HfpHangup();
							}
						}
						break;

					case BT_HFP_STATE_3WAY_INCOMING_CALL:
						//挂断正在通话的电话,接听呼入电话
						HfpCallHold(HF_HOLD_HOLD_ACTIVE_CALLS);
						break;
						
					case BT_HFP_STATE_3WAY_OUTGOING_CALL:
						HfpHangup();
						break;
						
					case BT_HFP_STATE_3WAY_ATCTIVE_CALL:
						HfpHangup();
						SetHfpState(BT_HFP_STATE_ACTIVE);
						break;

					default:
						break;
				}
				break;
			
			case MSG_BT_HF_CALL_REJECT:
				switch(GetHfpState())
				{
					case BT_HFP_STATE_INCOMING:
					case BT_HFP_STATE_OUTGOING:
					case BT_HFP_STATE_ACTIVE:
						APP_DBG("call reject\n");
						HfpHangup();
						ExitBtHfMode();
						break;
						
					case BT_HFP_STATE_3WAY_INCOMING_CALL:
					case BT_HFP_STATE_3WAY_OUTGOING_CALL:
						HfpHangup();
						break;
					
					case BT_HFP_STATE_3WAY_ATCTIVE_CALL:
						SetHfpState(BT_HFP_STATE_ACTIVE);
						break;
					
					default:
						break;
				}
				break;
			
			case MSG_BT_HF_TRANS_CHANGED:
				//if(BtHfCt->callingState == BT_HF_CALLING_ACTIVE)
				{
					APP_DBG("Hfp Audio Transfer.\n");
					HfpAudioTransfer();
				}
				break;
				
//			case MSG_REMIND_SOUND_PLAY_START:
//				break;
			
			case MSG_REMIND_SOUND_PLAY_DONE://提示音播放结束
				//AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
				break;
			
			default:
				if(BtHfCt->state == TaskStateRunning)
				{
					CommonMsgProccess(msgRecv.msgId);
				}
				break;

		}
		BtHf_MicProcess();

		//BtHfModeRunning();
		
#ifdef BT_HFP_CALL_DURATION_DISP
		if(BtHfCt->BtHfTimeUpdate)
		{
			uint8_t hour = (uint8_t)(BtHfCt->BtHfActiveTime / 3600);
			uint8_t minute = (uint8_t)((BtHfCt->BtHfActiveTime % 3600) / 60);
			uint8_t second = (uint8_t)(BtHfCt->BtHfActiveTime % 60);
			BtHfCt->BtHfTimeUpdate = 0;
			APP_DBG("通话中：%02d:%02d:%02d", hour, minute, second);
		}
#endif

#ifdef CFG_BT_NUMBER_REMIND
		BtHfRemindNumberRunning();
#endif

		//delay exit hf mode
		if(BtHfCt->flagDelayExitBtHf)
		{
			if(BtHfCt->flagDelayExitBtHf++ >= 1000)
			{
				BtHfCt->flagDelayExitBtHf=0;
				ExitBtHfMode();
			}
		}
	}
}


/*****************************************************************************************
 * 创建HF任务
 ****************************************************************************************/
bool BtHfCreate(MessageHandle parentMsgHandle)
{
	bool ret = 0;

	ret = BtHfInit(parentMsgHandle);
	if(ret)
	{
		BtHfCt->taskHandle = NULL;
		xTaskCreate(BtHfEntrance, "BtHfMode", BT_HF_TASK_STACK_SIZE, NULL, BT_HF_TASK_PRIO, &BtHfCt->taskHandle);
		if(BtHfCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
	}
	if(!ret)
		APP_DBG("BtAudioPlay app create fail!\n");
	return ret;
}


/*****************************************************************************************
 * 删除HF任务
 ****************************************************************************************/
static void BtHfMsbcKill(void)
{
	BtHfCt->MsbcInitFlag = 0;
	BtAppiFunc_SaveScoData(NULL);
	DecoderServiceKill();
	BtHf_MsbcDecoderDeinit();
	BtHf_MsbcEncoderDeinit();
}

extern	void AudioEffectsDeInit_HFP(void);
bool BtHfKill(void)
{
	//clear voice recognition flag
	mainAppCt.EffectMode = BtHfCt->SystemEffectMode;
	
	SetBtHfpVoiceRecognition(0);
	
	BtAppiFunc_SaveScoData(NULL);
	AudioCoreSourceDisable(BT_HF_SOURCE_NUM);
	
	if(BtHfCt == NULL)
	{
		return FALSE;
	}
	
#ifdef CFG_FUNC_FREQ_ADJUST
#ifndef CFG_FUNC_SOFT_ADJUST_IN
	AudioCoreSourceFreqAdjustDisable();
#endif
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
#ifdef CFG_BT_NUMBER_REMIND
	if(BtHfCt->PhoneNumberRemindStart)
		BtHfRemindNumberStop();
	else
#endif
	{
		//避免提示音播放过程中强制退出当前模式引入相应的问题,提示音做无条件复位操作
		RemindSoundServiceReset();
	}

	if(BtHfCt->SourceDecoder != NULL)
	{
		APP_DBG("SourceDecoder\n");
		osPortFree(BtHfCt->SourceDecoder);
		BtHfCt->SourceDecoder = NULL;
	}
#endif

//注意此处，如果在TaskStateCreating发起stop，它尚未init，暂时不会出错
	BtHfDeinitialize();	
	AudioCoreProcessConfig((void*)AudioNoAppProcess);

	//AEC
	BtHf_AECDeinit();
	//AudioEffectsDeInit_HFP();

	if(BtHfCt->BtScoSendFifo)
	{
		osPortFree(BtHfCt->BtScoSendFifo);
		BtHfCt->BtScoSendFifo = NULL;
	}

	//msbc
	//if(BtHfCt->codecType == HFP_AUDIO_DATA_mSBC)
	{
		//Kill used services
		DecoderServiceKill();
	}
#ifdef CFG_FUNC_RECORDER_EN
	if(BtHfCt->RecorderSync != TaskStateNone)//当录音创建失败时，需要强行回收
	{
		MediaRecorderServiceKill();
		BtHfCt->RecorderSync = TaskStateNone;
	}
#endif

	if(BtHfCt->msbcRecvFifo)
	{
		osPortFree(BtHfCt->msbcRecvFifo);
		BtHfCt->msbcRecvFifo = NULL;
	}

	if(BtHfCt->Sink2ScoFifoMutex != NULL)
	{
		vSemaphoreDelete(BtHfCt->Sink2ScoFifoMutex);
		BtHfCt->Sink2ScoFifoMutex = NULL;
	}

	if(BtHfCt->Sink2PcmFifo)
	{
		osPortFree(BtHfCt->Sink2PcmFifo);
		BtHfCt->Sink2PcmFifo = NULL;
	}

	if(BtHfCt->Sink2ScoFifo)
	{
		osPortFree(BtHfCt->Sink2ScoFifo);
		BtHfCt->Sink2ScoFifo = NULL;
	}

	//msbc
	//if(BtHfCt->codecType == HFP_AUDIO_DATA_mSBC)
	{
		//msbc decoder/encoder deinit
		BtHf_MsbcDecoderDeinit();
		BtHf_MsbcEncoderDeinit();
	}
	//else
	//cvsd
	{
		//deinit cvsd_plc
		if(BtHfCt->cvsdPlcState)
		{
			osPortFree(BtHfCt->cvsdPlcState);
			BtHfCt->cvsdPlcState = NULL;
		}
	}

	if(BtHfCt->Source1Buf_ScoData)
	{
		osPortFree(BtHfCt->Source1Buf_ScoData);
		BtHfCt->Source1Buf_ScoData = NULL;
	}

	if(BtHfCt->ScoBuffer)
	{
		osPortFree(BtHfCt->ScoBuffer);
		BtHfCt->ScoBuffer = NULL;
	}

	//cvsd
	if(BtHfCt->ScoOutSrcOutBuf)
	{
		osPortFree(BtHfCt->ScoOutSrcOutBuf);
		BtHfCt->ScoOutSrcOutBuf = NULL;
	}
	if(BtHfCt->ScoOutResamplerCt)
	{
		osPortFree(BtHfCt->ScoOutResamplerCt);
		BtHfCt->ScoOutResamplerCt = NULL;
	}

	if(BtHfCt->ScoInSrcOutBuf)
	{
		osPortFree(BtHfCt->ScoInSrcOutBuf);
		BtHfCt->ScoInSrcOutBuf = NULL;
	}
	if(BtHfCt->ScoInResamplerCt)
	{
		osPortFree(BtHfCt->ScoInResamplerCt);
		BtHfCt->ScoInResamplerCt = NULL;
	}

	//release used buffer: input/output sco fifo
	if(BtHfCt->ScoOutputFifo)
	{
		osPortFree(BtHfCt->ScoOutputFifo);
		BtHfCt->ScoOutputFifo = NULL;
	}
	if(BtHfCt->ScoInputFifo)
	{
		osPortFree(BtHfCt->ScoInputFifo);
		BtHfCt->ScoInputFifo = NULL;
	}

	//ADC/DAC resume
	ResumeAudioCoreMicSource();

	BtHfCt->AudioCoreBtHf = NULL;

	//task  先删任务，再删邮箱，收资源
	if(BtHfCt->taskHandle)
	{
		vTaskDelete(BtHfCt->taskHandle);
		BtHfCt->taskHandle = NULL;
	}

	//Msgbox
	if(BtHfCt->msgHandle)
	{
		MessageDeregister(BtHfCt->msgHandle);
		BtHfCt->msgHandle = NULL;
	}
	osPortFree(BtHfCt);
	BtHfCt = NULL;
	APP_DBG("!!BtHfCt\n");

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	//AudioEffectsDeInit();
	Communication_Effect_Init();
#endif
	
#ifdef CFG_FUNC_REMIND_SOUND_EN
	//置语音提示标志,退出当前模式,回到其他模式不播放提示音
	SoftFlagRegister(SoftFlagRemindMask);
#endif

	//恢复其他模式的音量值
	AudioMusicVolSet(mainAppCt.MusicVolume);

	//mainAppCt.EffectMode = BtHfCt->SystemEffectMode;
	#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	IsEffectBufSampleRateEnalbe = 0;
    #endif
	//通话模式退出完成，清除标志
	BtHfModeExitFlag = 0;
	BtHfModeEixtList = 0;
	
	gCtrlVars.sample_rate = CFG_PARA_SAMPLE_RATE;//sam 20201116
	return TRUE;
}

/*****************************************************************************************
 ****************************************************************************************/
bool BtHfStart(void)
{
	MessageContext		msgSend;

	if(BtHfCt == NULL)
		return FALSE;

	printf("BtHfStart\n");
	msgSend.msgId		= MSG_TASK_START;
	return MessageSend(BtHfCt->msgHandle, &msgSend);
}

bool BtHfStop(void)
{
	MessageContext		msgSend;

	if(BtHfCt == NULL)
		return FALSE;

	printf("BtHfStop\n");

	AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceMute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	AudioCoreSourceMute(REMIND_SOURCE_NUM, TRUE, TRUE);
#endif
	vTaskDelay(30);

	msgSend.msgId		= MSG_TASK_STOP;
	return MessageSend(BtHfCt->msgHandle, &msgSend);
}

/*****************************************************************************************
 ****************************************************************************************/
MessageHandle GetBtHfMessageHandle(void)
{
	if(BtHfCt == NULL)
	{
		return NULL;
	}
	return BtHfCt->msgHandle;
}

/*****************************************************************************************
 * 进入/退出HF模式
 ****************************************************************************************/
void EnterBtHfMode(void)
{
	if(BtHfCt)
		(BtHfCt->flagDelayExitBtHf) = 0;

	if((!btManager.btLinkState)||(SoftFlagGet(SoftFlagWaitBtRemindEnd)))
	{
		if(!SoftFlagGet(SoftFlagDelayEnterBtHf))
		{
			SoftFlagRegister(SoftFlagDelayEnterBtHf);
		}
		return;
	}

	//退出流程正在进行时，丢弃当前进入通话模式的消息，不处理
	if(BtHfModeExitFlag)
	{
		gHfFuncState |= 0x01;
		return;
	}
	
	if((GetSystemMode() != AppModeBtHfPlay)&&(!BtHfModeEnterFlag))
	{
		if((GetSystemMode() == AppModeBtAudioPlay))
		{
			extern bool GetBtCurPlayState(void);
			if(GetBtCurPlayState())
			{
				if(!SoftFlagGet(SoftFlagBtCurPlayStateMask))
					SoftFlagRegister(SoftFlagBtCurPlayStateMask);
			}
		}
		ResourceRegister(AppResourceBtHf);
		BtHfMsgToParent(MSG_DEVICE_SERVICE_BTHF_IN);
		
		//通话模式进入标志置起来
		BtHfModeEnterFlag = 1;
	}
}

void ExitBtHfMode(void)
{
	SoftFlagDeregister(SoftFlagDelayEnterBtHf);

	if(BtHfCt)
	{
		if((BtHfCt->flagDelayExitBtHf)&&(BtHfCt->flagDelayExitBtHf<=1000))
		{
			return;
		}
		BtHfCt->flagDelayExitBtHf = 0;
	}

	if(gHfFuncState)
		gHfFuncState = 0;

	//退出流程正在进行时，不再次处理，防止重入
	if(BtHfModeExitFlag)
		return;

	if(mainAppCt.appCurrentMode == AppModeBtHfPlay)
	{
		//当通话模式还未完全初始化完成，则将退出的消息插入到保护的队列，在初始化完成后，退出通话模式
		if((BtHfCt==NULL)||(BtHfModeEnterFlag))
		{
			BtHfModeEixtList = 1;
			return;
		}
		
		ResourceDeregister(AppResourceBtHf);
		BtHfMsgToParent(MSG_DEVICE_SERVICE_BTHF_OUT);

		//在退出通话模式前,先关闭sink2通路,避免AudioCore在轮转时,使用到sink相关的内存,和Hf模式kill流程相冲突,导致野指针问题
		AudioCoreSinkDisable(AUDIO_HF_SCO_SINK_NUM);

		BtHfCt->ModeKillFlag=1;

		//通话模式退出标志置起来
		BtHfModeExitFlag = 1;
	}
	else if(BtHfModeEnterFlag)
	{
		//当通话模式流程还未进行，则将退出的消息插入到保护的队列，在进入通话模式初始化完成后，退出通话模式
		BtHfModeEixtList = 1;
		return;
	}
}
void DelayExitBtHfModeSet(void)
{
	if(BtHfCt)
		BtHfCt->flagDelayExitBtHf = 1;
}

void DelayExitBtHfModeCancel(void)
{
	if(BtHfCt)
		BtHfCt->flagDelayExitBtHf = 0;
}

extern uint32_t gSpecificDevice;
void SpecialDeviceFunc(void)
{
	if(gSpecificDevice)
	{
		if((GetSystemMode() == AppModeBtHfPlay)&&(GetHfpState() >= BT_HFP_STATE_CONNECTED))
		{
			HfpAudioTransfer();
		}
	}

}

#else
/*****************************************************************************************
 * 
 ****************************************************************************************/
MessageHandle GetBtHfMessageHandle(void)
{
	return NULL;
}

uint32_t BtHf_MsbcDataLenGet(void)
{
	return 0;
}

bool BtHfCreate(MessageHandle parentMsgHandle)
{
	return FALSE;
}

bool BtHfKill(void)
{
	return FALSE;
}

bool BtHfStart(void)
{
	return FALSE;
}

bool BtHfStop(void)
{
	return FALSE;
}

uint8_t BtHfDecoderSourceNum(void)
{
	return 0;
}

void ExitBtHfMode(void)
{
}

void DelayExitBtHfModeSet(void)
{
}

void DelayExitBtHfModeCancel(void)
{
}

void SpecialDeviceFunc(void)
{
}

#endif
#endif

