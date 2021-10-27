/**
 **************************************************************************************
 * @file    misc.h
 * @brief   
 * 
 * @author  
 * @version 
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __MISC_H__
#define __MISC_H__

#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

void DetectMic(void);
void HWDeviceDected(void);
void HWDeviceDected_Init(void);
void ShunningModeProcess(void);
void SoftPowerInit(void);
void WaitSoftKey(void);
void SoftKeyPowerOff(void);
void DetectEarPhone(void);
void DetectMic3Or4Line(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__CODE_KEY_H__
