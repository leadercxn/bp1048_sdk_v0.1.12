/**
 **************************************************************************************
 * @file    bt_record_mode.c
 * @brief   ����K��¼��ģʽ(ʹ��ͨ����·)
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2019-8-18 18:00:00$
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
#include "bt_record_mode.h"
#include "bt_record_api.h"
#include "decoder_service.h"
#include "audio_core_service.h"
#include "mode_switch_api.h"
#include "audio_core_api.h"
#include "bt_app_interface.h"
#include "recorder_service.h"

#ifdef CFG_FUNC_REMIND_SOUND_EN
#include "remind_sound_service.h"
#endif
#include "ctrlvars.h"

extern void ResumeAudioCoreMicSource(void);

#ifdef CFG_APP_BT_MODE_EN

#ifdef BT_RECORD_FUNC_ENABLE

extern uint32_t gSysRecordMode2HfMode;

static void BtRecordModeStarted(void);
static void BtRecordModeCreated(void);
static void BtRecordModeStopping(uint16_t msgId);
static void BtRecordModeStopped(void);
static void BtRecordMsbcKill(void);

//record task������,��Ҫ��������ͨ���ϴ����ݵ�ת����������,��Ҫ����������ȼ�������4;
//�������ܱ�֤���ݵĴ���ʵʱ��
#define BT_RECORD_TASK_STACK_SIZE		384//256
#define BT_RECORD_TASK_PRIO				3
#define BT_RECORD_NUM_MESSAGE_QUEUE		10

//msbc encoder
#define MSBC_CHANNE_MODE	1 		// mono
#define MSBC_SAMPLE_REATE	16000	// 16kHz
#define MSBC_BLOCK_LENGTH	15


/**����appconfigȱʡ����:DMA 8��ͨ������**/
/*1��cec��PERIPHERAL_ID_TIMER3*/
/*2��SD��¼����PERIPHERAL_ID_SDIO RX/TX*/
/*3�����ߴ��ڵ�����PERIPHERAL_ID_UART1 RX/TX,����ʹ��USB HID����ʡDMA��Դ*/
/*4����·������PERIPHERAL_ID_AUDIO_ADC0_RX*/
/*5��Mic������PERIPHERAL_ID_AUDIO_ADC1_RX��mode֮��ͨ������һ��*/
/*6��Dac0������PERIPHERAL_ID_AUDIO_DAC0_TX mode֮��ͨ������һ��*/
/*7��DacX�迪��PERIPHERAL_ID_AUDIO_DAC1_TX mode֮��ͨ������һ��*/
/*ע��DMA 8��ͨ�����ó�ͻ:*/
/*a��UART���ߵ�����DAC-X�г�ͻ,Ĭ�����ߵ���ʹ��USB HID*/
/*b��UART���ߵ�����HDMI/SPDIFģʽ��ͻ*/
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

BtRecordContext	*BtRecordCt = NULL;

extern uint32_t hfModeRestart;
extern uint32_t hfModeSuspend;


static void BtRecordModeCreate(void);

/*******************************************************************************
 ******************************************************************************/
void BtRecordAudioResInitOk(void)
{
	if(BtRecordCt)
		BtRecordCt->hfModeAudioResInitOk = 1;
}

//��K��¼��ģʽ�л���ͨ��ģʽ
void BtRecordModeDeregister(void)
{
	//ע��bt_record_playģʽ,���ö�Ӧ�ı�־
	ResourceDeregister(AppResourceBtRecord);
	//���˳�ͨ��ģʽǰ,�ȹر�sink2ͨ·,����AudioCore����תʱ,ʹ�õ�sink��ص��ڴ�,��Hfģʽkill�������ͻ,����Ұָ������
	AudioCoreSinkDisable(AUDIO_HF_SCO_SINK_NUM);

	if(BtRecordCt)
		BtRecordCt->ModeKillFlag = 1;
}
/*******************************************************************************
 * TIMER2��ʱ������ͨ��ʱ���ļ�ʱ
 * 1MS����1��
 ******************************************************************************/
void BtRecord_Timer1msCheck(void)
{
#ifdef BT_HFP_CALL_DURATION_DISP
	if(BtRecordCt == NULL)
		return;
	
	if(GetHfpState() == BT_HFP_STATE_ACTIVE)
	{
		BtRecordCt->BtRecordTimerMsCount++;
		if(BtRecordCt->BtRecordTimerMsCount>=1000) //1s
		{
			BtRecordCt->BtRecordTimerMsCount = 0;
			BtRecordCt->BtRecordTimeUpdate = 1;
			BtRecordCt->BtRecordActiveTime++;
		}
	}
#endif
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtRecordResRelease(void)
{
	if(BtRecordCt->Source1Buf_ScoData != NULL)
	{
		APP_DBG("Source1Buf_ScoData\n");
		osPortFree(BtRecordCt->Source1Buf_ScoData);
		BtRecordCt->Source1Buf_ScoData = NULL;
	}
	
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(BtRecordCt->SourceDecoder != NULL)
	{
		APP_DBG("SourceDecoder\n");
		osPortFree(BtRecordCt->SourceDecoder);
		BtRecordCt->SourceDecoder = NULL;
	}
#endif
}

bool BtRecordResMalloc(uint16_t SampleLen)
{
	//source1 buf
	BtRecordCt->Source1Buf_ScoData = (uint16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);
	if(BtRecordCt->Source1Buf_ScoData == NULL)
	{
		return FALSE;
	}
	memset(BtRecordCt->Source1Buf_ScoData, 0, SampleLen * 2 * 2);
	
	//InCore buf
#ifdef CFG_FUNC_REMIND_SOUND_EN
	BtRecordCt->SourceDecoder = (uint16_t*)osPortMallocFromEnd(mainAppCt.SamplesPreFrame * 2 * 2 * 2);//One Frame
	if(BtRecordCt->SourceDecoder == NULL)
	{
		return FALSE;
	}
	memset(BtRecordCt->SourceDecoder, 0, mainAppCt.SamplesPreFrame * 2 * 2 * 2);//2K
#endif
	return TRUE;
}

void BtRecordResInit(void)
{
	if(BtRecordCt->Source1Buf_ScoData != NULL)
	{
		BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].PcmInBuf = (int16_t *)BtRecordCt->Source1Buf_ScoData;
	}
	
/*#ifdef CFG_FUNC_FREQ_ADJUST
	AudioCoreSourceFreqAdjustEnable(BT_PLAY_DECODER_SOURCE_NUM, BT_SBC_LEVEL_LOW, BT_SBC_LEVEL_HIGH);
#endif*/
}

/***********************************************************************************
 **********************************************************************************/
