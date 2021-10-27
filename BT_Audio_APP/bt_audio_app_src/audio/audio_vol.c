/**
 **************************************************************************************
 * @file    audio_vol.c
 * @brief   audio syetem vol set here
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2016-1-7 15:42:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "type.h"
#include "app_config.h"
#include "app_message.h"
#include "dac.h"
#include "audio_adc.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "debug.h"
#include "audio_vol.h"
#include "audio_core_api.h"
#include "main_task.h"
#include "timeout.h"
#include "bt_play_mode.h"
#include "cec.h"
#include "remind_sound_service.h"
#if (BT_HFP_SUPPORT == ENABLE)
#include "bt_hf_mode.h"
#include "bt_hf_api.h"
#endif
#include "hdmi_in_api.h"

#include "ctrlvars.h"
#ifdef CFG_FUNC_DISPLAY_EN
#include "display_service.h"
#endif

#ifdef CFG_FUNC_RTC_EN
#include "rtc_ctrl.h"
#endif
#include "breakpoint.h"
#include "eq_params.h"

bool gIsVolSetEnable = FALSE;
int32_t SetChannel = 0xff;
static int32_t Cnt = -1;

uint8_t ChannelValid[ AUDIO_CORE_SOURCE_MAX_MUN + AUDIO_CORE_SINK_MAX_NUM] = {0};
static volatile bool WhetherRecMusic = 1;
extern HDMIInfo         *gHdmiCt;

TIMER MenuTimer;//菜单键 时间控制，如果超过一定时间无相关按键触发，关闭菜单功能。

//音量表中数据表示的是音频通路数字部分的Gain值
//4095表示0dB,为0时表示Mute。音量可调整增益表中只做负增益
//需要正增益设置每个source源的预增益
//两级音量之间的计算公式为 "20*log(Vol1/Vol2)"，单位dB
const uint16_t gSysVolArr[CFG_PARA_MAX_VOLUME_NUM + 1] =
{
#if CFG_PARA_MAX_VOLUME_NUM == 32
	0/*-72db*/,
	3/*-56db*/,		6/*-56db*/,		15/*-49db*/,	26/*-44db*/,	41/*-40db*/,	65/*-36db*/,	103/*-32db*/,	145/*-29db*/,
	205/*-26db*/,	258/*-24db*/,	325/*-22db*/,	410/*-20db*/,	460/*-19db*/,	516/*-18db*/,	576/*-17db*/,	649/*-16db*/,
	728/*-15db*/,	817/*-14db*/,	917/*-13db*/,	1029/*-12db*/,	1154/*-11db*/,	1295/*-10db*/,	1453/*-9db*/,	1631/*-8db*/,
	1830/*-7db*/,	2053/*-6db*/,	2303/*-5db*/,	2584/*-4db*/,	2900/*-3db*/,	3254/*-2db*/,	3651/*-1db*/,	4095/*0db*/
#endif
#if CFG_PARA_MAX_VOLUME_NUM == 16
	0/*-72db*/,  	
	35/*-56db*/,41/*-40db*/,	145/*-29db*/,	258/*-24db*/,	410/*-20db*/,	576/*-17db*/,	728/*-15db*/,	917/*-13db*/,  
	1154/*-11db*/,	1453/*-9db*/,	1830/*-7db*/,	2303/*-5db*/,	2900/*-3db*/,	3254/*-2db*/,	3651/*-1db*/,	4095/*0db*/
#endif
};

uint8_t gBtAbsVolTable[17]={
	0x00, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37, 0x3f, 0x47,
	0x4f, 0x57, 0x5f, 0x67, 0x6f, 0x77, 0x7f};

uint8_t gBtAbsVolSetTable[17]={
	0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48,
	0x50, 0x58, 0x60, 0x68, 0x70, 0x78, 0x7f};

uint8_t BtAbsVolume2VolLevel(uint8_t absValue)
{
	uint8_t i;
	for(i=0;i<=16;i++)
	{
		if(absValue == gBtAbsVolTable[i])
			return i;

		if(absValue < gBtAbsVolTable[i])
			return (i-1);
	}

	if(i>16) i=16;
	return i;
}

