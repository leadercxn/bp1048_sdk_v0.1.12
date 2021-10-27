/**
 **************************************************************************************
 * @file    Radio_mode.c
 * @brief
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2017-12-26 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
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
#include "audio_effect.h"
#include "radio_api.h"
#ifdef FUNC_RADIO_RDA5807_EN
#include "RDA5807.h"
#endif
#ifdef FUNC_RADIO_QN8035_EN
  #include "QN8035.h"
#endif
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "recorder_service.h"
#include "decoder_service.h"
#include "remind_sound_item.h"
#include "remind_sound_service.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "audio_vol.h"
#include "clk.h"
#include "ctrlvars.h"
#include "main_task.h"
#include "reset.h"
#include "device_service.h"

#ifdef CFG_APP_RADIOIN_MODE_EN

#define RADIO_PLAY_TASK_STACK_SIZE		512//1024
#define RADIO_PLAY_TASK_PRIO			3
#define RADIO_NUM_MESSAGE_QUEUE			10

#define RADIO_SOURCE_NUM				APP_SOURCE_NUM

typedef struct _RadioPlayContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;

	TaskState			state;

	uint32_t			*ADCFIFO;			//ADC的DMA循环fifo
	uint16_t 			*Source1Buf_Radio;  //ADC 取LineIn数据
	AudioCoreContext 	*AudioCoreRadio;
#ifdef CFG_FUNC_REMIND_SOUND_EN
	uint16_t*			Source2Decoder;
	TaskState			DecoderSync;
#endif

#ifdef CFG_FUNC_RECORDER_EN
	TaskState			RecorderSync;
#endif

	//play
	uint32_t 			SampleRate;

}RadioPlayContext;


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
static uint8_t DmaChannelMap[29] = {
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

static  RadioPlayContext*		RadioPlayCt;

static void RadioPlayModeCreating(uint16_t msgId);
static void RadioPlayModeStarting(uint16_t msgId);
static void RadioPlayModeStopped(void);
static void RadioPlayRunning(uint16_t msgId);

RADIO_CONTROL* sRadioControl = NULL;//
RADIO_NAME 	Radio_Name = RADIO_NONE;//

void RadioMute(bool mute)
{
#ifdef FUNC_RADIO_RDA5807_EN
	RDA5807Mute(mute);
#endif
#ifdef FUNC_RADIO_QN8035_EN
	QN8035Mute(mute);
#endif
}

void RadioSetFreq(uint16_t freq)
{
#ifdef FUNC_RADIO_RDA5807_EN
	RDA5807SetFreq(freq);
#endif
#ifdef FUNC_RADIO_QN8035_EN
	QN8035SetFreq(freq);
#endif
}

bool RadioPowerOn(void)
{
#ifdef FUNC_RADIO_RDA5807_EN
	return RDA5807PowerOn();
#endif
#ifdef FUNC_RADIO_QN8035_EN
	return QN8035PowerOn();
#endif
}

void RadioInit(void)
{
#ifdef FUNC_RADIO_RDA5807_EN
	RDA5807Init();
#endif
#ifdef FUNC_RADIO_QN8035_EN
	QN8035Init();
#endif
}

void RadioPowerDown(void)
{
#ifdef FUNC_RADIO_RDA5807_EN
	RDA5807PowerDown();
#endif
#ifdef FUNC_RADIO_QN8035_EN
	QN8035PowerDown();
#endif
}

uint8_t RadioSearchRead(uint16_t freq)
{
#ifdef FUNC_RADIO_RDA5807_EN
	return RDA5807SearchRead(freq);
#endif
#ifdef FUNC_RADIO_QN8035_EN
	return QN8035SearchRead(freq);
#endif
}


static uint16_t RadioGetBaseFreq(void)
{
	switch(sRadioControl->CurFreqArea)
	{
		case FM_AREA_CHINA:
			return CHINA_BASE_FREQ;
		case FM_AREA_JAPAN:
			return JAPAN_BASE_FREQ;
		case FM_AREA_RUSSIA:
			return RUSSIA_BASE_FREQ;
		default:
			return OTHER_BASE_FREQ;
	}
}

#ifdef CFG_FUNC_BREAKPOINT_EN
void RadioGetBreakPoint(void)
{
	BP_RADIO_INFO *pBpRadioInfo;
//	BP_SYS_INFO *pBpSysInfo;

	pBpRadioInfo = (BP_RADIO_INFO *)BP_GetInfo(BP_RADIO_INFO_TYPE);
//	pBpSysInfo = (BP_SYS_INFO *)BP_GetInfo(BP_SYS_INFO_TYPE);

	//gSys.Volume = BP_GET_ELEMENT(pBpSysInfo->Volume);
    //gSys.Eq = BP_GET_ELEMENT(pBpSysInfo->Eq);
	sRadioControl->CurFreqArea = BP_GET_ELEMENT(pBpRadioInfo->CurBandIdx);
	sRadioControl->Freq = BP_GET_ELEMENT(pBpRadioInfo->CurFreq);
	sRadioControl->ChlCount = BP_GET_ELEMENT(pBpRadioInfo->StationCount);
	if(sRadioControl->ChlCount > 0)
	{
		uint32_t i;
		for(i = 0; i < sRadioControl->ChlCount; i++)
		{
			sRadioControl->Channel[i] = (uint16_t)(BP_GET_ELEMENT(pBpRadioInfo->StationList[i]));
			if((sRadioControl->Channel[i] + RadioGetBaseFreq()) == sRadioControl->Freq)
			{
				sRadioControl->CurStaIdx = (uint8_t)i;
			}
		}
	}
}
#endif

void RadioPlay(void)
{
	if(sRadioControl->State == RADIO_STATUS_SEEKING || sRadioControl->State == RADIO_STATUS_PREVIEW)
	{
		RadioMute(TRUE);
		sRadioControl->State = RADIO_STATUS_IDLE;
	}

	RadioMute(TRUE);
	RadioSetFreq(sRadioControl->Freq);
	APP_DBG("play Freq = %d\n", sRadioControl->Freq);
	RadioMute(FALSE);
	sRadioControl->State = RADIO_STATUS_PLAYING;
}

void RadioSeekEnd(void)
{
	if(sRadioControl->ChlCount > 0)
	{
		sRadioControl->Freq = sRadioControl->Channel[sRadioControl->CurStaIdx] + RadioGetBaseFreq();
	}
	else
	{
		sRadioControl->Freq = sRadioControl->MinFreq;
	}
	APP_DBG("ChlCount = %d\n", sRadioControl->ChlCount);
	APP_DBG("Freq = %d\n", sRadioControl->Freq);
	RadioPlay();
#ifdef CFG_FUNC_BREAKPOINT_EN
	BackupInfoUpdata(BACKUP_RADIO_INFO);
#endif
}

bool RadioChannelAdd(void)
{
	if(sRadioControl->ChlCount == MAX_RADIO_CHANNEL_NUM)
	{
		return FALSE;
	}
	sRadioControl->Channel[sRadioControl->ChlCount++] = sRadioControl->Freq - RadioGetBaseFreq();

#ifdef CFG_FUNC_BREAKPOINT_EN
	//RadioPlayBPUpdata();
	BackupInfoUpdata(BACKUP_RADIO_INFO);
#endif

	return TRUE;
}

void RadioSeekStart(void)
{
	RadioMute(TRUE);

	if(!(RADIO_STATUS_SEEKING == sRadioControl->State || RADIO_STATUS_PREVIEW == sRadioControl->State))
	{
		sRadioControl->Freq = sRadioControl->MinFreq;
		sRadioControl->State = RADIO_STATUS_SEEKING;
		sRadioControl->ChlCount = 0;
		sRadioControl->CurStaIdx = 0;
		memset(sRadioControl->Channel, 0, sizeof(sRadioControl->Channel));
	}
	RadioSetFreq(sRadioControl->Freq);

	//TimeOutSet(&sRadioControl->TimerHandle, 50);
}

void RadioTimerCB(void* unused)
{
#ifdef CFG_FUNC_DISPLAY_EN
	MessageContext	msgSend;
#endif
	switch(sRadioControl->State)
	{
		case RADIO_STATUS_PLAYING:
			break;

		case RADIO_STATUS_SEEKING_DOWN:
			if(RadioSearchRead(sRadioControl->Freq))
			 {
				RadioPlay();
			 }
			else
			{
				if(sRadioControl->Freq > sRadioControl->MinFreq)
				 {
				   sRadioControl->Freq--;
				   RadioSetFreq(sRadioControl->Freq);
				 }
				else
				 RadioPlay();
			}
			break;

		case RADIO_STATUS_SEEKING_UP:
			if(RadioSearchRead(sRadioControl->Freq))
			 {
				RadioPlay();
			 }
			else
			{
				if(sRadioControl->Freq < sRadioControl->MaxFreq)
				 {
				   sRadioControl->Freq++;
				   RadioSetFreq(sRadioControl->Freq);
				 }
				else
				 RadioPlay();
			}
			break;

		case RADIO_STATUS_SEEKING:
			if(RadioSearchRead(sRadioControl->Freq) && (sRadioControl->Freq < sRadioControl->MaxFreq))//
			{
				if(RadioChannelAdd())
				{
#ifdef CFG_FUNC_DISPLAY_EN
                    msgSend.msgId = MSG_DISPLAY_SERVICE_SEARCH_STATION;
                    MessageSend(GetDisplayMessageHandle(), &msgSend);
#endif
					if(sRadioControl->Freq >= sRadioControl->MaxFreq)//
					{
						RadioSeekEnd();
						break;
					}
					else
					{
						sRadioControl->Freq++;
						RadioSeekStart();
						//APP_DBG("sRadioControl->Freq = %d \n",sRadioControl->Freq);
					}
				}
				else
				{
					RadioSeekEnd();
				}
			}
			else
			{
				if(sRadioControl->Freq >= sRadioControl->MaxFreq)//
				{
					RadioSeekEnd();
					break;
				}
				else
				{
					sRadioControl->Freq++;
					RadioSeekStart();
#ifdef CFG_FUNC_DISPLAY_EN
                    //msgSend.msgId = MSG_DISPLAY_SERVICE_RADIO;
                    //MessageSend(GetDisplayMessageHandle(), &msgSend);
#endif
					//APP_DBG("sRadioControl->Freq = %d \n",sRadioControl->Freq);
				}
			}
			break;

		case RADIO_STATUS_PREVIEW: //
			{
				//static uint32_t LastTime = 0;
				//if(LastTime == 0)
				//{
				//	LastTime = TICKS_TO_MSECS(OSSysTickGet()) - MIN_TIMER_PERIOD;
				//}
/*
				if((TICKS_TO_MSECS(OSSysTickGet()) - LastTime) >= FM_PERVIEW_TIMEOUT)
				{
					RadioMute(TRUE);
					sRadioControl->State = RADIO_STATUS_SEEKING;
					if(sRadioControl->Freq < sRadioControl->MaxFreq)
					{
						sRadioControl->Freq++;
						RadioSeekStart();
					}
					else
					{
						RadioSeekEnd();
					}
					LastTime = 0;
				}  */
			}
			break;

		case RADIO_STATUS_IDLE:
		default:
			break;
	}
}

