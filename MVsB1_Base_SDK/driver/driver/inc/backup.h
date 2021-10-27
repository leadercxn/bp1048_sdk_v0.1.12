/**
 **************************************************************************************
 * @file    backup.h
 * @brief	backup module driver interface
 *
 * @author  tony
 * @version V1.0.7
 *
 * @Created: 201-4-16
 *
 * @Copyright (C) 2014, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
 /**
 * @addtogroup BACKUP
 * @{
 * @defgroup backup backup.h
 * @{
 */

#ifndef __BACKUP_H__
#define __BACKUP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "type.h"

/* Typedef -------------------------------------------------------------------*/
typedef enum _CLK_32K_SEL
{
	RC32K_SOURCE = 0,
	OSC32K_SOURCE = 1
}CLK_32K_SEL;

typedef enum _BAKCUP_CLK_32K_MODE
{
	OSC32K_MODE_LOW_FREQ_OSC = 0,
	xMHZ_MODE_HIGH_FREQ_OSC = 1
}BACKUP_CLK_32K_MODE;

typedef enum _OSC_Speed_MODE
{
	OSC_MODE_LOSC = 0,
	OSC_MODE_HOSC = 1
}OSC_SPEED_MODE;

typedef enum _OSC_CLK_SEL
{
	LOSC_ANA_MODE = 0,
	LOSC_DIG_MODE = 1
}OSC_CLK_SEL;

typedef enum _POWERUP_EVENT_SEL
{
	SEL_VIN 			 = (1	  ),
	GPIO_C0 			 = (1 << 1),
	VALID_POWERKEY_EVENT = (1 << 2),
	RTC_EVNET 			 = (1 << 3)
}POWERUP_EVENT_SEL;

typedef enum _OSC_DIRECT_INPUT_SEL
{
	 OSC_USE = 0,
	 EXTERNAL_DIRECT_INPUT_USE = 1
}OSC_DIRECT_INPUT_SEL;

typedef enum _POWERKEY_SWITCH_MODE
{
	HARD_MODE = 0,
	SOFT_MODE = 1
}POWERKEY_SWITCH_MODE;

typedef enum _HARD_TRIGGER_MODE
{
	LEVEL_TRIGGER = 0,
	EDGE_TRIGGER = 1
}HARD_TRIGGER_MODE;

typedef enum _POWERKEY_ACTIVE_LEVEL
{
	LOW_INDICATE_POWERON = 0,
	HIGH_INDICATE_POWERON = 1
}POWERKEY_ACTIVE_LEVEL;

typedef enum _POWERKEY_RES_TUNNING
{
	PULLUP_22K,
	PULLUP_330K
}POWERKEY_RES_TUNNING;

typedef enum _POWERKEY_VTH_TUNNING
{
	VTH_0V8,
	VTH_2V4
}POWERKEY_VTH_TUNNING;

/* Define --------------------------------------------------------------------*/
#define		BACKUP_GPIO_C0_REG_START						(0x40024160)

#define 	BKUP_GPIO_C0_REG_O_OFF							(0x00)
#define 	BKUP_GPIO_C0_REG_O_SET_OFF						(0x04)
#define 	BKUP_GPIO_C0_REG_O_CLR_OFF						(0x08)
#define 	BKUP_GPIO_C0_REG_O_TGL_OFF						(0x0C)

#define 	BKUP_GPIO_C0_REG_IE_OFF							(0x10)
#define 	BKUP_GPIO_C0_REG_OE_OFF							(0x11)
#define 	BKUP_GPIO_C0_REG_DS_OFF							(0x12)
#define 	BKUP_GPIO_C0_REG_PU_OFF							(0x13)
#define 	BKUP_GPIO_C0_REG_PD_OFF							(0x14)
#define 	BKUP_GPIO_C0_REG_PILLDOWN_OFF					(0x15)

/* Variables -----------------------------------------------------------------*/


/* API -----------------------------------------------------------------------*/

/**
 * @brief	backup register write enable
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_WriteEnable(void);

/**
 * @brief	backup register write disable
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_WriteDisable(void);

/**
 * @brief	trigger system entering power down
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_SystemPowerDown(void);

/**
 * @brief	 enable fast synchronous isolation when power fail
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_FastPowerDownEnable(void);

/**
 * @brief	 disable fast synchronous isolation when power fail
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_FastPowerDownDisable(void);

/**
 * @brief	Check the synchronous isolation enable or disable.
 *
 * @param
 *   		NONE
 * @return
 *   		TRUE  : Enable.
 *   		FALSE : Disable.
 * @note
 */
