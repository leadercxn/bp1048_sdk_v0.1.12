/**
 *****************************************************************************
 * @file     otg_device_audio.c
 * @author   Owen
 * @version  V1.0.0
 * @date     7-September-2015
 * @brief    device audio module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#include <string.h>
#include <nds32_intrinsic.h>
#include "type.h"
#include "otg_device_hcd.h"
#include "debug.h"
#include "otg_device_standard_request.h"
#include "audio_adc.h"
#include "dac.h"
#include "clk.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "mcu_circular_buf.h"
#include "otg_device_audio.h"
#include "usb_audio_api.h"
#include "sra.h"
#include "usb_audio_api.h"

#ifdef CFG_APP_USB_AUDIO_MODE_EN
extern uint8_t Setup[];
extern uint8_t Request[];
extern void OTG_DeviceSendResp(uint16_t Resp, uint8_t n);

UsbAudio UsbAudioSpeaker;
UsbAudio UsbAudioMic;
int16_t iso_buf[48*2];
SRAContext UsbSraObj;//软件微调pcm采样点数，结构体。
MCU_CIRCULAR_CONTEXT usb_speak_buff;
int16_t usb_in_buff[512*2];///==dac0 out buff
int16_t pcm_in_buf[SRA_BLOCK*4];///Temp buff
int16_t pcm_out_buf[SRA_BLOCK*4*2];///Temp buff
void UsbSraInit(void);
void UsbSraDataProcess(MCU_CIRCULAR_CONTEXT *InBuf, MCU_CIRCULAR_CONTEXT *OutBuf);
/////////////////////////////////////////
/**
 * @brief  USB声卡模式下，发送反向控制命令
 * @param  Cmd 反向控制命令
 * @return 1-成功，0-失败
 */
#define AUDIO_STOP        BIT(7) 
#define AUDIO_PP          BIT(6) 

#define AUDIO_MUTE        BIT(4)

#define AUDIO_NEXT        BIT(2) 
#define AUDIO_PREV        BIT(3) 

#define AUDIO_VOL_UP      BIT(0) 
#define AUDIO_VOL_DN      BIT(1)

/////////////////////////
void PCAudioStop(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_STOP);
}
void PCAudioPP(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_PP);
}
void PCAudioNext(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_NEXT);
}
void PCAudioPrev(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_PREV);
}

void PCAudioVolUp(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_VOL_UP);
}

void PCAudioVolDn(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_VOL_DN);
}

bool OTG_DeviceAudioSendPcCmd(uint8_t Cmd)
{
	OTG_DeviceInterruptSend(0x01,&Cmd, 1,1000);
	Cmd = 0;
	OTG_DeviceInterruptSend(0x01,&Cmd, 1,1000);
	return TRUE;
}


//转采样直接在中断中处理，转采样时间大约是180us。
//注意一下需要4字节对齐
void OnDeviceAudioRcvIsoPacket(void)
{
#ifdef CFG_RES_AUDIO_USB_IN_EN
	uint32_t Len;
	uint32_t s;
	int32_t left_pregain = UsbAudioSpeaker.LeftVol;
	int32_t rigth_pregain = UsbAudioSpeaker.RightVol;
	uint32_t channel = UsbAudioSpeaker.Channel;
	OTG_DeviceISOReceive(DEVICE_ISO_OUT_EP, (uint8_t*)iso_buf, 192, &Len);
	uint32_t sample = Len/(channel*2);
	UsbAudioSpeaker.FramCount++;
	if(UsbAudioSpeaker.Mute)
	{
		left_pregain = 0;
		rigth_pregain = 0;
	}

#ifdef CFG_RES_AUDIO_USB_VOL_SET_EN
	for(s = 0; s<sample; s++)
	{
		if(channel == 2)
		{
			iso_buf[2 * s + 0] = __nds32__clips((((int32_t)iso_buf[2 * s + 0] * left_pregain) >> 12), 16-1);
			iso_buf[2 * s + 1] = __nds32__clips((((int32_t)iso_buf[2 * s + 1] * rigth_pregain) >> 12), 16-1);
		}
		else
		{
			iso_buf[s] = __nds32__clips((((int32_t)iso_buf[s] * left_pregain) >> 12), 16-1);
		}
	}
#endif

   #ifdef CFG_RES_AUDIO_USB_SRC_EN
	if(UsbAudioSpeaker.AudioSampleRate != CFG_PARA_SAMPLE_RATE)
	{
		int32_t SRCDoneLen;
		SRCDoneLen = resampler_apply(UsbAudioSpeaker.Resampler, iso_buf, iso_buf, sample);

		MCUCircular_PutData(&usb_speak_buff, (uint8_t*)iso_buf, SRCDoneLen * 2 * channel);
		UsbSraDataProcess(&usb_speak_buff,&UsbAudioSpeaker.CircularBuf);
	}
	else
	#endif// end of  #ifdef CFG_RES_AUDIO_USB_SRC_EN
	{
		MCUCircular_PutData(&usb_speak_buff, (uint8_t*)iso_buf, Len);
		UsbSraDataProcess(&usb_speak_buff,&UsbAudioSpeaker.CircularBuf);
	}
#endif
}




