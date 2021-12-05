/**
 **************************************************************************************
 * @file    Key.c
 * @brief   
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-01-11 17:30:47$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include "adc_levels.h"
#include "app_config.h"
#include "app_message.h"
#include "debug.h"
#include "key.h"
#include "ctrlvars.h"
#include "timeout.h"
#ifdef CFG_RES_ADC_KEY_SCAN
#include "adc_key.h"
#endif
#ifdef CFG_RES_IR_KEY_SCAN
#include "ir_key.h"
#endif
#ifdef CFG_RES_CODE_KEY_USE
#include "code_key.h"
#endif
#ifdef CFG_RES_IO_KEY_SCAN
#include "io_key.h"
#endif

#if (defined(CFG_FUNC_BACKUP_EN) && defined(USE_POWERKEY_SOFT_PUSH_BUTTON))
#include "power_key.h"
#endif

extern void GIE_DISABLE(void); //defined@interrupt.c
extern void GIE_ENABLE(void);

uint16_t gFuncID = 0;
uint16_t gKeyValue;
#ifdef CFG_FUNC_DBCLICK_MSG_EN
KEYBOARD_MSG dbclick_msg;
TIMER	DBclicTimer;
#endif
/*************************************************
* ADC KEY按键属性对应消息列表
*
*  注：用户需要根据功能定义重新修改此表！！！
***************************************************/
static const uint16_t ADKEY_TAB[][KEY_MSG_DEFAULT_NUM] =
{
	//KEY_PRESS       SHORT_RELEASE                 LONG_PRESS                  KEY_HOLD                    LONG_PRESS_RELEASE

	//power adc key
	{MSG_NONE,        MSG_PITCH_UP,                 MSG_POWERDOWN,  			MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_MEDIA_PLAY_BROWER_UP,     MSG_MIC_BASS_DW,    		MSG_MIC_BASS_DW,   			MSG_NONE},
	{MSG_NONE,        MSG_MEDIA_PLAY_BROWER_DN,	    MSG_MIC_BASS_UP,    		MSG_MIC_BASS_UP,   			MSG_NONE},
	{MSG_NONE,        MSG_MEDIA_PLAY_BROWER_ENTER,	MSG_MIC_TREB_DW,    		MSG_MIC_TREB_DW,   			MSG_NONE},
	{MSG_NONE,        MSG_MEDIA_PLAY_BROWER_RETURN,	MSG_MIC_TREB_UP,    		MSG_MIC_TREB_UP,   			MSG_NONE},
	{MSG_NONE,        MSG_MIC_EFFECT_UP,	        MSG_MIC_EFFECT_UP,          MSG_MIC_EFFECT_UP,          MSG_NONE},
	{MSG_NONE,        MSG_MIC_EFFECT_DW,            MSG_MIC_EFFECT_DW,          MSG_MIC_EFFECT_DW,          MSG_NONE},
	{MSG_NONE,        MSG_NONE,   					MSG_NONE,  					MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NONE,  					MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NONE,     				MSG_NONE,   				MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NONE,  					MSG_NONE,  					MSG_NONE,                   MSG_NONE},

	//adc1 key
#ifdef CFG_APP_REST_MODE_EN
	{MSG_NONE,        MSG_PLAY_PAUSE,   			MSG_POWER,      		    MSG_NONE,                   MSG_NONE},
#else
	{MSG_NONE,        MSG_PLAY_PAUSE,   			MSG_DEEPSLEEP,      		MSG_NONE,                   MSG_NONE},
#endif
	{MSG_NONE,        MSG_PRE,          			MSG_FB_START,               MSG_FB_START,               MSG_FF_FB_END},
	{MSG_NONE,        MSG_NEXT,	        			MSG_FF_START,               MSG_FF_START,               MSG_FF_FB_END},
	{MSG_NONE,        MSG_MUSIC_VOLDOWN,	    	MSG_MUSIC_VOLDOWN,   		MSG_MUSIC_VOLDOWN,			MSG_NONE},
	{MSG_NONE,        MSG_MUSIC_VOLUP,	    		MSG_MUSIC_VOLUP,     		MSG_MUSIC_VOLUP,     		MSG_NONE},
	{MSG_NONE,        MSG_EQ,	        			MSG_3D,        				MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_MUTE,         			MSG_VB,        				MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_EFFECTMODE,   			MSG_VOCAL_CUT, 				MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_REPEAT,  					MSG_REPEAT_AB,              MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_REC,     					MSG_REC_PLAYBACK,           MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_MODE,  					MSG_STOP, 				    MSG_NONE,                   MSG_NONE},

	//adc2 key
	{MSG_NONE,        MSG_BROWSE,   				MSG_REC_FILE_DEL,     		MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_FOLDER_PRE,    			MSG_BT_HF_CALL_REJECT,      MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_FOLDER_NEXT,	 			MSG_BT_HF_REDAIL_LAST_NUM,  MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_RTC_SET_TIME,	     		MSG_RTC_DISP_TIME,          MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_RTC_SET_ALARM,	     	MSG_MIC_FIRST,              MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_RTC_UP,	     		    MSG_BT_HF_VOICE_RECOGNITION,MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_RTC_DOWN,       		    MSG_HDMI_AUDIO_ARC_ONOFF,   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_BT_CONNECT_CTRL, 			MSG_BT_ENTER_DUT_MODE,      MSG_NONE,                   MSG_NONE},
#ifdef	CFG_XIAOAI_AI_EN
	{MSG_NONE,        MSG_BT_XM_AI_START,           MSG_UPDATE,         MSG_NONE,                   MSG_BT_XM_AI_STOP},
#else
	#ifdef	CFG_FUNC_AI_EN
	{MSG_NONE,        MSG_BT_AI,           			MSG_UPDATE,         MSG_NONE,                   MSG_BT_XM_AI_STOP},
	#else
	{MSG_NONE,        MSG_NONE,           			MSG_UPDATE,         MSG_NONE,                   MSG_BT_XM_AI_STOP},
	#endif
#endif
	{MSG_NONE,        MSG_MIC_VOLDOWN,   			MSG_MIC_VOLDOWN, 			MSG_MIC_VOLDOWN,  			MSG_NONE},
	{MSG_NONE,        MSG_MIC_VOLUP,     			MSG_MIC_VOLUP,   			MSG_MIC_VOLUP,    			MSG_NONE},

};

