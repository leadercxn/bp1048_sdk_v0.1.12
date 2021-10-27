///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: bt_app_func.h
//  maintainer: Halley
///////////////////////////////////////////////////////////////////////////////

#ifndef __BT_APP_FUNC_H__
#define __BT_APP_FUNC_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "bt_config.h"

typedef unsigned char (*BBSniffNotifyCallback)(void);

typedef struct _BtBbParams
{
	uint8_t		*localDevName;
	uint8_t		localDevAddr[BT_ADDR_SIZE];
	uint8_t		freqTrim;
	uint32_t	em_start_addr;

	//bb agc config
	uint8_t		pAgcDisable;
	uint8_t		pAgcLevel;
	
	//sniff config
	uint8_t		pSniffNego;
	uint16_t	pSniffDelay;
	uint16_t	pSniffInterval;
	uint16_t	pSniffAttempt;
	uint16_t	pSniffTimeout;
	
	BBSniffNotifyCallback	bbSniffNotify;
}BtBbParams;

/**
 * @brief  bt stack memory init
 * @param  NONE
 * @return NONE
 * @Note 
 *
 */
void BtStackMemoryInit(void);

/**
 * @brief  bt stack init
 * @param  NONE
 * @return TRUE - init success
 * @Note 
 *
 */
bool BtStackInit(void);

/**
 * @brief  bt stack run uninit
 * @param  NONE
 * @return NONE
 * @Note 
 *
 */
void UninitBt(void);

/**
 * @brief  bt stack uninit
 * @param  NONE
 * @return TRUE - uninit success
 * @Note 
 *
 */
bool BtStackUninit(void);

uint32_t GetSupportProfiles(void);


void ConfigBtBbParams(BtBbParams *params);
void LoadBtConfigurationParams(void);



#ifdef __cplusplus
}
#endif//__cplusplus

#endif/*__BT_APP_FUNC_H__*/

