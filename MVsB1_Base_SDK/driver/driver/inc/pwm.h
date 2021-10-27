/**
 **************************************************************************************
 * @file    pwm.h
 * @brief   Pulse Width Modulation ( Reuse with General Timer 3&4 ) API
 *
 * @author  Grayson Chen
 * @version V1.0.0
 *
 * $Created: 2017-09-29 13:25:30$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

/**
 * @addtogroup PWM
 * @{
 * @defgroup pwm pwm.h
 * @{
 */

#ifndef __PWM_H__
#define __PWM_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "timer.h"

#define PWM_MAX_FREQ_DIV_VALUE   (65535 << 15)

//PWM 输出通道GPIO复用关系
#define   TIMER3_PWM_A0_A8_A22_A27   	0    //TIMER3的PWM引脚可复用在A0, A8, A22或者A27
#define   TIMER4_PWM_A1_A9_A23_A28   	1    //TIMER4的PWM引脚可复用在A1, A9, A23或者A28
#define   TIMER5_PWM_A10_A24_B0         2    //TIMER5的PWM引脚可复用在A10, A24, B0
#define   TIMER6_PWM_A11_A25_B1         3    //TIMER6的PWM引脚可复用在A11, A25, B1


/**
 * PWM IO Mode select definition
 */ 
typedef enum __PWM_IO_MODE
{
    PWM_IO_MODE_NONE = 0,    //还原为GPIO
    PWM_IO_MODE_OUT  = 1,    //直接输出
    PWM_IO_MODE_PD1  = 2,    //PWM下拉2.4mA输出
    
}PWM_IO_MODE;


/**
 * @brief  PWM输出类型定义  
 */
typedef enum __PWM_OUTPUT_TYPE
{
    PWM_OUTPUT_FORCE_LOW    = 4,      //强制输出低电平
    PWM_OUTPUT_FORCE_HIGH   = 5,      //强制输出高电平
    
    PWM_OUTPUT_SINGLE_1     = 6,      //标准输出：Duty为高脉冲占空比,如果是COUNT UP MODE，输出波形为先高后低，如果是COUNT DOWN MODE,输出波形为先低后高
    PWM_OUTPUT_SINGLE_2     = 7,      //标准输出：Duty为负脉冲占空比,如果是COUNT UP MODE，输出波形为先低后高，如果是COUNT DOWN MODE,输出波形为先高后低

    PWM_OUTPUT_ONE_PULSE	= 8	      //输出一个脉冲后结束,B1X没有此功能，保留选择项，兼容前期代码
}PWM_OUTPUT_TYPE;

/**
 * @brief  PWM计数模式 
 */
typedef enum __PWM_COUNTER_MODE
{
    PWM_COUNTER_MODE_DOWN = 0,        //从高到低计数，PWM启动到输出之间的IDLE值是高电平
    PWM_COUNTER_MODE_UP,              //从低到高计数，PWM启动到输出之间的IDLE值是低电平
    PWM_COUNTER_MODE_CENTER_ALIGNED1, //先从低加到高，再从高减至低，只在下溢出时产生中断
    PWM_COUNTER_MODE_CENTER_ALIGNED2, //先从低加到高，再从高减至低，只在上溢出时产生中断
    PWM_COUNTER_MODE_CENTER_ALIGNED3  //先从低加到高，再从高减至低，在上溢出和下溢出时都产生中断
}PWM_COUNTER_MODE;


/**
 * @brief  PWM触发模式：中断或DMA
 */
typedef enum _PWM_DMA_INTERRUPT_MODE
{
	PWM_REQ_INTERRUPT_MODE = 0,  /**< 中断模式  */

	PWM_REQ_DMA_MODE             /**< DMA模式  */

} PWM_DMA_INTERRUPT_MODE;


/**
 * @brief  PWM配置错误类型定义  
 */