uint8_t BtLocalVolLevel2AbsVolme(uint8_t localValue)
{
	return gBtAbsVolSetTable[localValue];
}

//静音设置与取消
//当前case是静音所有输出通道
//应用层自行保存相关状态
//客户可以根据需要自行灵活调整
void AudioPlayerMute(void)
{
	uint16_t i;
	mainAppCt.gSysVol.MuteFlag = !mainAppCt.gSysVol.MuteFlag;

	TimeOutSet(&MenuTimer, 5000);
	// 调用驱动层静音设置接口
	//if(mainAppCt.gSysVol.MuteFlag)
	if(IsAudioPlayerMute() == TRUE)
	{
		for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM ;i++)
		{
			AudioCoreSinkMute(i, TRUE, TRUE);
		}
		APP_DBG("\nMute\n");
	}
	else
	{
		for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM ;i++)
		{
			AudioCoreSinkUnmute(i, TRUE, TRUE);
		}
		SystemVolSet();
		APP_DBG("\nUnmute\n");
	}
}


bool IsAudioPlayerMute(void)
{
	return mainAppCt.gSysVol.MuteFlag;
}

void AudioPlayerMenu(void)
{
	uint32_t ValidNum = 0;;
	uint32_t i;
	
	TimeOutSet(&MenuTimer, 5000);
	gIsVolSetEnable = TRUE;

	for(i=0; i<AUDIO_CORE_SOURCE_MAX_MUN; i++)
	{
		if(mainAppCt.AudioCore->AudioSource[i].Enable)
		{
			ChannelValid[ValidNum] =  i;
			ValidNum++;
		}
	}
	for(i=0; i<AUDIO_CORE_SINK_MAX_NUM; i++)
	{
		if(mainAppCt.AudioCore->AudioSink[i].Enable)
		{
			ChannelValid[ValidNum] = AUDIO_CORE_SOURCE_MAX_MUN + i;//VOL_SET_CHANNEL_BASE_SINK
			ValidNum++;
		}
	}

	APP_DBG("Cnt=%d\n", (int)Cnt);
	SetChannel = ChannelValid[Cnt > ValidNum ? ValidNum : Cnt];

	Cnt++;
	if(Cnt > ValidNum)
	{
		Cnt = 0;
	}
	
	//APP_DBG
//	{
//	switch(SetChannel)
//	{
//		case MIC_SOURCE_NUM:
//			APP_DBG("MIC_SOURCE_NUM vol set\n");
//		break;
//		case APP_SOURCE_NUM:
//			APP_DBG("APP_SOURCE_NUM vol set\n");
//		break;
//		case REMIND_SOURCE_NUM:
//			APP_DBG("REMIND_SOURCE_NUM vol set\n");
//		break;
//		  case PLAYBACK_SOURCE_NUM:
//			APP_DBG("PLAYBACK_SOURCE_NUM vol set\n");
//		break;
//		case AUDIO_DAC0_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN:
//		       APP_DBG("Sink0 vol set(dac0)\n");
//		break;
//
//          #ifdef CFG_FUNC_RECORDER_EN
//	      case AUDIO_RECORDER_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN:
//		       APP_DBG("Sink1 vol set(rec)\n");
//		break;
//          #endif
//
//          #if defined(CFG_RES_AUDIO_DACX_EN )
//	      case AUDIO_DACX_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN:
//		APP_DBG("Sink2 vol set(DACX)\n");
//		break;
//          #endif
//		      default:
//		         break;
//	    }
//	}
}

//检查Menu 定时器超时
void AudioPlayerMenuCheck(void)
{
	if(gIsVolSetEnable && IsTimeOut(&MenuTimer))
	{
		gIsVolSetEnable = FALSE;
		SetChannel = 0xff;
		if(Cnt > 0)
		{
			Cnt--;
		}
		else
		{
			Cnt = AUDIO_CORE_SOURCE_MAX_MUN + AUDIO_CORE_SINK_MAX_NUM;
		}
		APP_DBG("Menu Timer OUT\n");
	}
}

uint8_t AudioMusicVolGet(void)
{
	return mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM];
}

