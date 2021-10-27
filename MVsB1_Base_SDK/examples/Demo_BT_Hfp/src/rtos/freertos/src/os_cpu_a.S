!********************************************************************************************************
!                                               FreeRTOS
!
!                               (c) Copyright 2010, Andes Techonology
!                                          All Rights Reserved
!
!                                           NDS32 Generic Port
!                                            GNU C Compiler
!
! File    : os_cpu_a.S
! Version : V1.0
! By      : greentime @ Andes Technology
!
!********************************************************************************************************
!********************************************************************************************************
! INCLUDE ASSEMBLY CONSTANTS
!********************************************************************************************************

#include "n12_def.h"
#include "FreeRTOSConfig.h"
#include "os_cpu_common.h"

!********************************************************************************************************
! IMPORT and EXPORT
!********************************************************************************************************

	.global OS_Trap_Interrupt_SWI
	.global CtxSave
	.global CtxRestore

	
	.macro TCBSaveMacro
		! Store the new top of stack for the task.
		la	$r3, pxCurrentTCB	! pxCurrentTCB->pxTopOfStack = $SP
		lwi	$r4, [$r3]
		swi	$sp, [$r4]
	.endm



	! Restore to new task's context
	.macro TCBRestoreMacro
		! use new pxCurrentTCB to get new stack pointer
		la	$r0, pxCurrentTCB
		lwi	$r1, [$r0]
		lwi	$sp, [$r1]		! $SP = pxCurrentTCB->pxTopOfStack
	.endm	



!*************************************************************************
!
!	PortMacro Function, Call by C function
!	Used by First start Scheduler
!	Warm up a Initalized Task
!
!*************************************************************************
!CtxSave:
!
!		mfsr	$r0, $PSW
!		mtsr	$r0, $IPSW
!		mtsr	$lp, $IPC
!		pushm $r0, $r1
!		SAVE_ALL
!		TCBSaveMacro
!		ret

	! Restore to new task's context
CtxRestore:
		TCBRestoreMacro
		RESTORE_ALL_OS
		popm $r0, $r1
		iret



!********************************************************************************************************
!                   PERFORM A CONTEXT SWITCH - OSCtxSw()
!
! Note(s) : 1) OSIntCtxSw() is called with IRQ DISABLED
!
!           2) The pseudo-code for OSCtxSw() is:
!              a) $IPSW <- $PSW, $IPC <- $lp
!              b) CtxSaveMacro		: Save current task's context
!	       c) TCBSaveMacro		: Save current task's sp into TCB
!              d) vTaskSwitchContext	: get the highest priority of Task
!              e) IntlSwitch		: Switch to interruption level 1
!	       f) TCBRestoreMacro	: Restore to the highest priority task's sp from TCB
!              g) CtxRestoreMacro	: Restore to the highest priority task's context 
!
!********************************************************************************************************
.section .startup_section
OS_Trap_Interrupt_SWI:
	movi    $r0, 0
	mtsr    $r0, $INT_PEND                  ! clean SWI pending
	
	IntlSwitch #0                   	! Switch to interrupt level 0

	SAVE_ALL_OS

	pushm $r0,$r7
	jal OS_int5int
	popm $r0,$r7

	mfsr $r1,$psw
	movi $r0,#0xfff8fff8
	and33 $r1,$r0
	movi $r0,#((configMAX_SYSCALL_INTERRUPT_PRIORITY<<16)|1)
	or33 $r0,$r1
	mtsr $r0,$psw
	dsb

	TCBSaveMacro				! Save current task's sp into TCB

	CallFn	vTaskSwitchContext		! get the highest priority of Task

	TCBRestoreMacro				! Restore to original task's TCB

    setgie.d
    dsb

    pushm $r0,$r7
	jal OS_int5out
	popm $r0,$r7

	RESTORE_ALL_OS
	popm $r0, $r1
	iret