bool RadioSwitchChannel(uint32_t Msg)
{
#ifdef CFG_FUNC_DISPLAY_EN
	MessageContext	msgSend;
#endif
	if(sRadioControl->State == RADIO_STATUS_SEEKING)
	{
		return FALSE;
	}
	if(sRadioControl->ChlCount == 0)
	{
		return FALSE;
	}

	if((Msg == MSG_RADIO_PLAY_NEXT) ||(Msg == MSG_NEXT))
	{
		sRadioControl->CurStaIdx++;
		sRadioControl->CurStaIdx %= sRadioControl->ChlCount;
	}
	else
	{
		if(sRadioControl->CurStaIdx > 0)
		{
			sRadioControl->CurStaIdx--;
		}
		else
		{
			sRadioControl->CurStaIdx = (uint8_t)(sRadioControl->ChlCount - 1);
		}
	}
/*
#ifdef RADIO_DELAY_SWITCH_CHANNEL
	if(!sRadioControl->DelayDoTimer.IsRunning)
	{
		InitTimer(&sRadioControl->DelayDoTimer, 300, RadioDelaySwithChlCB);
		StartTimer(&sRadioControl->DelayDoTimer);
	}
#else */
	sRadioControl->Freq = sRadioControl->Channel[sRadioControl->CurStaIdx] + RadioGetBaseFreq();
	RadioPlay();
//#endif

#ifdef CFG_FUNC_BREAKPOINT_EN
	//RadioPlayBPUpdata();
	BackupInfoUpdata(BACKUP_RADIO_INFO);
#endif

	return TRUE;
}