void AudioMusicVolUp(void)
{
	//if(mainAppCt.gSysVol.MuteFlag)
	if(IsAudioPlayerMute() == TRUE)
	{
		AudioPlayerMute();
	}
#ifdef CFG_APP_HDMIIN_MODE_EN
	if(GetSystemMode() == AppModeHdmiAudioPlay)
	{
		HDMISourceUnmute();
	}
#endif

	
#if (defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE))
	if(GetSystemMode() == AppModeBtHfPlay)
	{
		if(mainAppCt.HfVolume < CFG_PARA_MAX_VOLUME_NUM)
		{
			mainAppCt.HfVolume++;
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
		}
	    mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.HfVolume;
	}
	else
#endif
	{
		if(mainAppCt.MusicVolume < CFG_PARA_MAX_VOLUME_NUM)
		{
			mainAppCt.MusicVolume++;
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
		}
	    mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.MusicVolume;
	}
	
	mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM] = mainAppCt.MusicVolume;	
	#if CFG_PARAM_FIXED_REMIND_VOL
	mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = CFG_PARAM_FIXED_REMIND_VOL;
	#else
	mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = mainAppCt.MusicVolume;
	#endif
	APP_DBG("APP_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
	//APP_DBG("REMIND_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]);
	//APP_DBG("PLAYBACK_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]);
	AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
	AudioCoreSourceVolSet(REMIND_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]]);
	AudioCoreSourceVolSet(PLAYBACK_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]]);
#ifdef CFG_APP_BT_MODE_EN
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
	//add volume sync(bluetooth play mode)
	if(GetSystemMode() == AppModeBtAudioPlay)
	{
		MessageContext		msgSend;

		SetBtSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);

		msgSend.msgId		= MSG_BT_PLAY_VOLUME_SET;
		MessageSend(GetBtPlayMessageHandle(), &msgSend);
	}
#endif

#if (BT_HFP_SUPPORT == ENABLE)
	if(GetSystemMode() == AppModeBtHfPlay)
	{
		SetBtHfSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
	}
#endif
#endif

#ifdef CFG_APP_HDMIIN_MODE_EN
	if(GetSystemMode() == AppModeHdmiAudioPlay)
	{
		gHdmiCt->hdmiActiveReportVolUpDownflag = 2;
	}
#endif
}

void AudioMusicVolDown(void)
{
	//if(mainAppCt.gSysVol.MuteFlag)
	if(IsAudioPlayerMute() == TRUE)
	{
		AudioPlayerMute();
	}
	
#ifdef CFG_APP_HDMIIN_MODE_EN
	if(GetSystemMode() == AppModeHdmiAudioPlay)
	{
		HDMISourceUnmute();
	}
#endif

#if (defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE))
	if(GetSystemMode() == AppModeBtHfPlay)
	{
		if(mainAppCt.HfVolume > 0)
		{
			mainAppCt.HfVolume--;
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
		}
		mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.HfVolume;
	}
	else
#endif
	{
		if(mainAppCt.MusicVolume > 0)
		{
			mainAppCt.MusicVolume--;
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
		}
	    mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.MusicVolume;
	}
	
	mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM] = mainAppCt.MusicVolume;	
	#if CFG_PARAM_FIXED_REMIND_VOL
	mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = CFG_PARAM_FIXED_REMIND_VOL;
	#else
	mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = mainAppCt.MusicVolume;
	#endif
	APP_DBG("APP_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
//	APP_DBG("REMIND_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]);
//	APP_DBG("PLAYBACK_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]);
	AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
	AudioCoreSourceVolSet(REMIND_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]]);
	AudioCoreSourceVolSet(PLAYBACK_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]]);
#ifdef CFG_APP_BT_MODE_EN
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
	//add volume sync(bluetooth play mode)
	if(GetSystemMode() == AppModeBtAudioPlay)
	{
		MessageContext		msgSend;

		SetBtSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);

		msgSend.msgId		= MSG_BT_PLAY_VOLUME_SET;
		MessageSend(GetBtPlayMessageHandle(), &msgSend);
	}
#endif

#if (BT_HFP_SUPPORT == ENABLE)
	if(GetSystemMode() == AppModeBtHfPlay)
	{
		SetBtHfSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
	}
#endif
#endif

#ifdef CFG_APP_HDMIIN_MODE_EN
	if(GetSystemMode() == AppModeHdmiAudioPlay)
	{
		gHdmiCt->hdmiActiveReportVolUpDownflag = 2;
	}
