///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: string_convert.c
//  maintainer: Halley
///////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include "type.h"
#include "string_convert.h"
#include "ff.h"
//#include "character_encoding.h"
#include "debug.h"

#define SWAP_WORD(Val) (uint16_t)((Val<<8) | (Val>>8))
#define		TAB_4E00_9FA5_SZ	(20902)


static uint16_t unicode_to_gbk(uint16_t src)
{
	uint16_t Result;

	//if(0x4E00 <= src && src <= 0x9FA5)
	//{
		/*
		* search one-one index table directly
		*/
		if(src <= 0x00ff)
			return src;
		Result = ff_uni2oem(ff_wtoupper(src), 936);
		return SWAP_WORD(Result);
	//}
//	if(0xff == (unic2gbk_segtab[segidx]))
//	{
//		/*
//		* no segment map
//		*/
//		return src;
//	}
//
//	segidx = unic2gbk_segtab[segidx];
//	pgidx  = (src >> 4) & 0x0f;
//	mask   = *(unsigned short*)&unic2gbk_pgtab[16 * segidx * 3 + 3 * pgidx + 1];
//
//	if(!(mask & (1 << (src & 0x0f))))
//	{
//		/*
//		* empty page table
//		*/
//		return src;
//	}
//
//	/*
//	* segment offset puls page offset
//	*/
//	offset  = unic2gbk_segoff[segidx] + unic2gbk_pgtab[16 * segidx * 3 + 3 * pgidx];
//	itemidx = src & 0x0f;
//
//	for(i = 0; i < itemidx; i++)
//	{
//		offset += (mask >> i) & 0x01;
//	}
//
//	if(0x9FA5 < src)
//	{
//		offset -= TAB_4E00_9FA5_SZ;
//	}
//
//	Result = unic2gbk[offset];
//
//	return SWAP_WORD(Result);
}


/**********************************************
Convert char to special encode format (GBK, UNICODE, ...)
Param:
    SrcChar - Input - Character code to be converted
    Type    - Input - Convert Option: UNICODE_TO_GBK, GBK_TO_UNICODE, ...
Return:
    Converted code, 0 means conversion error
**********************************************/
uint16_t CharConvert(uint16_t SrcChar, uint32_t Type)
{
	uint16_t DstChar;

	if(SrcChar < 0x80)
	{
		/* ASCII */
		return SrcChar;
	}

	switch(Type)
	{
		case UNICODE_TO_GBK:
			DstChar = unicode_to_gbk(SrcChar);
			break;
		case GBK_TO_UNICODE:
		default:
			DstChar = 0;
			break;
	}

	return DstChar;
}


/**********************************************
Convert string to special encode format (GBK, UNICODE, ...)
Param:
    DstText   - Output - converted string
    MaxDstLen - Input  - DstText's max Length(Byte)
    SrcChar   - Input  - String to be converted
    SrcLen    - Input  - SrcChar's length (Byte)
    Type      - Input  - Convert option: UNICODE_TO_GBK, GBK_TO_UNICODE, ...
Return:
    Converted string length, 0 means conversion error
**********************************************/
uint32_t StringConvert(uint8_t* DstText, uint32_t MaxDstLen,
                       uint8_t* SrcText, uint32_t SrcLen, uint32_t Type)
{
	uint32_t i;
	uint32_t DstTextLen;
//	int16_t Data;

	if(DstText == NULL ||
	        SrcText == NULL || SrcLen == 0 || MaxDstLen == 0)
	{
		return 0;
	}

	DstTextLen = 0;

	switch(Type)
	{
		case UTF8_TO_GBK:
			{
				uint16_t TmpValue;
				for(i = 0; i < SrcLen && i < MaxDstLen;)
				{
					if(SrcText[i] < 0x80)
					{
						DstText[DstTextLen++] = SrcText[i++];
					}
					else if(SrcText[i] < 0xC2)
					{
						i++;
					}
					else if(SrcText[i] < 0xE0)
					{
						if(i >= (SrcLen - 1))
						{
							break;
						}
						TmpValue = unicode_to_gbk((uint16_t)(((SrcText[i] & 0x1F) << 6) | (SrcText[i + 1] & 0x3F)));
						if(TmpValue)
						{
							DstText[DstTextLen++] = (uint8_t)(TmpValue & 0xFF);
							DstText[DstTextLen++] = (uint8_t)(TmpValue >> 8);
						}

						i += 2;
					}
					else if(SrcText[i] < 0xF0)
					{
						if(i >= (SrcLen - 2))
						{
							break;
						}

						TmpValue = unicode_to_gbk((uint16_t)(((SrcText[i] & 0x0F) << 12) |
						                                     ((SrcText[i + 1] & 0x3F) << 6) |
						                                     (SrcText[i + 2] & 0x3F)));
						if(TmpValue)
						{
							DstText[DstTextLen++] = (uint8_t)(TmpValue & 0xFF);
							DstText[DstTextLen++] = (uint8_t)(TmpValue >> 8);
						}

						i += 3;
					}
					else
					{
						i += 4;
					}
				}
			}
			break;
		case UNICODE_BIG_TO_GBK:
		case UNICODE_TO_GBK:
			// 不支持Ascii码与Unicode码混合的情况
			{
				uint16_t Ret, TmpVar;
				for(i = 0; i < (SrcLen) && i < (MaxDstLen); i += 2)
				{
					if(UNICODE_TO_GBK == Type)
					{
						TmpVar = *((uint16_t*)&SrcText[i]);

					}
					else
					{
						TmpVar = SWAP_WORD(*((uint16_t*)&SrcText[i]));
					}

					Ret = unicode_to_gbk(TmpVar);
					if(Ret != TmpVar || Ret > 0xFF)
					{
						*(uint16_t*)(DstText + DstTextLen) = Ret;
						DstTextLen += 2;
					}
					else
					{
						DstText[DstTextLen++] = (uint8_t)Ret;
					}
				}
			}
			break;
		case UNKNOWN_CONVERT:
			memcpy(DstText, SrcText, (MaxDstLen > SrcLen ? SrcLen : MaxDstLen));
			break;
		default:
//			return UserPlusStringConvert(DstText, MaxDstLen, SrcText, SrcLen, Type);
			break;
	}

	return DstTextLen;
}

