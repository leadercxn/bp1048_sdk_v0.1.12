#include "type.h"
#include <nds32_intrinsic.h>
#include "app_config.h"
#include "display.h"
#include "seg_panel.h"
#include "seg_led_disp.h"
#include "debug.h"
#include "main_task.h"
#include "media_play_api.h"
#include "radio_api.h"

#ifdef DISP_DEV_7_LED

bool gLedDispRefreshFlag = FALSE;
extern uint16_t DispHoldTime;
uint8_t  Led57IconSet[ICON_NUM] = {ICON57_PLAY, ICON57_PAUSE, ICON57_COLON, ICON57_USB, ICON57_SD, ICON57_FM_MHZ, ICON57_MP3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,0xFF,0xFF,0xFF};

void DispLedIcon(ICON_IDX Icon, ICON_STATU Light)
{
	uint8_t IconVal;
	uint8_t IconPos;

	IconVal = Led57IconSet[Icon];

	if(IconVal == 0xFF)
	{
		return;
	}
	IconPos = IconVal / 8;
	IconVal = (1 << (IconVal % 8));

	if(Light == LIGHT_ON)
	{
		gDispBuff[IconPos] |= (DISP_TYPE)IconVal;
	}
	else if(Light == LIGHT_OFF)
	{
		gDispBuff[IconPos] &= (DISP_TYPE)(~IconVal);
	}
}	


void LedDispDevSymbol(void)
{
	DispHoldTime = DEV_SYMBOL_INTERVAL;
	switch(GetSystemMode())
 	 {
		case AppModeIdle:
#ifndef CFG_APP_RTC_MODE_EN
				DispString(" NOD");
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_MP3, LIGHT_OFF);
				DispIcon(ICON_PLAY, LIGHT_OFF);
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
#else
				//DispRtc();
#endif
				break;

#ifdef CFG_APP_USB_PLAY_MODE_EN
			case AppModeUDiskAudioPlay:
				DispString(" USB");
				DispIcon(ICON_USB, LIGHT_ON);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				DispHoldTime = 0;
				break;
#endif

#ifdef CFG_APP_CARD_PLAY_MODE_EN
			case AppModeCardAudioPlay:
				DispString(" SD ");
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_ON);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				DispHoldTime = 0;
				break;
#endif

#ifdef CFG_APP_RTC_MODE_EN
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

#ifdef CFG_APP_BT_MODE_EN
			case AppModeBtAudioPlay:
				DispString(" Bt ");
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

#ifdef CFG_APP_LINEIN_MODE_EN
			case AppModeLineAudioPlay:
				DispString(" AUX");
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_MP3, LIGHT_OFF);
				DispIcon(ICON_PLAY, LIGHT_OFF);
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;
#endif

#ifdef CFG_APP_RADIOIN_MODE_EN
			case AppModeRadioAudioPlay:
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_ON);
				DispIcon(ICON_MP3, LIGHT_OFF);
				DispIcon(ICON_PLAY, LIGHT_OFF);
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);	
				if(sRadioControl->Freq == 0)
				{
					sRadioControl->Freq = 875;
				}
				Num2Char(sRadioControl->Freq, FALSE);
				DispHoldTime = 0;
				break;
#endif

#ifdef CFG_APP_USB_AUDIO_MODE_EN
			case AppModeUsbDevicePlay:
				DispString(" PC ");
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_MP3, LIGHT_OFF);
				DispIcon(ICON_PLAY, LIGHT_OFF);
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				break;
#endif

#ifdef FUNC_STANDBY_EN
			case SYS_MODE_STANDBY:	
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_MP3, LIGHT_OFF);
				DispIcon(ICON_PLAY, LIGHT_OFF);
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
#ifndef FUNC_RTC_EN
				DispString(" CLO"); 
#else
				DispRtc();
#endif				
				break;
