/**
 *******************************************************************************
 * @file    ffpresearch.c
 * @author  Castle
 * @version V0.1.0
 * @date    24-August-2017
 * @brief
 * @maintainer Castle
 *******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#include <stdio.h>
#include <string.h>
#include "type.h"
#include "diskio.h"
#include "ff.h"
#include "ffconf.h"
#include "integer.h"
#include "ffpresearch.h"

#ifdef CFG_APP_CONFIG
extern uint32_t GetSystemMode(void);
#include "main_task.h"
#include "app_config.h"
#endif

//注意如果是Demo，则需要注意搜索到的文件类型是否符合预期
#ifndef CFG_APP_CONFIG
#define USE_MP3_DECODER
#define USE_WMA_DECODER
#define USE_SBC_DECODER
#define USE_WAV_DECODER
#define USE_FLAC_DECODER
#define USE_AAC_DECODER
#define USE_AIF_DECODER
#define USE_AMR_DECODER
#define USE_APE_DECODER
#endif

#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define IsUpper(c)	((c) >= 'A' && (c) <= 'Z')
#define IsLower(c)	((c) >= 'a' && (c) <= 'z')
#define IsDigit(c)	((c) >= '0' && (c) <= '9')


/**
 * @func        get_audio_type
 * @brief       Find file type
 * @param       TCHAR *string: filename
 * @Output      None
 * @return      FileType:Supported type or FILE_TYPE_UNKOWN
 * @Others      
 * Record
 */

FileType get_audio_type(TCHAR *string)
{
	UINT len;

	len = strlen(string);
	while(--len)
	{
		if(string[len] == '.')
			break;
	}

	len ++;
	if (IsLower(string[len])) string[len] -= 0x20;
	if (IsLower(string[len + 1])) string[len + 1] -= 0x20;
	if (IsLower(string[len + 2])) string[len + 2] -= 0x20;
	if (IsLower(string[len + 3])) string[len + 3] -= 0x20;
#ifdef USE_MP3_DECODER
	if((string[len + 0] == 'M') && (string[len + 1] == 'P') && (string[len + 2] == '3' || string[len + 2] == '2'))
	{
		return FILE_TYPE_MP3;
	}
#endif
#ifdef USE_WMA_DECODER
	if((string[len + 0] == 'W') && (string[len + 1] == 'M') && (string[len + 2] == 'A'))
	{
		return FILE_TYPE_WMA;
	}
	if((string[len + 0] == 'A') && (string[len + 1] == 'S') && (string[len + 2] == 'F'))
	{
		return FILE_TYPE_WMA;
	}
	if((string[len + 0] == 'W') && (string[len + 1] == 'M') && (string[len + 2] == 'V'))
	{
		return FILE_TYPE_WMA;
	}
	if((string[len + 0] == 'A') && (string[len + 1] == 'S') && (string[len + 2] == 'X'))
	{
		return FILE_TYPE_WMA;
	}
#endif
#ifdef USE_WAV_DECODER
	if((string[len + 0] == 'W') && (string[len + 1] == 'A') && (string[len + 2] == 'V'))
	{
		return FILE_TYPE_WAV;
	}
#endif
#ifdef USE_SBC_DECODER
	if((string[len + 0] == 'S') && (string[len + 1] == 'B') && (string[len + 2] == 'C'))
	{
		return FILE_TYPE_SBC;
	}
#endif
#ifdef USE_FLAC_DECODER
	if((string[len + 0] == 'F') && (string[len + 1] == 'L') && (string[len + 2] == 'A') && (string[len + 3] == 'C'))
	{
		return FILE_TYPE_FLAC;
	}
#endif
#ifdef USE_AAC_DECODER
	if(((string[len + 0] == 'A') && (string[len + 1] == 'A') && string[len + 2] == 'C')
	|| ((string[len + 0] == 'M') && (string[len + 1] == 'P') && string[len + 2] == '4')
	|| ((string[len + 0] == 'M') && (string[len + 1] == '4') && string[len + 2] == 'A'))
	{
		return FILE_TYPE_AAC;
	}
#endif
#ifdef USE_AIF_DECODER
	if((string[len] == 'A') && (string[len + 1] == 'I') && (string[len + 2] == 'F'))
	{
		return FILE_TYPE_AIF;
	}
#endif
#ifdef USE_APE_DECODER
	if((string[len] == 'A') && (string[len + 1] == 'P') && (string[len + 2] == 'E'))
	{
		return FILE_TYPE_APE;
	}
#endif
#if FLASH_BOOT_EN 
	if((string[len + 0] == 'M') && (string[len + 1] == 'V') && (string[len + 2] == 'A'))
	{
		if(GetSystemMode() == AppModeCardAudioPlay)
		{
			SoftFlagRegister(SoftFlagMvaInCard);
		}
		else if(GetSystemMode() == AppModeUDiskAudioPlay)
		{
			SoftFlagRegister(SoftFlagMvaInUDisk);
		}
		return FILE_TYPE_UNKOWN;
	}
#endif
	else
	{
		return FILE_TYPE_UNKOWN;
	}
}