void OnDeviceAudioSendIsoPacket(void)
{
#ifdef CFG_RES_AUDIO_USB_OUT_EN
	uint32_t s;
	int32_t left_pregain = UsbAudioMic.LeftVol;
	int32_t rigth_pregain = UsbAudioMic.RightVol;
	uint32_t channel = UsbAudioMic.Channel;
	uint32_t RealLen = 0;
	uint32_t delay_ms = 0;
	UsbAudioMic.FramCount++;
	RealLen = 44;
	if(UsbAudioMic.AudioSampleRate == 44100)
	{
		RealLen = 44;
		if((UsbAudioMic.FramCount%10) == 0)
		{
			RealLen = 45;
		}
	}
	else if(UsbAudioMic.AudioSampleRate == 48000)
	{
		RealLen = 48;
	}
	else
	{
		//notsuppt
	}
	delay_ms = CFG_PARA_SAMPLE_RATE/44;
	if(UsbAudioMic.FramCount < (delay_ms+20))
	{
		memset(iso_buf,0,sizeof(iso_buf));
		OTG_DeviceISOSend(DEVICE_ISO_IN_EP,(uint8_t*)iso_buf,RealLen*2*channel);
		return;
	}
	if(MCUCircular_GetDataLen(&UsbAudioMic.CircularBuf) < RealLen*4)
	{
		//printf("*\n");
	}
	MCUCircular_GetData(&UsbAudioMic.CircularBuf, iso_buf,RealLen*4);
	if(UsbAudioMic.Mute)
	{
		left_pregain  = 0;
		rigth_pregain = 0;
	}
	if(RealLen == 0)
	{
		printf("error\n");
	}
#ifdef CFG_RES_AUDIO_USB_VOL_SET_EN
	for(s = 0; s<RealLen; s++)
	{
		if(channel == 2)
		{
			iso_buf[2 * s + 0] = __nds32__clips((((int32_t)iso_buf[2 * s + 0] * left_pregain) >> 12), 16-1);
			iso_buf[2 * s + 1] = __nds32__clips((((int32_t)iso_buf[2 * s + 1] * rigth_pregain) >> 12), 16-1);
		}
		else
		{
			iso_buf[s] = __nds32__clips((((int32_t)iso_buf[s] * left_pregain) >> 12), 16-1);
		}
	}
#endif//end of #ifdef CFG_RES_AUDIO_USB_VOL_SET_EN

#endif//end of #ifdef CFG_RES_AUDIO_USB_OUT_EN

	OTG_DeviceISOSend(DEVICE_ISO_IN_EP,(uint8_t*)iso_buf,RealLen*2*channel);
}

void OTG_DeviceAudioInit()
{

}


void UsbAudioMicSampleRateChange(uint32_t SampleRate);
void UsbAudioSpeakerSampleRateChange(uint32_t SampleRate);

