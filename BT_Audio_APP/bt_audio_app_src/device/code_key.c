/**
 **************************************************************************************
 * @file    code_key.c
 * @brief   
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-01-18 15:30:47$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <type.h>
#include "app_config.h"
#include "gpio.h"
#include "irqn.h"
#include "code_key.h"

#ifndef CFG_APP_CONFIG 
/*********** default config, determined by the hardware circuit **************/
#define		CFG_RES_CODE_KEY_USE
#define  CFG_CODE_KEY1P_BANK                			'A'
#define  CFG_CODE_KEY1P_PIN								(9)
#define  CFG_CODE_KEY1N_BANK							'A'
#define  CFG_CODE_KEY1N_PIN                 			(10)

/****************************************************************************/
#endif //no CFG_APP_CONFIG

#ifdef CFG_RES_CODE_KEY_USE
#define CODE_KEY_GPIO_A_BANK		(CFG_CODE_KEY1P_BANK)
#define CODE_KEY_GPIO_A_PIN			(CFG_CODE_KEY1P_PIN )
#define CODE_KEY_GPIO_B_BANK		(CFG_CODE_KEY1N_BANK)
#define CODE_KEY_GPIO_B_PIN			(CFG_CODE_KEY1N_PIN )

#define CODE_KEY_GPIO_A_PIN_MASK    (1 << CODE_KEY_GPIO_A_PIN)
#define CODE_KEY_GPIO_A_IE          ((CODE_KEY_GPIO_A_BANK - 'A') * 12 + GPIO_A_IE)
#define CODE_KEY_GPIO_A_OE          ((CODE_KEY_GPIO_A_BANK - 'A') * 12 + GPIO_A_OE)
#define CODE_KEY_GPIO_A_PU          ((CODE_KEY_GPIO_A_BANK - 'A') * 12 + GPIO_A_PU)
#define CODE_KEY_GPIO_A_PD          ((CODE_KEY_GPIO_A_BANK - 'A') * 12 + GPIO_A_PD)
#define CODE_KEY_GPIO_A_IN          ((CODE_KEY_GPIO_A_BANK - 'A') * 12 + GPIO_A_IN)
#define CODE_KEY_GPIO_A_INT         ((CODE_KEY_GPIO_A_BANK - 'A') *  5 + GPIO_A_INT)
#define CODE_KEY_GPIO_B_PIN_MASK    (1 << CODE_KEY_GPIO_B_PIN)
#define CODE_KEY_GPIO_B_IE          ((CODE_KEY_GPIO_B_BANK - 'A') * 12 + GPIO_A_IE)
#define CODE_KEY_GPIO_B_OE          ((CODE_KEY_GPIO_B_BANK - 'A') * 12 + GPIO_A_OE)
#define CODE_KEY_GPIO_B_PU          ((CODE_KEY_GPIO_B_BANK - 'A') * 12 + GPIO_A_PU)
#define CODE_KEY_GPIO_B_PD          ((CODE_KEY_GPIO_B_BANK - 'A') * 12 + GPIO_A_PD)
#define CODE_KEY_GPIO_B_IN          ((CODE_KEY_GPIO_B_BANK - 'A') * 12 + GPIO_A_IN)
#define CODE_KEY_GPIO_B_INT         ((CODE_KEY_GPIO_B_BANK - 'A') * 5  + GPIO_A_INT)

//clockwise rotation
//    ----|		   |--------|		 |-------------------
//A       |        |        |        |
//        |--------|        |--------|
//     -------|        |--------|		 |---------------
//B           |        |        |		 |
//            |--------|        |--------|
//counterclockwise rotation
//    --------|		   |--------|		 |---------------
//A           |		   |        |		 |
//            |--------|        |--------|
//     ---|        |--------|		 |-------------------
//B       |        |        |		 |
//        |--------|        |--------|

static uint8_t  ClockWiseCnt;
static uint8_t  CounterClockWiseCnt;

// Initialize coding key scan (GPIO) operation.
// Config interrupt at negative edge of signal-A
void CodeKeyInit(void)
{
	//enable pull up resister.
	GPIO_RegOneBitSet(CODE_KEY_GPIO_A_IE, CODE_KEY_GPIO_A_PIN_MASK);
	GPIO_RegOneBitClear(CODE_KEY_GPIO_A_OE, CODE_KEY_GPIO_A_PIN_MASK);
	GPIO_RegOneBitSet(CODE_KEY_GPIO_A_PU, CODE_KEY_GPIO_A_PIN_MASK);
	GPIO_RegOneBitClear(CODE_KEY_GPIO_A_PD, CODE_KEY_GPIO_A_PIN_MASK);

	//enable pull up resister.
    GPIO_RegOneBitSet(CODE_KEY_GPIO_B_IE, CODE_KEY_GPIO_B_PIN_MASK);
	GPIO_RegOneBitClear(CODE_KEY_GPIO_B_OE, CODE_KEY_GPIO_B_PIN_MASK);
	GPIO_RegOneBitSet(CODE_KEY_GPIO_B_PU, CODE_KEY_GPIO_B_PIN_MASK);
	GPIO_RegOneBitClear(CODE_KEY_GPIO_B_PD, CODE_KEY_GPIO_B_PIN_MASK);

	//enable int
	GPIO_INTEnable(CODE_KEY_GPIO_A_INT, CODE_KEY_GPIO_A_PIN_MASK, GPIO_NEG_EDGE_TRIGGER);

	ClockWiseCnt        = 0;
	CounterClockWiseCnt = 0;

	//enable gpio irqc
	NVIC_EnableIRQ(Gpio_IRQn);
}

// Key process, image key value to key event.
CodeKeyType CodeKeyScan(void)
{
	CodeKeyType CodeKey = CODE_KEY_NONE;

	if(ClockWiseCnt)
	{
		CodeKey = CODE_KEY_FORWARD;
	}
	else if(CounterClockWiseCnt)
	{
		CodeKey = CODE_KEY_BACKWARD;
	}

	ClockWiseCnt        = 0;
	CounterClockWiseCnt = 0;

	return CodeKey;
}

#endif //CFG_RES_CODE_KEY_USE

// Coding key signal-A interrupt.
void GpioInterrupt(void)
{
    #ifdef CFG_RES_CODE_KEY_USE
	if(GPIO_INTFlagGet(CODE_KEY_GPIO_A_INT) & CODE_KEY_GPIO_A_PIN_MASK)
	{
		GPIO_INTFlagClear(CODE_KEY_GPIO_A_INT, CODE_KEY_GPIO_A_PIN_MASK);

		if((GPIO_RegGet(CODE_KEY_GPIO_A_IN) & CODE_KEY_GPIO_A_PIN_MASK) || (ClockWiseCnt != 0) || (CounterClockWiseCnt != 0))
		{			
			return;
		}

		if(GPIO_RegGet(CODE_KEY_GPIO_B_IN) & CODE_KEY_GPIO_B_PIN_MASK)
		{
			//clockwise rotation
			ClockWiseCnt++;
		}
		else
		{
			//counterclockwise rotation
			CounterClockWiseCnt++;
		}
	}
    
    #endif //CFG_RES_CODE_KEY_USE
}