bool BtRecord_CvsdInit(void)
{
//CVSD ת�������� 8K->16K       16K->8K
	//CVSD��������ת���� 8K->16K
	BtRecordCt->ScoInResamplerCt = (ResamplerPolyphaseContext*)osPortMalloc(sizeof(ResamplerPolyphaseContext));
	if(BtRecordCt->ScoInResamplerCt == NULL)
	{
		return FALSE;
	}
	memset(BtRecordCt->ScoInResamplerCt, 0, sizeof(ResamplerPolyphaseContext));
	BtRecordCt->ScoInSrcOutBuf = (int16_t*)osPortMalloc(BTHF_RESAMPLE_FIFO_LEN);
	if(BtRecordCt->ScoInSrcOutBuf == NULL)
	{
		return FALSE;
	}
	memset(BtRecordCt->ScoInSrcOutBuf, 0, sizeof(BTHF_RESAMPLE_FIFO_LEN));
	//resampler_init(BtRecordCt->ScoInResamplerCt, 1, BTHF_CVSD_SAMPLE_RATE, CFG_BTHF_PARA_SAMPLE_RATE, 0, 0);
	resampler_polyphase_init(BtRecordCt->ScoInResamplerCt, 1, RESAMPLER_POLYPHASE_SRC_RATIO_2_1);
	
	//CVSD��������ת���� 16K->8K
	BtRecordCt->ScoOutResamplerCt = (ResamplerPolyphaseContext*)osPortMalloc(sizeof(ResamplerPolyphaseContext));
	if(BtRecordCt->ScoOutResamplerCt == NULL)
	{
		return FALSE;
	}
	memset(BtRecordCt->ScoOutResamplerCt, 0, sizeof(ResamplerPolyphaseContext));
	if(BtRecordCt->ScoOutSrcOutBuf == NULL)
	{
		BtRecordCt->ScoOutSrcOutBuf = (int16_t*)osPortMalloc(BTHF_RESAMPLE_FIFO_LEN);
		if(BtRecordCt->ScoOutSrcOutBuf == NULL)
		{
			return FALSE;
		}
	}
	memset(BtRecordCt->ScoOutSrcOutBuf, 0, sizeof(BTHF_RESAMPLE_FIFO_LEN));
	//resampler_init(BtRecordCt->ScoOutResamplerCt, 1, CFG_BTHF_PARA_SAMPLE_RATE, BTHF_CVSD_SAMPLE_RATE, 0, 0);
	resampler_polyphase_init(BtRecordCt->ScoOutResamplerCt, 1, RESAMPLER_POLYPHASE_SRC_RATIO_1_2);

	//cvsd plc config
	BtRecordCt->cvsdPlcState = osPortMalloc(sizeof(CVSD_PLC_State));
	if(BtRecordCt->cvsdPlcState == NULL)
	{
		APP_DBG("CVSD PLC error\n");
		return FALSE;
	}
	memset(BtRecordCt->cvsdPlcState, 0, sizeof(CVSD_PLC_State));
	cvsd_plc_init(BtRecordCt->cvsdPlcState);

	BtRecordCt->CvsdInitFlag = 1;
	return TRUE;
}

/***********************************************************************************
 **********************************************************************************/
bool BtRecord_MsbcInit(void)
{
	//msbc decoder init
	if(BtRecord_MsbcDecoderInit() != 0)
	{
		APP_DBG("msbc decoder error\n");
		return FALSE;
	}

	//encoder init
	BtRecord_MsbcEncoderInit();

	BtRecordCt->MsbcInitFlag = 1;
	return TRUE;
}

/***********************************************************************************
 **********************************************************************************/
void BtRecordModeRunning(void)
{
	if(BtRecordCt->ModeKillFlag)
		return;
	
	if(GetHfpState() >= BT_HFP_STATE_CONNECTED)
	{
		if(GetScoConnectFlag() && hfModeRestart && BtRecordCt->hfModeAudioResInitOk)
		{
			if((!BtRecordCt->CvsdInitFlag)&&(!BtRecordCt->MsbcInitFlag))
			{
				APP_DBG("calling start...\n");

				hfModeRestart = 0;
				
#ifdef CFG_FUNC_REMIND_SOUND_EN
				if(GetHfpState() == BT_HFP_STATE_INCOMING)
				{
					DecoderSourceNumSet(REMIND_SOURCE_NUM);
					BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].Enable = 0;
					BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
					BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
					BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
					BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 1;//2; //stereo
					BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)BtRecordCt->SourceDecoder;
				}
				else
#endif
				DecoderSourceNumSet(BT_RECORD_SOURCE_NUM);

				//call resume 
				if(GetHfpScoAudioCodecType())
					BtRecordCt->codecType = HFP_AUDIO_DATA_mSBC;
				else
					BtRecordCt->codecType = HFP_AUDIO_DATA_PCM;
				
#ifdef CFG_FUNC_REMIND_SOUND_EN
				//������ʾ�������ʼ��DECODER
				BtRecord_MsbcInit();
				BtRecordModeCreate();
#else
				//����ʾ������ֻ�е����ݸ�ʽΪmsbcʱ�ų�ʼ��DECODER
				if(BtRecordCt->codecType == HFP_AUDIO_DATA_mSBC)
				{
					BtRecord_MsbcInit();
					BtRecordModeCreate();
				}
#endif
				if(BtRecordCt->codecType == HFP_AUDIO_DATA_PCM)
				{
					BtRecord_CvsdInit();
				}

				//sink1
				if(BtRecordCt->codecType == HFP_AUDIO_DATA_mSBC)
				{
					//Audio init
					//Soure1.
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].Enable = 0;
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].FuncDataGetLen = DecodedPcmDataLenGet;//NULL;
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].IsSreamData = FALSE;//Decoder
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].PcmFormat = 1; //mono
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].PcmInBuf = (int16_t*)BtRecordCt->Source1Buf_ScoData;
					
				}
				else
				{
					//Audio init
					//Soure1.
					BtRecordCt->DecoderSourceNum = BT_RECORD_SOURCE_NUM;
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].Enable = 0;
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].FuncDataGet = BtRecord_ScoDataGet;
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].FuncDataGetLen = BtRecord_ScoDataLenGet;
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].IsSreamData = FALSE;//Decoder
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].PcmFormat = 1; //mono
					BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].PcmInBuf = (int16_t*)BtRecordCt->Source1Buf_ScoData;
				}
				
				//sink2 channel
				memset(BtRecordCt->ResamplerPreFifo, 0, BT_RECORD_SCO_FIFO_LEN);
				MCUCircular_Config(&BtRecordCt->ResamplerPreFifoCircular, BtRecordCt->ResamplerPreFifo, BT_RECORD_SCO_FIFO_LEN);

				BtRecordCt->callingState = BT_RECORD_CALLING_ACTIVE;

				AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
				
				if(BtRecordCt->codecType != HFP_AUDIO_DATA_mSBC)
				{
					bool flag = 0;
					BtAppiFunc_SaveScoData(BtRecord_SaveScoData);
					AudioCoreSourceEnable(MIC_SOURCE_NUM);
					AudioCoreSourceEnable(BT_RECORD_SOURCE_NUM);
					AudioCoreSinkEnable(AUDIO_DAC0_SINK_NUM);
					AudioCoreSinkEnable(AUDIO_HF_SCO_SINK_NUM);

					BtRecordCt->btHfScoSendReady = 1;
					BtRecordCt->btHfScoSendStart = 0;

					GetBtHfpVoiceRecognition(&flag);
					if(flag)
					{
						SetHfpState(BT_HFP_STATE_ACTIVE);
					}
					
				#ifndef CFG_FUNC_REMIND_SOUND_EN
					AudioCoreSourceEnable(BT_RECORD_SOURCE_NUM);
					AudioCoreSourceUnmute(BT_RECORD_SOURCE_NUM, TRUE, TRUE);
				#endif
				}
			}
			else
			{
				BtRecordTaskResume();
			}
			hfModeRestart = 0;
		}
	}
}