#endif
		
			default:
				DispIcon(ICON_USB, LIGHT_OFF);
				DispIcon(ICON_SD, LIGHT_OFF);
				DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
				DispIcon(ICON_MP3, LIGHT_OFF);
				DispIcon(ICON_PLAY, LIGHT_OFF);
				DispIcon(ICON_PAUSE, LIGHT_OFF);
				DispIcon(ICON_COLON, LIGHT_OFF);
				DispHoldTime = 0;
				break;
		}
}


void LedDispRepeat(void)
{/*
	if(gPlayCtrl.RepeatMode == REPEAT_ALL)
	{
		DispString(" ALL");
	}
	else if(gPlayCtrl.RepeatMode == REPEAT_ONE)
	{
		DispString(" ONE");
	}
	else if(gPlayCtrl.RepeatMode == REPEAT_RANDOM)
	{
		DispString(" RAN");
	}
	else if(gPlayCtrl.RepeatMode == REPEAT_INTRO)
	{
		DispString("INTO");
	}
	else if(gPlayCtrl.RepeatMode == REPEAT_FOLDER)
	{
		DispString("FLOD");
	}
	DispIcon(ICON_COLON, LIGHT_OFF);
	DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);	*/
}


void LedDispPlayState(void)
{
#if (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN))

	  if(GetMediaPlayerState() == PLAYER_STATE_PAUSE)
		{
			//APP_DBG(("LED57||\n"));
			DispPlayTime(DIS_PLAYTIME_PAUSE);
			DispIcon(ICON_PLAY, LIGHT_OFF);
			DispIcon(ICON_PAUSE, LIGHT_ON);
			DispIcon(ICON_COLON, LIGHT_ON);
		}
	  else if((GetMediaPlayerState() == PLAYER_STATE_PLAYING) || (GetMediaPlayerState() == PLAYER_STATE_FF) || (GetMediaPlayerState() == PLAYER_STATE_FB))
		{
		    DispPlayTime(DIS_PLAYTIME_PAUSE);
		    DispIcon(ICON_PLAY, LIGHT_ON);
			DispIcon(ICON_PAUSE, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_ON);
		}
		else // if((gPlayCtrl.State == PLAY_STATE_STOP) || (gPlayCtrl.State == PLAY_STATE_IDLE) || (gPlayCtrl.State == PLAY_STATE_BROWSE))
		{
			DispIcon(ICON_PLAY, LIGHT_OFF);
			DispIcon(ICON_PAUSE, LIGHT_OFF);
			DispIcon(ICON_COLON, LIGHT_OFF);
			if(GetMediaPlayerState() == PLAYER_STATE_STOP)
			{
			   DispString("STOP");
			}
		}
	  if(GetSystemMode() == AppModeUDiskAudioPlay)
	    {
		  DispIcon(ICON_USB, LIGHT_ON);
		  DispIcon(ICON_SD, LIGHT_OFF);
	    }
	  if(GetSystemMode() == AppModeCardAudioPlay)
	    {
		  DispIcon(ICON_USB, LIGHT_OFF);
		  DispIcon(ICON_SD, LIGHT_ON);
	    }
	  DispIcon(ICON_DP_FM_MHZ, LIGHT_OFF);
#endif
}

