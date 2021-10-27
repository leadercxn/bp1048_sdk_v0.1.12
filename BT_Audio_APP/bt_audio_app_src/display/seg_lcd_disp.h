#ifndef __SEG_LCD_DISP_H__
#define __SEG_LCD_DISP_H__


#ifdef FUNC_SEG_LCD_EN

typedef enum _AMPL_CURVE
{
	Ampl_Zero,
	Ampl_One,
	Ampl_Two,
	Ampl_Three,
	Ampl_Four
		
} AMPL_CURVE;

VOID LcdFlushDispData(VOID);
VOID DispLcdIcon(ICON_IDX Icon, ICON_STATU Light);
VOID LcdDispDevSymbol(VOID);
VOID LcdDispRepeat(VOID);
VOID LcdDispPlayState(VOID);


#endif

#endif
