
/**
* @addtogroup FreeRTOS
* @{
* @defgroup os_cpu os_cpu.h
* @{
*/

#ifndef OS_CPU_H
#define OS_CPU_H

#include "n12_def.h"
/*
 * *********************************************************************************************************
 * *                                              DATA TYPES
 * *                                         (Compiler Specific)
 * *********************************************************************************************************
 * */
typedef unsigned char   BOOLEAN;
typedef unsigned char   INT8U;                  /* Unsigned  8 bit quantity                           */
typedef signed   char   INT8S;                  /* Signed    8 bit quantity                           */
typedef unsigned short  INT16U;                 /* Unsigned 16 bit quantity                           */
typedef signed   short  INT16S;                 /* Signed   16 bit quantity                           */
typedef unsigned int    INT32U;                 /* Unsigned 32 bit quantity                           */
typedef signed   int    INT32S;                 /* Signed   32 bit quantity                           */
typedef float           FP32;                   /* Single precision floating point                    */
typedef double          FP64;                   /* Double precision floating point                    */

typedef unsigned long   OS_STK;                 /* Each stack entry is 32-bit wide                    */
typedef unsigned long   OS_CPU_SR;              /* Define size of CPU status register (PSR = 32 bits) */

void OSCtxSw(void);
void CtxSave(void);
void CtxRestore(void);

#endif