#endif
}

void AudioMusicVolSet(uint8_t musicVol)
{
	if(IsAudioPlayerMute() == TRUE)
	{
		AudioPlayerMute();
	}
	
	if(musicVol > CFG_PARA_MAX_VOLUME_NUM)
		mainAppCt.MusicVolume = CFG_PARA_MAX_VOLUME_NUM;
	else
		mainAppCt.MusicVolume = musicVol;
	mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.MusicVolume;
	
	#if CFG_PARAM_FIXED_REMIND_VOL
	mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = CFG_PARAM_FIXED_REMIND_VOL;
	#else
	mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = mainAppCt.MusicVolume;
	#endif
	APP_DBG("APP_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);			
	AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
}

#ifdef CFG_APP_BT_MODE_EN
void AudioHfVolSet(uint8_t HfVol)
{
	if(IsAudioPlayerMute() == TRUE)
	{
		AudioPlayerMute();
	}
	
	if(HfVol > CFG_PARA_MAX_VOLUME_NUM)
		mainAppCt.HfVolume = CFG_PARA_MAX_VOLUME_NUM;
	else
		mainAppCt.HfVolume = HfVol;
	mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.HfVolume;
	
	APP_DBG("source1 vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);			
	AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
}
#endif

#if CFG_RES_MIC_SELECT
void AudioMicVolUp(void)
{
	if(IsAudioPlayerMute() == TRUE)
	{
		AudioPlayerMute();
	}

	if(mainAppCt.MicVolume < CFG_PARA_MAX_VOLUME_NUM)
	{
		mainAppCt.MicVolume++;
		#ifdef CFG_FUNC_BREAKPOINT_EN
		BackupInfoUpdata(BACKUP_SYS_INFO);
		#endif
	}
    mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = mainAppCt.MicVolume;
	APP_DBG("MIC_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
	AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
}

void AudioMicVolDown(void)
{
	if(IsAudioPlayerMute() == TRUE)
	{
		AudioPlayerMute();
	}

	if(mainAppCt.MicVolume > 0)
	{
		mainAppCt.MicVolume--;
		#ifdef CFG_FUNC_BREAKPOINT_EN
		BackupInfoUpdata(BACKUP_SYS_INFO);
		#endif
	}
    mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = mainAppCt.MicVolume;
	APP_DBG("MIC_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
	AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
}

void AudioMicVolSet(uint8_t micVol)
{
	if(micVol > CFG_PARA_MAX_VOLUME_NUM)
		mainAppCt.MicVolume = CFG_PARA_MAX_VOLUME_NUM;
	else
		mainAppCt.MicVolume = micVol;

	mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = mainAppCt.MicVolume;
	AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
	APP_DBG("MicVolume = %d\n",mainAppCt.MicVolume);
}
#endif

void SystemVolUp(void)
{
	//if(mainAppCt.gSysVol.MuteFlag)
	if(IsAudioPlayerMute() == TRUE)
	{
		AudioPlayerMute();
	}

	gIsVolSetEnable = TRUE;

	if(gIsVolSetEnable == TRUE)
	{
		APP_DBG("vol up \n");
		TimeOutSet(&MenuTimer, 5000);
		switch(SetChannel)
		{

			case AUDIO_DAC0_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN://sink0
				if(mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM] < CFG_PARA_MAX_VOLUME_NUM)
				{
					mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]++;
				}

				APP_DBG("dac 0sink0 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]);
				AudioCoreSinkVolSet(AUDIO_DAC0_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]]);
				break;


#ifdef CFG_FUNC_RECORDER_EN
			case AUDIO_RECORDER_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN://sink1
				if(mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM] < CFG_PARA_MAX_VOLUME_NUM)
				{
					mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]++;
				}

				APP_DBG("rec sink1 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]);
				AudioCoreSinkVolSet(AUDIO_RECORDER_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]]);
				break;
#endif

#if defined(CFG_RES_AUDIO_DACX_EN )
			case AUDIO_DACX_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN:
				if(mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM] < CFG_PARA_MAX_VOLUME_NUM)
				{
					mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]++;
				}

				APP_DBG("dacx sink2 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]);
				AudioCoreSinkVolSet(AUDIO_DACX_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]]);
				break;
