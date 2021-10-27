
#include "type.h"
#include "app_config.h"
#include "timer.h"
#include "display.h"
#include "seg_panel.h"
//#include "seg_lcd_disp.h"
#include "seg_led_disp.h"
//#include "sled_disp.h"
#include "breakpoint.h"
#include "power_monitor.h"
#include "debug.h"
#include "app_message.h"
#include "media_play_api.h"
#include "main_task.h"
#include "audio_core_api.h"
#include "ctrlvars.h"
#include "radio_api.h"
#ifdef CFG_APP_RTC_MODE_EN
#include "rtc_mode.h"
#endif
#ifdef CFG_FUNC_DISPLAY_EN

#define MAX_COM_NUM 5
extern bool gLedDispRefreshFlag;
void ClearSceen(void);

//==============================================================================
// Global variants.
#ifdef DISP_DEV_7_LED
DISP_TYPE	gDispBuff[MAX_COM_NUM] = {0};
uint8_t     gDispVo = 0;;
//uint8_t	gDispPrevRep;
//uint8_t 	gDispPrevEQ;
//uint8_t 	gDispPrevVol;
//uint8_t 	gDispPrevPlayState;
#endif

TIMER		DispTmr;
TIMER		DispSetTmr;

UN_BLINK gBlink;

uint16_t DispHoldTime;
uint16_t Temp;

#ifdef FUNC_NUMBER_KEY_EN
TIMER NumKeyErrDispTimer;
extern TIMER RecvNumTimer;
extern BOOL NumKeyErrorDispFlag;
extern BOOL NumKeyRadioFreqErrorFlag;
#endif


typedef enum _DISP_STATE
{
	DISP_STATE_IDLE = 0,	//此状态下允许显示播放时间，Radio频率等
	DISP_STATE_DEV,
	DISP_STATE_FILE	//MP3才有此状态	

} DISP_STATE;

DISP_STATE gDispState = DISP_STATE_DEV;

//----------------------------------------------------------

// Clear digit-value(清数值显示位).
void ClearDigit(void)
{
#ifdef DISP_DEV_7_LED
	DispString("    ");	  //灭
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
#endif
#ifdef DISP_DEV_SLED
	DISP_LED_OFF();
#endif

}


// Clear display(显示全灭).
void ClearScreen(void)
{
#ifdef DISP_DEV_7_LED
	memset(gDispBuff, 0, sizeof(gDispBuff));
	LedPinGpioInit();
#endif
}


// Display initilization
// Indicate whether the system in "STAND-BY" state or not.
void DispInit(bool IsStandBy)
{
	TimeOutSet(&DispTmr, 0);
#ifdef FUNC_NUMBER_KEY_EN
	TimeOutSet(&NumKeyErrDispTimer, 0);
#endif
	//gDispPrevRep = gPlayCtrl.RepeatMode;
	//gDispPrevEQ = gPlayCtrl.Eq;
	//gDispPrevPlayState = gPlayCtrl.State;
	//gDispPrevVol = gSys.Volume;
	gBlink.BlinkFlag = 0;
	gDispState = DISP_STATE_DEV;

#ifdef DISP_DEV_SLED
	DISP_LED_INIT();
#endif

#ifdef DISP_DEV_7_LED
	memset(&gDispBuff, 0, sizeof(gDispBuff));
	LedPinGpioInit();
	gLedDispRefreshFlag = TRUE;
#endif

#ifdef DISP_DEV_7_LED
	if(IsStandBy == TRUE)
	{
		//APP_DBG(("CLS\n"));
		ClearScreen();
	}
	else
	{
		//APP_DBG(("DispLoad()\n"));
		DispPowerOn();
		TimeOutSet(&DispTmr, NORMAL_INTERVAL);
		//DispDev();
	}
#endif	
}


// Display de-initialization.
// Indicate whether the system in "STAND-BY" state or not.
void DispDeInit(void)
{
#ifdef DISP_DEV_7_LED
	ClearScreen();
#endif

#ifdef DISP_DEV_SLED
	DISP_LED_INIT();
	DISP_LED_OFF();
#endif

#ifdef DISP_DEV_7_LED
	gLedDispRefreshFlag = FALSE;	//解决进入Power down 过程中还会有段点亮问题
	LedPinGpioInit();
#endif
	//gDispPrevRep = gPlayCtrl.RepeatMode;
	//gDispPrevEQ = gPlayCtrl.Eq;
	//gDispPrevPlayState = gPlayCtrl.State;
	//gDispPrevVol = gSys.Volume;
}


// 设置Repeat模式时调用.
void DispRepeat(void)
{
#ifdef DISP_DEV_7_LED
#if (defined(CFG_APP_CARD_PLAY_MODE_EN) )|| defined(CFG_APP_USB_PLAY_MODE_EN)
	char RepeatName[5][4] =
	{
			" ALL", " RAN", " ONE",  "F AL", "F RA"
		};

		DispString(&RepeatName[gpMediaPlayer->CurPlayMode][0]);
		DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
		DispIcon(ICON_COLON, LIGHT_OFF);
			TimeOutSet(&DispTmr, INTERIM_INTERVAL);
		gDispVo = 1;
#endif
#endif
}

