##==============================================================================
##
##  crt0.S
##
##  startup code
##
##==============================================================================

#include <nds32_init.inc>
#include "interrupt.h"

##------------------------------------------------------------------------------
	.macro vector name
	.align 2
	j OS_Trap_\name
	.endm

	.macro ENABLE_IT_DT
	.align 2
		pushm   $r0, $r1
		mfsr    $r1, $IPSW
		andi	$r1, $r1, #0x20c0 !Ê¹ÄÜZOLºÍCahce
		mfsr    $r0, $PSW
		add   	$r0, $r0, $r1
		mtsr    $r0, $PSW
		isb
		popm    $r0, $r1
	.endm

	.section .vector, "ax"
!========================================================================
! Vector table
!========================================================================
	.align 2
exception_vector:
_start:
	j ___start                    !  (0) Trap Reset
	vector TLB_Fill             !  (1) Trap TLB fill
	vector PTE_Not_Present      !  (2) Trap PTE not present
	vector TLB_Misc             !  (3) Trap TLB misc
	vector TLB_VLPT_Miss        !  (4) Trap TLB VLPT miss
	vector Machine_Error        !  (5) Trap Machine error
	vector Debug_Related        !  (6) Trap Debug related
	vector General_Exception    !  (7) Trap General exception
	vector Syscall              !  (8) Syscall

	/* HW interrupt vector layout */
	hal_hw_vectors
exception_vector_end:

.section .startup_section, "ax"
	.align  2
ISR_TABLE:
	.long SystickInterrupt //0
	.long WakeupInterrupt  //1
	.long GpioInterrupt    //2
	.long RtcInterrupt     //3
	.long SpdifInterrupt   //4
	.long SWInterrupt      //5
	.long I2C_Interrupt    //6
	.long UART0_Interrupt  //7
	.long Timer2Interrupt  //8
	.long DMA0_Interrupt   //9
	.long DMA1_Interrupt   //10
	.long DMA2_Interrupt   //11
	.long DMA3_Interrupt   //12
	.long DMA4_Interrupt   //13
	.long DMA5_Interrupt   //14
	.long DMA6_Interrupt   //15
	.long DMA7_Interrupt   //16
	.long UART1_Interrupt  //17
	.long BT_Interrupt     //18
	.long BLE_Interrupt    //19
	.long UsbInterrupt     //20
	.long SPIM_Interrupt   //21
	.long I2sInterrupt     //22
	.long Timer3Interrupt  //23
	.long Timer4Interrupt  //24
	.long Timer5Interrupt  //25
	.long Timer6Interrupt  //26
	.long SDIO0_Interrupt  //27
	.long SPIS_Interrupt   //28
	.long FFTInterrupt     //29
	.long IRInterrupt      //30
/*
 * exception handlers
 */

/*----------------------------------------------------------------------
   Since N903 doesn't have MMU, make them share the common error handler.
----------------------------------------------------------------------*/
OS_Trap_TLB_Fill:
//1:  b   1b
	pushm $lp,$sp
	pushm $r0,$r20
	mov55 $r0, $sp
	li	$r1, 0x1
	b   ExceptionCommHandler


OS_Trap_PTE_Not_Present:
//1:  b   1b
	pushm $lp,$sp
	pushm $r0,$r20
	mov55 $r0, $sp
	li	$r1, 0x2
	b   ExceptionCommHandler

OS_Trap_TLB_Misc:
//1:  b   1b
	pushm $lp,$sp
	pushm $r0,$r20
	mov55 $r0, $sp
	li	$r1, 0x3
	b   ExceptionCommHandler


OS_Trap_TLB_VLPT_Miss:
//1:  b   1b
	pushm $lp,$sp
	pushm $r0,$r20
	mov55 $r0, $sp
	li	$r1, 0x4
	b   ExceptionCommHandler

OS_Trap_Machine_Error:
//1:  b   1b
	pushm $lp,$sp
	pushm $r0,$r20
	mov55 $r0, $sp
	li	$r1, 0x5
	b   ExceptionCommHandler