#endif
			case MIC_SOURCE_NUM://source0
				if(mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] < CFG_PARA_MAX_VOLUME_NUM)
				{
					mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]++;
				}

				APP_DBG("MIC_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
				AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
				break;
			case APP_SOURCE_NUM://source1
				if(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] < CFG_PARA_MAX_VOLUME_NUM)
				{
					mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]++;
				}

				APP_DBG("APP_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
				AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
#ifdef CFG_APP_BT_MODE_EN
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
				//add volume sync(bluetooth play mode)
				if(GetSystemMode() == AppModeBtAudioPlay)
				{
					MessageContext		msgSend;

					SetBtSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);

					msgSend.msgId		= MSG_BT_PLAY_VOLUME_SET;
					MessageSend(GetBtPlayMessageHandle(), &msgSend);
				}
#endif

#if (BT_HFP_SUPPORT == ENABLE)
				if(GetSystemMode() == AppModeBtHfPlay)
				{
					SetBtHfSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
				}
#endif
#endif
				break;
			case REMIND_SOURCE_NUM://source2
				if(mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] < CFG_PARA_MAX_VOLUME_NUM)
				{
					mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]++;
				}

				//APP_DBG("REMIND_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]);
				AudioCoreSourceVolSet(REMIND_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]]);
				break;
			case PLAYBACK_SOURCE_NUM://source3
				if(mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM] < CFG_PARA_MAX_VOLUME_NUM)
				{
					mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]++;
				}
				
				//APP_DBG("PLAYBACK_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]);
				AudioCoreSourceVolSet(PLAYBACK_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]]);
				break;
/*
			case VOL_SET_CHANNEL_SOURCE4://source3
				if(mainAppCt.gSysVol.AudioSourceVol[4] < CFG_PARA_MAX_VOLUME_NUM)
				{
					mainAppCt.gSysVol.AudioSourceVol[4]++;
				}

				APP_DBG("source4 vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[4]);
				AudioCoreSourceVolSet(4, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[4]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[4]]);
				break;
*/
			default:
			break;
		}
	}
	else
	{
		APP_DBG("vol set disable\n");
	}
}

void SystemVolDown(void)
{
	//if(mainAppCt.gSysVol.MuteFlag)
	if(IsAudioPlayerMute() == TRUE)
	{
		AudioPlayerMute();
	}
	gIsVolSetEnable = TRUE;

	if(gIsVolSetEnable == TRUE)
	{
		APP_DBG("vol down \n");
		TimeOutSet(&MenuTimer, 5000);
		switch(SetChannel)
		{

			case AUDIO_DAC0_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN://sink0
				if(mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM] > 0)
				{
					mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]--;
				}

				APP_DBG("dac0 sink vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]);
				AudioCoreSinkVolSet(AUDIO_DAC0_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]]);
				break;


#ifdef CFG_FUNC_RECORDER_EN
			case AUDIO_RECORDER_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN://sink1
				if(mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM] > 0)
				{
					mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]--;
				}

				APP_DBG("rec sink vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]);
				AudioCoreSinkVolSet(AUDIO_RECORDER_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]]);
				break;
#endif

#if defined(CFG_RES_AUDIO_DACX_EN )
			case AUDIO_DACX_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN:
				if(mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM] > 0)
				{
					mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]--;
				}
				
				APP_DBG("dacx sink vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]);
				AudioCoreSinkVolSet(AUDIO_DACX_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]]);
				break;
#endif

			case MIC_SOURCE_NUM://source0
				if(mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] > 0)
				{
					mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]--;
				}

				APP_DBG("MIC_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
				AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
				break;
			case APP_SOURCE_NUM://source1
				if(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] > 0)
				{
					mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]--;
				}

				APP_DBG("APP_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
				AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
#ifdef CFG_APP_BT_MODE_EN
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
				//add volume sync(bluetooth play mode)
				if(GetSystemMode() == AppModeBtAudioPlay)
				{
					MessageContext		msgSend;

					SetBtSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);

					msgSend.msgId		= MSG_BT_PLAY_VOLUME_SET;
					MessageSend(GetBtPlayMessageHandle(), &msgSend);
				}