#ifdef  CFG_FUNC_MUSIC_EQ_MODE_EN
// 设置EQ时调用.
void DispEQ(void)
{
#ifdef DISP_DEV_7_LED
	char EqName[6][4] =
		{
			" NOR", " CLA", " POP",  " ROC", " JAZ", " VOC"
		};

		DispString(&EqName[mainAppCt.EqMode][0]);
		DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
		DispIcon(ICON_COLON, LIGHT_OFF);
		TimeOutSet(&DispTmr, INTERIM_INTERVAL);
		gDispVo = 1;
#endif		
}
#endif

// 播放状态变化时调用(Play/Pause/Stop/FF/FB).
void DispPlayState(void)
{
#ifdef DISP_DEV_7_LED
	if(!gDispVo)
	 {
	    LedDispPlayState();
		TimeOutSet(&DispTmr, NORMAL_INTERVAL);
	 }
#endif

#ifdef DISP_DEV_SLED
		  if(GetMediaPlayerState() == PLAYER_STATE_PAUSE)
			{
			  DISP_LED_ON();
			  gBlink.BlinkFlag = 0;
			}
		  else if((GetMediaPlayerState() == PLAYER_STATE_PLAYING) || (GetMediaPlayerState() == PLAYER_STATE_FF) || (GetMediaPlayerState() == PLAYER_STATE_FB))
			{
			  gBlink.Blink.MuteBlink = 1;
			}
			else // if((gPlayCtrl.State == PLAY_STATE_STOP) || (gPlayCtrl.State == PLAY_STATE_IDLE) || (gPlayCtrl.State == PLAY_STATE_BROWSE))
			{
				  DISP_LED_ON();
				  gBlink.BlinkFlag = 0;
			}
#endif
}


// 文件夹功能打开/关闭时调用.
void DispFoldState(void)
{
#if (defined(FUNC_SEG_LCD_EN) || defined(FUNC_SEG_LED_EN))
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{
		if(gPlayCtrl.FolderEnable == TRUE)
		{
			DispString(" FDI");
		}
		else
		{
			DispString(" FEN");
		}
		DispIcon(ICON_COLON, LIGHT_OFF);
		DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
		TimeOutSet(&DispTmr, NORMAL_INTERVAL);
#ifdef FUNC_SEG_LCD_EN
		if(gDisplayMode == DISP_DEV_LCD58)
		{
			LcdFlushDispData();
		}
#endif
	}
#endif
}

#ifdef CFG_FUNC_ALARM_EN
void DispRtcTime(void)
{
#if 0//def DISP_DEV_7_LED

	DispChar((RtcModeCt->stDateTimer.Hour / 10), POS_COM_THOUSAND);
	DispChar((RtcModeCt->stDateTimer.Hour % 10), POS_COM_HUNDREDS);
	DispChar((RtcModeCt->stDateTimer.Mon / 10), POS_COM_TENS);
    DispChar((RtcModeCt->stDateTimer.Mon % 10), POS_COM_UNITS);

	TimeOutSet(&DispTmr, 0);
	gBlink.Blink.RepeatBlink = 1;
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_ON);
#endif
}
#endif

// 音量设置时调用.
void DispVolume(uint8_t channel)
{
#ifdef DISP_DEV_7_LED
	if(channel == APP_SOURCE_NUM)
	 {
	   DispChar('1', POS_COM_THOUSAND);
	   DispChar('U', POS_COM_HUNDREDS);
	   DispChar((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] / 10), POS_COM_TENS);
	   DispChar((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] % 10), POS_COM_UNITS);
	 }
	else if(channel == MIC_SOURCE_NUM)
	 {
	   DispChar('0', POS_COM_THOUSAND);
	   DispChar('U', POS_COM_HUNDREDS);
	   DispChar((mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] / 10), POS_COM_TENS);
	   DispChar((mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] % 10), POS_COM_UNITS);
	 }
	else if(channel == (10 + AUDIO_DAC0_SINK_NUM))
	 {
	   DispChar(' ', POS_COM_THOUSAND);
	   DispChar('U', POS_COM_HUNDREDS);
	   DispChar((mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM] / 10), POS_COM_TENS);
	   DispChar((mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM] % 10), POS_COM_UNITS);
	 }
	TimeOutSet(&DispTmr, INTERIM_INTERVAL);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
	gDispVo = 1;
#endif

}

#ifdef CFG_FUNC_MIC_TREB_BASS_EN
// 高音调节.
void DispTre(void)
{
#ifdef DISP_DEV_7_LED
	DispChar(' ', POS_COM_THOUSAND);
	DispChar('T', POS_COM_HUNDREDS);

	if(mainAppCt.MicTrebStep >= 7)
	 {
		DispChar(' ', POS_COM_TENS);
	    DispChar(((mainAppCt.MicTrebStep - 7) % 10), POS_COM_UNITS);
	 }
	else
	 {
		DispChar('-', POS_COM_TENS);
	    DispChar(((7 - mainAppCt.MicTrebStep) % 10), POS_COM_UNITS);
	 }

	TimeOutSet(&DispTmr, INTERIM_INTERVAL);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
	gDispVo = 1;
#endif

}

// 低音调节.
void DispBas(void)
{
#ifdef DISP_DEV_7_LED
	DispChar(' ', POS_COM_THOUSAND);
	DispChar('B', POS_COM_HUNDREDS);

	if(mainAppCt.MicBassStep >= 7)
	 {
		DispChar(' ', POS_COM_TENS);
	    DispChar(((mainAppCt.MicBassStep - 7) % 10), POS_COM_UNITS);
	 }
	else
	 {
		DispChar('-', POS_COM_TENS);
	    DispChar(((7 - mainAppCt.MicBassStep) % 10), POS_COM_UNITS);
	 }

	TimeOutSet(&DispTmr, INTERIM_INTERVAL);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
	gDispVo = 1;
#endif

}
#endif

