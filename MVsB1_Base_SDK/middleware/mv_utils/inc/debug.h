////////////////////////////////////////////////////////////////////////////////
//                   Mountain View Silicon Tech. Inc.
//		Copyright 2011, Mountain View Silicon Tech. Inc., ShangHai, China
//                   All rights reserved.
//
//		Filename	:debug.h
//
//		Description	:
//					Define debug ordinary print & debug routine
//
//		Changelog	:
///////////////////////////////////////////////////////////////////////////////

#ifndef __DEBUG_H__
#define __DEBUG_H__

/**
 * @addtogroup mv_utils
 * @{
 * @defgroup debug debug.h
 * @{
 */
 
#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include <stdio.h>
#include "type.h"
#include "gpio.h"


/************************** DEBUG**********************************
*下面为打印调试信息的接口,共有两个:
*1.打印前缀接口 : 
*  1)如果需要在打印时输出模块前缀 ,请调用 APP_DBG(),
*  2)需要关闭某模块的调试信息,可以用注释屏蔽 XXX_MODULE_DEBUG的宏
*	此模块宏,是根据如下规则 来划分的 : 
*	共划分为如下9个模块,
*	其中7个模块在Apps目录:分为media play,bt,hdmi_in,Main_task,usb_audio_mode,waiting_mode模块
*	另外两个模块是:Device模块、Services模块
*	剩下统一放在：DEB模块
*
*2.不打印前缀接口 : 调用 DBG()
*************************************************************/




#define	DEVICE_MODULE_DEBUG			//DEVICE模块打印调试信息开关
#define	SERVICE_MODULE_DEBUG		//SERVICE模块打印调试信息开关
#define	MEDIA_MODULE_DEBUG			//MEDIA模块打印调试信息开关
#define	BT_MODULE_DEBUG				//BT模块打印调试信息开关
#define	MAINTSK_MODULE_DEBUG		//MAINTSK模块打印调试信息开关
#define	USBAUDIO_MODULE_DEBUG		//USBAUDIO模块打印调试信息开关
#define	WAITING_MODULE_DEBUG		//WAITING模块打印调试信息开关



uint8_t DBG_Global(char * str,char **fmt, ...);

#define	APP_DBG(format, ...)	do{char *fmt=format;if(DBG_Global(__FILE__,&fmt, ##__VA_ARGS__))printf(fmt, ##__VA_ARGS__);}while(0)
#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define	OTG_DBG(format, ...)		//printf(format, ##__VA_ARGS__)
#define	BT_DBG(format, ...)			printf(format, ##__VA_ARGS__)//do{printf("[BT] "); printf(format, ##__VA_ARGS__);}while(0)

int DbgUartInit(int Which, unsigned int BaudRate, unsigned char DatumBits, unsigned char Parity, unsigned char StopBits);



/************************** TOGGLE DEBUG**********************************
 *
 * 为了方便调试时序，debug文件提供IO toggle功能
 * 1. LED_IO_TOGGLE 端口和序号配置列表是IO，实际api按0~n 需要使用，
 * 2、OS_INT_TOGGLE Os任务和中断时间片 观测，以task名字表为序号，led0~x，中断排在之后，需要先开启LED_IO_TOGGLE
 *
 *************************************************************/

/**OS任务切换和中断 时IO toggle，高电平表征执行时间片**/
//#define OS_INT_TOGGLE

/**需要toggle的task Name,可用部分,建议优先级高的在前；顺序 对应LED 0、LED 1....**/
#define DBG_TASK_LIST	{{"AudioCore"}, {"Decoder"}, {"Device"},  {"IDLE"}}
//uint8_t dbgtasklist[][configMAX_TASK_NAME_LEN];
void DbgTaskTGL_set();
void DbgTaskTGL_clr();


/*********************************************
 * 中断toggle入口，需要修缮crt0.S的OS_Trap_Int_Comm 代码如下文

 	 原先语句代码:
 	jral $r1

 	修改为:
 	pushm $r0,$r1
 	jal   OS_dbg_int_in
 	popm $r0,$r1
 	push $r0
	jral $r1
 	pop  $r0
 	jal   OS_dbg_int_out
 ********************************************/
