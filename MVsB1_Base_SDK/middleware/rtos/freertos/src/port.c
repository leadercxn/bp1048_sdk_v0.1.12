/*
    FreeRTOS V8.0.0 - Copyright (C) 2014 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that has become a de facto standard.             *
     *                                                                       *
     *    Help yourself get started quickly and support the FreeRTOS         *
     *    project by purchasing a FreeRTOS tutorial book, reference          *
     *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>! NOTE: The modification to the GPL is included to allow you to distribute
    >>! a combined work that includes FreeRTOS without being obliged to provide
    >>! the source code for proprietary components outside of the FreeRTOS
    >>! kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/


/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Processor constants. */
#include "os_cpu.h"

//#include "irqn.h"
#include <nds32_intrinsic.h>

/* Constants required to setup the task context. */
#define portNO_CRITICAL_SECTION_NESTING	( ( portSTACK_TYPE ) 0 )

/*-----------------------------------------------------------*/

extern void vPortISRStartFirstTask( void );
#define OLD_VER 0

/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See header file for description.
 *
 *
 * Stack Layout:
 *		  High  |-----------------|
 *                      |       $R1       |
 *                      |-----------------|
 *                      |       $R0       |
 *                      |-----------------|
 *                      |       $R30 (LP) |
 *                      |-----------------|
 *                      |       $R29 (GP) |
 *                      |-----------------|
 *                      |       $R28 (FP) |
 *                      |-----------------|
 *                      |   $R15   $R27   |
 *                      |-----------------|
 *                      |   $R10   $R26   |
 *                      |-----------------|
 *                      |        .        |
 *                      |        .        |
 *                      |-----------------|
 *                      |       $R2       |
 *                      |-----------------|
 *                      |       $d0.hi    |
 *                      |          .lo    |
 *                      |-----------------|
 *                      |       $d1.hi    |
 *                      |          .lo    |
 *                      |-----------------|
 *                      |       $IFC_LP   |
 *                      |-----------------|
 *                      |       $IPSW     |
 *                      |-----------------|
 *                      |       $IPC      |
 *                      |-----------------|
 *                      |       $PSW      |
 *                      |-----------------|
 *                      |       $SP       |
 *                      |-----------------|
 *                      |       $SP       |  (8-byte alignment)
 *                      |-----------------|
 *                      |       $FPU      |
 *                      |-----------------|
 *                Low
 *
 */
/* For relax support, must initial $gp at task init*/
extern INT32U _SDA_BASE_ __attribute__ ((weak));

portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
	int i;
	//portSTACK_TYPE *pxOriginalTOS;
	portSTACK_TYPE *old_stk;

	//pxOriginalTOS = pxTopOfStack;

	/* Setup the initial stack of the task.  The stack is set exactly as
	expected by the portRESTORE_CONTEXT() macro. */

	/* First on the stack is the return address - which in this case is the
	start of the task.  The offset is added to make the return address appear
	as it would within an IRQ ISR. */
        *--pxTopOfStack = (INT32U)0x01010101L * 1;			/* R1 */
        *--pxTopOfStack = (portSTACK_TYPE)pvParameters;           	/* R0 : argument */
        *--pxTopOfStack = (INT32U)vPortEndScheduler;             	/* $30:LP */
        *--pxTopOfStack = (INT32U)&_SDA_BASE_;                   	/* $29:GP For relax support */
        *--pxTopOfStack = (INT32U)0x01010101UL * 28;			/* $28:FP */
	#ifdef __NDS32_REDUCE_REGS__
        *--pxTopOfStack = (INT32U)0x01010101UL * 15;
	i = 10;
	#else
	i = 27;
	#endif
        for (; i >= 2; i--)
		*--pxTopOfStack = (INT32U)0x01010101UL * i;
	#if (defined(__NDS32_ISA_V2__) && defined(__NDS32_DX_REGS__)) || (!defined(__NDS32_ISA_V2__) && (defined(__NDS32_DIV__)||defined(__NDS32_MAC__)))
	for (i = 35; i >= 32; i--)                      	  	/* d0.hi, d0.lo, d1.hi, d1.lo */
		*--pxTopOfStack = (INT32U)0x01010101UL * i;
	#endif