/***********************************************************************************
 **********************************************************************************/
/**
 * @func        BtRecordInit
 * @brief       BtRecordģʽ�������ã���Դ��ʼ��
 * @param       MessageHandle parentMsgHandle  
 * @Output      None
 * @return      int32_t
 * @Others      ����顢Dac��AudioCore���ã�����Դ��DecoderService
 * @Others      ��������Decoder��audiocore���к���ָ�룬audioCore��Dacͬ����audiocoreService����������
 * @Others      ��ʼ��ͨ��ģʽ�������ģ��,��ģʽ������,����SCO��·�������������ݸ�ʽ��Ӧ�Ĵ�������������
 * Record
 */
static bool BtRecordInit(MessageHandle parentMsgHandle)
{
	//DMA channel
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);
	
	//Task & App Config
	BtRecordCt = (BtRecordContext*)osPortMalloc(sizeof(BtRecordContext));
	if(BtRecordCt == NULL)
	{
		return FALSE;
	}
	memset(BtRecordCt, 0, sizeof(BtRecordContext));
	
	BtRecordCt->msgHandle = MessageRegister(BT_RECORD_NUM_MESSAGE_QUEUE);
	if(BtRecordCt->msgHandle == NULL)
	{
		return FALSE;
	}
	BtRecordCt->state = TaskStateCreating;
	BtRecordCt->parentMsgHandle = parentMsgHandle;
	
	/* Create media audio services */
	BtRecordCt->SampleRate = CFG_PARA_SAMPLE_RATE;//����ͨ��ģʽ�²�����44.1K
	
	//ADC1 ��������
	//AudioADC1ParamsSet(BtRecordCt->SampleRate, BT_HFP_MIC_PGA_GAIN, 4);

	BtRecordCt->AudioCoreBtHf = (AudioCoreContext*)&AudioCore;

	//Soure0.
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].Enable = 0;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].FuncDataGet = AudioADC1DataGet;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].FuncDataGetLen = AudioADC1DataLenGet;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].IsSreamData = TRUE;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].PcmFormat = 2;//test mic audio effect
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf = (int16_t *)mainAppCt.Source0Buf_ADC1;
	//AudioCoreSourceEnable(MIC_SOURCE_NUM);

	//sco input/output ��������
	BtRecordCt->ScoInputFifo = osPortMalloc(SCO_INPUT_FIFO_SIZE);
	if(BtRecordCt->ScoInputFifo == NULL)
	{
		return FALSE;
	}
	memset(BtRecordCt->ScoInputFifo, 0, SCO_INPUT_FIFO_SIZE);
	BtRecordCt->ScoOutputFifo = osPortMalloc(SCO_OUTPUT_FIFO_SIZE);
	if(BtRecordCt->ScoOutputFifo == NULL)
	{
		return FALSE;
	}
	memset(BtRecordCt->ScoOutputFifo, 0, SCO_OUTPUT_FIFO_SIZE);
	
	//call resume 
	if(GetHfpScoAudioCodecType())
		BtRecordCt->codecType = HFP_AUDIO_DATA_mSBC;
	else
		BtRecordCt->codecType = HFP_AUDIO_DATA_PCM;
	
	if(BtRecordCt->codecType == HFP_AUDIO_DATA_PCM)
	{
		//sco ���ջ��� fifo
		BtRecordCt->ScoBuffer = (int8_t*)osPortMalloc(BT_RECORD_SCO_FIFO_LEN);
		if(BtRecordCt->ScoBuffer == NULL)
		{
			return FALSE;
		}
		MCUCircular_Config(&BtRecordCt->BtScoCircularBuf, BtRecordCt->ScoBuffer, BT_RECORD_SCO_FIFO_LEN);
		memset(BtRecordCt->ScoBuffer, 0, BT_RECORD_SCO_FIFO_LEN);
	}

//AudioCoreͨ·����
	//source in buf (��������Ϊnomo,������audio_effect�д���Ϊstereo)
	//size:512*2*2 = 2K 
	BtRecordCt->Source1Buf_ScoData = (uint16_t*)osPortMalloc(CFG_PARA_BT_RECORD_SAMPLES_PER_FRAME * 2 * 2);
	if(BtRecordCt->Source1Buf_ScoData == NULL)
	{
		return FALSE;
	}
	memset(BtRecordCt->Source1Buf_ScoData, 0, CFG_PARA_BT_RECORD_SAMPLES_PER_FRAME * 2 * 2);
	
	//BtRecordCt->AudioCoreBtHf = (AudioCoreContext*)&AudioCore;
	
	BtRecordCt->AudioCoreBtHf->AudioEffectProcess = (AudioCoreProcessFunc)AudioEffectProcessBTRecord;

	//sink2 (audio_effect sink2���buf, nomo����, ������44.1K, ���ڷ��͵��ֻ�������)
	//size: 512*2 = 1k
	BtRecordCt->Sink2PcmOutBuf = (int16_t*)osPortMalloc(CFG_PARA_BT_RECORD_SAMPLES_PER_FRAME * 2);
	if(BtRecordCt->Sink2PcmOutBuf == NULL)
	{
		return FALSE;
	}

	//sink2������ݵĻ���fifo,���ں󼶽������ݴ���(nomo����, ������44.1K)
	//size: 512*2*2*2 = 4K
	BtRecordCt->ResamplerPreFifo = (int16_t*)osPortMalloc(BT_RECORD_SCO_FIFO_LEN);
	if(BtRecordCt->ResamplerPreFifo == NULL) 
	{
		return FALSE;
	}
	MCUCircular_Config(&BtRecordCt->ResamplerPreFifoCircular, BtRecordCt->ResamplerPreFifo, BT_RECORD_SCO_FIFO_LEN);