#endif

#if (BT_HFP_SUPPORT == ENABLE)
				if(GetSystemMode() == AppModeBtHfPlay)
				{
					SetBtHfSyncVolume(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
				}
#endif
#endif
				break;
			case REMIND_SOURCE_NUM://source2
				if(mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] > 0)
				{
					mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]--;
				}

				//APP_DBG("REMIND_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]);
				AudioCoreSourceVolSet(REMIND_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]]);
				break;
			case PLAYBACK_SOURCE_NUM://source3
				if(mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM] > 0)
				{
					mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]--;
				}

				//APP_DBG("PLAYBACK_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]);
				AudioCoreSourceVolSet(PLAYBACK_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]]);
				break;

	
			default:
			break;
		}
	}
	else
	{
		APP_DBG("vol set disable\n");
	}
}

void SystemVolSet(void)
{
	uint32_t i;
	
	for(i=0; i<AUDIO_CORE_SOURCE_MAX_MUN; i++)
	{
		AudioCoreSourceVolSet(i, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[i]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[i]]);
	}
	for(i=0; i<AUDIO_CORE_SINK_MAX_NUM; i++)
	{
		#ifdef CFG_FUNC_RECORDER_EN
		if(i == AUDIO_RECORDER_SINK_NUM)
		{
			AudioCoreSinkVolSet(AUDIO_RECORDER_SINK_NUM, CFG_PARA_REC_GAIN, CFG_PARA_REC_GAIN);
		}
		else
		#endif
		{
			AudioCoreSinkVolSet(i, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[i]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[i]]);
		}
	}
}

void SystemVolSetChannel(int8_t SetChannel, uint8_t volume)
{
	switch(SetChannel)
	{

		case AUDIO_DAC0_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN:
			//if(mainAppCt.gSysVol.MuteFlag)
			if(IsAudioPlayerMute() == TRUE)
			{
				AudioPlayerMute();
			}
			mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM] = volume;
			APP_DBG("dac0 sink0 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]);
			AudioCoreSinkVolSet(AUDIO_DAC0_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]]);
			break;

#ifdef CFG_FUNC_RECORDER_EN
		case AUDIO_RECORDER_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN:
			//if(mainAppCt.gSysVol.MuteFlag)
			if(IsAudioPlayerMute() == TRUE)
			{
				AudioPlayerMute();
			}
			mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM] = volume;
			APP_DBG("rec sink1 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]);
			AudioCoreSinkVolSet(AUDIO_RECORDER_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]]);
			break;
#endif
			
#if defined(CFG_RES_AUDIO_DACX_EN )
		case AUDIO_DACX_SINK_NUM + AUDIO_CORE_SOURCE_MAX_MUN:
			//if(mainAppCt.gSysVol.MuteFlag)
			if(IsAudioPlayerMute() == TRUE)
			{
				AudioPlayerMute();
			}
			mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM] = volume;
			APP_DBG("dacx sink2 vol = %d\n", mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]);
			AudioCoreSinkVolSet(AUDIO_DACX_SINK_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DACX_SINK_NUM]]);
			break;
#endif
			
		case MIC_SOURCE_NUM:
			//if(mainAppCt.gSysVol.MuteFlag)
			if(IsAudioPlayerMute() == TRUE)
			{
				AudioPlayerMute();
			}
			mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = volume;
			APP_DBG("MIC_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
			AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
			break;
		
		case APP_SOURCE_NUM:
			//if(mainAppCt.gSysVol.MuteFlag)
			if(IsAudioPlayerMute() == TRUE)
			{
				AudioPlayerMute();
			}
			mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = volume;
			APP_DBG("APP_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
			AudioCoreSourceVolSet(APP_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
			break;
			
		case REMIND_SOURCE_NUM:
			//if(mainAppCt.gSysVol.MuteFlag)
			if(IsAudioPlayerMute() == TRUE)
			{
				AudioPlayerMute();
			}
			mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = volume;
			//APP_DBG("REMIND_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]);
			AudioCoreSourceVolSet(REMIND_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]]);
			break;
			
		case PLAYBACK_SOURCE_NUM:
			//if(mainAppCt.gSysVol.MuteFlag)
			if(IsAudioPlayerMute() == TRUE)
			{
				AudioPlayerMute();
			}
			mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM] = volume;
			//APP_DBG("PLAYBACK_SOURCE_NUM vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]);
			AudioCoreSourceVolSet(PLAYBACK_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]]);
			break;

	
		default:
			break;
	}
}

