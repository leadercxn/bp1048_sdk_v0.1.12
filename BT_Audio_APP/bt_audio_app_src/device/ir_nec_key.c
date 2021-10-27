/**
 **************************************************************************************
 * @file    ir_nec_key.c
 * @brief   nec key
 *
 * @author  Cecilia Wang
 * @version V1.1.0
 *
 * $Created: 2015-08-25 16:24:11$
 *
 * @Copyright (C) 2015, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
#include "app_config.h"
#include <string.h>
#include "gpio.h"
#include "type.h"
#include "debug.h" 
#include "ir_nec_key.h" 
#include "pwc.h"
#include "dma.h"
#include "clk.h"
#include "irqn.h"
#include "ir.h"

 #ifdef CFG_RES_IR_KEY_USE
 
static uint8_t Last_RP_CNT = 0xff;
static bool IRHasReveived=0;
static uint32_t Cmd=0;

//初始化配置
void IRNecKeyInit(void)
{
	IR_Config(IR_MODE_NEC, CFG_RES_IR_PIN, IR_NEC_32BITS);
	IR_Enable();

}

//清空数据buf 避免数据滞留  典型：sleep前，连续的电源键数据需清理
void IRNecKeyReset(void)
{

#if 0
	PWC_Disable(IR_NEC_TIMER_INDEX);
	IRNecRcvCnt = 0;
	memset(IRNecRcvBuf, 0, sizeof(IRNecRcvBuf));
	memset(IRNecDmaRcvBuf, 0, sizeof(IRNecDmaRcvBuf));
	PWC_Enable(IR_NEC_TIMER_INDEX);
#endif
	
}


uint32_t GetIRNecKeyIndex(void)
{
	uint8_t RP_CNT = 0;

	if(IRHasReveived==0)
	{		
		if(IR_CommandFlagGet())
		{
			Cmd =  IR_CommandDataGet();
			IR_IntFlagClear();
			RP_CNT = IR_RepeatCountGet();  
			Last_RP_CNT=RP_CNT;
			//APP_DBG("\n RP_CNT_0:%x\n",RP_CNT);
			IRHasReveived=1;
			goto IR_Receive_Out;

		}
	}	
	
	if(IRHasReveived==1)
	{		
		RP_CNT = IR_RepeatCountGet();
		//APP_DBG("\n RP_CNT_now=%x last=%x\n",RP_CNT,Last_RP_CNT);

		if( (Last_RP_CNT - RP_CNT) > 0 )
		{
			Cmd =  IR_CommandDataGet();
			APP_DBG("\n*New IR_CmdTTTTTTTT:%lx\n",Cmd);
			IR_IntFlagClear();
			IRHasReveived=1;
		}

		if(RP_CNT == Last_RP_CNT)
		{			
			Cmd=0;
			IRHasReveived=0;
			Last_RP_CNT=0;
			IR_CommandFlagClear();			
		}
		else
		{		
			Last_RP_CNT = RP_CNT;			
		}
	}
		
IR_Receive_Out:
	
    return Cmd;
}

#endif