//AudioCore sink2 config -- ����HFP SCO��������
	BtRecordCt->AudioCoreSinkRecordSco = &AudioCore.AudioSink[AUDIO_HF_SCO_SINK_NUM];
	BtRecordCt->AudioCoreSinkRecordSco->Enable = 0;
	BtRecordCt->AudioCoreSinkRecordSco->PcmFormat = 1;//nomo
	BtRecordCt->AudioCoreSinkRecordSco->FuncDataSet = BtRecord_SinkScoDataSet;
	BtRecordCt->AudioCoreSinkRecordSco->FuncDataSpaceLenGet = BtRecord_SinkScoDataSpaceLenGet;
	BtRecordCt->AudioCoreSinkRecordSco->PcmOutBuf = (int16_t *)BtRecordCt->Sink2PcmOutBuf;

	AudioCoreSinkEnable(AUDIO_HF_SCO_SINK_NUM);

	if((BtRecordCt->ResamplerPreFifoMutex = xSemaphoreCreateMutex()) == NULL)
	{
		return FALSE;
	}

	//��Ҫ�������ݻ���
	//encoder���msbc���ݻ���
	//size: 60 * 12packet
	BtRecordCt->BtEncoderSendFifo = (uint8_t*)osPortMalloc(BT_ENCODER_FIFO_LEN);
	if(BtRecordCt->BtEncoderSendFifo == NULL)
	{
		return FALSE;
	}
	MCUCircular_Config(&BtRecordCt->BtEncoderSendCircularBuf, BtRecordCt->BtEncoderSendFifo, BT_ENCODER_FIFO_LEN);
	memset(BtRecordCt->BtEncoderSendFifo, 0, BT_ENCODER_FIFO_LEN);


	//ת�������� 44.1k->16k
	BtRecordCt->AudioSink2OutResamplerCt = (ResamplerPolyphaseContext*)osPortMalloc(sizeof(ResamplerPolyphaseContext));
	if(BtRecordCt->AudioSink2OutResamplerCt == NULL)
	{
		return FALSE;
	}
	memset(BtRecordCt->AudioSink2OutResamplerCt, 0, sizeof(ResamplerPolyphaseContext));
	//resampler_init(BtRecordCt->AudioSink2OutResamplerCt, 1, 44100, CFG_BTHF_PARA_SAMPLE_RATE, 0, 0);
#if CFG_PARA_SAMPLE_RATE == 44100
	resampler_polyphase_init(BtRecordCt->AudioSink2OutResamplerCt, 1, RESAMPLER_POLYPHASE_SRC_RATIO_160_441);
#else if CFG_PARA_SAMPLE_RATE == 48000
	resampler_polyphase_init(BtRecordCt->AudioSink2OutResamplerCt, 1, RESAMPLER_POLYPHASE_SRC_RATIO_1_3);
#endif

	//sink2 44.1k nomo->16k nomo
	//ת�������� ǰ����ȡbuf
	BtRecordCt->AudioSink2OutBuf = (int16_t*)osPortMalloc(128*2*2);
	if(BtRecordCt->AudioSink2OutBuf == NULL)
	{
		return FALSE;
	}
	memset(BtRecordCt->AudioSink2OutBuf, 0, sizeof(128*2*2));
	
	//sink2 resample
	//ת�����󻺴�����,�ڱ���֮ǰ�Ļ���FIFO
	//size: 1024
	BtRecordCt->ResampleOutFifo = (int16_t*)osPortMalloc(BT_RESAMPLE_16K_FIFO_LEN);
	if(BtRecordCt->ResampleOutFifo == NULL) 
	{
		return FALSE;
	}
	MCUCircular_Config(&BtRecordCt->ResampleOutFifoCircular, BtRecordCt->ResampleOutFifo, BT_RESAMPLE_16K_FIFO_LEN);

	if(BtRecordCt->ScoOutSrcOutBuf == NULL)
	{
		BtRecordCt->ScoOutSrcOutBuf = (int16_t*)osPortMalloc(BTHF_RESAMPLE_FIFO_LEN);
		if(BtRecordCt->ScoOutSrcOutBuf == NULL)
		{
			return FALSE;
		}
	}
	memset(BtRecordCt->ScoOutSrcOutBuf, 0, sizeof(BTHF_RESAMPLE_FIFO_LEN));


	if(!BtRecordResMalloc(mainAppCt.SamplesPreFrame))
	{
		APP_DBG("BtRecordResMalloc Res Error!\n");
		return FALSE;
	}
	
#ifdef CFG_FUNC_REMIND_SOUND_EN
		BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].Enable = 0;
		BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
		BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
		BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
		BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 1;//2; //stereo
		BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)BtRecordCt->SourceDecoder;
#endif
#ifdef CFG_FUNC_RECORDER_EN
	BtRecordCt->RecorderSync = TaskStateNone;
#endif



	return TRUE;
}