typedef enum __PWM_ERROR_CODE
{
    PWM_ERROR_INVALID_TIMER_INDEX = -128,
    PWM_ERROR_INVALID_PWM_TYPE,
    PWM_ERROR_INVALID_PWM_COUNTER_MODE,
    PWM_ERROR_OK = 0
}PWM_ERROR_CODE;


typedef struct __PWM_StructInit
{
    uint8_t     CounterMode;        //PWM计数模式，参数取值范围 #PWM_COUNTER_MODE

    uint8_t     OutputType;         //PWM输出类型,参数取值为#PWM_OUTPUT_TYPE

    uint32_t    FreqDiv;            //PWM频率与系统时钟的分频比，取值范围[1, PWM_MAX_FREQ_DIV_VALUE]

    uint16_t    Duty;               //占空比,取值范围[0~100]对应占空比为[0%~100%]

    bool        DMAReqEnable;       //是否使能DMA从Mem搬运占空比到外设中，参数取值： 1 -- 使能DMA请求； 0 -- 禁能DMA请求

    uint8_t     MasterModeSel;		//000: Reset - timer3_ug is used as trigger out
	 	 	 	 	 	 	 	 	//001: Update - the update event is used as trigger out
		 	 	 	 	 	 	 	//010: PWM - pwm_o1 is used as trigger out.

    uint8_t     MasterSlaveSel;		//0: trigger input is no action
    								//1: the counter is reset by the posedge of trigger input
    /*TriggerInSrc
         * 	Timer3:
         *      00: Internal trigger 0 (itr0), Timer4
         *      01: Internal trigger 1 (itr1), Timer5
         *      10: Internal trigger 2 (itr2), Timer6
         *  Timer4:
         *      00: Internal trigger 0 (itr0), Timer3
         *      01: Internal trigger 1 (itr1), Timer5
         *      10: Internal trigger 2 (itr2), Timer6
         *  Timer5:
         *      00: Internal trigger 0 (itr0), Timer3
         *      01: Internal trigger 1 (itr1), Timer4
         *      10: Internal trigger 2 (itr2), Timer6
         *  Timer6:
         *      00: Internal trigger 0 (itr0), Timer3
         *      01: Internal trigger 1 (itr1), Timer4
         *      10: Internal trigger 2 (itr2), Timer5*/
    uint8_t     TriggerInSrc;

    uint8_t     CcxmAndCccUpdataSel;//0: the timerX_ccxm and timerX_ccc only update when timerX_ug or timerX_com set
	   	   	   	   	   	   	   	   	  //1:  the timerX_ccxm and timerX_ccc is update when timerX_ug or timerX_com set, or posedge of trigger input happen
}PWM_StructInit;   


/**
 * @brief PWM IOCTRL 命令
 */
typedef enum _PWM_IOCTRL_CMD
{
	//update and Select cmd
	OUTPUT_SOFTWARE_UPDATE 		= 0x1,	    /**< TIMERx软件update寄存器  */

	OUTPUT_TYPE_UPDATE 			= 0x2,		/**< #PWM_OUTPUT_TYPE  */

	OUTPUT_FREQUENCY_UPDATE 	= 0x10,	    /**< 频率更新  */

	OUTPUT_DUTY_UPDATE 			= 0x20,		/**< 占空比更新  */

} PWM_IOCTRL_CMD;
/**
 * @brief PWM IOCTRL 参数
 */
typedef struct _PWM_IOCTRL_ARG
{
	PWM_OUTPUT_TYPE     	   OutputType;         /**< PWM输出类型  */

	uint32_t    			   FreqDiv;            /**< PWM频率与系统时钟的分频比，取值范围[1, PWM_MAX_FREQ_DIV_VALUE]  */

	uint16_t				   Duty;			   /**< 占空比  */

} PWM_IOCTRL_ARG;
 