bool BACKUP_IsFastPowerDownEnable(void);

/**
 * @brief	  enable automatic power up from passive power off state when power ready 
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_AutoPowerOnEnable(void);

/**
 * @brief	  disable automatic power up from passive power off state when power ready 
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_AutoPowerOnDisable(void);

/**
 * @brief	Check the automatic power up mode enable or disable.
 *
 * @param
 *   		NONE
 * @return
 *   		TRUE  : Enable.
 *   		FALSE : Disable.
 * @note
 */
bool BACKUP_IsAutoPowerOnEnable(void);

/**
 * @brief	enable ldo12 off when powerdown
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_LDO12OffEnable(void);

/**
 * @brief	disable ldo12 off when powerdown
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_LDO12OffDisable(void);

/**
 * @brief	get the ldo12 status when powerdown
 *
 * @param
 *   		NONE
 * @return
 *   		TRUE  :Enable.
 *   		FALSE :Disable.
 * @note
 */
bool BACKUP_IsLDO12OffEnable(void);

/**
 * @brief	select RC32K or OSC32K
 *
 * @param
 *			RC32K_SOURCE  : select RC32K
 *			OSC32K_SOURCE : select OSC32K
 * @return
 *   		NONE
 * @note
 */
void BACKUP_32KSourceSelect(CLK_32K_SEL Clk32kSel);

/**
 * @brief	Get Backup 32K is RC32K or OSC32K
 *
 * @param
 *			NONE
 * @return
 *   		RC32K_SOURCE  : select RC32K
 *			OSC32K_SOURCE : select OSC32K
 * @note
 */
CLK_32K_SEL BACKUP_32KSourceGet(void);

/**
 * @brief	select between low speed osc clock and high speed osc clock
 *
 * @param
 *			OSC32K_MODE_LOW_FREQ_OSC = 0:select 32K clock(RC32K/OSC32K)
 *			xMHZ_MODE_HIGH_FREQ_OSC  = 1:select xMHz high frequency osc clock
 * @return
 *   		NONE
 * @note
 */
void BACKUP_32KModeSelect(BACKUP_CLK_32K_MODE Clk32kMode);

/**
 * @brief	Get 32K is low speed osc clock or high speed osc clock
 *
 * @param
 *			NONE
 * @return
 *   		OSC32K_MODE_LOW_FREQ_OSC :select 32K clock(RC32K/OSC32K).
 *			xMHZ_MODE_HIGH_FREQ_OSC  :select xMHz high frequency osc clock.
 * @note
 */
BACKUP_CLK_32K_MODE BACKUP_32KModeGet(void);

/**
 * @brief	 Indicate osc clock is toggling, when this bit is toggling
 *
 * @param
 *			NONE
 * @return
 *   		TRUE  :backup clk is low level.
 *   		FLASE :backup clk is high level.
 * @note
 * 			this function return would be 0 1 0 1 0...
 */
bool BACKUP_IsOscClkToggle(void);

/**
 * @brief	enable RC32K or OSC32K
 *
 * @param
 *			RC32K_SOURCE :enable RC32K
 *			OSC32K_SOURCE:enable OSC32K
 * @return
 *   		NONE
 * @note
 */
void BACKUP_32KEnable(CLK_32K_SEL Clk32kSel);

/**
 * @brief	disable RC32K or OSC32K
 *
 * @param
 *			RC32K_SOURCE:disable RC32K
 *			OSC32K_SOURCE:disable OSC32K
 * @return
 *   		NONE
 * @note
 */
void BACKUP_32KDisable(CLK_32K_SEL Clk32kSel);

/**
 * @brief	get RC32K status or OSC32K status.
 *
 * @param
 *			RC32K_SOURCE  : RC32K
 *			OSC32K_SOURCE : OSC32K
 * @return
 *   		TRUE  :Enable.
 *   		FALSE :Disable.
 * @note
 */
bool BACKUP_Is32KEnable(CLK_32K_SEL Clk32kSel);

/**
 * @brief	 choose if 32K clock is from oscillator or external direct input
 * @param	OscDircet
 *   @arg 0 : use oscillator
 *   @arg 1 : external direct input
 * @return	NONE
 * @note
 */
void BACKUP_OscDirectInputSelect(OSC_DIRECT_INPUT_SEL OscDircet);

/**
 * @brief	clear powerkey trigger state
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_PowerKeyStateClear(void);

/**
 * @brief	set powerkey mode,hard mode or soft mode
 *
 * @param
 *	 HARD_MODE = 0:powerkey hardware mode
 *	 SOFT_MODE = 1:powerkey software mode
 * @return
 *   		NONE
 * @note
 */
