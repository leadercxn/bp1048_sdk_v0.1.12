#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "type.h"
#include "timeout.h"

#define AMPLITUDE_INTERVAL		100
#define FFFB_BLINK_INTERVAL		300
#define PLAY_BLINK_INTERVAL		500
#define MUTE_BLINK_INTERVAL		1000
#define SCAN_BLINK_INTERVAL		300
#define DEV_SYMBOL_INTERVAL		1000
#define NORMAL_INTERVAL			1000
#define INTERIM_INTERVAL		2000	// EQ、Repeat、Volume显示保持状态
#define SET_INTERVL				5000	// RTC设置等显示状态保持时间

typedef struct _ST_BLINK
{
	uint8_t MuteBlink		:1;	//复用做SLED MP3 PAUSE/STOP/MUTE闪烁标志(慢速闪烁)
	uint8_t HourSet		:1;	//复用做SLED MP3快进快退/Radio搜台闪烁标志(快速闪烁)
	uint8_t MinSet			:1;	//复用做SLED MP3播放闪烁标志(常速闪烁)
	uint8_t RepeatBlink	:1;
	uint8_t MuteBlankScreen:1;
	uint8_t HourBlankScreen:1;
	uint8_t MinBlankScreen	:1;
	uint8_t RepeatOff		:1;

} ST_BLINK;

typedef union _UN_BLINK
{
	ST_BLINK 	Blink;
	uint8_t		BlinkFlag;

} UN_BLINK;
	
extern UN_BLINK gBlink;
extern TIMER	DispTmr;

extern uint8_t  gDispVo;
//extern uint8_t gDispPrevRep;
//extern uint8_t gDispPrevEQ;
//extern uint8_t gDispPrevVol;
//extern uint8_t gDispPrevPlayState;

// Clear Screen
void ClearScreen(void);

// Display initilization
// Indicate whether the system in "STAND-BY" state or not.
void DispInit(bool IsStandBy);

// Display de-initialization.
// Indicate whether the system in "STAND-BY" state or not.
void DispDeInit(void);

// 设置Repeat模式时调用.
void DispRepeat(void);

// 设置EQ时调用.
void DispEQ(void);

// 播放状态变化时调用(Play/Pause/Stop/FF/FB).
void DispPlayState(void);

// 文件夹功能打开/关闭时调用.
void DispFoldState(void);

// 音量设置时调用.
void DispVolume(uint8_t channel);

// Mute状态变化时调用.
void DispMute(void);

//" LOD"
void DispLoad(void);

// 文件切换时调用.
void DispDev(void);

#ifdef FUNC_RTC_EN
void DispRtc(void);
#endif

void DispNum(uint16_t Num);

// 设备切换时调用.
void DispFileNum(void);

#define DIS_PLAYTIME_PALY    0
#define DIS_PLAYTIME_PAUSE   1
void DispPlayTime(uint8_t Type);
void DispResume(void);

// 主函数调用.
void Display(uint16_t msgRecv);

//显示Power on信息
void DispPowerOn(void);

// 进入Audio显示时调用
void DispAudio(void);

#endif