/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtRecordMsgToParent(uint16_t id)
{
	MessageContext		msgSend;

	// Send message to main app
	msgSend.msgId		= id;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

uint8_t BtRecordDecoderSourceNum(void)
{
	return BtRecordCt->DecoderSourceNum;
}

static void BtRecordDeinitialize(void)
{
	MessageContext		msgSend;

	BtRecordCt->state = TaskStateNone;
	// Send message to main app
	msgSend.msgId		= MSG_BT_RECORD_MODE_STOPPED;
	MessageSend(BtRecordCt->parentMsgHandle, &msgSend);
}

/*****************************************************************************************
 * ���񴴽�
 ****************************************************************************************/
//MSBC:�ȴ���ģʽ����Ȼ��ȴ�Decoder���񴴽������ж�ΪHFģʽ�������
static void BtRecordModeCreate(void)
{
#if defined(CFG_FUNC_REMIND_SOUND_EN) && !defined(CFG_FUNC_REMIND_SBC)
	DecoderServiceCreate(BtRecordCt->msgHandle, DECODER_BUF_SIZE_MP3, DECODER_FIFO_SIZE_FOR_MP3);
#else
	DecoderServiceCreate(BtRecordCt->msgHandle, DECODER_BUF_SIZE_SBC, DECODER_FIFO_SIZE_FOR_SBC);
#endif
	BtRecordCt->DecoderSync = TaskStateCreating;
}

//CVSD:ֱ�ӿ���HFģʽ������ҪDecoder
static void BtRecordModeCreated(void)
{
	MessageContext		msgSend;
	
	APP_DBG("Bt Reocrd Mode Created\n");
	
	BtRecordCt->state = TaskStateReady;
	msgSend.msgId		= MSG_BT_RECORD_MODE_CREATED;
	MessageSend(BtRecordCt->parentMsgHandle, &msgSend);
}

/*****************************************************************************************
 * ��������
 ****************************************************************************************/
static void BtRecordModeStarted(void)
{
	MessageContext		msgSend;
	
	APP_DBG("Bt Record Mode Started\n");
	
	BtRecordCt->state = TaskStateRunning;
	msgSend.msgId		= MSG_BT_RECORD_MODE_STARTED;
	MessageSend(BtRecordCt->parentMsgHandle, &msgSend);
}

/*****************************************************************************************
 * ����ֹͣ
 ****************************************************************************************/
//MSBC:
static void BtRecordModeStop(void)
{
	bool NoService = TRUE;
	if(BtRecordCt->DecoderSync != TaskStateStopped && BtRecordCt->DecoderSync != TaskStateNone)
	{
		//��decoder stop
		DecoderServiceStop();
		NoService = FALSE;
		BtRecordCt->DecoderSync = TaskStateStopping;
	}
	BtRecordCt->state = TaskStateStopping;
	if(NoService)
	{
		BtRecordModeStopping(MSG_NONE);
	}
}

static void BtRecordModeStopping(uint16_t msgId)
{
	if(msgId == MSG_DECODER_SERVICE_STOPPED)
	{
		APP_DBG("BtRecord:Decoder service Stopped\n");
		BtRecordCt->DecoderSync = TaskStateStopped;
	}
	if((BtRecordCt->state = TaskStateStopping)
#ifdef CFG_FUNC_RECORDER_EN
		&& (BtRecordCt->RecorderSync == TaskStateNone)
#endif
		&& (BtRecordCt->DecoderSync == TaskStateNone || BtRecordCt->DecoderSync == TaskStateStopped)
		)
	{
		BtRecordModeStopped();
	}
}

//CVSD
static void BtRecordModeStopped(void)
{
	MessageContext		msgSend;
	
	APP_DBG("Bt Record Mode Stopped\n");
	//Set para
	
	//clear msg
	MessageClear(BtRecordCt->msgHandle);

	//Set state
	BtRecordCt->state = TaskStateStopped;

	//reply
	msgSend.msgId		= MSG_BT_RECORD_MODE_STOPPED;
	MessageSend(BtRecordCt->parentMsgHandle, &msgSend);
}


/*****************************************************************************************
 * 
 ****************************************************************************************/
void BtRecordTaskPause(void)
{
	MessageContext		msgSend;
	if(!BtRecordCt)
		return;
	
	APP_DBG("Bt Record Mode Pause\n");
	
	//Set state
	BtRecordCt->state = TaskStatePausing;

	//reply
	msgSend.msgId		= MSG_TASK_PAUSE;
	MessageSend(BtRecordCt->msgHandle, &msgSend);
}

void BtRecordTaskResume(void)
{
	MessageContext		msgSend;
	if(!BtRecordCt)
		return;
	
	APP_DBG("Bt Record Mode Resume\n");
	
	//Set state
	BtRecordCt->state = TaskStateResuming;

	//reply
	msgSend.msgId		= MSG_TASK_RESUME;
	MessageSend(BtRecordCt->msgHandle, &msgSend);
}

/*****************************************************************************************
 * 
 ****************************************************************************************/
static void BtRecordEntrance(void * param)
{
	MessageContext		msgRecv;

	BtRecordCt->hfModeAudioResInitOk = 0;

	gSysRecordMode2HfMode = 0;

	APP_DBG("Bt Record Call mode\n");
	BtRecordModeCreated();

#ifdef BT_HFP_CALL_DURATION_DISP
	BtRecordCt->BtRecordActiveTime = 0;
	BtRecordCt->BtRecordTimerMsCount = 0;
	BtRecordCt->BtRecordTimeUpdate = 0;
#endif

#ifdef CFG_FUNC_AUDIO_EFFECT_EN	
	AudioEffectModeSel(mainAppCt.EffectMode, 2);//0=init hw,1=effect,2=hw+effect
#ifdef CFG_COMMUNICATION_BY_UART
	UART1_Communication_Init((void *)(&UartRxBuf[0]), 1024, (void *)(&UartTxBuf[0]), 1024);
#endif
#endif

#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceUnmute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif

	while(1)
	{
		MessageRecv(BtRecordCt->msgHandle, &msgRecv, 1);

		switch(msgRecv.msgId)
		{
			//decoder service event
			case MSG_DECODER_SERVICE_CREATED:
				if(BtRecordCt->state == TaskStateRunning)
				{
					BtRecordCt->DecoderSync = TaskStateReady;
					DecoderServiceStart();
					BtRecordCt->DecoderSync = TaskStateStarting;
				}
				break;
				
			//decoder service event
			case MSG_DECODER_SERVICE_STARTED:
				if(BtRecordCt->state == TaskStateRunning)
				{
					bool flag = 0;
					BtRecordCt->DecoderSync = TaskStateRunning;
					APP_DBG("MSG_DECODER_SERVICE_STARTED\n");
					
					BtAppiFunc_SaveScoData(BtRecord_SaveScoData);
					AudioCoreSourceEnable(MIC_SOURCE_NUM);
					AudioCoreSourceEnable(BT_RECORD_SOURCE_NUM);
					AudioCoreSinkEnable(AUDIO_HF_SCO_SINK_NUM);
					
					BtRecordCt->callingState = BT_RECORD_CALLING_ACTIVE;
					BtRecordCt->btHfScoSendReady = 1;

					
					GetBtHfpVoiceRecognition(&flag);
					if(flag)
					{
						SetHfpState(BT_HFP_STATE_ACTIVE);
					}
					
					DecoderSourceNumSet(BT_RECORD_SOURCE_NUM);
					AudioCoreSourceEnable(BT_RECORD_SOURCE_NUM);
					AudioCoreSourceUnmute(BT_RECORD_SOURCE_NUM, TRUE, TRUE);
				}
				break;
			
			case MSG_TASK_START:
				//���´���effect ��������֮����Ҫɾ��
				/*#ifdef CFG_FUNC_AUDIO_EFFECT_EN
				gCtrlVars.SamplesPerFrame = 256;
				if(gCtrlVars.SamplesPerFrame != mainAppCt.SamplesPreFrame)
				{
					extern bool IsEffectSampleLenChange;
					IsEffectSampleLenChange = 1;
					APP_DBG("MSG_TASK_START IsEffectSampleLenChange == 1\n");
				}
				else
				{
					BtRecordCt->hfModeAudioResInitOk = 1;
				}
				#endif
				*/
				BtRecordModeStarted();

				break;

			case MSG_TASK_PAUSE:
				APP_DBG("MSG_TASK_PAUSED\n");
				if((BtRecordCt->callingState == BT_RECORD_CALLING_ACTIVE)&&(hfModeSuspend))
				{
					//ͨ������
					BtRecordCt->state = TaskStatePaused;
					
					BtRecordCt->callingState = BT_RECORD_CALLING_SUSPEND;

					//��ͣsource��sink����
					BtAppiFunc_SaveScoData(NULL);

					//close source0
					AudioCoreSourceDisable(MIC_SOURCE_NUM);

					//clear sco data
					//msbc
					BtRecord_MsbcMemoryReset();
					//cvsd
					if(BtRecordCt->ScoBuffer)
					{
						memset(BtRecordCt->ScoBuffer, 0, BT_RECORD_SCO_FIFO_LEN);
					}
					
					BtRecordCt->btHfScoSendStart = 0;
				}
				break;

			case MSG_TASK_RESUME:
				//ͨ���ָ�
				if(GetHfpScoAudioCodecType() != (BtRecordCt->codecType - 1))
				{
					APP_DBG("!!!SCO Audio Codec Type is changed %d -> %d", (BtRecordCt->codecType-1), GetHfpScoAudioCodecType());
				}
				else
				{
					APP_DBG("sco audio resume...\n");
					BtAppiFunc_SaveScoData(BtRecord_SaveScoData);
					BtRecordCt->btHfScoSendReady = 1;
					BtRecordCt->btHfScoSendStart = 0;
					
					AudioCoreSourceEnable(MIC_SOURCE_NUM);
					AudioCoreSourceEnable(BT_RECORD_SOURCE_NUM);
					AudioCoreSinkEnable(AUDIO_DAC0_SINK_NUM);
					AudioCoreSinkEnable(AUDIO_HF_SCO_SINK_NUM);
				}
				
				BtRecordCt->state = TaskStateRunning;
				
				break;
			
			case MSG_TASK_STOP://�˴���ӦCreatedʱexit

#ifdef CFG_FUNC_RECORDER_EN
			if(BtRecordCt->RecorderSync != TaskStateNone)
			{//��service ������Kill
				MediaRecorderServiceStop();
				BtRecordCt->RecorderSync = TaskStateStopping;
			}
#endif

				if(BtRecordCt->codecType == HFP_AUDIO_DATA_mSBC)
					BtRecordModeStop();
				else
					BtRecordModeStopped();
				break;

#ifdef CFG_FUNC_RECORDER_EN	
		case MSG_REC:
			if(ResourceValue(AppResourceCard) || ResourceValue(AppResourceUDisk))
			{
				if(BtRecordCt->RecorderSync == TaskStateNone)
				{
					if(!MediaRecordHeapEnough())
					{
						break;
					}
					MediaRecorderServiceCreate(BtRecordCt->msgHandle);
					BtRecordCt->RecorderSync = TaskStateCreating;
				}
				else if(BtRecordCt->RecorderSync == TaskStateRunning)//�ٰ�¼���� ֹͣ
				{
					MediaRecorderStop();
					MediaRecorderServiceStop();
					BtRecordCt->RecorderSync = TaskStateStopping;
				}
			}
			else
			{//flashfs¼�� ������
				APP_DBG("Btplay:error, no disk!!!\n");
			}
			break;
			
		case MSG_MEDIA_RECORDER_SERVICE_CREATED:
			#ifdef CFG_FUNC_REMIND_SOUND_EN
			//RemindSound request
			//¼���¼���ʾ�������¼���ļ�Я������ʾ����ʹ��������ʱ
			RemindSoundServiceItemRequest(SOUND_REMIND_LUYIN, FALSE);
			DelayMs(350);//����¼������ʾ������ʱ��
			#endif
			BtRecordCt->RecorderSync = TaskStateStarting;
			MediaRecorderServiceStart();
			break;

		case MSG_MEDIA_RECORDER_SERVICE_STARTED:
			MediaRecorderRun();
			BtRecordCt->RecorderSync = TaskStateRunning;
			break;
		case MSG_MEDIA_RECORDER_STOPPED:
			MediaRecorderServiceStop();
			BtRecordCt->RecorderSync = TaskStateStopping;
			break;
			
		case MSG_MEDIA_RECORDER_ERROR:
			if(BtRecordCt->RecorderSync == TaskStateRunning)
			{
				MediaRecorderStop();
				MediaRecorderServiceStop();
				BtRecordCt->RecorderSync = TaskStateStopping;
			}
			break;

#endif //¼��
			case MSG_MEDIA_RECORDER_SERVICE_STOPPED:
#ifdef CFG_FUNC_RECORDER_EN
				if(msgRecv.msgId == MSG_MEDIA_RECORDER_SERVICE_STOPPED)
				{
					BtRecordCt->RecorderSync = TaskStateNone;
					APP_DBG("Btrec:RecorderKill");
					MediaRecorderServiceKill();
				}
#endif	
				break;
			case MSG_DECODER_SERVICE_STOPPED:
				if(BtRecordCt->ModeKillFlag)
				{
					BtRecordModeStopping(msgRecv.msgId);
				}
				else
				{	
					BtRecordCt->DecoderSync = TaskStateStopped;
					APP_DBG("msg:BtHf:Decoder service Stopped\n");
					BtRecordMsbcKill();
				}
				break;

			case MSG_APP_RES_RELEASE:
				BtRecordResRelease();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_RELEASE_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_MALLOC:
				BtRecordResMalloc(mainAppCt.SamplesPreFrame);
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_MALLOC_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_INIT:
				BtRecordResInit();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_INIT_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}

				BtRecordCt->hfModeAudioResInitOk = 1;
				break;
				
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CFG_FUNC_REMIND_SOUND_EN
			case MSG_DECODER_RESET://���������úͻ��գ��ٽ���Ϣ��
				if(SoftFlagGet(SoftFlagDecoderSwitch))
				{
					APP_DBG("BtRecord:Switch out\n");
					AudioCoreSourceDisable(BT_RECORD_SOURCE_NUM);

					BtRecordCt->AudioCoreBtHf->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)BtRecordCt->SourceDecoder;

					SoftFlagRegister(SoftFlagDecoderRemind);//���ý�����
					if(BtRecord_MsbcDecoderIsInitialized())
					{
						BtRecord_MsbcDecoderStartedSet(FALSE);
					}
					RemindSoundServicePlay();
				}
				else//��appʹ�ý�����ʱ ����
				{
					APP_DBG("BtRecord:Switch in\n");
					
					DecoderSourceNumSet(BT_RECORD_SOURCE_NUM);
					SoftFlagDeregister(SoftFlagDecoderRemind);//���ս�����
					AudioCoreSourceUnmute(BT_RECORD_SOURCE_NUM, TRUE, TRUE);
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
				break;
	
			case MSG_REMIND_SOUND_NEED_DECODER:
				if(!SoftFlagGet(SoftFlagDecoderMask & ~SoftFlagDecoderApp))
				{
					AudioCoreSourceMute(BT_RECORD_SOURCE_NUM, TRUE, TRUE);
					SoftFlagRegister(SoftFlagDecoderSwitch);
					DecoderReset();//�����������λ��׼�����á�
				}
				break;
				
			case MSG_REMIND_SOUND_PLAY_REQUEST_FAIL:
				break;
			
			case MSG_REMIND_SOUND_PLAY_RENEW:
				break;
#endif

			///////////////////////////////////////////////////////
			//HFP control
			case MSG_PLAY_PAUSE:
#ifdef CFG_FUNC_RECORDER_EN	
				APP_DBG("MSG_PLAY_PAUSE\n");
				if(GetMediaRecorderMessageHandle() !=  NULL)
				{
					EncoderServicePause();
					break;
				}
#endif				
				switch(GetHfpState())
				{
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

						ExitBtRecordMode();
						break;

					default:
						break;
				}
				break;
			
			case MSG_BT_HF_TRANS_CHANGED:
				APP_DBG("Hfp Audio Transfer.\n");
				HfpAudioTransfer();
				break;
				
			case MSG_REMIND_SOUND_PLAY_START:
				break;
			
			case MSG_REMIND_SOUND_PLAY_DONE://��ʾ�����Ž���
				//AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
				break;
			
			default:
				if(BtRecordCt->state == TaskStateRunning)
				{
					CommonMsgProccess(msgRecv.msgId);
				}
				break;

		}
		BtRecord_MicProcess();

		BtRecordModeRunning();
		
#ifdef BT_HFP_CALL_DURATION_DISP
		if(BtRecordCt->BtRecordTimeUpdate)
		{
			uint8_t hour = (uint8_t)(BtRecordCt->BtRecordActiveTime / 3600);
			uint8_t minute = (uint8_t)((BtRecordCt->BtRecordActiveTime % 3600) / 60);
			uint8_t second = (uint8_t)(BtRecordCt->BtRecordActiveTime % 60);
			BtRecordCt->BtRecordTimeUpdate = 0;
			APP_DBG("ͨ���У�%02d:%02d:%02d", hour, minute, second);
		}
#endif
	}
}


