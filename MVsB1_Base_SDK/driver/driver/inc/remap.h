/**
  ******************************************************************************
  * @file    remap.h
  * @author  Peter Zhai
  * @version V1.0
  * @date    2017-10-27
  * @brief
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/**
 * @addtogroup SYSTEM
 * @{
 * @defgroup remap remap.h
 * @{
 */

#ifndef __REMAP_H__
#define __REMAP_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include"debug.h"

#define REMAP_DBG(format, ...)		//DBG(format, ##__VA_ARGS__)

#define FLASH_ADDR 			0x0
#define TCM_SIZE   			12
#define SRAM_END_ADDR		0x20048000
#define TCM_SRAM_START_ADDR	0x20000000

extern uint32_t gSramEndAddr;
typedef enum
{
	ADDR_REMAP0=0,
	ADDR_REMAP1=1,
	ADDR_REMAP2
} ADDR_REMAP_ID;

typedef enum
{
	REMAP_UNALIGNED_ADDRESS = -1,
	REMAP_OK = 0
}REMAP_ERROR;

/**
 * @brief	使能remap功能
 * @param	num	 有ADDR_REMAP1、ADDR_REMAP2，指定一个即可
 * @param	src	 源地址，必须4KB对齐
 * @param	dst	 目的地址，必须4KB对齐
 * @param	size 单位是KB，必须4KB对齐
 * @return	错误码，详情见REMAP_ERROR
 * @note
 */
REMAP_ERROR Remap_AddrRemapSet(ADDR_REMAP_ID num, uint32_t src, uint32_t dst, uint32_t size);

/**
 * @brief	关闭remap功能
 * @param	num	 有ADDR_REMAP1、ADDR_REMAP2，指定一个即可
 * @return	错误码，详情见REMAP_ERROR
 * @note
 */
void Remap_AddrRemapDisable(ADDR_REMAP_ID num);

/**
 * @brief	使能TCM功能，主要是将flash驱动映射到memcpy中
 * @param	StartAddr	源地址，必须4KB对齐
 * @param	size 		单位是KB，必须4KB对齐
 * @return	错误码，详情见REMAP_ERROR
 * @note
 */
REMAP_ERROR Remap_InitTcm(uint32_t StartAddr, uint32_t size);

/**
 * @brief	禁能TCM功能
 * @param	None
 * @return	None
 * @note
 */
void Remap_DisableTcm(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__REMAP_H__
/**
 * @}
 * @}
 */
