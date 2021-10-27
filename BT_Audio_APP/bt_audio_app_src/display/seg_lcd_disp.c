#include <string.h>
#include <stdio.h>

#include "type.h"										//include project header files
#include "utility.h"
#include "syscfg.h"

#include "sysctrl.h"
#include "timer.h"
#include "display.h"
#include "seg_panel.h"
#include "seg_lcd_disp.h"
#include "lcd_seg.h"
#include "rtc.h"
#include "playctrl.h"
#include "breakpoint.h"
#include "pcm_amplitude.h"
#include "device_audio.h"
#include "debug.h"


#ifdef FUNC_SEG_LCD_EN

TIMER	gAmplitudeTmr;

extern WORD DispHoldTime;

extern BOOL IsUSBLink(VOID);
extern BOOL IsCardLink(VOID);
extern VOID ClearDigit(VOID);

#if defined(AU6210K_BOOMBOX_DEMO)
DISP_TYPE CODE LcdIconSet[ICON_NUM] = {0xFF, 0xFF, ICON58_COLON, ICON58_USB, ICON58_SD, ICON58_FM_MHZ, 0xFF, ICON58_REP, ICON58_RAN, ICON58_INTO, ICON58_MUTE, ICON58_L1, ICON58_L2, ICON58_L3, ICON58_L4, 0xFF, 0xFF, 0xFF, ICON58_STERO, ICON58_MEMORY, ICON58_ALARM1, ICON58_ALARM2};
#else
DISP_TYPE CODE LcdIconSet[ICON_NUM] = {0xFF, 0xFF, ICON58_COLON, ICON58_USB, ICON58_SD, ICON58_FM_MHZ, 0xFF, ICON58_REP, ICON58_RAN, ICON58_INTO, ICON58_MUTE, ICON58_L1, ICON58_L2, ICON58_L3, ICON58_L4};
#endif


// ÏÔÊ¾Ë¢ÐÂ.
// Lcd display refresh ISR.
VOID LcdFlushDisp(VOID)
{
	static BYTE Count = 0;

	Count ++;
	if(Count == 2)	// 2   * 2ms = 2000us  2ms	 50
	{
		Count = 0;
		LcdSetRefMem();
	}
}


VOID DispLcdIcon(ICON_IDX Icon, ICON_STATU Light)
{
	DISP_TYPE IconVal;

	IconVal = LcdIconSet[Icon];
	
	if(IconVal == 0xFF)
	{
		return;
	}

#if (MAX_SEG_NUM > 8)
	IconVal &= 0x1FFF;
#else
	IconVal = (1 << (IconVal % 8));
#endif
	//APP_DBG(("LcdIcon,%bd,IconVal:%bx,%bd\n", LcdIconSet[Icon]/8, IconVal, Light));

	if(Light == LIGHT_ON)
	{
#if (MAX_SEG_NUM > 8)
		gDispBuff[LcdIconSet[Icon] >> 13] |= (DISP_TYPE)IconVal;
#else
		gDispBuff[LcdIconSet[Icon] / 8] |= (DISP_TYPE)IconVal;
#endif
	}
	else if(Light == LIGHT_OFF)
	{
#if (MAX_SEG_NUM > 8)
		gDispBuff[LcdIconSet[Icon] >> 13] &= (DISP_TYPE)(~IconVal);
#else
		gDispBuff[LcdIconSet[Icon] / 8] &= (DISP_TYPE)(~IconVal);
#endif
	}
	else if(Light == LIGHT_BLINK)
	{
//		if((gDisplayMode != DISP_DEV_NONE) && (gDisplayMode != DISP_DEV_LCD58))
//		{
		if(HalfSecStatus())
		{
#if (MAX_SEG_NUM > 8)
			gDispBuff[LcdIconSet[Icon] >> 13] |= (DISP_TYPE)IconVal;
#else
			gDispBuff[LcdIconSet[Icon] / 8] |= (DISP_TYPE)IconVal;
#endif
		}
		else
		{
#if (MAX_SEG_NUM > 8)
			gDispBuff[LcdIconSet[Icon] >> 13] |= (DISP_TYPE)IconVal;
#else
			gDispBuff[LcdIconSet[Icon] / 8] &= (DISP_TYPE)(~IconVal);
#endif
		}
//		}
//		else if(gDisplayMode == DISP_DEV_LCD58)
//		{
////			LcdBlinkCtrl(5, LcdIconSet[Icon] % 8, LcdIconSet[Icon] / 8);
//			//// 0 -> B0, 1-> B1... 7->B7, 8 -> D2, 9 -> D3 ... 13 -> D7
//			//BYTE CODE LcdSegChanNo[MAX_SEG_NUM]={6, 5, 3, 2, 1, 0, 4, 7};
//			//LcdBlinkCtrl(5, LcdSegChanNo[LcdIconSet[Icon]%MAX_SEG_NUM], LcdIconSet[Icon]/MAX_COM_NUM);
//		}
	}
}