/**
 * @brief     设置一路PWM的GPIO复用
 *
 * @param[in] PWMChSel  PWM通道选择，参数可见#PWM 通道GPIO复用关系宏定义
 * @param[in] PwmIoSel  例如：当选择TIMER3_PWM_A23_A27_B0_B2，
 *                            PwmIoSel = 0,则PWM复用与A23，
 *                            PwmIoSel = 1,则复用于A27,
 *                            PwmIoSel = 2,则复用于B0，
 *                            PwmIoSel = 3,则复用于B2。
 * @param[in] PWMMode   PWM复用关系，详细参考 #PWM_IO_MODE
 *
 * @return    无
 */
void PWM_GpioConfig(uint8_t PWMChSel, uint8_t PwmIoSel, PWM_IO_MODE PWMMode);




/**
 * @brief     选择一个定时器下的PWM通道并配置其参数
 *
 * @param[in] TimerIdx  定时器索引号，只支持TIMER3、TIMER4
 * @param[in] PWMParam  PWM初始化参数，详细参考 PWMInfo
 * 						Slave模式下，CounterMode和OutputType参数需要注意：
 * 						OutputType=PWM_OUTPUT_SINGLE_1，CounterMode=PWM_COUNTER_MODE_DOWN时，PWM启动时会有一个毛刺，
 * 						建议使用OutputType=PWM_OUTPUT_SINGLE_1，CounterMode=PWM_COUNTER_MODE_UP替换
 *
 * @return    错误号：0 - 正确，其他为错误，详细参考PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_Config(TIMER_INDEX TimerIdx, PWM_StructInit *PWMParam);

/**
 * @brief     开启定时器TimerIdx的一个通道PWM
 *
 * @param[in] TimerIdx  定时器索引号，只支持TIMER3、TIMER4
 *
 * @return    错误号：0 - 正确，其他为错误，详细参考PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_Enable(TIMER_INDEX TimerIdx);

/**
 * @brief     关闭定时器TimerIdx的一个通道PWM
 *
 * @param[in] TimerIdx  定时器索引号，只支持TIMER3、TIMER4
 *
 * @return    错误号：0 - 正确，其他为错误，详细参考PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_Disable(TIMER_INDEX TimerIdx);

/**
 * @brief     暂停定时器TimerIdx的一个通道PWM
 *
 * @param[in] TimerIdx  定时器索引号，详见 # TIMER_INDEX
 * @param[in] IsPause   1: PWM暂停，保持在最后输出电平  0：PWM正常输出
 *
 * @return    错误号：0 - 正确，其他为错误，详细参考PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_Pause(TIMER_INDEX TimerIdx, bool IsPause);


/**
 * @brief     TimerIdx下的一路PWM进行操作
 *
 * @param[in] TimerIdx  定时器索引号，详见 # TIMER_INDEX
 * @param[in] IOCtrl    IOCtrl类型
 * @param[in] Arg       IOCtrl参数
 *
 * @return    错误号：0 - 正确，其他为错误，详细参考PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_IOCTRl(TIMER_INDEX TimerIdx, PWM_IOCTRL_CMD Cmd, PWM_IOCTRL_ARG *Arg);




/**
 * @brief      占空比动态配置
 *
 * @param[in]  TimerIdx 定时器索引号,只支持TIMER3、TIMER4
 * @param[in]  Duty     0~0xFFFF对应0~100%
 *
 * @return     错误号：0 - 正确，其他为错误，详细参考PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_DutyConfigIQ16(TIMER_INDEX TimerIdx, uint16_t Duty);

/**
 * @brief      将定时器TimerIdx占空比参数进行转换，将百分比数据转成十六进制数据
 *
 * @param[in]  TimerIdx 定时器索引号,只支持TIMER3、TIMER4
 * @param[in]  BufIn    占空比输入参数地址，单位％
 * @param[out] BufOut   占空比输出参数地址，十六进制
 * @param[in]  Len      BufIn的长度,单位：字节
 *
 * @return     BufOut   长度
 */
uint32_t PWM_DutyPercentoHex(TIMER_INDEX TimerIdx, uint8_t *BufIn, uint16_t *BufOut, uint16_t Len);



#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__PWM_H__
/**
 * @}
 * @}
 */