void RadioPlayResRelease(void)
{
#if (RADIO_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	AudioADC_Disable(ADC1_MODULE);
	//Reset_FunctionReset(DMAC_FUNC_SEPA);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
#else
	AudioADC_Disable(ADC0_MODULE);
	//Reset_FunctionReset(DMAC_FUNC_SEPA);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
#endif
	if(RadioPlayCt->ADCFIFO != NULL)
	{
		APP_DBG("ADCFIFO\n");
		osPortFree(RadioPlayCt->ADCFIFO);
		RadioPlayCt->ADCFIFO = NULL;
	}
	if(RadioPlayCt->Source1Buf_Radio != NULL)
	{
		APP_DBG("Source1Buf_Radio\n");
		osPortFree(RadioPlayCt->Source1Buf_Radio);
		RadioPlayCt->Source1Buf_Radio = NULL;
	}
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(RadioPlayCt->Source2Decoder != NULL)
	{
		APP_DBG("SourceDecoder\n");
		osPortFree(RadioPlayCt->Source2Decoder);
		RadioPlayCt->Source2Decoder = NULL;
	}
#endif
	//return TRUE;
}

bool RadioPlayResMalloc(uint16_t SampleLen)
{
	//LineIn4  digital (DMA)
	RadioPlayCt->ADCFIFO = (uint32_t*)osPortMallocFromEnd(SampleLen * 2 * 2 * 2);
	if(RadioPlayCt->ADCFIFO == NULL)
	{
		return FALSE;
	}
	memset(RadioPlayCt->ADCFIFO, 0, SampleLen * 2 * 2 * 2);

	//InCore1 buf
	RadioPlayCt->Source1Buf_Radio = (uint16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);//stereo
	if(RadioPlayCt->Source1Buf_Radio == NULL)
	{
		return FALSE;
	}
	memset(RadioPlayCt->Source1Buf_Radio, 0, SampleLen * 2 * 2);

	//InCore buf
#ifdef CFG_FUNC_REMIND_SOUND_EN
	RadioPlayCt->Source2Decoder = (uint16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);//One Frame
	if(RadioPlayCt->Source2Decoder == NULL)
	{
		return FALSE;
	}
	memset(RadioPlayCt->Source2Decoder, 0, SampleLen * 2 * 2);//2K
#endif
	return TRUE;
}

void RadioPlayResInit(void)
{
	if(RadioPlayCt->Source1Buf_Radio != NULL)
	{
		RadioPlayCt->AudioCoreRadio->AudioSource[RADIO_SOURCE_NUM].PcmInBuf = (int16_t *)RadioPlayCt->Source1Buf_Radio;
	}
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(RadioPlayCt->Source2Decoder != NULL)
	{
		RadioPlayCt->AudioCoreRadio->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)RadioPlayCt->Source2Decoder;
	}
#endif
#if (RADIO_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	//DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_ADC1_RX, mainAppCt.SamplesPreFrame * 2 * 2, (void*)RadioPlayCt->ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
	if(AudioADC_IsOverflow(ADC1_MODULE))
	{
		AudioADC_OverflowClear(ADC1_MODULE);
	}
	AudioADC_Clear(ADC1_MODULE);
    DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC1_RX);
    AudioADC_LREnable(ADC1_MODULE, 1, 1);
    AudioADC_Enable(ADC1_MODULE);