//ÆµÆ×ÏÔÊ¾
VOID LcdAmplitudeCurve(AMPL_CURVE Ampl)
{
	switch(Ampl)
	{
		case Ampl_Zero:
			DispIcon(ICON_L1, LIGHT_OFF);
			DispIcon(ICON_L2, LIGHT_OFF);
			DispIcon(ICON_L3, LIGHT_OFF);
			DispIcon(ICON_L4, LIGHT_OFF);
			break;
	
		case Ampl_One:
			DispIcon(ICON_L1, LIGHT_ON);
			DispIcon(ICON_L2, LIGHT_OFF);
			DispIcon(ICON_L3, LIGHT_OFF);
			DispIcon(ICON_L4, LIGHT_OFF);
			break;
	
		case Ampl_Two:
			DispIcon(ICON_L1, LIGHT_ON);
			DispIcon(ICON_L2, LIGHT_ON);
			DispIcon(ICON_L3, LIGHT_OFF);
			DispIcon(ICON_L4, LIGHT_OFF);
			break;
		case Ampl_Three:
			DispIcon(ICON_L1, LIGHT_ON);
			DispIcon(ICON_L2, LIGHT_ON);
			DispIcon(ICON_L3, LIGHT_ON);
			DispIcon(ICON_L4, LIGHT_OFF);
			break;
	
		case Ampl_Four:
			DispIcon(ICON_L1, LIGHT_ON);
			DispIcon(ICON_L2, LIGHT_ON);
			DispIcon(ICON_L3, LIGHT_ON);
			DispIcon(ICON_L4, LIGHT_ON);
			break;
	
		default:
			break;
	}	
	
}

VOID LcdDispAmplCurve(VOID)
{
	if(gSys.MuteFg == TRUE)
	{
		LcdAmplitudeCurve(Ampl_Zero);
	}
	else if((((gSys.SystemMode == SYS_MODE_USB) || (gSys.SystemMode == SYS_MODE_SD)) && (gPlayCtrl.State == PLAY_STATE_PLAY))
#ifdef FUNC_AUDIO_EN
		|| ((gSys.SystemMode == SYS_MODE_AUDIO) && (gAudioState == AUDIO_PLAY))
#endif
#ifdef FUNC_AUDIO_READER_EN
		|| ((gSys.SystemMode == SYS_MODE_AUDIO_READER) && (gAudioState == AUDIO_PLAY))
#endif
	)
	{
		if(IsTimeOut(&gAmplitudeTmr))
		{
			//APP_DBG(("LcdAmplitudeCurve,11\n"));
			LcdAmplitudeCurve(GetPCMAmplitude());
			TimeOutSet(&gAmplitudeTmr, AMPLITUDE_INTERVAL);	
		}		
	}
//	else if(((gSys.SystemMode == SYS_MODE_RADIO) && (gRadioCtrl.State == RADIO_IDLE))
//		|| (gSys.SystemMode == SYS_MODE_LINEIN)
//	)
//	{
//		if(IsTimeOut(&gAmplitudeTmr))
//		{
//			//APP_DBG(("LcdAmplitudeCurve,22\n"));
//			///////////////LcdAmplitudeCurve(GetPCMAmplitude());
//			TimeOutSet(&gAmplitudeTmr, AMPLITUDE_INTERVAL);	
//		}
//	}
}


