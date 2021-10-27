#ifndef __SEG_PANEL_H__
#define __SEG_PANEL_H__

#include "type.h"
#include "app_config.h"

#ifdef DISP_DEV_7_LED

#define DISP_TYPE	uint8_t

#define POS_COM_THOUSAND	0	// COM0	// D6
#define POS_COM_HUNDREDS	1	// COM1	// D5
#define POS_COM_TENS		2	// COM2	// D4
#define POS_COM_UNITS		3	// COM3	// D3

//SEG定义
#define SEGA_BITNO	0
#define SEGB_BITNO	1
#define SEGC_BITNO	2
#define SEGD_BITNO	3
#define SEGE_BITNO	4
#define SEGF_BITNO	5
#define SEGG_BITNO	6
#define SEGH_BITNO	7

#define	SEG_A	(1 << SEGA_BITNO)
#define	SEG_B	(1 << SEGB_BITNO)
#define	SEG_C	(1 << SEGC_BITNO)
#define	SEG_D	(1 << SEGD_BITNO)
#define	SEG_E	(1 << SEGE_BITNO)
#define	SEG_F	(1 << SEGF_BITNO)
#define	SEG_G	(1 << SEGG_BITNO)
#define	SEG_H	(1 << SEGH_BITNO)

#define	NUM_0	(SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define	NUM_1	(SEG_B | SEG_C)
#define	NUM_2	(SEG_A | SEG_B | SEG_D | SEG_E | SEG_G)
#define	NUM_3	(SEG_A | SEG_B | SEG_C | SEG_D | SEG_G)
#define	NUM_4	(SEG_B | SEG_C | SEG_F | SEG_G)
#define	NUM_5	(SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define	NUM_6	(SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define	NUM_7	(SEG_A | SEG_B | SEG_C)
#define	NUM_8	(SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define	NUM_9	(SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)

#define	CHR_A	(SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define	CHR_B	(SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define	CHR_C	(SEG_A | SEG_D | SEG_E | SEG_F)
#define	CHR_D	(SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)

#define	CHR_E	(SEG_A | SEG_D | SEG_E | SEG_F | SEG_G)
#define	CHR_F	(SEG_A | SEG_E | SEG_F | SEG_G)
#define	CHR_G	(SEG_A | SEG_C | SEG_D | SEG_E | SEG_F)
#define	CHR_H	(SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
//#define	CHR_H	(SEG_C | SEG_E | SEG_F | SEG_G)			//为了与X(显示位H)区别开，H显示位'h'
#define	CHR_I	(SEG_E | SEG_F)
#define	CHR_J	(SEG_B | SEG_C | SEG_D)
#define	CHR_K	(SEG_D | SEG_E | SEG_F | SEG_G)
#define	CHR_L	(SEG_D | SEG_E | SEG_F)
#define	CHR_M	(SEG_A | SEG_B | SEG_E | SEG_F)
#define	CHR_N	(SEG_A | SEG_B | SEG_C | SEG_E | SEG_F)
#define	CHR_O	(SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define	CHR_P	(SEG_A | SEG_B | SEG_E | SEG_F | SEG_G)
#define	CHR_Q	(SEG_A | SEG_B | SEG_C | SEG_F | SEG_G)
#define	CHR_R	(SEG_E | SEG_F | SEG_G)
#define	CHR_S	(SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define	CHR_T	(SEG_D | SEG_E | SEG_F | SEG_G)
#define	CHR_U	(SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define	CHR_V	(SEG_C | SEG_D | SEG_E)
#define	CHR_W	(SEG_C | SEG_D | SEG_E | SEG_F)
#define	CHR_X	(SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define	CHR_Y	(SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)
#define	CHR_Z	(SEG_A | SEG_D)
#define	CHR_	(SEG_G)
#define	CHR_NUL 0

#define	ICON57_PLAY		(4 * 8 + SEGA_BITNO)	//COM5 SEGA	//sizeof(DISP_TYPE)*COMn+SEGA_BITNO
#define	ICON57_PAUSE	(4 * 8 + SEGB_BITNO)	//COM5 SEGB
#define ICON57_USB		(4 * 8 + SEGC_BITNO) 	//COM5 SEGC
#define ICON57_SD		(4 * 8 + SEGD_BITNO)	//COM5 SEGD
#define ICON57_COLON	(4 * 8 + SEGE_BITNO)	//COM5 SEGE
#define ICON57_FM_MHZ	(4 * 8 + SEGF_BITNO)	//COM5 SEGF
#define ICON57_MP3		(4 * 8 + SEGG_BITNO)	//COM5 SEGG

// 请根据POWER指示实际的图标笔段设置
#define ICON57_SLEEP	0xFF	//COM5 SEGD
#define ICON57_BATFUL	0xFF	//COM5 SEGF
#define ICON57_BATHAF	0xFF	//COM5 SEGG

typedef enum _ICON_IDX
{
	ICON_PLAY = 0,  	// 0
	ICON_PAUSE,	  		// 1
	ICON_COLON,	 	 	// 2
	ICON_USB,		  	// 3
	ICON_SD,			// 4
	ICON_DP_FM_MHZ,		//5
	ICON_MP3,		  	//6
	ICON_REP,		  	//7
	ICON_RAN,		  	//8
	ICON_INTO,	  		//9
	ICON_MUTE,	  		//10
	ICON_L1,		  	//11
	ICON_L2,		  	//12
	ICON_L3,		  	//13
	ICON_L4,		  	//14
	ICON_SLEEP,			//15
	ICON_BATFUL,		//16
	ICON_BATHAF,		//17
	ICON_NUM

} ICON_IDX;

typedef enum _ICON_STATU
{
	LIGHT_OFF,
	LIGHT_ON,
	LIGHT_BLINK

} ICON_STATU;

extern DISP_TYPE gDispBuff[];
extern uint16_t DispIconSet[];
extern DISP_TYPE DispCharSet[];

bool DispChar(DISP_TYPE Char, uint8_t DisCharPos);
void DispString(char* Str);
void DispIcon(ICON_IDX Icon, ICON_STATU Light);
void Num2Char(uint16_t Num, bool ZeroFill);
#endif

#endif
