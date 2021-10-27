#include "type.h"										//include project header files
#include "app_config.h"
#include "display.h"
#include "seg_panel.h"
#include "seg_lcd_disp.h"
#include "seg_led_disp.h"
#include "sled_disp.h"
#include "lcd_seg.h"
#include "debug.h"

#ifdef DISP_DEV_7_LED

#define CHAR_A_OFFSET	10
#define MASK_STRING		0x7F	
#define MAX_STRING_LEN	4

DISP_TYPE DispCharSet[] =
{
	NUM_0, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9,
	CHR_A, CHR_B, CHR_C, CHR_D, CHR_E, CHR_F, CHR_G, CHR_H, CHR_I, CHR_J,
	CHR_K, CHR_L, CHR_M, CHR_N, CHR_O, CHR_P, CHR_Q, CHR_R, CHR_S, CHR_T,
	CHR_U, CHR_V, CHR_W, CHR_X, CHR_Y, CHR_Z,
};

bool DispChar(DISP_TYPE Char, uint8_t DisCharPos)
{
	DISP_TYPE DispVal;

	uint8_t DispOrd[5] = {0, 1, 2, 3, 4};

	if(DisCharPos >= MAX_STRING_LEN)
	{
		return FALSE;
	}

    if((Char <= 'Z') && (Char >= 'A'))
	{
		DispVal = DispCharSet[Char - 'A' + CHAR_A_OFFSET];
		//APP_DBG(("Char:%C, off:%bd, Val:%bx\n", Char, (Char - 'A' + CHAR_A_OFFSET), DispVal));
	}
	else if((Char <= 'z') && (Char >= 'a'))
	{
		DispVal = DispCharSet[Char - 'a' + CHAR_A_OFFSET];
	}
	else if((Char <= '9') && (Char >= '0'))
	{
		DispVal = DispCharSet[Char - '0'];
		//APP_DBG(("Char:%C, off:%bd, Val:%bx\n", Char, (Char - '0'), DispVal));
	}
	else if((Char <= 9) && (Char >= 0))
	{
		DispVal = DispCharSet[Char];
		//APP_DBG(("Char:%C, off:%bd, Val:%bx\n", Char, (Char), DispVal));
	}
	else if(Char == '-')
	{
		DispVal = CHR_;
	}
	else
	{
		DispVal = CHR_NUL;
		//APP_DBG(("%C, Val:%bx\n", c, DispVal));
	}
/*
	if((DISP_DEV_LED1888 == gDisplayMode) && (DisCharPos == POS_COM_THOUSAND))
	{
		//if((DispVal == 1) || (DispVal == '1') || (DispVal == 'I'))
		if((DispVal == NUM_1) || (DispVal == CHR_I))
		{
			DispVal = NUM_7;	// 针对LED1888千位1显示可能为AB,AC不同情况而做修订。
		}
		else
		{
			//APP_DBG(("1888,!1\n"));
			DispVal = CHR_NUL;	
		}	
	}
*/
	DispVal |= gDispBuff[DispOrd[DisCharPos]] & (~MASK_STRING);	//保留ICON状态.
	gDispBuff[DispOrd[DisCharPos]] = DispVal;
	//APP_DBG(("gDispBuff[%bd]: %bx\n", uDisCharPos, gDispBuff[uDisCharPos]));

	return TRUE;
}


void DispString(char* Str)
{
	uint8_t CharCnt = 0;
	while(*Str) 
	{
		if(!DispChar(*Str++, CharCnt++))
		{
			break;
		}
	}
}


void DispIcon(ICON_IDX Icon, ICON_STATU Light)
{
	DispLedIcon(Icon, Light);
}


void Num2Char(uint16_t Num, bool ZeroFill)
{
	uint8_t Cnt;
	uint8_t DigitNum;
	DISP_TYPE Char;

	if(Num > 9999)
	{
		DigitNum = 5;
	}
	if(Num > 999)
	{
		DigitNum = 4;
	}
	else if(Num > 99)
	{
		DigitNum = 3;
	}
	else if(Num > 9)
	{
		DigitNum = 2;
	}
	else
	{
		DigitNum = 1;
	}
			
	for(Cnt = 0; Cnt < MAX_STRING_LEN; Cnt++)
	{
		Char = Num % 10;
		Num /= 10;
		if((Char == 0) && (Cnt >= DigitNum))
		{	
			if(ZeroFill == TRUE)
			{
				Char = '0';
			}
			else
			{
				Char = ' ';
			}
		}
		else
		{
			Char += '0';
		}

//		// 数值千、百、十、个在显示Buffer中顺序
//		BYTE CODE DigitOrder = {0, 1, 2, 3};
//		DispChar(Char, DigitOrder[Cnt]);
		//当千、百、十、个位值在Buffer中次序为3,2,1,0时有效。
		//APP_DBG(("%bd, %bd(%bd), Num:%u\n", Cnt, Char, (Char-'0'), Num));
		DispChar(Char, (MAX_STRING_LEN - Cnt - 1));
	}
}

#endif
