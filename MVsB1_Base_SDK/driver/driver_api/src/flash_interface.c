/**
 **************************************************************************************
 * @file    flash_interface.c
 * @brief   flash interface
 *
 * @author  Peter
 * @version V1.0.0
 *
 * $Created: 2019-05-30 11:30:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "type.h"
#include "spi_flash.h"
#include <stdarg.h>

#ifdef FUNC_OS_EN
#include "rtos_api.h"

osMutexId FlashMutex = NULL;
#endif

//SPI_FLASH_ERR_CODE SpiFlashInitWpr(uint32_t flash_clk, SPIFLASH_IO_MODE IOMode, bool HpmEn, FSHC_CLK_MODE ClkSrc)
//{
//	SPI_FLASH_ERR_CODE ret;
//#ifdef FUNC_OS_EN
//	osMutexLock(FlashMutex);
//#endif
//	ret = SpiFlashInit(flash_clk, IOMode, HpmEn, ClkSrc);
//#ifdef FUNC_OS_EN
//	osMutexUnlock(FlashMutex);
//#endif
//	return ret;
//}

SPI_FLASH_ERR_CODE SpiFlashReadWpr( uint32_t StartAddr, uint8_t* Buffer, uint32_t Length, uint32_t TimeOut)
{
	SPI_FLASH_ERR_CODE ret;
#ifdef FUNC_OS_EN
	osMutexLock(FlashMutex);
#endif
	ret = SpiFlashRead(StartAddr, Buffer, Length, TimeOut);
#ifdef FUNC_OS_EN
	osMutexUnlock(FlashMutex);
#endif
	return ret;
}


SPI_FLASH_ERR_CODE SpiFlashWriteWpr( uint32_t	Addr, uint8_t	*Buffer, uint32_t 	Length, uint32_t TimeOut)
{
	SPI_FLASH_ERR_CODE ret;
#ifdef FUNC_OS_EN
	osMutexLock(FlashMutex);
#endif
	ret = SpiFlashWrite(Addr, Buffer, Length, TimeOut);
#ifdef FUNC_OS_EN
	osMutexUnlock(FlashMutex);
#endif
	return ret;
}

void SpiFlashEraseWpr(ERASE_TYPE_ENUM erase_type, uint32_t index, bool IsSuspend)
{
#ifdef FUNC_OS_EN
	osMutexLock(FlashMutex);
#endif
	SpiFlashErase(erase_type, index, IsSuspend);
#ifdef FUNC_OS_EN
	osMutexUnlock(FlashMutex);
#endif
}

int32_t FlashEraseWpr(uint32_t Offset,uint32_t Size)
{
	SPI_FLASH_ERR_CODE ret;
#ifdef FUNC_OS_EN
	osMutexLock(FlashMutex);
#endif
	ret = FlashErase(Offset, Size);
#ifdef FUNC_OS_EN
	osMutexUnlock(FlashMutex);
#endif
	return ret;
}


SPI_FLASH_ERR_CODE SpiFlashClkSwitchWpr(FSHC_CLK_MODE ClkSrc, uint32_t FlashClk)
{
	SPI_FLASH_ERR_CODE ret;
#ifdef FUNC_OS_EN
	osMutexLock(FlashMutex);
#endif
	ret = SpiFlashClkSwitch(ClkSrc, FlashClk);
#ifdef FUNC_OS_EN
	osMutexUnlock(FlashMutex);
#endif
	return ret;
}


SPI_FLASH_INFO* SpiFlashInfoGetWpr(void)
{
	SPI_FLASH_INFO *ret;
#ifdef FUNC_OS_EN
	osMutexLock(FlashMutex);
#endif
	ret = SpiFlashInfoGet();
#ifdef FUNC_OS_EN
	osMutexUnlock(FlashMutex);
#endif
	return ret;
}

SPI_FLASH_ERR_CODE SpiFlashResumDelaySetWpr(uint32_t time)
{
	SPI_FLASH_ERR_CODE ret;
#ifdef FUNC_OS_EN
	osMutexLock(FlashMutex);
#endif
	ret = SpiFlashResumDelaySet(time);
#ifdef FUNC_OS_EN
	osMutexUnlock(FlashMutex);
#endif
	return ret;
}

//SPI_FLASH_ERR_CODE SpiFlashIOCtrlWpr(IOCTL_FLASH_T Cmd, ...)
//{
//	SPI_FLASH_ERR_CODE ret;
//	va_list List;
//#ifdef FUNC_OS_EN
//	osMutexLock(FlashMutex);
//#endif
//	va_start(List,Cmd);
//	ret = SpiFlashIOCtrl(Cmd, List);
//	va_end(List);
//#ifdef FUNC_OS_EN
//	osMutexUnlock(FlashMutex);
//#endif
//
//	return ret;
//}