#ifdef __NDS32_EXT_IFC__
# ifndef CONFIG_NO_NDS32_EXT_IFC
		*--pxTopOfStack = (INT32U)0x01010101UL * 40;		/* $IFC_LP */
# endif
#endif

#if ( configSUPPORT_ZOL == 1 )
        *--pxTopOfStack = (portSTACK_TYPE)0;//ZOL
        *--pxTopOfStack = (portSTACK_TYPE)0;
        *--pxTopOfStack = (portSTACK_TYPE)0;
#endif

	/* IPSW, enable GIE, clear IFCON */
        *--pxTopOfStack = (portSTACK_TYPE)((GET_PSW() | (1UL << PSW_offGIE)) & (~(1UL << PSW_offIFCON)));
	/* IPC */
        *--pxTopOfStack = (portSTACK_TYPE)pxCode;
	/* PSW */
        *--pxTopOfStack = (portSTACK_TYPE)(GET_PSW() | (1UL << PSW_offINTL) );
       
       old_stk = pxTopOfStack;                                	  	/* Save curent unaligned sp */
        *--pxTopOfStack = (portSTACK_TYPE)((INT32U)old_stk);    	/* sp(R31) */
        if ((INT32U)pxTopOfStack & 0x7)
                *--pxTopOfStack = (portSTACK_TYPE)((INT32U)old_stk);    /* sp(R31) */
	

	/* push fpu */
#if defined(__NDS32_EXT_FPU_DP__) || defined(__NDS32_EXT_FPU_SP__)
	int start, end;
#if defined(__NDS32_EXT_FPU_CONFIG_0__)
        end = 36; start = 35 + 2 * 4;
#elif defined(__NDS32_EXT_FPU_CONFIG_1__)
        end = 36; start = 35 + 2 * 8;
#elif defined(__NDS32_EXT_FPU_CONFIG_2__)
        end = 36; start = 35 + 2 * 16;
#elif defined(__NDS32_EXT_FPU_CONFIG_3__)
        end = 36; start = 35 + 2 * 32;
#else
	end = 1; start = 0;
#endif
	for (i = start; i >= end; i--)
		*--pxTopOfStack = (INT32U)0x01010101UL * i;
#endif


	return pxTopOfStack;

}

/*-----------------------------------------------------------*/

portBASE_TYPE xPortStartScheduler( void )
{
	extern void SysTickInit(void);

	SysTickInit();

	/* Start the first task. */
	vPortISRStartFirstTask();

	/* Should not get here! */
	return 0;
}

/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	printf("Current Task will be deleted\n");
	
	/* 
 	 * FreeRTOS vTaskDelete() just remove TCB from list.
 	 * vTaskDelete() would call vPortYiled change to Idle task which would do garbage collection.
 	 * So called vTaskDelete() or return from task, make sure GIE is turn-on if context switch by SWI.
	 */ 
	vTaskDelete(NULL);
}

void vPortYield()
{
       /* trigger swi*/
       __nds32__mtsr(0x10000,NDS32_SR_INT_PEND);
       __nds32__isb();
}
#define portNO_CRITICAL_NESTING		( ( unsigned long ) 0 )
volatile unsigned long ulCriticalNesting = 0UL;
static OS_CPU_SR psw_0 = 0;
OS_CPU_SR psw_1 = 0;
void vPortEnterCritical()
{
       /*
        *       in order to avoid race condition
        *       1.store psw into stack
        *       2.disable gie
        *       3.store psw into global if ulCriticalNesting==0
        *       4.ulCriticalNesting++ 
        */
       volatile unsigned int psw = __nds32__mfsr(NDS32_SR_PSW);
       GIE_DISABLE();
       if (ulCriticalNesting == portNO_CRITICAL_NESTING )
               psw_0 = psw;
       ulCriticalNesting++;

}