/**中断toggle的IO(LED_PORT_LIST) 排在 OS 序号后面,单列DBG_INT_ID, 其他非OS中断用一个IO**/
#define DBG_INT_ID				18 //Int18为BT INT0为OS 其他参见crt0.S

void OS_dbg_int_in(uint32_t int_num);
void OS_dbg_int_out(uint32_t int_num);



//#define LED_IO_TOGGLE //使用IO toggle调试  led或逻分观测 配合 LedPortInit()


/**测试IO,常用于程序执行时序可视化，标准SDK不使用**/
/**输出高低电平，上升沿，下降沿，翻转，led等，1~4端口需配置，可屏蔽，led需配置有效电平。**/
/**调用LedPortInit初始化端口配置。**/
#define LED_ON_LEVEL	1//灯亮高高电平 设定。
/**根据需要启动对应端口和序号,led 0~n-1，注意端口复用规避**/
#define LED_PORT_LIST	{{'A', 0}, {'A', 1}, {'A', 2}, {'A', 3}, {'A', 4}, {'A', 5}, {'A', 6}, {'A', 7}, {'A', 8}, {'A', 9}, {'A', 10}, {'A', 11}, {'A', 12}, {'A', 13}, {'A', 14}, {'A', 15}}



#define PORT_IN_REG(X)			(X=='A'? GPIO_A_IN : GPIO_B_IN)
#define PORT_OUT_REG(X)			(X=='A'? GPIO_A_OUT : GPIO_B_OUT)
#define PORT_SET_REG(X)			(X=='A'? GPIO_A_SET : GPIO_B_SET)
#define PORT_CLR_REG(X)			(X=='A'? GPIO_A_CLR : GPIO_B_CLR)
#define PORT_TGL_REG(X)			(X=='A'? GPIO_A_TGL : GPIO_B_TGL)
#define PORT_IE_REG(X)			(X=='A'? GPIO_A_IE : GPIO_B_IE)
#define PORT_OE_REG(X)			(X=='A'? GPIO_A_OE : GPIO_B_OE)
#define PORT_DS_REG(X)			(X=='A'? GPIO_A_DS : GPIO_B_DS)
#define PORT_PU_REG(X)			(X=='A'? GPIO_A_PU : GPIO_B_PU)
#define PORT_PD_REG(X)			(X=='A'? GPIO_A_PD : GPIO_B_PD)
#define PORT_ANA_REG(X)			(X=='A'? GPIO_A_ANA_EN : GPIO_B_ANA_EN)
#define PORT_PULLDOWN_REG(X)	(X=='A'? GPIO_A_PULLDOWN : GPIO_B_PULLDOWN)
#define PORT_CORE_REG(X)		(X=='A'? GPIO_A_CORE_OUT_MASK : GPIO_B_CORE_OUT_MASK)
#define PORT_DMA_REG(X)			(X=='A'? GPIO_A_DMA_OUT_MASK : GPIO_B_DMA_OUT_MASK)

void LedPortInit(void);
void LedOn(uint8_t Index);//1~4 是否生效取决于LEDx_PORT是否定义
void LedOff(uint8_t Index);
void LedToggle(uint8_t Index);
void LedPortRise(uint8_t Index);
void LedPortDown(uint8_t Index);
bool LedPortGet(uint8_t Index);//TRUE:高电平，FALSE:低电平，index无效时 默认FALSE



/**
 * 使用习惯的打印格式
 */
#ifdef TRACE_ENABLE

#define TRACE_LEVEL_DISABLED                    0
#define TRACE_LEVEL_ASSERT                      1
#define TRACE_LEVEL_ERROR                       2
#define TRACE_LEVEL_WARN                        3
#define TRACE_LEVEL_NOTICE                      4
#define TRACE_LEVEL_INFO                        5
#define TRACE_LEVEL_DEBUG                       6
#define TRACE_LEVEL_VERBOSE                     7