#else
	//DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_ADC0_RX, mainAppCt.SamplesPreFrame * 2 * 2, (void*)RadioPlayCt->ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
	if(AudioADC_IsOverflow(ADC0_MODULE))
	{
		AudioADC_OverflowClear(ADC0_MODULE);
	}
	AudioADC_Clear(ADC0_MODULE);
    DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC0_RX);
    AudioADC_LREnable(ADC0_MODULE, 1, 1);
    AudioADC_Enable(ADC0_MODULE);
#endif
}

//Radio模式参数配置，资源初始化
static bool RadioPlay_Init(MessageHandle parentMsgHandle)
{
	DMA_ChannelAllocTableSet(DmaChannelMap);//FMIn

	RadioPlayCt = (RadioPlayContext*)osPortMalloc(sizeof(RadioPlayContext));
	if(RadioPlayCt == NULL)
	{
		return FALSE;
	}
	memset(RadioPlayCt, 0, sizeof(RadioPlayContext));
	RadioPlayCt->msgHandle = MessageRegister(RADIO_NUM_MESSAGE_QUEUE);
	RadioPlayCt->parentMsgHandle = parentMsgHandle;
	RadioPlayCt->state = TaskStateCreating;

	sRadioControl = (RADIO_CONTROL *)osPortMalloc(sizeof(RADIO_CONTROL));
	sRadioControl->State = RADIO_STATUS_IDLE;

#ifdef	CFG_FUNC_BREAKPOINT_EN
	RadioGetBreakPoint();
#endif

	sRadioControl->CurFreqArea = FM_AREA_CHINA;

	// Audio core config
	RadioPlayCt->SampleRate = CFG_PARA_SAMPLE_RATE;

	TimeOutSet(&sRadioControl->TimerHandle, 100);
	switch(sRadioControl->CurFreqArea)
	{
		case FM_AREA_CHINA:
			sRadioControl->MinFreq = RADIO_DEF_MIN_FREQ_VALUE;
			sRadioControl->MaxFreq = RADIO_DEF_MAX_FREQ_VALUE;
			break;
		case FM_AREA_JAPAN:
			sRadioControl->MinFreq = RADIO_JAP_MIN_FREQ_VALUE;
			sRadioControl->MaxFreq = RADIO_JAP_MAX_FREQ_VALUE;
			break;
		case FM_AREA_RUSSIA:
			sRadioControl->MinFreq = RADIO_RUS_MIN_FREQ_VALUE;
			sRadioControl->MaxFreq = RADIO_RUS_MAX_FREQ_VALUE;
			break;
		default:
			sRadioControl->MinFreq = RADIO_OTH_MIN_FREQ_VALUE;
			sRadioControl->MaxFreq = RADIO_OTH_MAX_FREQ_VALUE;
			break;
	}

	if(sRadioControl->Freq < sRadioControl->MinFreq)
	{
		sRadioControl->Freq = sRadioControl->MinFreq;
	}
	else if(sRadioControl->Freq > sRadioControl->MaxFreq)
	{
		sRadioControl->Freq = sRadioControl->MaxFreq;
	}
	APP_DBG("sRadioControl->Freq = %d \n",sRadioControl->Freq);

	//Radio  analog
	//Clock_GPIOOutSel(GPIO_CLK_OUT_A29, HOSC_CLK_DIV);
	//GPIO_PortBModeSet(GPIOB6, 0x02);
#ifdef CFG_RADIO_CLK_M12
	Clock_GPIOOutSel(GPIO_CLK_OUT_A29, HOSC_CLK_DIV);
#endif
	//osTaskDelay(100);
	//Clock_GPIOOutDisable(GPIO_CLK_OUT_B6);
	//Clock_GPIOOutDisable(GPIO_CLK_OUT_A29);

	//Clock_GPIOOutSel(GPIO_CLK_OUT_B7, OSC_32K_CLK);
	//osTaskDelay(100);
	//Clock_GPIOOutDisable(GPIO_B7);
	if(RadioPowerOn())
	{
		APP_DBG("Radio PowerOn\n");
		RadioInit();
		RadioMute(TRUE);
		//sRadioControl->Freq = 971;
		RadioSetFreq(sRadioControl->Freq);
		RadioMute(FALSE);
	}
	else
	{
		//APP_DBG("err: radio 硬件异常!!!,退出radio mode\n");
		//extern void ResourceDeregister(uint32_t Resources);
		//ResourceDeregister(AppResourceRadio);
		//return FALSE;
	}

	if(!RadioPlayResMalloc(mainAppCt.SamplesPreFrame))
	{
		APP_DBG("RadioPlayResMalloc Res Error!\n");
		return FALSE;
	}    

#if (RADIO_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	AudioADC_DigitalInit(ADC1_MODULE, RadioPlayCt->SampleRate, (void*)RadioPlayCt->ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 *2);
#else
	AudioADC_DigitalInit(ADC0_MODULE, RadioPlayCt->SampleRate, (void*)RadioPlayCt->ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 *2);
#endif

	//Audio init
//	//note Soure0.和sink0已经在main app中配置，不要随意配置
	RadioPlayCt->AudioCoreRadio = (AudioCoreContext*)&AudioCore;

	//Soure1.
	RadioPlayCt->AudioCoreRadio->AudioSource[RADIO_SOURCE_NUM].Enable = 0;
#if (RADIO_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	RadioPlayCt->AudioCoreRadio->AudioSource[RADIO_SOURCE_NUM].FuncDataGet = AudioADC1DataGet;
	RadioPlayCt->AudioCoreRadio->AudioSource[RADIO_SOURCE_NUM].FuncDataGetLen = AudioADC1DataLenGet;
#else
	RadioPlayCt->AudioCoreRadio->AudioSource[RADIO_SOURCE_NUM].FuncDataGet = AudioADC0DataGet;
	RadioPlayCt->AudioCoreRadio->AudioSource[RADIO_SOURCE_NUM].FuncDataGetLen = AudioADC0DataLenGet;
#endif

	RadioPlayCt->AudioCoreRadio->AudioSource[RADIO_SOURCE_NUM].IsSreamData = TRUE;//FALSE;
	RadioPlayCt->AudioCoreRadio->AudioSource[RADIO_SOURCE_NUM].PcmFormat = 2;//radio stereo
	RadioPlayCt->AudioCoreRadio->AudioSource[RADIO_SOURCE_NUM].PcmInBuf = (int16_t *)RadioPlayCt->Source1Buf_Radio;

#ifdef CFG_FUNC_REMIND_SOUND_EN
	//Core Soure2 Para
	DecoderSourceNumSet(REMIND_SOURCE_NUM);
	RadioPlayCt->AudioCoreRadio->AudioSource[REMIND_SOURCE_NUM].Enable = 0;
	RadioPlayCt->AudioCoreRadio->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
	RadioPlayCt->AudioCoreRadio->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
	RadioPlayCt->AudioCoreRadio->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
	RadioPlayCt->AudioCoreRadio->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 2; //stereo
	RadioPlayCt->AudioCoreRadio->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)RadioPlayCt->Source2Decoder;