/*************************************************
* GPIO按键属性对应消息列表
*
*  注：用户需要根据功能定义重新修改此表！！！
***************************************************/
static const uint16_t IOKEY_TAB[][KEY_MSG_DEFAULT_NUM] =
{
    //KEY_PRESS       SHORT_RELEASE                 LONG_PRESS                  KEY_HOLD                    LONG_PRESS_RELEASE

	{MSG_NONE,        MSG_EFFECTMODE,               MSG_SOFT_POWER,  			MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NONE,      				MSG_NONE,    				MSG_NONE,   				MSG_NONE},
	{MSG_NONE,        MSG_NONE,	    				MSG_NONE,    				MSG_NONE,   				MSG_NONE},
};

/*************************************************
* code按键属性对应消息列表
*
*  注：用户需要根据功能定义重新修改此表！！！
***************************************************/
static const uint16_t CODEKEY_TAB[][KEY_MSG_DEFAULT_NUM] =
{
	//KEY_PRESS       SHORT_RELEASE                 LONG_PRESS                  KEY_HOLD                    LONG_PRESS_RELEASE

	{MSG_NONE,        MSG_MUSIC_VOLDOWN,        	MSG_MUSIC_VOLDOWN,   		MSG_MUSIC_VOLDOWN,  		MSG_NONE},
	{MSG_NONE,        MSG_MUSIC_VOLUP,          	MSG_MUSIC_VOLUP,     		MSG_MUSIC_VOLUP,    		MSG_NONE},
};