VOID LcdFlushDispData(VOID)
{
	LcdDispAmplCurve();	
	LcdSetData((BYTE *)gDispBuff);
}


VOID LcdDispDevSymbol(VOID)
{
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		DispHoldTime = DEV_SYMBOL_INTERVAL;
		
		switch(gSys.SystemMode)
		{
			case SYS_MODE_NONE:
#ifndef FUNC_RTC_EN	
				//APP_DBG(("NOD2"));
				DispString(" NOD");
				LcdAmplitudeCurve(Ampl_Zero);
				DispIcon(ICON_RAN, LIGHT_OFF);
				DispIcon(ICON_INTO, LIGHT_OFF);
				DispIcon(ICON_REP, LIGHT_OFF);
				DispIcon(ICON_MUTE, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
#else
				DispRtc();
#endif
				break;

#ifdef FUNC_USB_EN
			case SYS_MODE_USB:
				DispString(" LOD");
				LcdAmplitudeCurve(Ampl_Zero);
				DispIcon(ICON_RAN, LIGHT_OFF);
				DispIcon(ICON_INTO, LIGHT_OFF);
				DispIcon(ICON_REP, LIGHT_OFF);
				DispIcon(ICON_MUTE, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_USB, LIGHT_ON);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				DispHoldTime = 0;
				break;
#endif

#ifdef FUNC_CARD_EN
			case SYS_MODE_SD:
				DispString(" LOD");
				LcdAmplitudeCurve(Ampl_Zero);
				DispIcon(ICON_RAN, LIGHT_OFF);
				DispIcon(ICON_INTO, LIGHT_OFF);
				DispIcon(ICON_REP, LIGHT_OFF);
				DispIcon(ICON_MUTE, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_ON);
				DispIcon(ICON_COLON, LIGHT_OFF);
				DispHoldTime = 0;
				break;
#endif
		
#ifdef FUNC_LINEIN_EN
			case SYS_MODE_LINEIN:
				DispString(" AUX");
				LcdAmplitudeCurve(Ampl_Zero);
				DispIcon(ICON_RAN, LIGHT_OFF);
				DispIcon(ICON_INTO, LIGHT_OFF);
				DispIcon(ICON_REP, LIGHT_OFF);
				DispIcon(ICON_MUTE, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;
#endif

#ifdef FUNC_RADIO_EN
			case SYS_MODE_RADIO:
				LcdAmplitudeCurve(Ampl_Zero);
				DispIcon(ICON_RAN, LIGHT_OFF);
				DispIcon(ICON_INTO, LIGHT_OFF);
				DispIcon(ICON_REP, LIGHT_OFF);
				DispIcon(ICON_MUTE, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_ON);
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				DispHoldTime = 0;
				if(gRadioData.CurrFreq == 0)
				{
					gRadioData.CurrFreq = RADIO_LOWER_BOUND;
				}
				Num2Char(gRadioData.CurrFreq, FALSE);
				break;
#endif

#ifdef FUNC_AUDIO_EN
			case SYS_MODE_AUDIO:
				DispString(" PC ");
				LcdAmplitudeCurve(Ampl_Zero);
				DispIcon(ICON_RAN, LIGHT_OFF);
				DispIcon(ICON_INTO, LIGHT_OFF);
				DispIcon(ICON_REP, LIGHT_OFF);
				DispIcon(ICON_MUTE, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;
#endif

#ifdef FUNC_READER_EN
			case SYS_MODE_READER:
				DispString(" REA");
				LcdAmplitudeCurve(Ampl_Zero);
				DispIcon(ICON_RAN, LIGHT_OFF);
				DispIcon(ICON_INTO, LIGHT_OFF);
				DispIcon(ICON_REP, LIGHT_OFF);
				DispIcon(ICON_MUTE, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;
#endif

#ifdef FUNC_AUDIO_READER_EN
			case SYS_MODE_AUDIO_READER:
				DispString(" PC ");
				LcdAmplitudeCurve(Ampl_Zero);
				DispIcon(ICON_RAN, LIGHT_OFF);
				DispIcon(ICON_INTO, LIGHT_OFF);
				DispIcon(ICON_REP, LIGHT_OFF);
				DispIcon(ICON_MUTE, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;
#endif

#ifdef FUNC_RTC_EN
			case SYS_MODE_RTC:
				DispString("TIME");
				LcdAmplitudeCurve(Ampl_Zero);
				DispIcon(ICON_RAN, LIGHT_OFF);
				DispIcon(ICON_INTO, LIGHT_OFF);
				DispIcon(ICON_REP, LIGHT_OFF);
				DispIcon(ICON_MUTE, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;
#endif
		
			default:
				ClearDigit();
				DispHoldTime = 0;
				LcdAmplitudeCurve(Ampl_Zero);
				DispIcon(ICON_RAN, LIGHT_OFF);
				DispIcon(ICON_INTO, LIGHT_OFF);
				DispIcon(ICON_REP, LIGHT_OFF);
				DispIcon(ICON_MUTE, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;
		}
	}
}


VOID LcdDispRepeat(VOID)
{
	if(gPlayCtrl.RepeatMode == REPEAT_ALL)
	{
		//APP_DBG(("ALL\n"));
		DispIcon(ICON_REP, LIGHT_OFF);
		DispIcon(ICON_RAN, LIGHT_OFF);
		DispIcon(ICON_INTO, LIGHT_OFF);
	}
	else if(gPlayCtrl.RepeatMode == REPEAT_ONE)
	{
		//APP_DBG(("ONE\n"));
		DispIcon(ICON_REP, LIGHT_ON);
		DispIcon(ICON_RAN, LIGHT_OFF);
		DispIcon(ICON_INTO, LIGHT_OFF);
	}
	else if(gPlayCtrl.RepeatMode == REPEAT_RANDOM)
	{
		//APP_DBG(("RAN\n"));
		DispIcon(ICON_REP, LIGHT_OFF);
		DispIcon(ICON_RAN, LIGHT_ON);
		DispIcon(ICON_INTO, LIGHT_OFF);
	}
	else if(gPlayCtrl.RepeatMode == REPEAT_INTRO)
	{
		//APP_DBG(("INTO\n"));
		DispIcon(ICON_REP, LIGHT_OFF);
		DispIcon(ICON_RAN, LIGHT_OFF);
		DispIcon(ICON_INTO, LIGHT_ON);
	}
}


VOID LcdDispPlayState(VOID)
{
#if (defined(FUNC_USB_EN) || defined(FUNC_CARD_EN))
	if(gDisplayMode == DISP_DEV_LCD58)
	{
		if((gPlayCtrl.State != PLAY_STATE_PLAY) || (gSys.MuteFg == TRUE))
		{
			LcdAmplitudeCurve(Ampl_Zero);
		}

		if(gPlayCtrl.State == PLAY_STATE_PAUSE)
		{
			DispIcon(ICON_PAUSE, LIGHT_ON);
			DispIcon(ICON_PLAY, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);
			DispString("PAUS");
		}
		else if(gPlayCtrl.State == PLAY_STATE_STOP)
		{
			DispIcon(ICON_PAUSE, LIGHT_OFF);
			DispIcon(ICON_PLAY, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);
			if(
#ifdef FUNC_USB_EN
				((gSys.SystemMode == SYS_MODE_USB) && IsUSBLink()) 
#endif
#ifdef FUNC_CARD_EN
				|| ((gSys.SystemMode == SYS_MODE_SD) && IsCardLink())
#endif
			)
			{
				DispString("STOP");
			}
		}
		DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
	}
#endif
}


#endif