#endif

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
    #ifdef CFG_FUNC_MIC_KARAOKE_EN
	RadioPlayCt->AudioCoreRadio->AudioEffectProcess = (AudioCoreProcessFunc)AudioEffectProcess;
	#else
	RadioPlayCt->AudioCoreRadio->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
	#endif
#else
	RadioPlayCt->AudioCoreRadio->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
#endif

#ifdef CFG_FUNC_RECORDER_EN
	RadioPlayCt->RecorderSync = TaskStateNone;
#endif

	AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
	return TRUE;
}

//创建从属services
static void RadioPlayModeCreate(void)
{
	bool NoService = TRUE;

	// Create service task
#ifdef CFG_FUNC_REMIND_SBC
	DecoderServiceCreate(RadioPlayCt->msgHandle, DECODER_BUF_SIZE_SBC, DECODER_FIFO_SIZE_FOR_SBC);//提示音格式决定解码器内存消耗
	NoService = FALSE;
#elif defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceCreate(RadioPlayCt->msgHandle, DECODER_BUF_SIZE_MP3, DECODER_FIFO_SIZE_FOR_MP3);
	NoService = FALSE;
#endif
	if(NoService)
	{
		RadioPlayModeCreating(MSG_NONE);
	}
}

//All of services is created
//Send CREATED message to parent
static void RadioPlayModeCreating(uint16_t msgId)
{
	MessageContext		msgSend;
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(msgId == MSG_DECODER_SERVICE_CREATED)
	{
		RadioPlayCt->DecoderSync = TaskStateReady;
	}
	if(RadioPlayCt->DecoderSync == TaskStateReady)
#endif
	{
		msgSend.msgId		= MSG_RADIO_AUDIO_MODE_CREATED;
		MessageSend(RadioPlayCt->parentMsgHandle, &msgSend);
		RadioPlayCt->state = TaskStateReady;
	}
}

static void RadioPlayModeStart(void)
{
	bool NoService = TRUE;
#ifdef CFG_FUNC_REMIND_SOUND_EN
	DecoderServiceStart();
	RadioPlayCt->DecoderSync = TaskStateStarting;
	NoService = FALSE;
#endif
	if(NoService)
	{
		RadioPlayModeStarting(MSG_NONE);
	}
	else
	{
		RadioPlayCt->state = TaskStateStarting;
	}
}

static void RadioPlayModeStarting(uint16_t msgId)
{
	MessageContext		msgSend;
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(msgId == MSG_DECODER_SERVICE_STARTED)
	{
		RadioPlayCt->DecoderSync = TaskStateRunning;
	}
	if(RadioPlayCt->DecoderSync == TaskStateRunning)
#endif
	{
		msgSend.msgId		= MSG_RADIO_AUDIO_MODE_STARTED;
		MessageSend(RadioPlayCt->parentMsgHandle, &msgSend);

		AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
		vTaskDelay(20);
		AudioCoreSourceEnable(RADIO_SOURCE_NUM);

		RadioPlayCt->state = TaskStateRunning;
#ifndef CFG_FUNC_REMIND_SOUND_EN
		AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
#else
		if(!SoftFlagGet(SoftFlagRemindMask))
		{
			if(!RemindSoundServiceItemRequest(SOUND_REMIND_FMMODE, FALSE)) //插播提示音
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
			else
				AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
			
		}
#endif
		if(sRadioControl->ChlCount == 0)
		{
			//进入收音模式后，如果无电台，则进行自动收台
			msgSend.msgId		= MSG_RADIO_PLAY_SCAN;
			MessageSend(RadioPlayCt->msgHandle, &msgSend);
		}
	}
}