void Disp_3D(void)
{
#ifdef DISP_DEV_7_LED
#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
	if(gCtrlVars.music_threed_unit.three_d_en)
	 {
		DispString("  3D");
	 }
	else
	 {
		DispString(" OFF");
	 }

	TimeOutSet(&DispTmr, INTERIM_INTERVAL);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
	gDispVo = 1;
#endif
#endif

}

void Disp_Vb(void)
{
#ifdef DISP_DEV_7_LED
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
	if(gCtrlVars.music_vb_unit.vb_en)
	 {
		DispString("  VB");
	 }
	else
	 {
		DispString(" OFF");
	 }

	TimeOutSet(&DispTmr, INTERIM_INTERVAL);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
	gDispVo = 1;
#endif
#endif

}

void Disp_Vocal_Cut(void)
{
#ifdef DISP_DEV_7_LED

	if(gCtrlVars.vocal_cut_unit.vocal_cut_en)
	 {
		DispString(" VOC");
	 }
	else
	 {
		DispString(" OFF");
	 }

	TimeOutSet(&DispTmr, INTERIM_INTERVAL);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
	gDispVo = 1;
#endif

}

//extern bool Shunning_En;

void Disp_Shunning(void)
{
#ifdef DISP_DEV_7_LED

	if(gCtrlVars.ShunningMode)
	 {
		DispString("SHUN");
	 }
	else
	 {
		DispString(" OFF");
	 }

	TimeOutSet(&DispTmr, INTERIM_INTERVAL);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
	gDispVo = 1;
#endif

}

// 显示数值.
void DispNum(uint16_t Num)
{
#ifdef DISP_DEV_7_LED
	Num2Char(Num, FALSE);
	TimeOutSet(&DispTmr, INTERIM_INTERVAL);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
	DispIcon(ICON_PAUSE, LIGHT_OFF);
	gDispVo = 1;
#endif
}

// 文件切换时调用.
void DispFileNum(void)
{
#ifdef DISP_DEV_7_LED
#if (defined(CFG_APP_CARD_PLAY_MODE_EN) )|| defined(CFG_APP_USB_PLAY_MODE_EN)
	//APP_DBG((">>DispFileNum\n"));
	if(gpMediaPlayer->CurFileIndex > 0)	//if(gFsInitOK == 1)
	{
		//APP_DBG(("gPlayCtrl.FileNum:%u\n", gPlayCtrl.FileNum));
		Num2Char(gpMediaPlayer->CurFileIndex, FALSE);
	}
	else
	{
		//APP_DBG(("NOF\n"));
		DispString(" NOF");
	}
	TimeOutSet(&DispTmr, DEV_SYMBOL_INTERVAL);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	DispIcon(ICON_COLON, LIGHT_OFF);
	DispIcon(ICON_PAUSE, LIGHT_OFF);
	gDispVo = 1;
#endif
#endif
}


// 显示播放时间.
void DispPlayTime(uint8_t Type)
{
#ifdef DISP_DEV_7_LED
#if (defined(CFG_APP_CARD_PLAY_MODE_EN) )|| defined(CFG_APP_USB_PLAY_MODE_EN)
	WORD TimeMMSS;

	TimeMMSS = gpMediaPlayer->CurPlayTime / 60;
	TimeMMSS *= 100;
	TimeMMSS += gpMediaPlayer->CurPlayTime % 60;
	Num2Char(TimeMMSS, TRUE);
#endif
#endif	
}

void DispRadioStation(uint8_t num)
{
#ifdef DISP_DEV_7_LED
	DispChar(' ', POS_COM_THOUSAND);
	DispChar('P', POS_COM_HUNDREDS);
	if(num > 9)
	{
		DispChar(((num) / 10), POS_COM_TENS);
		DispChar(((num) % 10), POS_COM_UNITS);
	}
	else
	{
		DispChar(' ', POS_COM_TENS);
		DispChar(num, POS_COM_UNITS);
	}
	TimeOutSet(&DispTmr, DEV_SYMBOL_INTERVAL);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
#endif
}

