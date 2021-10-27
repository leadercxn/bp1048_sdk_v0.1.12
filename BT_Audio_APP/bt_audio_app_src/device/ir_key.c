/**
 **************************************************************************************
 * @file    ir_key.c
 * @brief   ir key
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2018-3-15 14:03:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include "type.h"
#include "gpio.h"
#include "timeout.h"
#include "ir_nec_key.h"
#include "ir_key.h"
#include "debug.h"
#include "app_config.h"
#include "ir.h"

#define 	IR_KEY_SCAN_TIME			110		//注意该时间长短设置略大于IR长按建码间隔，NEC=108ms
#define		IR_KEY_CP_TIME				1000	//CP condition is 1s
#define 	IR_KEY_CPH_TIME				350
#define 	IR_KEY_CPH_SPEED			100
#define		IR_KEY_SUM					21		//IR KEY SUM

#ifdef CFG_RES_IR_KEY_USE
typedef enum _IR_KEY_STATE
{
    IR_KEY_STATE_IDLE,
    IR_KEY_STATE_JITTER,
    IR_KEY_STATE_PRESS_DOWN,
    IR_KEY_STATE_CP

} IR_KEY_STATE;


static TIMER			IRKeyWaitTimer;
static TIMER			IRKeyScanTimer;
static IR_KEY_STATE		IRKeyState;
static uint8_t KeyIndex = IR_KEY_NONE;
static IRKeyMsg         IRKeyCp;
#ifndef IR_MANU_ID
#define IR_MANU_ID		0x7F80//0xFF00//不同遥控器会不同
#endif

// Customer IR key value table, fix it by case.
static const uint8_t gIrVal[IR_KEY_SUM] =
{
#ifdef IR_KEY_TABLE
    IR_KEY_TABLE,
#else
	0xBA,		// POWER		//配合.h中#define		IR_KEY_POWER   ??
	0xB9,		// MODE
	0xB8,		// MUTE

	0xBB,		// PLAY/PAUSE
	0xBF,		// PRE
	0xBC,		// NEXT

	0xF8,		// EQ
	0xEA,		// VOL-
	0xF6,		// VOL+

	0xE9,		// 0
	0xE6,		// REPEAT
	0xF2,		// SCN

	0xF3,		// 1
	0xE7,		// 2
	0xA1,		// 3

	0xF7,		// 4
	0xE3,		// 5
	0xA5,		// 6

	0xBD,		// 7
	0xAD,		// 8
	0xB5,		// 9
#endif
};

//get ir key index
//static 
uint8_t IRKeyIndexGet(void)
{
    static uint32_t IrKeyVal;

    IrKeyVal = GetIRNecKeyIndex();

    if(((IrKeyVal >> 24) & 0x000000FF) + ((IrKeyVal >> 16) & 0x000000FF) != 0xFF)
    {
        KeyIndex = IR_KEY_NONE;
        return IR_KEY_NONE;
    }

   // APP_DBG("IrKeyVal = %08x\n", IrKeyVal);

    if((IrKeyVal & 0x0000FFFF) == IR_MANU_ID)
    {
        APP_DBG("IrKeyVal = %lx\n", IrKeyVal);
        for(KeyIndex = 0; KeyIndex < IR_KEY_SUM; KeyIndex++)
        {
            if(((IrKeyVal >> 24) & 0x000000FF) == gIrVal[KeyIndex])
            {    
                return KeyIndex;
            }
        }
    }

    return IR_KEY_NONE;
}

IRKeyMsg IRKeyScan(void)
{
	uint8_t				KeyIndex;
	static  uint8_t 	PreKeyIndex = IR_KEY_NONE;
	IRKeyMsg 			Msg = {IR_KEY_NONE, IR_KEY_UNKOWN_TYPE};

	if(!IsTimeOut(&IRKeyScanTimer))
	{
		if(IsTimeOut(&IRKeyWaitTimer))
		{
			if ((IRKeyCp.index != IR_KEY_NONE) && (IRKeyCp.type == IR_KEY_LONG_PRESS_HOLD))
			{
                TimeOutSet(&IRKeyWaitTimer, IR_KEY_CPH_SPEED);
                Msg.index =  IRKeyCp.index;
			    Msg.type =   IRKeyCp.type;
			}
		}
		return Msg;
	}
	TimeOutSet(&IRKeyScanTimer, IR_KEY_SCAN_TIME);

	KeyIndex = IRKeyIndexGet();
	switch(IRKeyState)
	{
		case IR_KEY_STATE_IDLE:
			IRKeyCp.index = IR_KEY_NONE;
            IRKeyCp.type = IR_KEY_UNKOWN_TYPE;
			if(KeyIndex == IR_KEY_NONE)
			{
				return Msg;
			}

			//APP_DBG("IR KEY PDS!\n");
			PreKeyIndex = KeyIndex;
			TimeOutSet(&IRKeyWaitTimer, IR_KEY_CP_TIME);
			IRKeyState = IR_KEY_STATE_PRESS_DOWN;
			Msg.index = PreKeyIndex;
			Msg.type = IR_KEY_PRESSED;
			return Msg;

		case IR_KEY_STATE_PRESS_DOWN:
			if(PreKeyIndex != KeyIndex)
			{
				//APP_DBG("IR KEY SP!%x\n", PreKeyIndex);
				IRKeyState = IR_KEY_STATE_IDLE;
				Msg.index = PreKeyIndex;
				Msg.type = IR_KEY_RELEASED;
				return Msg;
			}
			else if(IsTimeOut(&IRKeyWaitTimer))
			{
				//APP_DBG("IR KEY CPS!\n");
				TimeOutSet(&IRKeyWaitTimer, IR_KEY_CPH_TIME);
				IRKeyState = IR_KEY_STATE_CP;
				Msg.index = PreKeyIndex;
				Msg.type = IR_KEY_LONG_PRESSED;
				return Msg;
			}
			break;

		case IR_KEY_STATE_CP:
			if(PreKeyIndex != KeyIndex)
			{
				//APP_DBG("IR CPR\n");
				IRKeyState = IR_KEY_STATE_IDLE;
				Msg.index = PreKeyIndex;
				Msg.type = IR_KEY_LONG_RELEASED;
				IRKeyCp.index = IR_KEY_NONE;
	            IRKeyCp.type = IR_KEY_UNKOWN_TYPE;
				return Msg;
			}
			else if(IsTimeOut(&IRKeyWaitTimer))
			{
				//APP_DBG("IR CP\n");
                TimeOutSet(&IRKeyWaitTimer, IR_KEY_CPH_TIME);
                Msg.index = PreKeyIndex;
				Msg.type = IR_KEY_LONG_PRESS_HOLD;

				IRKeyCp.index = PreKeyIndex;
                IRKeyCp.type = IR_KEY_LONG_PRESS_HOLD;
				return Msg;
			}
			break;

		default:
			IRKeyState = IR_KEY_STATE_IDLE;
			break;
	}

	return Msg;
}

int32_t IRKeyInit(void)
{
	IRKeyState = IR_KEY_STATE_IDLE;
	KeyIndex = IR_KEY_NONE;
	TimeOutSet(&IRKeyScanTimer, 0);
    IRNecKeyInit();
    IR_CommandFlagClear();
	return 0;
}

#endif