void vPortExitCritical()
{

       --ulCriticalNesting;
       if ( ulCriticalNesting == portNO_CRITICAL_NESTING )
               GIE_RESTORE(psw_0);
}


/*-----------------------------------------------------------*/

/* Kernel interrupt mask, it imply the kernel priority comapre with interrupt */
inline unsigned long ulPortSetInterruptMask( void )
{
	/* By default, the kerenl has the highest priority */
	/* It's mean that we don't support kernel preemptive */

#ifdef CONFIG_HW_PRIO_SUPPORT
	/* @Apply HW priority */
	/* GIE on/off control by kernel ISR*/
	/* Mask(disable) all interrupt */
	return hal_intc_irq_mask(-1);
#else
	/* Apply SW priority */
	/* GIE on/off control by user ISR*/
	/* Disable GIE */
	unsigned long psw = __nds32__mfsr(NDS32_SR_PSW);
	__nds32__mtsr_isb((psw & (~(7<<16)))|(configMAX_SYSCALL_INTERRUPT_PRIORITY<<16), NDS32_SR_PSW);
	//GIE_DISABLE();
	return psw;
#endif
}

inline void vPortClearInterruptMask( unsigned long msk )
{
/* see ulPortSetInterruptMask comment */
#ifdef CONFIG_HW_PRIO_SUPPORT
	/* restore interrupt mask */
	hal_timer_irq_unmask(msk)
#else
	/* restore GIE */
	//GIE_RESTORE(msk);
	__nds32__mtsr_isb(msk, NDS32_SR_PSW);
#endif
}


/*-----------------------------------------------------------*/

#if configUSE_TICKLESS_IDLE == 1
#define TICK_HZ 100
#define ulTimerCountsForOneTick 		(MB_PCLK / TICK_HZ)
#define xMaximumPossibleSuppressedTicks 	(30*TICK_HZ) /* 30(sec)*TICK_HZ (Ticks!!)*/
#define ulStoppedTimerCompensation		10	/* FIXME add this miss config */	