// Exit from Special Display-Mode.
void DispResume(void)
{	
#ifdef DISP_DEV_7_LED

		switch(GetSystemMode())
		{
			case AppModeIdle:
#ifndef FUNC_RTC_EN
				DispString(" NOD");	
#else
				//DispRtc();
#endif
				break;

#if (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN))
			case AppModeUDiskAudioPlay:
			case AppModeCardAudioPlay:
				break;
#endif

#ifdef CFG_APP_BT_MODE_EN
			case AppModeBtAudioPlay:
				DispString(" Bt ");
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;
#endif

#if 0//def CFG_FUNC_ALARM_EN
			case AppModeRtcAlarm:
				//DispString(" Bt ");
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				DispIcon(ICON_PLAY, LIGHT_OFF);
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispHoldTime = 0;
				break;
#endif

#ifdef CFG_APP_OPTICAL_MODE_EN
			case AppModeOpticalAudioPlay:
				DispString(" OPT");
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				DispIcon(ICON_PLAY, LIGHT_OFF);
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispHoldTime = 0;
				break;
#endif

#ifdef CFG_APP_HDMIIN_MODE_EN
			case AppModeHdmiAudioPlay:
				DispString(" ARC");
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				DispIcon(ICON_PLAY, LIGHT_OFF);
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispHoldTime = 0;
				break;
#endif

#ifdef CFG_APP_COAXIAL_MODE_EN
			case AppModeCoaxialAudioPlay:
				DispString(" COA");
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				DispIcon(ICON_PLAY, LIGHT_OFF);
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispHoldTime = 0;
				break;
#endif

#ifdef CFG_APP_RADIOIN_MODE_EN
			case AppModeRadioAudioPlay:
				if((sRadioControl->Freq >=875) && (sRadioControl->Freq <=1080))
				 {
				  DispIcon(ICON_USB, LIGHT_OFF);
				  DispIcon(ICON_SD, LIGHT_OFF);
				  DispIcon(ICON_DP_FM_MHZ, LIGHT_ON);
				  DispIcon(ICON_MP3, LIGHT_OFF);
				  DispIcon(ICON_PLAY, LIGHT_OFF);
				  DispIcon(ICON_PAUSE, LIGHT_OFF);
				  DispIcon(ICON_COLON, LIGHT_OFF);
				  Num2Char(sRadioControl->Freq, FALSE);
				 }
				/*
				if(RadioDisFlag == RadioDisCurFreNum)
				{
					if((gRadioData.CurrFreq >= RADIO_LOWER_BOUND) && (gRadioData.CurrFreq <= RADIO_UPPER_BOUND))
					{
						if(gSys.MuteFg == TRUE)	//暂停状态
						{
							DispString("PAUS");
						}
						else
						{
							Num2Char(gRadioData.CurrFreq, FALSE);						
							DispIcon(ICON_DP_FM_MHZ, LIGHT_ON);
						}
					}					
				}
				else if(RadioDisFlag == RadioDisCurChAutoscan)
				{						
					DispChar(' ', POS_COM_THOUSAND);
					DispChar('P', POS_COM_HUNDREDS);
					if(gRadioData.StationSum > 9)
					{
						DispChar((gRadioData.StationSum / 10), POS_COM_TENS);
						DispChar((gRadioData.StationSum % 10), POS_COM_UNITS);
					}
					else
					{
						DispChar(' ', POS_COM_TENS);
						DispChar(gRadioData.StationSum, POS_COM_UNITS);
					}
					DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				}
				else if(RadioDisFlag == RadioDisCurChChange)
				{
					DispChar(' ', POS_COM_THOUSAND);
					DispChar('P', POS_COM_HUNDREDS);
					if((gRadioData.CurrStation + 1) > 9)
					{
						DispChar(((gRadioData.CurrStation + 1) / 10), POS_COM_TENS);
						DispChar(((gRadioData.CurrStation + 1) % 10), POS_COM_UNITS);
					}
					else
					{
						DispChar(' ', POS_COM_TENS);
						DispChar((gRadioData.CurrStation + 1), POS_COM_UNITS);
					}
					DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				}

				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);*/
				break;

#endif

#ifdef CFG_APP_LINEIN_MODE_EN
			case AppModeLineAudioPlay:
				DispString(" AUX");
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;
#endif

#ifdef CFG_APP_USB_AUDIO_MODE_EN
			case AppModeUsbDevicePlay:
				DispString(" PC ");
			 	break;
#endif

#ifdef FUNC_STANDBY_EN
			case SYS_MODE_STANDBY:				
#ifndef FUNC_RTC_EN
				DispString(" CLO"); 
#else
				DispRtc();
#endif				
				break;
#endif

			default:
				break;
		}

#endif	

#ifdef DISP_DEV_SLED
	DISP_LED_ON();
#endif

}


// Mute状态变化时调用.
void DispMute(void)
{
#ifdef DISP_DEV_7_LED
   if(mainAppCt.gSysVol.MuteFlag)
	{
	  DispString("----");
	  DispIcon(ICON_COLON, LIGHT_OFF);
	}
#endif

#ifdef DISP_DEV_SLED
  //SLedDispMute();
#endif
}

//// 换台、存台时调用.
//VOID DispStationNum(VOID)
//{
//
//}


// Show Device symbol.
void DispDevSymbol(void)
{
#ifdef DISP_DEV_7_LED
	gBlink.BlinkFlag = 0;
	LedDispDevSymbol();
	TimeOutSet(&DispTmr, DispHoldTime);
#endif

#ifdef DISP_DEV_SLED
	gBlink.BlinkFlag = 0;
	DISP_LED_ON();
#endif	
}


void DispLoad(void)
{
#ifdef	DISP_DEV_7_LED
	ClearScreen();
#ifdef CFG_FUNC_POWER_MONITOR_EN
	//PowerMonitorDisp();
#endif
	DispString(" LOD");
#endif
}


// 进入Audio显示时调用
void DispAudio(void)
{	
#ifdef	DISP_DEV_7_LED
	ClearScreen();
	DispString(" PC ");
#endif	
}


//显示Power on信息
void DispPowerOn(void)
{
#ifdef	DISP_DEV_7_LED
	ClearScreen();
	DispString("HIFI");
#endif
}