void OTG_DeviceAudioRequest(void)
{
	//AUDIO控制接口组件ID号定义（必须与device_stor_audio_request.c中的定义保持一致！）
	#define AUDIO_SPEAKER_IT_ID		1
	#define AUDIO_SPEAKER_FU_ID		2	//控制MUTE、VOLUME
	#define AUDIO_SPEAKER_OT_ID		3
	#define AUDIO_MIC_IT_ID			4
	#define AUDIO_MIC_FU_ID			5
	#define AUDIO_MIC_SL_ID			6
	#define AUDIO_MIC_OT_ID			7
	
	#define AudioCmd	((Setup[0] << 8) | Setup[1])
	#define Channel		Setup[2]
	#define Control		Setup[3]
	#define Entity		Setup[5]
	
	#define SET_CUR		0x2101
	#define SET_IDLE	0x210A
	#define GET_CUR		0xA181
	#define GET_MIN		0xA182
	#define GET_MAX		0xA183
	#define GET_RES		0xA184
	
	#define SET_CUR_EP	0x2201
	#define GET_CUR_EP	0xA281
	
	//AUDIO类请求处理
	if(AudioCmd == SET_CUR_EP)
	{
		if(Setup[4] == 0x84)
		{
			UsbAudioMic.AudioSampleRate = Request[1]*256 + Request[0];
			UsbAudioMicSampleRateChange(UsbAudioMic.AudioSampleRate);
		}
		else
		{
			UsbAudioSpeaker.AudioSampleRate = Request[1]*256 + Request[0];
			UsbAudioSpeakerSampleRateChange(UsbAudioSpeaker.AudioSampleRate);
		}
		return;
	}
	if(AudioCmd == GET_CUR_EP)
	{
		uint32_t Temp = 0;
		if(Setup[4] == 0x84)
		{
			Temp = UsbAudioMic.AudioSampleRate;
		}
		else
		{
			Temp = UsbAudioSpeaker.AudioSampleRate;
		}
		Setup[0] = (Temp>>0 ) & 0x000000FF;
		Setup[1] = (Temp>>8 ) & 0x000000FF;
		Setup[2] = (Temp>>16) & 0x000000FF;
		OTG_DeviceControlSend(Setup,3,3);
		return;
	}

	if((Entity == AUDIO_SPEAKER_FU_ID) && (Control == 0x01))
	{
		//Speaker mute的操作
		if(AudioCmd == GET_CUR)
		{
			Setup[0] = UsbAudioSpeaker.Mute;
			OTG_DeviceControlSend(Setup,1,3);
		}
		else if(AudioCmd == SET_CUR)
		{
			//OTG_DBG("Set speaker mute: %d\n", Request[0]);
			UsbAudioSpeaker.Mute=Request[0];
		}
		else
		{
			//OTG_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if((Entity == AUDIO_SPEAKER_FU_ID) && (Control == 0x02))
	{
		//Speaker volume的操作
		if(AudioCmd == GET_MIN)
		{
			//OTG_DBG("Get speaker min volume\n");
			OTG_DeviceSendResp(0x0000, 2);
		}
		else if(AudioCmd == GET_MAX)
		{
			//OTG_DBG("Get speaker max volume\n");
			OTG_DeviceSendResp(AUDIO_MAX_VOLUME, 2);
		}
		else if(AudioCmd == GET_RES)
		{
			//OTG_DBG("Get speaker res volume\n");
			OTG_DeviceSendResp(0x0001, 2);
		}
		else if(AudioCmd == GET_CUR)
		{
			uint32_t Vol = 0;
			if(Channel == 0x01)
			{
				Vol = UsbAudioSpeaker.LeftVol;//UsbAudioSpeaker.FuncLeftVolGet();
			}
			else
			{
				Vol = UsbAudioSpeaker.RightVol;//UsbAudioSpeaker.FuncRightVolGet();
			}
			OTG_DeviceSendResp(Vol, 2);
		}
		else if(AudioCmd == SET_CUR)
		{
			uint32_t Temp = 0;
			Temp = Request[1]* 256 + Request[0];
			if(Setup[2] == 0x01)
			{
				UsbAudioSpeaker.LeftVol = Temp;
			}
			else
			{
				UsbAudioSpeaker.RightVol = Temp;
			}
		}
		else
		{
			//OTG_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if((Entity == AUDIO_MIC_FU_ID) && (Control == 0x01))
	{
		//Mic mute的操作
		if(AudioCmd == GET_CUR)
		{
			OTG_DeviceSendResp(UsbAudioMic.Mute, 1);
		}
		else if(AudioCmd == SET_CUR)
		{
			UsbAudioMic.Mute = Request[0];
		}
		else
		{
			//OTG_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if((Entity == AUDIO_MIC_FU_ID) && (Control == 0x02))
	{
		//Mic volume的操作
		if(AudioCmd == GET_MIN)
		{
			//OTG_DBG("Get mic min volume\n");
			OTG_DeviceSendResp(0x0000, 2);
		}
		else if(AudioCmd == GET_MAX)
		{
			OTG_DeviceSendResp(AUDIO_MAX_VOLUME, 2);	//此处乘以4的原因请看本文件开头的注释说明
		}
		else if(AudioCmd == GET_RES)
		{
			//OTG_DBG("Get mic res volume\n");
			OTG_DeviceSendResp(0x0001, 2);
		}
		else if(AudioCmd == GET_CUR)
		{
			uint32_t Vol = 0;
			if(Channel == 0x01)
			{
				Vol = UsbAudioMic.LeftVol;
			}
			else
			{
				Vol = UsbAudioMic.RightVol;
			}
			OTG_DeviceSendResp(Vol, 2);
		}
		else if(AudioCmd == SET_CUR)
		{
			uint32_t Vol = Request[1] * 256 + Request[0];
			if(Setup[2] == 0x01)
			{
				UsbAudioMic.LeftVol = Vol;
			}
			else
			{
				UsbAudioMic.RightVol = Vol;
			}
		}
		else
		{
			//OTG_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if(Entity == AUDIO_MIC_SL_ID)
	{
		//Selector的操作
		if(AudioCmd == GET_CUR)
		{
			//OTG_DBG("Get selector: 1\n");
			OTG_DeviceSendResp(0x01, 1);
		}
		else
		{
			//OTG_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if(AudioCmd == SET_IDLE)
	{
		//OTG_DBG("Set idle\n");
	}	
	else
	{
		//其他AUDIO类的输入请求
		OTG_DeviceSendResp(0x0000, 1);
	}
}
/*
****************************************************************
* usb软件微调处理初始化函数
*
*
****************************************************************
*/
void UsbSraInit(void)
{
	sra_init(&UsbSraObj,2);
	MCUCircular_Config(&usb_speak_buff,&usb_in_buff, sizeof(usb_in_buff));
}

/*
****************************************************************
* usb软件微调处理数据输出函数
*
*
****************************************************************
*/
void UsbSraDataProcess(MCU_CIRCULAR_CONTEXT *InBuf, MCU_CIRCULAR_CONTEXT *OutBuf)
{
    #define WATER_LEVEL_COUNT  8
    #define MAX_REMAIN_SPACE_SAMPLE   128*4
    #define MIN_REMAIN_SPACE_SAMPLE   128*2

	static uint16_t  AmountWaterLevel = 0;
	static uint16_t  WaterLevelCount  = 0;
	static int8_t    AdjustVal        = 0;

	uint16_t len,remain_len;

	int8_t ModifyVal;

	len = MCUCircular_GetDataLen(InBuf);

	if(len < SRA_BLOCK*4 ) return;

	MCUCircular_GetData(InBuf, (uint8_t *)pcm_in_buf, SRA_BLOCK*4);

	if(sra_apply(&UsbSraObj, (int16_t *)pcm_in_buf, (int16_t *)pcm_out_buf, AdjustVal) == SRA_ERROR_OK)
	{
		len = (SRA_BLOCK + AdjustVal);
	}
	else//错误处理，存一帧原始数据到dac fifo，理论上不会
	{
		len = SRA_BLOCK;
	}

	remain_len = MCUCircular_GetSpaceLen(InBuf)/4;

	AmountWaterLevel += remain_len;

	if(++WaterLevelCount >= WATER_LEVEL_COUNT)
	{
		AmountWaterLevel /= WATER_LEVEL_COUNT;
		ModifyVal = 0;
		if(AmountWaterLevel > MAX_REMAIN_SPACE_SAMPLE)
		{
			ModifyVal = 1;
			DBG("+S\n");
		}
		else if(AmountWaterLevel < MIN_REMAIN_SPACE_SAMPLE)
		{
			ModifyVal = -1;
			DBG("-S\n");
		}
		AdjustVal = ModifyVal;
		WaterLevelCount = 0;
		AmountWaterLevel = 0;
	}
	MCUCircular_PutData(OutBuf, (uint8_t *)pcm_out_buf, len*4);
}

#endif
