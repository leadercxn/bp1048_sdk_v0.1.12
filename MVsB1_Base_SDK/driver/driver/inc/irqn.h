/**
  ******************************************************************************
  * @file    irqn.h
  * @author  Peter Zhai
  * @version V1.0
  * @date    2017-10-27
  * @brief
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/**
 * @addtogroup SYSTEM
 * @{
 * @defgroup irqn irqn.h
 * @{
 */
#include "type.h"

#ifndef __IRQN_H__
#define __IRQN_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

/**
 * @brief	外设中断ID
 */
typedef enum IRQn
{
    /*
     * internal interrupt(Exception/Fault)
     */
    RST_IRQn		= -9,
    TLB_Fill_IRQn		= -8,
    PTE_IRQn	= -7,
    TLB_Misc_IRQn		= -6,//programmable as below
    TLB_VLPT_Miss_IRQn		= -5,
    Machine_ERR_IRQn		= -4,
    Debug_IRQn		= -3,
    General_Exception_IRQn		= -2,
    Syscall_IRQn		= -1,
    /*
     * SOC interrupt(External Interrupt)
     */

    TMR1_IRQn	    = 0,
    Wakeup_IRQn		= 1,
    Gpio_IRQn		= 2,
    Rtc_IRQn		= 3,
    Spdif_IRQn		= 4,
    SWI_IRQn		= 5,
    I2C_InIRQn		= 6,
    UART0_IRQn		= 7,
    Timer2_IRQn		= 8,
    DMA0_IRQn		= 9,
    DMA1_IRQn		= 10,
    DMA2_IRQn		= 11,
    DMA3_IRQn		= 12,
    DMA4_IRQn		= 13,
    DMA5_IRQn		= 14,
    DMA6_IRQn		= 15,
    DMA7_IRQn		= 16,
	UART1_IRQn		= 17,
    BT_IRQn			= 18,
    BLE_IRQn	    = 19,
	Usb_IRQn		= 20,
    SPIM_IRQn	    = 21,
	I2s_IRQn		= 22,
    Timer3_IRQn		= 23,
	Timer4_IRQn		= 24,
	Timer5_IRQn		= 25,
	Timer6_IRQn		= 26,
    SDIO0_IRQn		= 27,
    SPIS_IRQn		= 28,
    FFTInt_IRQn		= 29,
	IR_IRQn			= 30

} IRQn_Type;

/**
 * @brief	外设默认中断优先级
 */
typedef enum IRQn_PRIO
{
    /*
     * SOC interrupt(External Interrupt)
     */
	TMR1_IRQn_PRIO	    = 3,
	Wakeup_IRQn_PRIO	= 2,
    GPIO_IRQn_PRIO		= 2,
    RTC_IRQn_PRIO		= 2,
	SPDIF_IRQn_PRIO		= 2,
	SWI_IRQn_PRIO		= 3,
	I2C_IRQn_PRIO		= 2,
    UART0_IRQn_PRIO		= 2,
    TMR2_IRQn_PRIO		= 2,
	DMA0_IRQn_PRIO		= 2,
	DMA1_IRQn_PRIO		= 2,
	DMA2_IRQn_PRIO		= 2,
	DMA3_IRQn_PRIO		= 2,
	DMA4_IRQn_PRIO		= 2,
	DMA5_IRQn_PRIO		= 2,
	DMA6_IRQn_PRIO		= 2,
	DMA7_IRQn_PRIO		= 2,
    UART1_IRQn_PRIO		= 2,
	BT_IRQn_PRIO		= 2,
	BLE_IRQn_PRIO		= 2,
	USB_IRQn_PRIO		= 2,
	SPIM_IRQn_PRIO		= 2,
	I2S_IRQn_PRIO		= 2,
	TMR3_IRQn_PRIO	    = 2,
    TMR4_IRQn_PRIO		= 2,
	TMR5_IRQn_PRIO	    = 2,
    TMR6_IRQn_PRIO		= 2,
	SDIO0_IRQn_PRIO		= 2,
	SPIS_IRQn_PRIO		= 2,
	FFT_IRQn_PRIO		= 2,
	IR_IRQn_PRIO		= 2
} IRQn_PRIO_TYPE;

/**
 * @brief	设置CPU Powerbrake时使用
 */
#define NDS_THROTTLE_LEVEL_0  (0x00)
#define NDS_THROTTLE_LEVEL_1  (0x10)
#define NDS_THROTTLE_LEVEL_2  (0x20)
#define NDS_THROTTLE_LEVEL_3  (0x30)
#define NDS_THROTTLE_LEVEL_4  (0x40)
#define NDS_THROTTLE_LEVEL_5  (0x50)
#define NDS_THROTTLE_LEVEL_6  (0x60)
#define NDS_THROTTLE_LEVEL_7  (0x70)
#define NDS_THROTTLE_LEVEL_8  (0x80)
#define NDS_THROTTLE_LEVEL_9  (0x90)
#define NDS_THROTTLE_LEVEL_10 (0xA0)
#define NDS_THROTTLE_LEVEL_11 (0xB0)
#define NDS_THROTTLE_LEVEL_12 (0xC0)
#define NDS_THROTTLE_LEVEL_13 (0xD0)
#define NDS_THROTTLE_LEVEL_14 (0xE0)
#define NDS_THROTTLE_LEVEL_15 (0xF0)

/**
 * @brief	使能全局中断
 * @param	None
 * @return	None
 * @note
 */
void GIE_ENABLE(void);

/**
 * @brief	禁能全局中断
 * @param	None
 * @return	None
 * @note
 */
void GIE_DISABLE(void);

/**
 * @brief	获取全局中断状态
 * @param	None
 * @return	TRUE：全局中断使能； FALSE：全局中断禁能
 * @note
 */
bool GIE_STATE_GET();
/**
 * @brief	使能PowerBrake功能
 * @param	None
 * @return	None
 * @note
 */
void Throttle_Enable(void);

/**
 * @brief	禁能PowerBrake功能
 * @param	None
 * @return	None
 * @note
 */
void Throttle_Disable(void);

/**
 * @brief	设置Throttle level
 * @param	setting
 * 			NDS_THROTTLE_LEVEL_0，   16个CPU clock全部有效，具有最高的性能
 * 			...
 * 			NDS_THROTTLE_LEVEL_15，16个CPU clock，只有1个clock起作用,具有最低的性能
 * @return	None
 * @note
 */
void Throttle_Setup(uint32_t setting);

/**
 * @brief	使能外设中断
 * @param	IRQn	外设ID
 * @return	None
 * @note
 */
void NVIC_EnableIRQ(IRQn_Type IRQn);

/**
 * @brief	禁能外设中断
 * @param	IRQn	外设ID
 * @return	None
 * @note
 */
void NVIC_DisableIRQ(IRQn_Type IRQn);

/**
 * @brief	设置外设的中断优先级
 * @param	IRQn	外设ID
 * @param	priority
 * 			0,1,2,3 0=highest 3=lowest
 * @return	None
 * @note
 */
void NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority);

/**
 * @brief	获取外设的中断优先级
 * @param	IRQn	外设ID
 * @return	中断优先级
 * @note
 */
uint32_t NVIC_GetPriority(IRQn_Type IRQn);

/**
 * @brief	根据中断的优先级来使能中断，只有优先级高于Level的中断才会被使能
 * @param	Level
 * 			0,1,2,3 0=highest 3=lowest
 * @return	None
 * @note
 */
void InterruptLevelSet(uint8_t Level);

/**
 * @brief	恢复所有中断
 * @return	None
 * @note
 */
void InterruptLevelRestore(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__IRQN_H__