// 设备切换时调用.
void DispDev(void)
{
//	APP_DBG((">>DispDev()\n"));

#ifdef	DISP_DEV_7_LED

		//APP_DBG(("gDispState:%bd\n", gDispState));
	
		if(IsTimeOut(&DispTmr))
		{
			switch(gDispState)
			{
				case DISP_STATE_DEV:
					//APP_DBG(("DISP_STATE_DEV\n"));
					DispDevSymbol();
					if((GetSystemMode() == AppModeUDiskAudioPlay) || (GetSystemMode() == AppModeCardAudioPlay))
					{
						//if(gFsInitOK == 1)
						//{
						//	gDispState = DISP_STATE_FILE;
						//}
						//if(gFsInitOK != 0)
						{
							gDispState = DISP_STATE_IDLE;	
						}
					}
					else
					{
						gDispState = DISP_STATE_IDLE;
					}
					break;
		
				case DISP_STATE_FILE:
					//APP_DBG(("DISP_STATE_FILE\n"));
					if((GetSystemMode() == AppModeUDiskAudioPlay) || (GetSystemMode() == AppModeCardAudioPlay))
					{
						//if(gFsInitOK == 1)
						{
							DispFileNum();
						}
						//else
						//{
						//	DispString(" FER");
						//}
					}
					gDispState = DISP_STATE_IDLE;
					break;
		
				default:
					//APP_DBG(("DISP_STATE_IDLE\n"));
					gDispState = DISP_STATE_IDLE;
					break;
			}
		}
#endif

#ifdef DISP_DEV_SLED
	//SLedDispDevSymbol();
#endif

//	APP_DBG(("<<DispDev()\n"));
}