static void RadioPlayModeStop(void)
{
	bool NoService = TRUE;
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(RadioPlayCt->DecoderSync != TaskStateNone && RadioPlayCt->DecoderSync != TaskStateStopped)
	{//解码器是 随app kill
		DecoderServiceStop();
		RadioPlayCt->DecoderSync = TaskStateStopping;
		NoService = FALSE;
	}
#endif
#ifdef CFG_FUNC_RECORDER_EN
	if(RadioPlayCt->RecorderSync != TaskStateNone)
	{//此service 随用随Kill
		MediaRecorderServiceStop();
		RadioPlayCt->RecorderSync = TaskStateStopping;
		NoService = FALSE;
	}

#endif
	RadioPlayCt->state = TaskStateStopping;
	if(NoService)
	{
		RadioPlayModeStopped();
	}
}

static void RadioPlayModeStopping(uint16_t msgId)
{
#ifdef CFG_FUNC_RECORDER_EN
	if(msgId == MSG_MEDIA_RECORDER_SERVICE_STOPPED)
	{
		RadioPlayCt->RecorderSync = TaskStateNone;
		APP_DBG("Radio:RecorderKill");
		MediaRecorderServiceKill();
	}
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(msgId == MSG_DECODER_SERVICE_STOPPED)
	{
		RadioPlayCt->DecoderSync = TaskStateNone;
	}
#endif
	if((RadioPlayCt->state == TaskStateStopping)
#ifdef CFG_FUNC_RECORDER_EN
		&& (RadioPlayCt->RecorderSync == TaskStateNone)
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
		&& (RadioPlayCt->DecoderSync == TaskStateNone)
#endif
	)
	{
		RadioPlayModeStopped();
	}
}

static void RadioPlayModeStopped(void)
{
	MessageContext		msgSend;


	MessageClear(RadioPlayCt->msgHandle);
	RadioPlayCt->state = TaskStateStopped;
	msgSend.msgId		= MSG_RADIO_AUDIO_MODE_STOPPED;
	MessageSend(RadioPlayCt->parentMsgHandle, &msgSend);
}

static void RadioPlayEntrance(void * param)
{
	MessageContext		msgRecv;

	// Create services
	RadioPlayModeCreate();
#ifdef CFG_FUNC_AUDIO_EFFECT_EN	
	AudioEffectModeSel(mainAppCt.EffectMode, 2);//0=init hw,1=effect,2=hw+effect
#ifdef CFG_COMMUNICATION_BY_UART
	UART1_Communication_Init((void *)(&UartRxBuf[0]), 1024, (void *)(&UartTxBuf[0]), 1024);
#endif
#endif

    //LineIn4  analog (ADC)
	AudioAnaChannelSet(RADIO_INPUT_CHANNEL);

	APP_DBG("Radio Mode\n");
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
		if(IsTimeOut(&sRadioControl->TimerHandle))
		{
			TimeOutSet(&sRadioControl->TimerHandle, 100);
			RadioTimerCB(NULL);
		}

		MessageRecv(RadioPlayCt->msgHandle, &msgRecv, 10);//MAX_RECV_MSG_TIMEOUT);
		
		switch(msgRecv.msgId)
		{
			case MSG_DECODER_SERVICE_CREATED:
				RadioPlayModeCreating(msgRecv.msgId);
				break;

			case MSG_TASK_START:
				RadioPlayModeStart();
				break;

			case MSG_DECODER_SERVICE_STARTED:
				RadioPlayModeStarting(msgRecv.msgId);
				break;

			case MSG_TASK_STOP:
#ifdef CFG_FUNC_REMIND_SOUND_EN
				RemindSoundServiceReset();
#endif
#if 0//CFG_COMMUNICATION_BY_USB
				NVIC_DisableIRQ(Usb_IRQn);
				OTG_DeviceDisConnect();
#endif
				RadioPlayModeStop();
				break;

			case MSG_MEDIA_RECORDER_SERVICE_STOPPED:
			case MSG_DECODER_SERVICE_STOPPED:
				RadioPlayModeStopping(msgRecv.msgId);
				break;

			case MSG_APP_RES_RELEASE:
				RadioPlayResRelease();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_RELEASE_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_MALLOC:
				RadioPlayResMalloc(mainAppCt.SamplesPreFrame);
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_MALLOC_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;

			case MSG_APP_RES_INIT:
				RadioPlayResInit();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_INIT_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
				
			case MSG_REMIND_SOUND_PLAY_START:
				APP_DBG("radioin: MSG_REMIND_SOUND_PLAY_START：\n");
				break;

			case MSG_REMIND_SOUND_PLAY_DONE://提示音播放结束
			case MSG_REMIND_SOUND_PLAY_REQUEST_FAIL:
				AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
				break;
			
			default:
				if(RadioPlayCt->state == TaskStateRunning)
				{
					RadioPlayRunning(msgRecv.msgId);
                  #ifdef CFG_RES_IR_NUMBERKEY
					if(Number_select_flag)
					{
						if(IsTimeOut(&Number_selectTimer))
						{
							if((Number_value >= sRadioControl->MinFreq) && (Number_value <= sRadioControl->MaxFreq))
							  {
								sRadioControl->Freq = Number_value;
								RadioSetFreq(sRadioControl->Freq);
							  }
							else if(Number_value && (Number_value < MAX_RADIO_CHANNEL_NUM))
							 {
								sRadioControl->CurStaIdx = Number_value - 1;
								sRadioControl->CurStaIdx %= sRadioControl->ChlCount;

								sRadioControl->Freq = sRadioControl->Channel[sRadioControl->CurStaIdx] + RadioGetBaseFreq();
								RadioPlay();
							    #ifdef CFG_FUNC_BREAKPOINT_EN
								  BackupInfoUpdata(BACKUP_RADIO_INFO);
							    #endif
							 }
							Number_select_flag = 0;
						}
					}
                  #endif
				}
				break;
		}
	}
}