/*************************************************
* 遥控按键属性对应消息列表
*
*  注：用户需要根据功能定义重新修改此表！！！
***************************************************/
static const uint16_t IRKEY_TAB[][KEY_MSG_DEFAULT_NUM] =
{
	//KEY_PRESS       SHORT_RELEASE                 LONG_PRESS                  KEY_HOLD                    LONG_PRESS_RELEASE
#ifdef CFG_APP_REST_MODE_EN
	{MSG_NONE,        MSG_POWER,    			    MSG_POWER,  			    MSG_NONE,                   MSG_NONE},
#else
	{MSG_NONE,        MSG_DEEPSLEEP,    			MSG_DEEPSLEEP,  			MSG_NONE,                   MSG_NONE},
#endif
	{MSG_NONE,        MSG_MODE,  					MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_MUTE,	        			MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_PLAY_PAUSE,   		    MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_PRE,          			MSG_FB_START,         		MSG_FB_START,               MSG_FF_FB_END},
	{MSG_NONE,        MSG_NEXT,	        			MSG_FF_START,         		MSG_FF_START,               MSG_FF_FB_END},
	{MSG_NONE,        MSG_EQ,           			MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_MUSIC_VOLUP,	    		MSG_MUSIC_VOLUP,      		MSG_MUSIC_VOLUP,     		MSG_NONE},
	{MSG_NONE,        MSG_MUSIC_VOLDOWN,	    	MSG_MUSIC_VOLDOWN,    		MSG_MUSIC_VOLDOWN,   		MSG_NONE},
	{MSG_NONE,        MSG_REPEAT,                   MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_RADIO_PLAY_SCAN,          MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_1,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_2,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_3,	        		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_4,	        		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_5,	        		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_6,	        		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_7,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_8,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_9,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
	{MSG_NONE,        MSG_NUM_0,            		MSG_NONE,                   MSG_NONE,                   MSG_NONE},
};

static const uint16_t PWRKey_TAB = MSG_PLAY_PAUSE;

static char *mp_key_event[] = {
	"ADC_KEY_UNKOWN_TYPE"		,
	"ADC_KEY_PRESSED"			,
	"ADC_KEY_RELEASED"			,
	"ADC_KEY_LONG_PRESSED"		,
	"ADC_KEY_LONG_PRESS_HOLD"	,
	"ADC_KEY_LONG_RELEASED"		,
};

uint32_t GetGlobalKeyValue(void)
{
    return gFuncID;
}

void ClrGlobalKeyValue(void)
{
    gFuncID = 0;
}


#if (defined(CFG_RES_ADC_KEY_SCAN) ||defined(CFG_RES_IO_KEY_SCAN)|| defined(CFG_RES_IR_KEY_SCAN) || defined(CFG_RES_CODE_KEY_USE) ||  defined(CFG_ADC_LEVEL_KEY_EN))

/**
 * @func        SetGlobalKeyValue
 * @brief       SetGlobalKeyValue,本函数主要目的是多种按键类的消息归一化映射
 * @param       uint8_t KeyType:注意 按键事件类型带转译
                uint16_t KeyValue，Uart提供单独一列映射，键值和旋钮一列，按键事件类型一列，如有必要可拆开单列。
 * @Output      gFuncID:
 				gKeyValue
 * @return      void
 * @Others      
 * Record
 * 1.Date        : 20180119
 *   Author      : pi.wang
 *   Modification: Created function
*/
void SetGlobalKeyValue(uint8_t KeyType, uint16_t KeyValue)
{
}

void SetIrKeyValue(uint8_t KeyType, uint16_t KeyValue)
{
	gFuncID = IRKEY_TAB[KeyValue][KeyType - 1];
}

void SetAdcKeyValue(uint8_t KeyType, uint16_t KeyValue)
{
	gFuncID = ADKEY_TAB[KeyValue][KeyType - 1];
}


void KeyInit(void)
{
#ifdef CFG_RES_ADC_KEY_USE
	AdcKeyInit();
#endif

#ifdef CFG_RES_IO_KEY_SCAN
	IOKeyInit();
#endif

#ifdef CFG_RES_IR_KEY_USE
	IRKeyInit();
#endif

#ifdef CFG_RES_CODE_KEY_USE
	CodeKeyInit();
#endif

#ifdef CFG_ADC_LEVEL_KEY_EN
	ADCLevelsKeyInit();
#endif

#if (defined(CFG_FUNC_BACKUP_EN) && defined(USE_POWERKEY_SOFT_PUSH_BUTTON))
	PowerKeyScanInit();
#endif
#ifdef CFG_FUNC_DBCLICK_MSG_EN
	DbclickInit();
#endif
}

inline bool GIE_STATE_GET(void);

/**
 * @func        KeyScan
 * @brief       KeyScan,根据键值和事件类型查表，输出消息值
 * @param       None  
 * @Output      None
 * @return      MessageId 对应按键的事件代号
 * @Others      
 * Record
 * 1.Date        : 20180123
 *   Author      : pi.wang
 *   Modification: Created function
*/
MessageId KeyScan(void)
{
	MessageId KeyMsg = MSG_NONE;
#ifdef CFG_RES_IO_KEY_SCAN//bkd add
	IOKeyMsg ioKeyMsg;
#endif
	
#ifdef CFG_RES_ADC_KEY_SCAN//bkd add
	AdcKeyMsg AdcKeyMsg;
#endif
#ifdef CFG_RES_IR_KEY_SCAN
	IRKeyMsg IRKeyMsg;
#endif
#ifdef CFG_RES_CODE_KEY_USE
	CodeKeyType CodeKey = CODE_KEY_NONE;
#endif

#if (defined(CFG_FUNC_BACKUP_EN) && defined(USE_POWERKEY_SOFT_PUSH_BUTTON))
	PWRKeyMsg PWRKeyMsg;
#endif

	/**
	 * @brief adc按键
	 */
#ifdef CFG_RES_ADC_KEY_USE
	AdcKeyMsg = AdcKeyScan();
	if(AdcKeyMsg.index != ADC_CHANNEL_EMPTY && AdcKeyMsg.type != ADC_KEY_UNKOWN_TYPE)
	{
		BeepEnable();
		gFuncID = ADKEY_TAB[AdcKeyMsg.index][AdcKeyMsg.type - 1];
		trace_verboseln("AdcKeyMsg index = %d, event = %s", AdcKeyMsg.index, mp_key_event[AdcKeyMsg.type]);
	}
#endif

	/**
	 * @brief IO按键
	 */
#ifdef CFG_RES_IO_KEY_SCAN
	ioKeyMsg = IOKeyScan();
	if(ioKeyMsg.index != IO_CHANNEL_EMPTY && ioKeyMsg.type != IO_KEY_UNKOWN_TYPE)
	{
		gFuncID = IOKEY_TAB[ioKeyMsg.index][ioKeyMsg.type - 1];
		APP_DBG("IOKeyindex = %d, %d\n", ioKeyMsg.index, ioKeyMsg.type);
	}
#endif

	/**
	 * @brief IR红外按键
	 */
#ifdef CFG_RES_IR_KEY_USE
	IRKeyMsg = IRKeyScan();
	if(IRKeyMsg.index != IR_KEY_NONE && IRKeyMsg.type != IR_KEY_UNKOWN_TYPE)
	{
		gFuncID = IRKEY_TAB[IRKeyMsg.index][IRKeyMsg.type - 1];
		APP_DBG("IRKeyindex = %d, %d\n", IRKeyMsg.index, IRKeyMsg.type);
	}
#endif

	/**
	 * @brief 编码器按键
	 */
#ifdef CFG_RES_CODE_KEY_USE
	CodeKey = CodeKeyScan();
	if(CodeKey != CODE_KEY_NONE)
	{
		gFuncID = CODEKEY_TAB[CodeKey-1][1];
	}
#endif

#ifdef CFG_ADC_LEVEL_KEY_EN
	KeyMsg = AdcLevelKeyProcess();

	if(KeyMsg != MSG_NONE)
	{
		gFuncID = KeyMsg;
	}			
#endif

#if (defined(CFG_FUNC_BACKUP_EN) && defined(USE_POWERKEY_SOFT_PUSH_BUTTON))
	PWRKeyMsg = PowerKeyScan();
	if(PWRKeyMsg.type == PWR_KEY_SP)
	{
		gFuncID = PWRKey_TAB;
	}
#endif

	{
		/**
		 * 按键双击处理
		 */
#ifdef CFG_FUNC_DBCLICK_MSG_EN
		DbclickGetMsg(gFuncID);

		DbclickProcess();
#endif

		KeyMsg = GetGlobalKeyValue();

		ClrGlobalKeyValue();
	}
	return KeyMsg;
}

#endif
/*************************************************
* 按键双击初始化处理函数，只支持单击释放的双击
* dbclick_msg 变量如果定义成数组，则可实现多个双击功能
*
***************************************************/
void DbclickInit(void)
{
#ifdef CFG_FUNC_DBCLICK_MSG_EN
	dbclick_msg.dbclick_en            = 1;
	dbclick_msg.dbclick_counter       = 0;
	dbclick_msg.dbclick_timeout       = 0;

	dbclick_msg.KeyMsg                = CFG_PARA_CLICK_MSG;////Single click msg
	dbclick_msg.dbclick_msg           = CFG_PARA_DBCLICK_MSG;//double  click msg
	TimeOutSet(&DBclicTimer, 0);
#endif

}

/*************************************************
* 按键双击或多击处理函数
*
*
***************************************************/
void DbclickProcess(void)
{
#ifdef CFG_FUNC_DBCLICK_MSG_EN
	if(!IsTimeOut(&DBclicTimer))
	{
		return;
	}
	TimeOutSet(&DBclicTimer, 4);

	if(dbclick_msg.dbclick_en)
	{
		if(dbclick_msg.dbclick_timeout)
		{
			dbclick_msg.dbclick_timeout--;
			if(dbclick_msg.dbclick_timeout == 0)
			{
				gFuncID = dbclick_msg.KeyMsg;//Single click msg
				dbclick_msg.dbclick_counter = 0;
				dbclick_msg.dbclick_timeout = 0;
				APP_DBG("shot click_msg \n");
			}
		}
	}
#endif ///#ifdef CFG_FUNC_DBCLICK_MSG_EN
}
/*************************************************
* 判断按键双击键值处理函数
*
*
***************************************************/
uint8_t DbclickGetMsg(uint16_t Msg)
{
#ifdef CFG_FUNC_DBCLICK_MSG_EN
	if(dbclick_msg.dbclick_en == 0)   return 0;
	/////此处可增加工作模式判断，不是所有模式需要双击功能////////
	//if( WORK_MODE != BT_MODE) return 0
	if((dbclick_msg.KeyMsg == Msg))
	{
    	gFuncID = 0;

		if(dbclick_msg.dbclick_timeout == 0)
		{
    	   dbclick_msg.dbclick_timeout =  CFG_PARA_DBCLICK_DLY_TIME;///4ms*20=80ms
			dbclick_msg.dbclick_counter += 1;
    	   APP_DBG("start double click_msg \n");
		}
		else
		{
			gFuncID =  dbclick_msg.dbclick_msg;//double  click msg
			dbclick_msg.dbclick_counter = 0;
			dbclick_msg.dbclick_timeout = 0;
			APP_DBG("double click_msg \n");
		}
		return 1;
	}
	else
	{
		if(Msg != MSG_NONE)
		{
			dbclick_msg.dbclick_timeout = 0;
			dbclick_msg.dbclick_counter = 0;
		}
		return 0;
	}
#else
	return 0;
#endif
}