#ifdef FUN_SYSTEM_POWEROFF_WAIT_TIME
//该函数级别高主要处理设置
static BYTE DispSysTimOff(VOID)
{
	if((gSysTimeOffState.ProState > SYS_OFF_TIMEING_IDLE) && (gSysTimeOffState.ProState < SYS_OFF_TIMEING_WAIT_POWEROFF))
	{
		if(gDisplayMode == DISP_DEV_LED57)
		{
			DispIcon(ICON_PLAY, LIGHT_OFF);	
			DispIcon(ICON_PAUSE, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);
			DispIcon(ICON_MP3, LIGHT_OFF);				
		}
		else if(gDisplayMode == DISP_DEV_LED8888)
		{
			DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);		
		}
							
		if(gSysTimeOffState.EnState == TRUE)
		{			
			DispIcon(ICON_SLEEP, LIGHT_ON);	
			
			switch(gSysTimeOffState.SleepState)
			{
				case SLEEP_15:
					DispString("  15");
					break;

				case SLEEP_30:
					DispString("  30");
					break;

				case SLEEP_45:
					DispString("  45");
					break;
					
				case SLEEP_60:
					DispString("  60");
					break;

				case SLEEP_75:
					DispString("  75");
					break;
					
				case SLEEP_90:
					DispString("  90");
					break;

				case SLEEP_105:
					DispString(" 105");
					break;
					
				case SLEEP_120:
					DispString(" 120");
					break;	
					
				default:
					break;
			}	
		}
		else
		{
			DispString(" CLO");
			DispIcon(ICON_SLEEP, LIGHT_OFF);		
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
#endif


#ifdef FUNC_RTC_EN
VOID DispRtc(VOID)
{
	RTC_DATE_TIME gDisRtcVal;
	
#ifdef FUNC_ALARM_EN		
	if(RtcState == RTC_STATE_SET_ALARM)
	{
		gDisRtcVal = gAlarmTime;
	}
	 else
#endif	 	
	{
		gDisRtcVal = gRtcTime;
	}
	
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
	if((gDisplayMode <= DISP_DEV_LCD58) && (gDisplayMode >= DISP_DEV_LED1888))
	{	
		if(gDisplayMode == DISP_DEV_LED57)
		{
			DispIcon(ICON_PLAY, LIGHT_OFF);	
			DispIcon(ICON_PAUSE, LIGHT_OFF);
			DispIcon(ICON_USB, LIGHT_OFF);
			DispIcon(ICON_SD, LIGHT_OFF);
			DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
			DispIcon(ICON_MP3, LIGHT_OFF);
		}
		else if(gDisplayMode == DISP_DEV_LED8888)
		{
			DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
		}
		
#ifdef	AU6210K_CAR_MODEL_DEMO
		if(RtcState == RTC_STATE_SET_SOURCE)
		{
			switch(RtcSourceState)
			{
				case RTC_SOURCE_CLOCK:
					Temp = gRtcTime.Hour*100;
					Temp += gRtcTime.Min;		
					Num2Char(Temp, TRUE);
					DispIcon(ICON_SLEEP, LIGHT_OFF);
					if(gRtcTime.Sec%2)//秒闪烁
					{
						DispIcon(ICON_COLON, LIGHT_ON);	
					}
					else
					{
						DispIcon(ICON_COLON, LIGHT_OFF);	
					}			
					break;
					
				case RTC_SOURCE_YEAR:
					Temp = gRtcTime.Year;	
					Num2Char(Temp, TRUE);
					DispIcon(ICON_COLON, LIGHT_OFF);				
					break;

				case RTC_SOURCE_MON:
					Temp = gRtcTime.Mon*100;
					Temp += gRtcTime.Date;		
					Num2Char(Temp, TRUE);
					DispIcon(ICON_COLON, LIGHT_OFF);					
					break;

				case RTC_SOURCE_ALARM:
					Temp = gAlarmTime.Hour*100;
					Temp += gAlarmTime.Min;		
					Num2Char(Temp, TRUE);
					DispIcon(ICON_COLON, LIGHT_ON);	
					if(gRtcTime.Sec%2)//秒闪烁
					{
						DispIcon(ICON_SLEEP, LIGHT_ON);
					}
					else
					{
						DispIcon(ICON_SLEEP, LIGHT_OFF);
					}		
					break;

				case RTC_SOURCE_ONOFF:
					DispIcon(ICON_COLON, LIGHT_OFF);	
					if(gRtcTime.Sec%2)//秒闪烁
					{
						DispIcon(ICON_SLEEP, LIGHT_ON);
					}
					else
					{
						DispIcon(ICON_SLEEP, LIGHT_OFF);
					}		
					if(gAlarmState == ALARM_STATUS_OPEN)
					{
						DispChar(' ', 0);
						DispChar('O', 1);
						DispChar('N', 2);
						DispChar(' ', 3);
					}
					else if(gAlarmState == ALARM_STATUS_CLOSE)
					{
						DispChar(' ', 0);
						DispChar('O', 1);
						DispChar('F', 2);
						DispChar('F', 3);
					}
					break;

				default:
					break;			
			}
		}
		else
#endif
		{
			switch(RtcSubState)
			{
				case RTC_SET_NONE:			
					Temp = gRtcTime.Hour * 100;
					Temp += gRtcTime.Min;		
					Num2Char(Temp, TRUE);
					
#ifdef AU6210K_BOOMBOX_DEMO
					if(gAlarm1State == ALARM_STATUS_OPEN)
					{
						DispIcon(ICON_ALARM1, LIGHT_ON);
					}
					else
					{
						DispIcon(ICON_ALARM1, LIGHT_OFF);
					}
					
					if(gAlarm2State == ALARM_STATUS_OPEN)
					{
						DispIcon(ICON_ALARM2, LIGHT_ON);
					}
					else
					{
						DispIcon(ICON_ALARM2, LIGHT_OFF);
					}

					DispIcon(ICON_COLON, LIGHT_ON);	
					
#else
					
					if(gRtcTime.Sec % 2)//秒闪烁
					{
						DispIcon(ICON_COLON, LIGHT_ON);	
					}
					else
					{
						DispIcon(ICON_COLON, LIGHT_OFF);	
					}				
#endif
					break;
				
#ifdef FUNC_ALARM_EN
				case RTC_SET_ALARM_NUM:	
					DispIcon(ICON_COLON, LIGHT_OFF);	
					DispChar('A', 0);
					DispChar('L', 1);
					DispChar(' ', 2);
					
					if(RtcAutOutTimeCount % 2)
					{
						DispChar(RtcAlarmNum + 0x30, 3);
					}
					else
					{
						DispChar(' ', 3);
					}
					break;
		
				case RTC_SET_ALARM_MODE:
					DispIcon(ICON_COLON, LIGHT_OFF);	
					DispChar('A', 0);
					DispChar('O', 1);
					DispChar(' ', 2);
					
					if(RtcAutOutTimeCount % 2)
					{
						DispChar(gAlarmMode + 0x30, 3);
					}
					else
					{
						DispChar(' ', 3);
					}
					break;
		
				case RTC_SET_WEEK:
					DispIcon(ICON_COLON, LIGHT_OFF);	
					DispChar('A', 0);
					DispChar(' ', 1);
					DispChar(' ', 2);
					if(RtcAutOutTimeCount%2)
					{
						DispChar((gDisRtcVal.WDay + 0x30), 3);
					}
					else
					{
						DispChar(' ', 3);
					}
					break;
#endif
				
				case RTC_SET_YEAR:
					DispIcon(ICON_COLON, LIGHT_OFF);		
					if(gRtcRdWrFlag == SET_RTC_TIME_YEAR)		
					{					
						if(RtcAutOutTimeCount % 2)
						{
							Temp = gDisRtcVal.Year;
							Num2Char(Temp, TRUE);				
						}
						else
						{
							DispChar(' ', 3);
							DispChar(' ', 2);					
							DispChar(' ', 1);
							DispChar(' ', 0);	
						}	
					}
					else if(gRtcRdWrFlag == READ_RTC_TIME)//year->mon data->weekday->hour min
					{					
						if(RtcAutOutTimeCount > 15)				
						{
							Temp = gDisRtcVal.Year;
						}
						else if(RtcAutOutTimeCount > 10)
						{
							Temp = gDisRtcVal.Mon * 100;
							Temp += gDisRtcVal.Date;
						}
						else if(RtcAutOutTimeCount > 5)
						{
							Temp = gDisRtcVal.WDay;
						}
						else
						{
							Temp = gDisRtcVal.Hour * 100;
							Temp += gDisRtcVal.Min;
						}
						Num2Char(Temp, TRUE);									
					}
					break;
		
				case RTC_SET_MON:
					DispIcon(ICON_COLON, LIGHT_OFF);	
					if(RtcAutOutTimeCount % 2)
					{
						Temp = gDisRtcVal.Mon * 100;
						Temp += gDisRtcVal.Date;		
						Num2Char(Temp, TRUE);
					}
					else
					{
						DispChar(' ', 1);
						DispChar(' ', 0);
					}		
					break;
		
				case RTC_SET_DATE:
					DispIcon(ICON_COLON, LIGHT_OFF);	
					if(RtcAutOutTimeCount % 2)
					{
						Temp = gDisRtcVal.Mon * 100;
						Temp += gDisRtcVal.Date;		
						Num2Char(Temp, TRUE);
					}
					else
					{					
						DispChar(' ', 3);
						DispChar(' ', 2);
					}	
					break;
											
				case RTC_SET_HR:
					DispIcon(ICON_COLON, LIGHT_ON);	
					if(RtcAutOutTimeCount % 2)
					{
						Temp = gDisRtcVal.Hour * 100;
						Temp += gDisRtcVal.Min;		
						Num2Char(Temp, TRUE);
					}
					else
					{
						DispChar(' ', 1);
						DispChar(' ', 0);				
					}		
					break;
		
				case RTC_SET_MIN:
					DispIcon(ICON_COLON, LIGHT_ON);	
					if(RtcAutOutTimeCount % 2)
					{
						Temp = gDisRtcVal.Hour * 100;
						Temp += gDisRtcVal.Min;		
						Num2Char(Temp, TRUE);					
					}
					else
					{					
						DispChar(' ', 3);
						DispChar(' ', 2);
					}	
					break;

#ifdef	AU6210K_CAR_MODEL_DEMO
				case RTC_SET_ALARM_ONOFF:
					DispIcon(ICON_COLON, LIGHT_OFF);						
					if(gAlarmState == ALARM_STATUS_OPEN)
					{
					   	if(RtcAutOutTimeCount%2)
						{
							DispChar(' ', 0);
							DispChar('O', 1);
							DispChar('N', 2);
							DispChar(' ', 3);
						}
						else
						{
							DispChar(' ', 3);
							DispChar(' ', 2);
							DispChar(' ', 1);
							DispChar(' ', 0);
						}
					}
					else if(gAlarmState == ALARM_STATUS_CLOSE)
					{
					   	if(RtcAutOutTimeCount%2)
						{
							DispChar(' ', 0);
							DispChar('O', 1);
							DispChar('F', 2);
							DispChar('F', 3);
						}
						else
						{
							DispChar(' ', 3);
							DispChar(' ', 2);
							DispChar(' ', 1);
							DispChar(' ', 0);
						}
					}
					break;
#endif
					
#ifdef AU6210K_BOOMBOX_DEMO
				case RTC_SET_ALARM_SOURCE:
					if(gAlarmSource == RTC_ALARM_SOURCE_RADIO)
					{
						DispChar(' ', 0);
						DispChar('A', 1);
						DispChar('F', 2);
						DispChar(' ', 3);
					}
					else if(gAlarmSource == RTC_ALARM_SOURCE_USB)
					{
						DispChar(' ', 0);
						DispChar('A', 1);
						DispChar('U', 2);
						DispChar(' ', 3);
					}
					else if(gAlarmSource == RTC_ALARM_SOURCE_CARD)
					{
						DispChar(' ', 0);
						DispChar('A', 1);
						DispChar('D', 2);
						DispChar(' ', 3);
					}
					else if(gAlarmSource == RTC_ALARM_SOURCE_AUX)
					{
						DispChar(' ', 0);
						DispChar('A', 1);
						DispChar('A', 2);
						DispChar(' ', 3);
					}
					else if(gAlarmSource == RTC_ALARM_ONOFF)
					{
						DispChar(' ', 0);
						DispChar('O', 1);
						DispChar('F', 2);
						DispChar('F', 3);
					}
					DispIcon(ICON_COLON, LIGHT_OFF);	
					break;
		
				case RTC_SET_ALARM_VOLUME:					
					DispChar(' ', POS_COM_THOUSAND);
					DispChar('U', POS_COM_HUNDREDS);
					DispChar((gAlarmVolume / 10), POS_COM_TENS);
					DispChar((gAlarmVolume % 10), POS_COM_UNITS);
					DispIcon(ICON_COLON, LIGHT_OFF);	
					break;
#endif
					
				default:
					break;
			}	
		}
	}
#endif
}


//该 函数级别高主要处理RTC按键和判断闹钟是否到来
static BYTE DispRtcSet(VOID)
{
	BOOL Ret = FALSE;

	if(RtcSubState == RTC_SET_NONE)
	{	
#ifdef FUNC_ALARM_EN	
		if(RtcAutOutTimeCount > 0)//闹钟到	
		{
			Temp = gRtcTime.Hour * 100;//此时二者时间相等
			Temp += gRtcTime.Min;	
			
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN))
			if(gDisplayMode == DISP_DEV_LED57)
			{
				DispIcon(ICON_PLAY, LIGHT_OFF);	
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_MP3, LIGHT_OFF);
			}
			else if(gDisplayMode == DISP_DEV_LED8888)
			{
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
			}
		
			if(RtcAutOutTimeCount % 2)
			{
				Num2Char(Temp, TRUE);
			}
			else
			{
				ClearDigit();
			}

			Ret = TRUE;		
#endif		
		}
#endif				
	}
	else
	{
		DispRtc();//设置时间或者闹钟
		Ret = TRUE;
	}

