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


/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "os_cpu.h"
#include "timer.h"


/* Constants required to handle critical sections. */
/*-----------------------------------------------------------*/

//void vPortISRStartFirstTask( void );
/*-----------------------------------------------------------*/

__attribute__((weak)) void vPortISRStartFirstTask( void )
{
         //Simply start the scheduler.
        portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/


/* 
 * The ISR used for the scheduler tick depends on whether the cooperative or
 * the preemptive scheduler is being used.
 */
void ClearTimer1IntFlag()
{
	SysTimerIntFlagClear();
}

extern volatile uint32_t gSysTick;

__attribute__((section(".driver.isr")))
void SystickInterrupt(void)
#if configUSE_PREEMPTION == 1
{
	uint32_t msk;
	SysTimerIntFlagClear();
	gSysTick++;

	msk =  portSET_INTERRUPT_MASK_FROM_ISR();
	/* Increment the tick count - this may wake a task. */
	if ( xTaskIncrementTick() == pdTRUE )
	{
		/* Raise a SWI pending bit */
		portYIELD();
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR(msk);

}
#else
{
	uint32_t msk;
	ClearTimer1IntFlag();
	gSysTick++;

	/* The cooperative scheduler requires a normal IRQ service routine to
	simply increment the system tick. */
	{
		unsigned long ulDummy;

		/* Increment the tick count - which may wake some tasks but as the
		preemptive scheduler is not being used any woken task is not given
		processor time no matter what its priority. */
		xTaskIncrementTick();

	}
}
#endif

/*-----------------------------------------------------------*/
