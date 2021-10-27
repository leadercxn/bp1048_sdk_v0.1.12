/**
 **************************************************************************************
 * @file    power_key.h
 * @brief   power key
 *
 * @author  Tony
 * @version V1.0.0
 *
 * $Created: 2019-10-12 14:00:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __PWR_KEY_H__
#define __PWR_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
#include "type.h"


typedef enum _PWRKeyType
{
	PWR_KEY_UNKOWN_TYPE = 0,
	PWR_KEY_SP,
}PWRKeyType;

typedef struct _PWRKeyMsg
{
    uint16_t index;
    PWRKeyType type;
}PWRKeyMsg;


PWRKeyMsg PowerKeyScan(void);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__PWR_KEY_H__