#ifdef FUNC_SEG_LCD_EN
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		LcdFlushDispData();
	}
#endif

	return Ret;	 
}
#endif

// 主函数调用.
void Display(uint16_t msgRecv)
{
#ifdef FUN_SYSTEM_POWEROFF_WAIT_TIME
		if(DispSysTimOff())
		{
			return;
		}
#endif

#ifdef FUNC_RTC_EN
		if(DispRtcSet())
		{
			return;
		}
#endif

		switch(msgRecv)
		{

		   case MSG_DISPLAY_SERVICE_DEV:
			   DispDevSymbol();
			   break;

		   case MSG_DISPLAY_SERVICE_BT_UNLINK:
			   DispDevSymbol();
			   gBlink.BlinkFlag = 1;
			   break;

		   case MSG_DISPLAY_SERVICE_BT_LINKED:
			   DispDevSymbol();
			   break;

		   case MSG_DISPLAY_SERVICE_LINE:
			   DispDevSymbol();
			   break;

		   case MSG_DISPLAY_SERVICE_RADIO:
			   if(!mainAppCt.gSysVol.MuteFlag)
			   DispDevSymbol();
			   break;
#ifdef CFG_APP_RADIOIN_MODE_EN
		   case MSG_DISPLAY_SERVICE_SEARCH_STATION:
			     DispRadioStation(sRadioControl->ChlCount + 1);
			   break;

		   case MSG_DISPLAY_SERVICE_STATION:
			     DispRadioStation(sRadioControl->CurStaIdx + 1);
			   break;
#endif
		   case MSG_DISPLAY_SERVICE_MEDIA:
		   	   #ifdef CFG_RES_IR_NUMBERKEY
			   if((!mainAppCt.gSysVol.MuteFlag) && (!Number_select_flag))
			   #else
			   if(!mainAppCt.gSysVol.MuteFlag)
			   #endif
			     DispPlayState();
			   break;

		   case MSG_DISPLAY_SERVICE_NUMBER:
		   	   #ifdef CFG_RES_IR_NUMBERKEY
			   DispNum(Number_value);
			   #endif
			   break;

		   case MSG_DISPLAY_SERVICE_FILE_NUM:
			   DispFileNum();
			   break;

		   case MSG_DISPLAY_SERVICE_VOL:
			   DispVolume(10 + AUDIO_DAC0_SINK_NUM);
			   break;

		   case MSG_DISPLAY_SERVICE_MUSIC_VOL:
			   DispVolume(APP_SOURCE_NUM);
			   break;

		   case MSG_DISPLAY_SERVICE_MIC_VOL:
			   DispVolume(MIC_SOURCE_NUM);
			   break;

		   case MSG_DISPLAY_SERVICE_MUTE:
			   DispMute();
               break;
#ifdef CFG_FUNC_MIC_TREB_BASS_EN
		   case MSG_DISPLAY_SERVICE_TRE:
			   DispTre();
			   break;

		   case MSG_DISPLAY_SERVICE_BAS:
			   DispBas();
               break;
#endif
#ifdef  CFG_FUNC_MUSIC_EQ_MODE_EN
		   case MSG_DISPLAY_SERVICE_EQ:
			   DispEQ();
			   break;
#endif
		   case MSG_DISPLAY_SERVICE_REPEAT:
			   DispRepeat();
			   break;
		   case MSG_DISPLAY_SERVICE_PAUSE:
			   DispPlayState();
			   break;
#ifdef CFG_FUNC_ALARM_EN

		   case MSG_DISPLAY_SERVICE_RTC_TIME:
			   DispRtcTime();
			   break;
#endif

		   case MSG_DISPLAY_SERVICE_3D:
			   Disp_3D();
			   break;

		   case MSG_DISPLAY_SERVICE_VB:
			   Disp_Vb();
			   break;

		   case MSG_DISPLAY_SERVICE_VOCAL_CUT:
			   Disp_Vocal_Cut();
			   break;

		   case MSG_DISPLAY_SERVICE_SHUNNING:
			   Disp_Shunning();
			   break;

		   default:
			   break;
		}

		if(mainAppCt.gSysVol.MuteFlag)
		  return;
		if(IsTimeOut(&DispTmr))
		{
			if(gBlink.BlinkFlag)
			{
               #ifdef CFG_APP_BT_MODE_EN
				if(GetSystemMode() == AppModeBtAudioPlay)
					TimeOutSet(&DispTmr, AMPLITUDE_INTERVAL);
				else
               #endif
				TimeOutSet(&DispTmr, PLAY_BLINK_INTERVAL);
				if(gBlink.Blink.MuteBlink)		// MUTE闪烁
				{
					if(gBlink.Blink.MuteBlankScreen)
					{
						 gBlink.Blink.MuteBlankScreen = 0;
						 DispResume();
					}
					else
					{
						gBlink.Blink.MuteBlankScreen = 1;
						ClearDigit();	//清数值显示
					}
				 }
				else if(gBlink.Blink.RepeatBlink)
				{
					if(gBlink.Blink.MuteBlankScreen)
					{
						 gBlink.Blink.MuteBlankScreen = 0;
						 //DispIcon(ICON_COLON, LIGHT_ON);
					}
					else
					{
						gBlink.Blink.MuteBlankScreen = 1;
						//DispIcon(ICON_COLON, LIGHT_OFF);
					}
				}
			}
			else
			{
				 DispResume();
			}
#ifdef	DISP_DEV_7_LED
			gDispVo = 0;
#endif
		}
}


#endif