// ÏÔÊ¾Ë¢ÐÂ.
// Call by display refresh ISR.
__attribute__((section(".tcm_section")))
void LedFlushDisp(void)
{
	static uint8_t ScanPinNum = 0;

	if(gLedDispRefreshFlag == FALSE)
	{
		return;
	}

	ScanPinNum++;

	if(ScanPinNum >= (MAX_LED_PIN_NUM + 1))
	{
		ScanPinNum = 1;
	}

	LedAllPinGpioInput();

// LED ¸÷¸öPin ½ÅÒÀ´ÎÉ¨Ãè
//7Ö»½Å¶ÎÂëLEDÆÁÇý¶¯É¨ÃèÊ¾Àý
	switch(ScanPinNum)
	{
		case 1:
			if(gDispBuff[0] & SEG_A)
			{
				LED_PIN2_IN_ON;
			}

			if(gDispBuff[0] & SEG_B)
			{
				LED_PIN3_IN_ON;
			}

			if(gDispBuff[0] & SEG_E)
			{
				LED_PIN4_IN_ON;
			}

			if(gDispBuff[4] & SEG_A)
			{
				LED_PIN6_IN_ON;
			}	//PLAY

			if(gDispBuff[4] & SEG_D)
			{
				LED_PIN5_IN_ON;
			}	//SD

			if(gDispBuff[3] & SEG_C)
			{
				LED_PIN7_IN_ON;
			}
			LED_PIN1_OUT_HIGH;
			break;

		case 2:
			if(gDispBuff[0] & SEG_F)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[1] & SEG_A)
			{
				LED_PIN3_IN_ON;
			}

			if(gDispBuff[1] & SEG_B)
			{
				LED_PIN4_IN_ON;
			}

			if(gDispBuff[1] & SEG_E)
			{
				LED_PIN5_IN_ON;
			}

			if(gDispBuff[1] & SEG_D)
			{
				LED_PIN6_IN_ON;
			}

			if(gDispBuff[3] & SEG_G)
			{
				LED_PIN7_IN_ON;
			}
			LED_PIN2_OUT_HIGH;
			break;

		case 3:
			if(gDispBuff[0] & SEG_G)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[1] & SEG_F)
			{
				LED_PIN2_IN_ON;
			}

			if(gDispBuff[4] & SEG_E)
			{
				LED_PIN4_IN_ON;
			}  //K5 DOT

			if(gDispBuff[2] & SEG_B)
			{
				LED_PIN5_IN_ON;
			}

			if(gDispBuff[4] & SEG_B)
			{
				LED_PIN6_IN_ON;
			}  //K2 PAUSE

			if(gDispBuff[4] & SEG_G)
			{
				LED_PIN7_IN_ON;
			}  //K7 MP3
			LED_PIN3_OUT_HIGH;
			break;

		case 4:
			if(gDispBuff[0] & SEG_C)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[1] & SEG_G)
			{
				LED_PIN2_IN_ON;
			}

			if(gDispBuff[2] & SEG_F)
			{
				LED_PIN3_IN_ON;
			}

			if(gDispBuff[2] & SEG_C)
			{
				LED_PIN5_IN_ON;
			}

			if(gDispBuff[3] & SEG_E)
			{
				LED_PIN6_IN_ON;
			}

			if(gDispBuff[3] & SEG_B)
			{
				LED_PIN7_IN_ON;
			}
			LED_PIN4_OUT_HIGH;
			break;

		case 5:
			if(gDispBuff[0] & SEG_D)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[1] & SEG_C)
			{
				LED_PIN2_IN_ON;
			}

			if(gDispBuff[2] & SEG_G)
			{
				LED_PIN3_IN_ON;
			}

			if(gDispBuff[2] & SEG_A)
			{
				LED_PIN4_IN_ON;
			}
			LED_PIN5_OUT_HIGH;
			break;

		case 6:
			if(gDispBuff[2] & SEG_D)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[4] & SEG_C)
			{
				LED_PIN2_IN_ON;
			}  //K3 USB

			if(gDispBuff[2] & SEG_E)
			{
				LED_PIN3_IN_ON;
			}

			if(gDispBuff[3] & SEG_D)
			{
				LED_PIN4_IN_ON;
			}
			LED_PIN6_OUT_HIGH;
			break;

		case 7:
			if(gDispBuff[3] & SEG_F)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[4] & SEG_F)
			{
				LED_PIN3_IN_ON;
			} //K6 FM

			if(gDispBuff[3] & SEG_A)
			{
				LED_PIN4_IN_ON;
			}
			LED_PIN7_OUT_HIGH;
			break;

		default:
			break;
	}
}
#endif
