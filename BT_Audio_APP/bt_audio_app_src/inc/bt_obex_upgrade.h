/**
 *******************************************************************************
 * @file    bt_obex_upgrade.h
 * @author  KK
 * @version V1.0.0
 * @date    12-01-2020
 *******************************************************************************
 */

#ifndef _BT_OBEX_UPGRADE_H_
#define _BT_OBEX_UPGRADE_H_

#include "bt_obex_api.h"

//void DualBankUpdateReboot(void);

void ObexUpdateProc(const BT_OBEX_CALLBACK_PARAMS *Info);

void ObexUpgradeStart(void);

void ObexUpgradeEnd(void);

#endif