/*****************************************************************************************
 * ����HF����
 ****************************************************************************************/
bool BtRecordCreate(MessageHandle parentMsgHandle)
{
	bool ret = 0;

	ret = BtRecordInit(parentMsgHandle);
	if(ret)
	{
		BtRecordCt->taskHandle = NULL;
		xTaskCreate(BtRecordEntrance, "BtRecordMode", BT_RECORD_TASK_STACK_SIZE, NULL, BT_RECORD_TASK_PRIO, &BtRecordCt->taskHandle);
		if(BtRecordCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
	}
	if(!ret)
		APP_DBG("BtAudioPlay app create fail!\n");
	return ret;
}


/*****************************************************************************************
 * ɾ��HF����
 ****************************************************************************************/
static void BtRecordMsbcKill(void)
{
	BtRecordCt->MsbcInitFlag = 0;
	BtAppiFunc_SaveScoData(NULL);
	DecoderServiceKill();
	BtRecord_MsbcDecoderDeinit();
	BtRecord_MsbcEncoderDeinit();
}

extern uint8_t gEnterBtRecordMode;
bool BtRecordKill(void)
{
	gEnterBtRecordMode = 0;
	
	//clear voice recognition flag
	SetBtHfpVoiceRecognition(0);
	
	BtAppiFunc_SaveScoData(NULL);
	AudioCoreSourceDisable(BT_RECORD_SOURCE_NUM);
	
	if(BtRecordCt == NULL)
	{
		return FALSE;
	}
	
#ifdef CFG_FUNC_REMIND_SOUND_EN
	//������ʾ�����Ź�����ǿ���˳���ǰģʽ������Ӧ������,��ʾ������������λ����
	RemindSoundServiceReset();

	if(BtRecordCt->SourceDecoder != NULL)
	{
		APP_DBG("SourceDecoder\n");
		osPortFree(BtRecordCt->SourceDecoder);
		BtRecordCt->SourceDecoder = NULL;
	}
#endif

//ע��˴��������TaskStateCreating����stop������δinit����ʱ�������
	BtRecordDeinitialize();	
	AudioCoreProcessConfig((void*)AudioNoAppProcess);

	if(BtRecordCt->BtEncoderSendFifo)
	{
		osPortFree(BtRecordCt->BtEncoderSendFifo);
		BtRecordCt->BtEncoderSendFifo = NULL;
	}

	//msbc
	//if(BtRecordCt->codecType == HFP_AUDIO_DATA_mSBC)
	{
		//Kill used services
		DecoderServiceKill();
	}
#ifdef CFG_FUNC_RECORDER_EN
	if(BtRecordCt->RecorderSync != TaskStateNone)//��¼������ʧ��ʱ����Ҫǿ�л���
	{
		MediaRecorderServiceKill();
		BtRecordCt->RecorderSync = TaskStateNone;
	}

#endif
	if(BtRecordCt->ResamplerPreFifoMutex != NULL)
	{
		vSemaphoreDelete(BtRecordCt->ResamplerPreFifoMutex);
		BtRecordCt->ResamplerPreFifoMutex = NULL;
	}

	if(BtRecordCt->Sink2PcmOutBuf)
	{
		osPortFree(BtRecordCt->Sink2PcmOutBuf);
		BtRecordCt->Sink2PcmOutBuf = NULL;
	}

	if(BtRecordCt->ResamplerPreFifo)
	{
		osPortFree(BtRecordCt->ResamplerPreFifo);
		BtRecordCt->ResamplerPreFifo = NULL;
	}

	//msbc
	//if(BtRecordCt->codecType == HFP_AUDIO_DATA_mSBC)
	{
		//msbc decoder/encoder deinit
		BtRecord_MsbcDecoderDeinit();
		BtRecord_MsbcEncoderDeinit();
		
		//resample deinit
		if(BtRecordCt->ResampleOutFifo)
		{
			osPortFree(BtRecordCt->ResampleOutFifo);
			BtRecordCt->ResampleOutFifo = NULL;
		}
			
		if(BtRecordCt->AudioSink2OutBuf)
		{
			osPortFree(BtRecordCt->AudioSink2OutBuf);
			BtRecordCt->AudioSink2OutBuf = NULL;
		}

		if(BtRecordCt->AudioSink2OutResamplerCt)
		{
			osPortFree(BtRecordCt->AudioSink2OutResamplerCt);
			BtRecordCt->AudioSink2OutResamplerCt = NULL;
		}
	}
	//else
	//cvsd
	{
		//deinit cvsd_plc
		if(BtRecordCt->cvsdPlcState)
		{
			osPortFree(BtRecordCt->cvsdPlcState);
			BtRecordCt->cvsdPlcState = NULL;
		}
	}

	if(BtRecordCt->Source1Buf_ScoData)
	{
		osPortFree(BtRecordCt->Source1Buf_ScoData);
		BtRecordCt->Source1Buf_ScoData = NULL;
	}

	if(BtRecordCt->ScoBuffer)
	{
		osPortFree(BtRecordCt->ScoBuffer);
		BtRecordCt->ScoBuffer = NULL;
	}

	//cvsd
	if(BtRecordCt->ScoOutSrcOutBuf)
	{
		osPortFree(BtRecordCt->ScoOutSrcOutBuf);
		BtRecordCt->ScoOutSrcOutBuf = NULL;
	}
	if(BtRecordCt->ScoOutResamplerCt)
	{
		osPortFree(BtRecordCt->ScoOutResamplerCt);
		BtRecordCt->ScoOutResamplerCt = NULL;
	}

	if(BtRecordCt->ScoInSrcOutBuf)
	{
		osPortFree(BtRecordCt->ScoInSrcOutBuf);
		BtRecordCt->ScoInSrcOutBuf = NULL;
	}
	if(BtRecordCt->ScoInResamplerCt)
	{
		osPortFree(BtRecordCt->ScoInResamplerCt);
		BtRecordCt->ScoInResamplerCt = NULL;
	}

	//release used buffer: input/output sco fifo
	if(BtRecordCt->ScoOutputFifo)
	{
		osPortFree(BtRecordCt->ScoOutputFifo);
		BtRecordCt->ScoOutputFifo = NULL;
	}
	if(BtRecordCt->ScoInputFifo)
	{
		osPortFree(BtRecordCt->ScoInputFifo);
		BtRecordCt->ScoInputFifo = NULL;
	}

	//ADC/DAC resume
	ResumeAudioCoreMicSource();
	
	BtRecordCt->AudioCoreBtHf = NULL;
	
	//task  ��ɾ������ɾ���䣬����Դ
	if(BtRecordCt->taskHandle)
	{
		vTaskDelete(BtRecordCt->taskHandle);
		BtRecordCt->taskHandle = NULL;
	}

	//Msgbox
	if(BtRecordCt->msgHandle)
	{
		MessageDeregister(BtRecordCt->msgHandle);
		BtRecordCt->msgHandle = NULL;
	}
	osPortFree(BtRecordCt);
	BtRecordCt = NULL;
	APP_DBG("!!BtRecordCt\n");

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioEffectsDeInit();
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	//��������ʾ��־,�˳���ǰģʽ,�ص�����ģʽ��������ʾ��
	SoftFlagRegister(SoftFlagRemindMask);
#endif

	return TRUE;
}

/*****************************************************************************************
 ****************************************************************************************/
bool BtRecordStart(void)
{
	MessageContext		msgSend;

	if(BtRecordCt == NULL)
		return FALSE;

	printf("BtRecordStart\n");
	msgSend.msgId		= MSG_TASK_START;
	return MessageSend(BtRecordCt->msgHandle, &msgSend);
}

bool BtRecordStop(void)
{
	MessageContext		msgSend;

	if(BtRecordCt == NULL)
		return FALSE;

	printf("BtRecordStop\n");
	
	AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceMute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	AudioCoreSourceMute(REMIND_SOURCE_NUM, TRUE, TRUE);
#endif
	vTaskDelay(30);


	AudioCoreSourceDisable(BT_RECORD_SOURCE_NUM);
	AudioCoreSourceDisable(0);

	msgSend.msgId		= MSG_TASK_STOP;
	return MessageSend(BtRecordCt->msgHandle, &msgSend);
}

/*****************************************************************************************
 ****************************************************************************************/
MessageHandle GetBtRecordMessageHandle(void)
{
	if(BtRecordCt == NULL)
	{
		return NULL;
	}
	return BtRecordCt->msgHandle;
}

/*****************************************************************************************
 * ����/�˳�HFģʽ
 ****************************************************************************************/
void EnterBtRecordMode(void)
{
	if(GetSystemMode() != AppModeBtRecordPlay)
	{
		ResourceRegister(AppResourceBtRecord);
		BtRecordMsgToParent(MSG_DEVICE_SERVICE_BTRECORD_IN);
	}
}

void ExitBtRecordMode(void)
{
	if(mainAppCt.appCurrentMode == AppModeBtRecordPlay)
	{
		ResourceDeregister(AppResourceBtRecord);
		BtRecordMsgToParent(MSG_DEVICE_SERVICE_BTRECORD_OUT);

		//���˳�ͨ��ģʽǰ,�ȹر�sink2ͨ·,����AudioCore����תʱ,ʹ�õ�sink��ص��ڴ�,��Hfģʽkill�������ͻ,����Ұָ������
		AudioCoreSinkDisable(AUDIO_HF_SCO_SINK_NUM);

		BtRecordCt->ModeKillFlag=1;
	}
}

#else
/*****************************************************************************************
 * 
 ****************************************************************************************/
MessageHandle GetBtRecordMessageHandle(void)
{
	return NULL;
}

uint32_t BtRecord_MsbcDataLenGet(void)
{
	return 0;
}

bool BtRecordCreate(MessageHandle parentMsgHandle)
{
	return FALSE;
}

bool BtRecordKill(void)
{
	return FALSE;
}

bool BtRecordStart(void)
{
	return FALSE;
}

bool BtRecordStop(void)
{
	return FALSE;
}

uint8_t BtRecordDecoderSourceNum(void)
{
	return 0;
}

void ExitBtRecordMode(void)
{
}

#endif
#endif

