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
#define INTERIM_INTERVAL		2000	// EQ��Repeat��Volume��ʾ����״̬
#define SET_INTERVL				5000	// RTC���õ���ʾ״̬����ʱ��

typedef struct _ST_BLINK
{
	uint8_t MuteBlink		:1;	//������SLED MP3 PAUSE/STOP/MUTE��˸��־(������˸)
	uint8_t HourSet		:1;	//������SLED MP3�������/Radio��̨��˸��־(������˸)
	uint8_t MinSet			:1;	//������SLED MP3������˸��־(������˸)
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

// ����Repeatģʽʱ����.
void DispRepeat(void);

// ����EQʱ����.
void DispEQ(void);

// ����״̬�仯ʱ����(Play/Pause/Stop/FF/FB).
void DispPlayState(void);

// �ļ��й��ܴ�/�ر�ʱ����.
void DispFoldState(void);

// ��������ʱ����.
void DispVolume(uint8_t channel);

// Mute״̬�仯ʱ����.
void DispMute(void);

//" LOD"
void DispLoad(void);

// �ļ��л�ʱ����.
void DispDev(void);

#ifdef FUNC_RTC_EN
void DispRtc(void);
#endif

void DispNum(uint16_t Num);

// �豸�л�ʱ����.
void DispFileNum(void);

#define DIS_PLAYTIME_PALY    0
#define DIS_PLAYTIME_PAUSE   1
void DispPlayTime(uint8_t Type);
void DispResume(void);

// ����������.
void Display(uint16_t msgRecv);

//��ʾPower on��Ϣ
void DispPowerOn(void);

// ����Audio��ʾʱ����
void DispAudio(void);

#endif