#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
/*
****************************************************************
* EQ Mode调节函数
*
*
****************************************************************
*/
void EqModeSet(uint8_t EqMode)
{
    switch(EqMode)
	{
		case EQ_MODE_FLAT:
			LoadEqMode(&Flat[0]);
			break;
		case EQ_MODE_CLASSIC:
			LoadEqMode(&Classical[0]);
			break;
		case EQ_MODE_POP:
			LoadEqMode(&Pop[0]);
			break;	
		case EQ_MODE_ROCK:
			LoadEqMode(&Rock[0]);
			break;
		case EQ_MODE_JAZZ:
			LoadEqMode(&Jazz[0]);
			break;
		case EQ_MODE_VOCAL_BOOST:
			LoadEqMode(&Vocal_Booster[0]);
			break;			
	}
}
#endif
/*
****************************************************************
* 用户相关音效调节参数同步函数
*说明:
*    调用AudioEffectModeSel()设置音效模式之后，需要再调用下此函数，保证用户设置音效参数同步
****************************************************************
*/
void AudioEffectParamSync(void)
{
	if(gCtrlVars.IsEffectChangedByPcTool)
	{
		gCtrlVars.IsEffectChangedByPcTool = 0;
	}
	else
	{
#ifdef CFG_ADC_LEVEL_KEY_EN	
		AdcLevelParamSync();
#endif
#ifdef CFG_FUNC_MIC_TREB_BASS_EN
		MicBassTrebAjust(mainAppCt.MicBassStep, mainAppCt.MicTrebStep);	
#endif
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
		MusicBassTrebAjust(mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);	
#endif
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN	
		EqModeSet(mainAppCt.EqMode);	
#endif
#ifdef CFG_FUNC_MIC_KARAOKE_EN	
		ReverbStepSet(mainAppCt.ReverbStep);
        AudioMicVolSet(mainAppCt.MicVolume);
#endif
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		{
			extern bool IsEffectChange;
			IsEffectChange = 0;//EQ模式切换中或EQ模式参数更新，不需要再做音效及内存初始化处理
		}
#endif
	}
}

