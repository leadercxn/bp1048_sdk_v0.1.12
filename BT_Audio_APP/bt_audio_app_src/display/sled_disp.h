#ifndef __SLED_DISP_H__
#define __SLED_DISP_H__

//#define SLED_MP3_EN
//#define SLED_USB_EN
//#define SLED_SD_EN
//#define SLED_AUX_EN
//#define SLED_REP_EN
//#define SLED_RAD_EN
//#define SLED_AUD_EN

#define LIGHTOFF	0
#define LIGHTON		1

#define SLOW_BLINK_INTERVAL		1000
#define NORMAL_BLINK_INTERVAL	500
#define QUICK_BLINK_INTERVAL	50//100 
#define REPEAT_BLINK_INTERVAL	300 

#define SLED_PORT_IE		GPIO_D_IE
#define SLED_PORT_OE		GPIO_D_OE
#define SLED_PORT_OUT		GPIO_D_OUT
#define SLED_PORT_PU		GPIO_D_PU
#define SLED_PORT_PD		GPIO_D_PD

#define SLED_POWER_PORT		GPIO_D_OUT
#define MASK_SLED_POWER		(1 << 2)	// D[2]



typedef enum _SLED_IDX
{
	LED_POWER,
	LED_MP3,
	LED_USB,
	LED_SD,
	LED_AUX,
	LED_RADIO,
	LED_AUDIO,
	LED_REPEAT

} SLED_IDX;

void SLedLightOp(SLED_IDX Led, bool IsLightOn);
void SLedInit(void);
void SLedDeinit(void);
void SLedDispDevSymbol(void);
void SLedDispPlayState(void);
void SLedDispRepeat(void);
void SLedDispRadio(void);
void SLedDispAudio(void);
void SLedDispRTC(void);
void SLedDispMute(void);
void SLedDisplay(void);


#endif
