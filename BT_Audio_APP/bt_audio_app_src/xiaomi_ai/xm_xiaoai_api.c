/*
 * xm_xiaoai_api.c
 *
 *  Created on: Nov 27, 2019
 *      Author: tony
 */

#include <stdio.h>
#include "type.h"
#include "xm_auth.h"
#include "aivs_rcsp.h"
#include "xm_xiaoai_api.h"
#include "app_config.h"

#ifdef	CFG_XIAOAI_AI_EN
void xiaoai_app_decode(uint8_t length, uint8_t *pValue)
{
	aivs_app_decode(length, pValue);
}

void xiaoai_app_disconn()
{
	xm_ai_deinit();
	xm_speech_con_set(0);
	set_communicate_way(CURRENT_COMMUNICATE_BLE);
}

#endif	//CFG_XIAOAI_AI_EN
