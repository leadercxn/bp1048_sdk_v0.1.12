/**
 *****************************************************************************
 * @file     flash_interface.h
 * @author   Peter
 * @version  V0.0.1
 * @date     2019.4.4
 * @brief    spi code flash module driver interface for B1X
 * @brief    spi code flash driver header file
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2019 MVSilicon </center></h2>
 */

 /**
 * @addtogroup FLASH_INTERFACE
 * @{
 * @defgroup flash_interface flash_interface.h
 * @{
 */

#ifndef SRC_FLASH_INTERFACE_H_
#define SRC_FLASH_INTERFACE_H_

#include "type.h"
#include "spi_flash.h"


/**
 * @brief  read data from flash
 * @param  StartAddr the data addr need to read for flash
 * @param  Buffer the data addr need to save for MCU's mem
 * @param  Length data length
 * @param  TimeOut read data but fifo always empty
 * @return SPI_FLASH_ERR_CODE
 *         @arg  FIFO_ALWAYS_EMPTY_ERR  function execute time out
 *         @arg  DATALEN_LESS_THEN_ZERO_ERR    read data but data len is < 0
 *         @arg  NONE_ERR
 */
SPI_FLASH_ERR_CODE SpiFlashReadWpr( uint32_t StartAddr, uint8_t* Buffer, uint32_t Length, uint32_t TimeOut);

/**
 * @brief  write data into flash
 * @param  Addr the data addr need to write for flash
 * @param  Buffer the data addr need to read for MCU's mem
 * @param  Length data length
 * @param  TimeOut read data but fifo always empty
 * @return SPI_FLASH_ERR_CODE
 *         @arg  FIFO_ALWAYS_FULL_ERR  function execute time out
 *         @arg  DATALEN_LESS_THEN_ZERO_ERR    read data but data len is < 0
 *         @arg  NONE_ERR
 */
SPI_FLASH_ERR_CODE SpiFlashWriteWpr( uint32_t	Addr, uint8_t	*Buffer, uint32_t 	Length, uint32_t TimeOut);



/**
 * @brief  flash erase
 * @param  EraseType:	see ERASE_TYPE_ENUM type
 *										CHIP_ERASE:		Erase whole chip
 *										SECTOR_ERASE:	Erase One sector
 *										BLOCK_ERASE:	Erase One block
 *				  					for CHIP_ERASE type, param 2 & param 3 are NOT cared.
 * @param  Index:
 *										for SECTOR_ERASE type: 	means sector number
 *										for BLOCK_ERASE type: 	means block number
 * @param  IsSuspend: during flash write or sector/block erase need suspend set it to 1
 * @return NONE
 */
void SpiFlashEraseWpr(ERASE_TYPE_ENUM erase_type, uint32_t index, bool IsSuspend);

/**
 * @brief	擦除flash
 * @param	Offset	 擦除地址，必须4KB对齐
 * @param	Size	擦除size，必须4KB对齐
 * @return	ERASE_FLASH_ERR-擦除失败	FLASH_NONE_ERR-擦出成功
 * @note
 */
int32_t FlashEraseWpr(uint32_t Offset,uint32_t Size);

/**
 * @brief	调整flash时钟频率
 * @param	ClkSrc      时钟源选择
 * @param	FlashClk    flash时钟频率
 * @return	SWITCH_FLASH_CLK_ERR-失败	FLASH_NONE_ERR-成功
 * @note
 */
SPI_FLASH_ERR_CODE SpiFlashClkSwitchWpr(FSHC_CLK_MODE ClkSrc, uint32_t FlashClk);

/**
 * @brief  get spi flash information
 * @param  none
 * @return return the SPI_FLASH_INFO pointer
 * @note this function should be called after SpiFlashInit
 */
SPI_FLASH_INFO* SpiFlashInfoGetWpr(void);

/**
 * @brief	设置resume时间
 * @param	time 单位：us
 * @return	SWITCH_FLASH_CLK_ERR-失败	FLASH_NONE_ERR-成功
 * @note 如果erase和write时使用suspend功能，需要设置
 */
SPI_FLASH_ERR_CODE SpiFlashResumDelaySetWpr(uint32_t time);

#endif /* SRC_FLASH_INTERFACE_H_ */
