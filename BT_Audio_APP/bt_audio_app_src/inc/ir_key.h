/**
 **************************************************************************************
 * @file    ir_key.h
 * @brief   ir key
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2018-3-15 14:03:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __IR_KEY_H__
#define __IR_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
#include "type.h"


#define 	IR_KEY_NONE               0xFF
#define		IR_KEY_POWER				0 //必须与ir_key.c 键值表位置配合


typedef enum _IRKeyType
{
	IR_KEY_UNKOWN_TYPE = 0,
	IR_KEY_PRESSED,
	IR_KEY_RELEASED,
	IR_KEY_LONG_PRESSED,
	IR_KEY_LONG_PRESS_HOLD,
	IR_KEY_LONG_RELEASED,
}IRKeyType;

typedef struct _IRKeyMsg
{
    uint16_t index;
    uint16_t type;
}IRKeyMsg;

uint8_t IRKeyIndexGet(void);

IRKeyMsg IRKeyScan(void);

int32_t IRKeyInit(void);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__IR_KEY_H__