OS_Trap_Debug_Related:
//1:  b   1b
	pushm $lp,$sp
	pushm $r0,$r20
	mov55 $r0, $sp
	li	$r1, 0x6
	b   ExceptionCommHandler

OS_Trap_General_Exception:
//1:  b   1b
	pushm $lp,$sp
	pushm $r0,$r20
	mov55 $r0, $sp
	li	$r1, 0x7
	b   ExceptionCommHandler

OS_Trap_Syscall:
//1:  b   1b
	pushm $lp,$sp
	pushm $r0,$r20
	mov55 $r0, $sp
	li	$r1, 0x8
	b   ExceptionCommHandler

OS_Trap_Int_Comm:
	ENABLE_IT_DT
	SAVE_ALL_HW
	la  $r1, ISR_TABLE
	lw  $r1, [$r1+$r0<<2]
	jral $r1
	RESTORE_ALL_HW
	iret

! Interrupt wrapper function
OS_Trap_Interrupt_HW0:
	push	$r0
	li	$r0, 0x0
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW1:
	push	$r0
	li	$r0, 0x01
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW2:
	push	$r0
	li	$r0, 0x02
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW3:
	push	$r0
	li	$r0, 0x03
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW4:
	push	$r0
	li	$r0, 0x04
	j	OS_Trap_Int_Comm

/*OS_Trap_Interrupt_HW5:
	push	$r0
	li	$r0, 0x05
	j	OS_Trap_Int_Comm
*/

OS_Trap_Interrupt_HW5:
	pushm	$r0, $r1
	ENABLE_IT_DT
	j	OS_Trap_Interrupt_SWI

OS_Trap_Interrupt_HW6:
	push	$r0
	li	$r0, 0x06
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW7:
	push	$r0
	li	$r0, 0x07
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW8:
	push	$r0
	li	$r0, 0x08
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW9:
	push	$r0
	li	$r0, 0x09
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW10:
	push	$r0
	li	$r0, 0x0a
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW11:
	push	$r0
	li	$r0, 0x0b
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW12:
	push	$r0
	li	$r0, 0x0c
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW13:
	push	$r0
	li	$r0, 0x0d
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW14:
	push	$r0
	li	$r0, 0x0e
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW15:
	push	$r0
	li	$r0, 0x0f
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW16:
	push	$r0
	li	$r0, 0x10
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW17:
	push	$r0
	li	$r0, 0x11
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW18:
	push	$r0
	li	$r0, 0x12
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW19:
	push	$r0
	li	$r0, 0x13
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW20:
	push	$r0
	li	$r0, 0x14
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW21:
	push	$r0
	li	$r0, 0x15
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW22:
	push	$r0
	li	$r0, 0x16
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW23:
	push	$r0
	li	$r0, 0x17
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW24:
	push	$r0
	li	$r0, 0x18
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW25:
	push	$r0
	li	$r0, 0x19
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW26:
	push	$r0
	li	$r0, 0x1a
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW27:
	push	$r0
	li	$r0, 0x1b
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW28:
	push	$r0
	li	$r0, 0x1c
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW29:
	push	$r0
	li	$r0, 0x1d
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW30:
	push	$r0
	li	$r0, 0x1e
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW31:
	push	$r0
	li	$r0, 0x1f
	j	OS_Trap_Int_Comm

##------------------------------------------------------------------------------
## Startup code

	.section .startup_section
	.global _start
	.weak _call_exit
	.weak _SDA_BASE_
	.func _start
	.type _start, @function

	.align 2
___start:
	!************************** Begin of do-not-modify **************************
	! Please don't modify this code
	! Initialize the registers used by the compiler

	nds32_init		! NDS32 startup initial macro in <nds32_init.inc>
	!*************************** End of do-not-modify ***************************
	#bal _nds32_init_mem

	movi55 $r0,#0x0
 	mtsr $r0,$misc_ctl

	bal __init

	bal main
1:  b   1b

	.size _start, .-___start

	.end