void BACKUP_PowerKeyModeSet(POWERKEY_SWITCH_MODE PowerKeyMode);

/**
 * @brief	get powerkey mode,hardware mode or software mode
 *
 * @param
 *   		NONE
 * @return
 *	 HARD_MODE = 0:powerkey hardware mode
 *	 SOFT_MODE = 1:powerkey software mode
 * @note
 */
POWERKEY_SWITCH_MODE BACKUP_PowerKeyModeGet(void);

/**
 * @brief	config level trigger or edge mode when in hard mode
 *
 * @param
 * 	    LEVEL_TRIGGER = 0:config level mode
 *	    EDGE_TRIGGER = 1:config edge mode
 * @return
 *   		NONE
 * @note
 */
void BACKUP_PowerKeyHardModeSet(HARD_TRIGGER_MODE HardTriggerMode);


/**
 * @brief	get powerkey mode when in hardware mode,level trigger or edge mode
 *
 * @param
 *   		NONE
 * @return
 *  	LEVEL_TRIGGER = 0:config level mode
 *	    EDGE_TRIGGER = 1:config edge mode*
 * @note
 */
HARD_TRIGGER_MODE BACKUP_PowerKeyHardModeGet(void);


/**
 * @brief	config active mode when in hard level mode
 *
 * @param
 *		LOW_INDICATE_POWERON: powerkey high indicate power off ,powerkey low indicate power on
 *		HIGH_INDICATE_POWERON:1:powerkey high indicate power on ,powerkey low indicate power off
 * @return
 *   		NONE
 * @note
 */
void BACKUP_PowerKeyActiveLevelSet(POWERKEY_ACTIVE_LEVEL PowerkeyActiveLevel);


/**
 * @brief	get active mode when in hard level mode
 *
 * @param
 *   		NONE
 * @return
 *		LOW_INDICATE_POWERON:powerkey high indicate power off ,powerkey low indicate power on
 *		HIGH_INDICATE_POWERON:1:powerkey high indicate power on,powerkey low indicate power off
 * @note
 */
POWERKEY_ACTIVE_LEVEL BACKUP_PowerKeyActiveLevelGet(void);

/**
 * @brief	set the onkey trigger time
 *
 * @param
 *   		cnt:define how long PAD_ONKEY=1 lasts to power up the core. the time is equal to (256*s_trg_max_cnt+1)*(32k clock period)us.
 *              when okney_en open,s_trg_max_cnt should not lower than 1;

 * @return
 *   		NONE
 * @note
 */
void BACKUP_PowerKeyTrigMaxCntSet(uint8_t cnt);

/**
 * @brief	get the powerkey trigger time
 *
 * @param
 *   		NONE
 * @return
 *   		cnt:define how long PAD_ONKEY=1 lasts to power up the core. the time is equal to (256*s_trg_max_cnt+1)*(32k clock period)us.
 *              when okney_en open,s_trg_max_cnt should not lower than 1;
 * @note
 */
uint8_t BACKUP_PowerKeyTrigMaxCntGet(void);

/**
 * @brief	get powerkey trigger state
 *
 * @param
 *   		NONE
 * @return
 *   		true:powerkey trigger state is triggered
 *   		false:powerkey trigger state is not triggered
 * @note
 */
bool BACKUP_PowerKeyTrigStateGet(void);

/**
 * @brief	get powerkey pin state
 *
 * @param
 *   		NONE
 * @return
 *   		true:powerkey pin is high
 *   		false:powerkey state is low
 * @note
 */
bool BACKUP_PowerKeyPinStateGet(void);

/**
 * @brief	set powerkey enable
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_PowerKeyEnable(void);

/**
 * @brief	set powerkey disable
 *
 * @param
 *   		NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_PowerKeyDisable(void);

/**
 * @brief	get powerkey status,enable disable
 *
 * @param
 *   		NONE
 * @return
 *   		true:enable
 *   		false:disable
 * @note
 */
bool  BACKUP_IsPowerKeyEnable(void);

/**
 * @brief	get bool to determine if the backup pad was first powered on.
 * @param	NONE
 * @return	true:first time power on.
 *   		false:not the first time.
 * @note
 */
bool  BACKUP_FristPowerOnFlagGet(void);

/**
 * @brief	clean NVM mem when first power on
 * @param	none
 * @return  none
 * @note
 */
void BACKUP_NVMInit(void);

