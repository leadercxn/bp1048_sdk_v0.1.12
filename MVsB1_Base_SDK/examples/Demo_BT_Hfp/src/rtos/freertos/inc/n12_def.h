
/**
* @addtogroup FreeRTOS
* @{
* @defgroup n12_def n12_def.h
* @{
*/

#ifndef __N12_DEF_H__
#define __N12_DEF_H__

#include "nds32_defs.h"

#ifndef __ASSEMBLER__

#include "nds32_regs.h"

#define TLBOP(op)		__asm__ volatile ("tlbop "#op);
#define MSYNC(subtype)		__asm__ volatile ("msync "#subtype);
#define STANDBY(cond)		__asm__ volatile ("standby "#cond);

extern inline void GIE_ENABLE(); //defined @startup interrupt.c
extern inline void GIE_DISABLE(); //defined @startup interrupt.c
/* 
 * Static inline function would be traded as macro function.
 * It would be inlined into caller text object and wouldn't 
 * create an independent text object. 
 */
static inline void ISB(void)
{
	__asm__ volatile ("isb");
}

static inline void DSB(void)
{
	__asm__ volatile ("dsb");
}

 /*inline void GIE_ENABLE(void)
{
	__asm__ volatile ("setgie.e \n\t"
			// Junior 2014.08.04
			// add isb for -flto
			// -flto would generate special code path
			// ex :
			// 	setgie.e	(no dsb cause mfsr get old value)
			// 	nop
			// 	mfsr $r0, $psw
			"	dsb	\n\t");
}

 inline void GIE_DISABLE(void)
{
        __asm__ volatile (
			"	setgie.d	\n\t"
			"	dsb		\n\t"
			);
}*/

static inline void GIE_SAVE(unsigned long *var)
{
	*var = GET_PSW();
	GIE_DISABLE();
}

static inline void GIE_RESTORE(unsigned long var)
{
	if (var & PSW_mskGIE)
		GIE_ENABLE();
}

#endif /* __ASSEMBLER__ */

#endif /* __N12_DEF_H__ */