__attribute__((weak)) void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
	{
	uint32_t ulReloadValue, ulCompleteTickPeriods, ulCompletedSysTickDecrements;
	TickType_t xModifiableIdleTime;

		/* Make sure the SysTick reload value does not overflow the counter. */
		if( xExpectedIdleTime > xMaximumPossibleSuppressedTicks )
		{
			xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
		}

		/* Stop the SysTick momentarily.  The time the SysTick is stopped for
		is accounted for as best it can be, but using the tickless mode will
		inevitably result in some tiny drift of the time maintained by the
		kernel with respect to calendar time. */
		hal_timer_stop(1);

		/* Calculate the reload value required to wait xExpectedIdleTime
		tick periods.  -1 is used because this code will execute part way
		through one of the tick periods. */
		ulReloadValue = hal_timer_count_read(1) + ( ulTimerCountsForOneTick * ( xExpectedIdleTime - 1UL ) );

		if( ulReloadValue > ulStoppedTimerCompensation )
		{
			ulReloadValue -= ulStoppedTimerCompensation;
		}

		/* Enter a critical section but don't use the taskENTER_CRITICAL()
		method as that will mask interrupts that should exit sleep mode. */
		__nds32__gie_dis();

		/* If a context switch is pending or a task is waiting for the scheduler
		to be unsuspended then abandon the low power entry. */
		if( eTaskConfirmSleepModeStatus() == eAbortSleep )
		{
		
			/* Restart from whatever is left in the count register to complete
			this tick period. */
			/* TODO why need this step? Could skip? */
			hal_timer_set_period( 1, hal_timer_count_read(1) );		

			/* TODO set timer counter as 0 */
			/* Restart SysTick. */
			hal_timer_start(1);

			/* Reset the reload register to the value required for normal tick
			periods. */
			hal_timer_set_period(1, ulTimerCountsForOneTick - 1UL);

			/* Re-enable interrupts - see comments above the cpsid instruction()
			above. */
			__nds32__gie_en();
		}
		else
		{
			/* Set the new reload value. */
			hal_timer_set_period( 1 ,ulReloadValue );

			/* Restart SysTick. */
			hal_timer_start(1);

			/* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
			set its parameter to 0 to indicate that its implementation contains
			its own wait for interrupt or wait for event instruction, and so wfi
			should not be executed again.  However, the original expected idle
			time variable must remain unmodified, so a copy is taken. */
			xModifiableIdleTime = xExpectedIdleTime;
			configPRE_SLEEP_PROCESSING( xModifiableIdleTime );
			if( xModifiableIdleTime > 0 )
			{
				extern int puts(const char*);
				__nds32__msync_all();
				__nds32__standby_no_wake_grant();
				__nds32__isb();
				
				/* 
 				 * extern int puts(const char*);
				 * puts("          Leave STANDBY\r\n");
				 */

			}
			configPOST_SLEEP_PROCESSING( xExpectedIdleTime );

			/* Stop SysTick.  Again, the time the SysTick is stopped for is
			accounted for as best it can be, but using the tickless mode will
			inevitably result in some tiny drift of the time maintained by the
			kernel with respect to calendar time. */
			hal_timer_stop(1);

			/* Re-enable interrupts - see comments above the cpsid instruction()
			above. */
			__nds32__gie_en();

			if( hal_intc_get_all_pend() & (0x1 << 19) )
			{
				uint32_t ulCalculatedLoadValue;

				/* The tick interrupt has already executed, and the SysTick
				count reloaded with ulReloadValue.  Reset the
				portNVIC_SYSTICK_LOAD_REG with whatever remains of this tick
				period. */
				ulCalculatedLoadValue = ( ulTimerCountsForOneTick - 1UL ) - ( ulReloadValue - hal_timer_count_read(1) );


				/* Don't allow a tiny value, or values that have somehow
				underflowed because the post sleep hook did something
				that took too long. */
				if( ( ulCalculatedLoadValue < ulStoppedTimerCompensation ) || ( ulCalculatedLoadValue > ulTimerCountsForOneTick ) )
				{
					ulCalculatedLoadValue = ( ulTimerCountsForOneTick - 1UL );
				}

				hal_timer_set_period(1, ulCalculatedLoadValue);

				/* The tick interrupt handler will already have pended the tick
				processing in the kernel.  As the pending tick will be
				processed as soon as this function exits, the tick value
				maintained by the tick is stepped forward by one less than the
				time spent waiting. */
				ulCompleteTickPeriods = xExpectedIdleTime - 1UL;
			}
			else
			{
				/* Something other than the tick interrupt ended the sleep.
				Work out how long the sleep lasted rounded to complete tick
				periods (not the ulReload value which accounted for part
				ticks). */
				ulCompletedSysTickDecrements = ( xExpectedIdleTime * ulTimerCountsForOneTick ) - hal_timer_count_read(1);

				/* How many complete tick periods passed while the processor
				was waiting? */
				ulCompleteTickPeriods = ulCompletedSysTickDecrements / ulTimerCountsForOneTick;

				/* The reload value is set to whatever fraction of a single tick
				period remains. */
				hal_timer_set_period( 1, (( ulCompleteTickPeriods + 1 ) * ulTimerCountsForOneTick)-ulCompletedSysTickDecrements);

			}

			/* Restart SysTick so it runs from portNVIC_SYSTICK_LOAD_REG
			again, then set portNVIC_SYSTICK_LOAD_REG back to its standard
			value.  The critical section is used to ensure the tick interrupt
			can only execute once in the case that the reload register is near
			zero. */
			/* TODO reset timer counter as 0 */
			portENTER_CRITICAL();
			{
				hal_timer_start(1);
				vTaskStepTick( ulCompleteTickPeriods );
				hal_timer_set_period( 1, (ulTimerCountsForOneTick - 1UL) );
			}
			portEXIT_CRITICAL();
		}
	}


#endif /* #if configUSE_TICKLESS_IDLE */
/*-----------------------------------------------------------*/