/**
 * @brief	read NVM data
 *
 * @param
 *	NvmAddr:the start address is 0,the end address is 15
 *	Buf:the buffer you read from the NVM
 *	Length:it is form 1~16,NvmAddr + Length <= 16
 * @return
 *   		true:read success
 *   		false:read fail because the parameter has error
 * @note
 */
bool BACKUP_NvmRead(uint8_t NvmAddr, uint8_t* Buf, uint8_t Length);

/**
 * @brief	write NVM data
 *
 * @param
 *	NvmAddr:the start address is 0,the end address is 15
 *	Buf:the buffer you read from the NVM
 *	Length:it is form 1~16,NvmAddr + Length <= 16
 * @return
 *   		true:write success
 *   		false:write fail because the parameter has error
 * @note
 */
bool BACKUP_NvmWrite(uint8_t NvmAddr, uint8_t* Buf, uint8_t Length);

/**
 * @brief	set reserve register one bit to 1
 * @param	BitMask register bit mask[bit0--bit7]
 * @return	none
 * @note bit0,bit1 is used by system
 */
void BACKUP_ResRegOneBitSet(uint8_t BitMask);

/**
 * @brief	clear reserve register one bit to 0
 * @param	BitMask register bit mask[bit0--bit7]
 * @return	none
 * @note bit0,bit1 is used by system
 */
void BACKUP_ResRegOneBitClear(uint8_t BitMask);

/**
 * @brief	get reserve register value
 * @param	none
 * @return	register value
 * @note
 */
uint8_t BACKUP_ResRegGet(void);

/**
 * @brief	Set powerkey pull up resistance value 330K.
 *
 * @param
 *   		NONE.
 * @return
 *   		NONE.
 * @note
 */
void BACKUP_VBK330KMode(void);

/**
 * @brief	Set powerkey pull up resistance value 22K.
 *
 * @param
 *   		NONE.
 * @return
 *   		NONE.
 * @note
 */
void BACKUP_VBK22KMode(void);

/**
 * @brief	Get powerkey pull up resistance value.
 *
 * @param
 *   		NONE.
 * @return
 *   		PULLUP_22K  :pullup value 22K.
 *      	PULLUP_330K :pullup value 330K.
 * @note
 */
POWERKEY_RES_TUNNING BACKUP_VBKModeGet(void);

/**
 * @brief	Set the comparator reference 800mV.
 *
 * @param
 *   		NONE.
 * @return
 *   		NONE.
 * @note
 */
void BACKUP_VTH800mVMode(void);

/**
 * @brief	Set the comparator reference 2400mV.
 *
 * @param
 *   		NONE.
 * @return
 *   		NONE.
 * @note
 */
void BACKUP_VTH2400mVMode(void);

/**
 * @brief	Get the comparator reference.
 *
 * @param
 *   		NONE.
 * @return
 *   		VTH_0V8 :reference 0.8V.
 *			VTH_2V4 :reference 2.4V.
 * @note
 */
POWERKEY_VTH_TUNNING BACKUP_VTHModeGet(void);

/**
 * @brief	 when osc clock disappear,rc32K do not work.The all 32K disappear.
 * 			 Use this function reset clock switcher to recover to rc32k clock.
 *
 * @param
 *			NONE
 * @return
 *   		NONE
 * @note
 */
void BACKUP_ForceUsingRc32Enable(void);

/**
 * @brief	Get powerup event flag.
 *
 * @param
 * 			NONE
 * @return
 *   		SEL_VIN				 :VIN change to powerup.
 *			GPIO_C0				 :GPIO C0 change to powerup.
 *			VALID_POWERKEY_EVENT :Powerkey change to powerup.
 *			RTC_EVNET			 :RTC alarm to powerup.
 * @note
 */
POWERUP_EVENT_SEL BACKUP_PowerupEventGet(void);

/**
 * @brief	Clear powerup event flag.
 *
 * @param
 * 			NONE.
 * @return
 *   		NONE.
 * @note
 */
void BACKUP_PowerupEventClr(void);


void BACKUP_VBK22KMode(void);

/**
 * @brief	GPIO_C0 Register Set.
 *
 * @param
 * 			RegOffset: such as BKUP_GPIO_C0_REG_IE_OFF
 * @param
 * 			SetVal: such as TURE
 * @return
 *   		NONE.
 * @note
 */
void BACKUP_C0RegSet(uint32_t RegOffset,uint8_t SetVal);

#ifdef __cplusplus
}
#endif

#endif

/**
 * @}
 * @}
 */