#ifdef CFG_ADC_LEVEL_KEY_EN	
/*
****************************************************************
* 电位器参数同步处理
*说明:
*    AdcLevelCh和app_config.h中选择的GPIO有如下对应关系:

       #define  ADCLEVL_CHANNEL_MAP            (ADC_GPIOA20|ADC_GPIOA21|ADC_GPIOA22)
       
       ADC_GPIOA20对应AdcLevelCh为1；
       ADC_GPIOA21对应AdcLevelCh为2；
       ADC_GPIOA22对应AdcLevelCh为3；
****************************************************************
*/
void AdcLevelParamSync(void)
{
	mainAppCt.MicVolume = AdcLevelKeyOneChanScan(0);
	
	mainAppCt.ReverbStep = AdcLevelKeyOneChanScan(1);
	
	mainAppCt.MicTrebStep = AdcLevelKeyOneChanScan(2)/2;	
    mainAppCt.MicBassStep = 15-mainAppCt.MicTrebStep;
	
	mainAppCt.MicVolumeBak = mainAppCt.MicVolume;
	mainAppCt.ReverbStepBak= mainAppCt.ReverbStep;
	mainAppCt.MicTrebStepBak = mainAppCt.MicTrebStep;
	mainAppCt.MicBassStepBak = mainAppCt.MicBassStep;
	DBG("MicVolumeBak = %d\n", mainAppCt.MicVolumeBak);
	DBG("ReverbStepBak = %d\n", mainAppCt.ReverbStepBak);
	DBG("MicTrebStepBak = %d\n", mainAppCt.MicTrebStepBak);
	DBG("MicBassStepBak = %d\n", mainAppCt.MicBassStepBak);
}
/*
****************************************************************
* 电位器消息接收处理函数
*说明:
*    AdcLevelCh和app_config.h中选择的GPIO有如下对应关系:

       #define  ADCLEVL_CHANNEL_MAP            (ADC_GPIOA20|ADC_GPIOA21|ADC_GPIOA22)
       
       ADC_GPIOA20对应AdcLevelCh为1；
       ADC_GPIOA21对应AdcLevelCh为2；
       ADC_GPIOA22对应AdcLevelCh为3；
****************************************************************
*/
void AdcLevelMsgProcess(uint16_t Msg)//Sliding resistance
{
	uint16_t AdcLevelCh, AdcValue;

    if( (Msg > MSG_ADC_LEVEL_MSG_START)&&(Msg < MSG_ADC_LEVEL_MSG_END) )
	{				
		AdcLevelCh      =   Msg&0xff00;
		AdcLevelCh      -=  MSG_ADC_LEVEL_MSG_START;
		AdcLevelCh      >>= 8;

		AdcValue      =   Msg &0x00ff;//(0~~16) = (MSG_ADC_LEVEL_MSG_0~~MSG_ADC_LEVEL_MSG_16)

        APP_DBG("AdcLevelCh = %d\n", AdcLevelCh);
		APP_DBG("AdcValue = %d\n",AdcValue);
		switch(AdcLevelCh)
		{

			case 1://ADC LEVEL Channel 1
			    #if CFG_RES_MIC_SELECT
				mainAppCt.MicVolumeBak = AdcValue;
				APP_DBG("MicVolumeBak = %d\n", mainAppCt.MicVolumeBak);
			    //mainAppCt.gSysVol.AudioSourceVol[0] = mainAppCt.MicVolume = AdcValue;
				//APP_DBG("source0 vol = %d\n", mainAppCt.gSysVol.AudioSourceVol[0]);
				//AudioCoreSourceVolSet(0, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[0]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[0]]);
				#endif
				break;
				
			case 2://ADC LEVEL Channel 2
				#ifdef CFG_FUNC_MIC_KARAOKE_EN	
			    mainAppCt.ReverbStepBak = AdcValue;
				//ReverbStepSet(mainAppCt.ReverbStep);
				APP_DBG("ReverbStepBak = %d\n", mainAppCt.ReverbStepBak);
				#endif
				break;
				
			case 3://ADC LEVEL Channel 3
				mainAppCt.MicBassStepBak = 15 - AdcValue/2;
				mainAppCt.MicTrebStepBak = AdcValue/2;
				//MicBassTrebAjust(mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
				break;

			case 4://ADC LEVEL Channel 4
				
				break;
			case 5://ADC LEVEL Channel 5
				
				break;
			case 6://ADC LEVEL Channel 6
				
				break;
			case 7://ADC LEVEL Channel 7
				
				break;
			case 8://ADC LEVEL Channel 8
				
				break;
			case 9://ADC LEVEL Channel 9
				
				break;
			case 10://ADC LEVEL Channel 10
				
				break;
			case 11://ADC LEVEL Channel 11
				
				break;
		}
	}
}
#endif


//WhetherRecMusic: TRUE:rec music 	FALSE:not rec music
//if_para_use:0:not use para 1:not rec music 2:rec music
void SetRecMusic(uint8_t if_para_use)
{	
	if(if_para_use)
	{
		WhetherRecMusic=if_para_use-1;
	}
	else
	{
		if(WhetherRecMusic)
		{		
			APP_DBG("not rec music\n");
			WhetherRecMusic=FALSE;		
		}
		else
		{
			APP_DBG("rec music\n");
			WhetherRecMusic=TRUE;
		}
	}
}

//1:rec music 	0:not rec music
bool GetWhetherRecMusic(void)
{
	return WhetherRecMusic;
}

#ifdef  CFG_APP_HDMIIN_MODE_EN
void HDMISourceMute(void)
{
	mainAppCt.hdmiSourceMuteFlg = 1;
	AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
	//APP_DBG("hdmi mute\n");
}

void HDMISourceUnmute(void)
{
	mainAppCt.hdmiSourceMuteFlg = 0;
	AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
	//APP_DBG("hdmi unmute\n");
}

bool IsHDMISourceMute(void)
{
	return mainAppCt.hdmiSourceMuteFlg;
}
#endif