static void RadioPlayRunning(uint16_t msgId)
{
#ifdef CFG_FUNC_DISPLAY_EN
	MessageContext	msgSend;
#endif

	switch(msgId)
	{
#ifdef	CFG_FUNC_POWERKEY_EN
		case MSG_TASK_POWERDOWN:
			APP_DBG("MSG revice PowerDown, Please breakpoint\n");
			SystemPowerDown();
			break;
#endif
		case MSG_PLAY_PAUSE:
#ifdef CFG_FUNC_RECORDER_EN	
			if(GetMediaRecorderMessageHandle() !=  NULL)
			{
				EncoderServicePause();
				break;
			}
#endif			
		case MSG_RADIO_PLAY_SCAN:
			if(sRadioControl->State == RADIO_STATUS_SEEKING)
			{
				RadioSeekEnd();
			}
			else
			{
				RadioSeekStart();
			}
			break;
		case MSG_FB_START:
		case MSG_RADIO_PLAY_SCAN_DN:
			if(sRadioControl->State == RADIO_STATUS_SEEKING)
			{
				RadioSeekEnd();
			}
			else
			{
			   if(sRadioControl->Freq > sRadioControl->MinFreq)
				{
				  RadioMute(TRUE);
				  sRadioControl->Freq--;
				  RadioSetFreq(sRadioControl->Freq);
				  sRadioControl->State = RADIO_STATUS_SEEKING_DOWN;
				}
			}
			break;
		case MSG_FF_START:
		case MSG_RADIO_PLAY_SCAN_UP:
			if(sRadioControl->State == RADIO_STATUS_SEEKING)
			{
				RadioSeekEnd();
			}
			else
			{
			   if(sRadioControl->Freq < sRadioControl->MaxFreq)
				{
				   RadioMute(TRUE);
				  sRadioControl->Freq++;
				  RadioSetFreq(sRadioControl->Freq);
				  sRadioControl->State = RADIO_STATUS_SEEKING_UP;
				}
			}
			break;

#ifdef CFG_RES_IR_NUMBERKEY
		case MSG_NUM_0:
		case MSG_NUM_1:
		case MSG_NUM_2:
		case MSG_NUM_3:
		case MSG_NUM_4:
		case MSG_NUM_5:
		case MSG_NUM_6:
		case MSG_NUM_7:
		case MSG_NUM_8:
		case MSG_NUM_9:
			if(!Number_select_flag)
				Number_value = msgId - MSG_NUM_0;
			else
				Number_value = Number_value * 10 + (msgId - MSG_NUM_0);
			if(Number_value > 9999)
				Number_value = 0;
			Number_select_flag = 1;
			TimeOutSet(&Number_selectTimer, 2000);
            #ifdef CFG_FUNC_DISPLAY_EN
             msgSend.msgId = MSG_DISPLAY_SERVICE_NUMBER;
             MessageSend(GetDisplayMessageHandle(), &msgSend);
            #endif
			break;
#endif
		case MSG_PRE:
		case MSG_RADIO_PLAY_PRE:
			RadioSwitchChannel(msgId);
			break;
		case MSG_NEXT:
		case MSG_RADIO_PLAY_NEXT:
			RadioSwitchChannel(msgId);
			break;

		case MSG_DECODER_STOPPED:
#ifdef CFG_FUNC_REMIND_SOUND_EN
			{
				MessageContext		msgSend;

				msgSend.msgId = msgId;
				MessageSend(GetRemindSoundServiceMessageHandle(), &msgSend);//提示音期间转发解码器消息。
			}
#endif
			break;

#ifdef CFG_FUNC_RECORDER_EN
		case MSG_REC:
			if(ResourceValue(AppResourceCard) || ResourceValue(AppResourceUDisk))
			{
				if(RadioPlayCt->RecorderSync == TaskStateNone)
				{
					if(!MediaRecordHeapEnough())
					{
						break;
					}
					MediaRecorderServiceCreate(RadioPlayCt->msgHandle);
					RadioPlayCt->RecorderSync = TaskStateCreating;
				}
				else if(RadioPlayCt->RecorderSync == TaskStateRunning)//再按录音键 停止
				{
					MediaRecorderStop();
					MediaRecorderServiceStop();
					RadioPlayCt->RecorderSync = TaskStateStopping;
				}
			}
			else
			{//flashfs录音 待处理
				APP_DBG("Radio:error, no disk!!!\n");
			}
			break;

		case MSG_MEDIA_RECORDER_SERVICE_CREATED:
#ifdef CFG_FUNC_REMIND_SOUND_EN
			//RemindSound request
			//录音事件提示音，规避录音文件携带本提示音，使用阻塞延时
			RemindSoundServiceItemRequest(SOUND_REMIND_LUYIN, FALSE);
			osTaskDelay(350);//即“录音”提示音本身时长
#endif			
			RadioPlayCt->RecorderSync = TaskStateStarting;
			MediaRecorderServiceStart();
			break;

		case MSG_MEDIA_RECORDER_SERVICE_STARTED:
			MediaRecorderRun();
			RadioPlayCt->RecorderSync = TaskStateRunning;
			break;

		case MSG_MEDIA_RECORDER_STOPPED:
#ifdef CFG_FUNC_REMIND_SOUND_EN
			RemindSoundServiceItemRequest(SOUND_REMIND_STOPREC, FALSE);
#endif
			MediaRecorderServiceStop();
			RadioPlayCt->RecorderSync = TaskStateStopping;
			break;

		case MSG_MEDIA_RECORDER_ERROR:
			if(RadioPlayCt->RecorderSync == TaskStateRunning)
			{
				MediaRecorderStop();
				MediaRecorderServiceStop();
				RadioPlayCt->RecorderSync = TaskStateStopping;
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
bool RadioPlayCreate(MessageHandle parentMsgHandle)
{
	bool ret;
	ret = RadioPlay_Init(parentMsgHandle);
	if(ret)
	{
		xTaskCreate(RadioPlayEntrance,
					"RadioPlay",
					RADIO_PLAY_TASK_STACK_SIZE,
					NULL, RADIO_PLAY_TASK_PRIO,
					&RadioPlayCt->taskHandle);
		if(RadioPlayCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
	}
	if(!ret)
	{
		APP_DBG("Radio:app create fail!\n");
	}
	return ret;
}

bool RadioPlayStart(void)
{
	MessageContext		msgSend;
	if(RadioPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_START;
	return MessageSend(RadioPlayCt->msgHandle, &msgSend);
}

bool RadioPlayPause(void)
{
	MessageContext		msgSend;

	if(RadioPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_PAUSE;
	return MessageSend(RadioPlayCt->msgHandle, &msgSend);
}

bool RadioPlayResume(void)
{
	MessageContext		msgSend;

	if(RadioPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_RESUME;
	return MessageSend(RadioPlayCt->msgHandle, &msgSend);
}

bool RadioPlayStop(void)
{
	MessageContext		msgSend;

	if(RadioPlayCt == NULL)
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
	return MessageSend(RadioPlayCt->msgHandle, &msgSend);
}

bool RadioPlayKill(void)
{
	if(RadioPlayCt == NULL)
	{
		return FALSE;
	}

	//Kill used services
#ifdef CFG_FUNC_REMIND_SOUND_EN
	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
	DecoderServiceKill();
#endif
#ifdef CFG_FUNC_RECORDER_EN
	if(RadioPlayCt->RecorderSync != TaskStateNone)//当录音创建失败时，需要强行回收
	{
		MediaRecorderServiceKill();
		RadioPlayCt->RecorderSync = TaskStateNone;
	}
#endif
	//注意：AudioCore父任务调整到mainApp下，此处只关闭AudioCore通道，不关闭任务
	AudioCoreProcessConfig((void*)AudioNoAppProcess) ;
	AudioCoreSourceDisable(RADIO_SOURCE_NUM);

#if (RADIO_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	AudioADC_Disable(ADC1_MODULE);
	AudioADC_DeInit(ADC1_MODULE);
#else
	AudioADC_Disable(ADC0_MODULE);
	AudioADC_DeInit(ADC0_MODULE);
#endif

	AudioAnaChannelSet(ANA_INPUT_CH_NONE);
	//AudioADC_DeInit(ADC0_MODULE);

	//task
	if(RadioPlayCt->taskHandle != NULL)
	{
		vTaskDelete(RadioPlayCt->taskHandle);
		RadioPlayCt->taskHandle = NULL;
	}

	//Msgbox
	if(RadioPlayCt->msgHandle != NULL)
	{
		MessageDeregister(RadioPlayCt->msgHandle);
		RadioPlayCt->msgHandle = NULL;
	}

	//PortFree
	if(RadioPlayCt->Source1Buf_Radio != NULL)
	{
		osPortFree(RadioPlayCt->Source1Buf_Radio);
		RadioPlayCt->Source1Buf_Radio = NULL;
	}

	if(sRadioControl != NULL)
	{
		osPortFree(sRadioControl);
		sRadioControl = NULL;
	}

	if(RadioPlayCt->ADCFIFO != NULL)
	{
		osPortFree(RadioPlayCt->ADCFIFO);
		RadioPlayCt->ADCFIFO = NULL;
		RadioPlayCt->AudioCoreRadio = NULL;
	}
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(RadioPlayCt->Source2Decoder != NULL)
	{
		osPortFree(RadioPlayCt->Source2Decoder);
		RadioPlayCt->Source2Decoder = NULL;
	}
#endif

	osPortFree(RadioPlayCt);
	RadioPlayCt = NULL;

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioEffectsDeInit();
#endif

	RadioPowerDown();
#ifdef CFG_RADIO_CLK_M12
	Clock_GPIOOutDisable(GPIO_CLK_OUT_A29);
#endif

	return TRUE;
}

MessageHandle GetRadioPlayMessageHandle(void)
{
	if(RadioPlayCt == NULL)
	{
		return NULL;
	}
	return RadioPlayCt->msgHandle;
}

#endif //CFG_APP_RADIOIN_MODE_EN

