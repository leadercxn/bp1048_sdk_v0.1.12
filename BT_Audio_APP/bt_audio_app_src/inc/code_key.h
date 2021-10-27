/**
 **************************************************************************************
 * @file    code_key.h
 * @brief   code key 
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-1-17 16:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */


#ifndef __CODE_KEY_H__
#define __CODE_KEY_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include <type.h>

typedef enum _CodeKeyType
{
    CODE_KEY_NONE = 0,
    CODE_KEY_FORWARD,
    CODE_KEY_BACKWARD,
}CodeKeyType;

// Initialize coding key scan (GPIO) operation.
// Config interrupt at negative edge of signal-A
void CodeKeyInit(void);

// Key process, image key value to key event.
CodeKeyType CodeKeyScan(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__CODE_KEY_H__