#ifndef TRACE_LEVEL
#define TRACE_LEVEL                    			TRACE_LEVEL_DEBUG
#endif

#ifndef TRACE_MODULE
#define TRACE_MODULE __FILE__
#endif

#ifndef TRACE_ASSERT_FORMAT
#define TRACE_ASSERT_FORMAT                     "%-10s\t%4d [A] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_ERROR_FORMAT
#define TRACE_ERROR_FORMAT                      "%-10s\t%4d [E] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_WARN_FORMAT
#define TRACE_WARN_FORMAT                       "%-10s\t%4d [W] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_NOTICE_FORMAT
#define TRACE_NOTICE_FORMAT                     "%-10s\t%4d [N] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_INFO_FORMAT
#define TRACE_INFO_FORMAT                       "%-10s\t%4d [I] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_DEBUG_FORMAT
#define TRACE_DEBUG_FORMAT                      "%-10s\t%4d [D] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_VERBOSE_FORMAT
#define TRACE_VERBOSE_FORMAT                    "%-10s\t%4d [V] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_LINE_ENDING
#define TRACE_LINE_ENDING                       "\n"
#endif

#ifndef TRACE_PRINTF
#define TRACE_PRINTF                            printf
#endif


static void trace_dump(void *p_buffer, uint32_t len)
{
    uint8_t *p = (uint8_t *)p_buffer;
	uint32_t index;

    for (index = 0; index < len; index++)
    {
        TRACE_PRINTF("%02X", p[index]);
    }

    TRACE_PRINTF("\r\n");
}

#define trace_assert(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ASSERT){\
            TRACE_PRINTF(TRACE_ASSERT_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_assertln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ASSERT){\
            TRACE_PRINTF(TRACE_ASSERT_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_a(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ASSERT){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_error(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ERROR){\
            TRACE_PRINTF(TRACE_ERROR_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_errorln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ERROR){\
            TRACE_PRINTF(TRACE_ERROR_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_e(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ERROR){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_warn(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_WARN){\
            TRACE_PRINTF(TRACE_WARN_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_warnln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_WARN){\
            TRACE_PRINTF(TRACE_WARN_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_w(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_WARN){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_notice(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_NOTICE){\
            TRACE_PRINTF(TRACE_NOTICE_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_noticeln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_NOTICE){\
            TRACE_PRINTF(TRACE_NOTICE_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_n(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_NOTICE){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_info(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_INFO){\
            TRACE_PRINTF(TRACE_INFO_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_infoln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_INFO){\
            TRACE_PRINTF(TRACE_INFO_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_i(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_INFO){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_debug(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_DEBUG){\
            TRACE_PRINTF(TRACE_DEBUG_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_debugln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_DEBUG){\
            TRACE_PRINTF(TRACE_DEBUG_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_d(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_DEBUG){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_verbose(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_VERBOSE){\
            TRACE_PRINTF(TRACE_VERBOSE_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_verboseln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_VERBOSE){\
            TRACE_PRINTF(TRACE_VERBOSE_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_v(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_VERBOSE){\
            trace_dump(p_buffer,len);}\
    }while(0)

#else

#define trace_init(...)
#define trace_dump(...)
#define trace_level_set(...)
#define trace_level_get(...)

#define trace_assert(...)
#define trace_assertln(...)
#define trace_dump_a(...)

#define trace_error(...)
#define trace_errorln(...)
#define trace_dump_e(...)

#define trace_warn(...)
#define trace_warnln(...)
#define trace_dump_w(...)

#define trace_notice(...)
#define trace_noticeln(...)
#define trace_dump_n(...)

#define trace_info(...)
#define trace_infoln(...)
#define trace_dump_i(...)

#define trace_debug(...)
#define trace_debugln(...)
#define trace_dump_d(...)

#define trace_verbose(...)
#define trace_verboseln(...)
#define trace_dump_v(...)

#endif





#ifdef __cplusplus
}
#endif//__cplusplus


#endif //__DBG_H__ 

