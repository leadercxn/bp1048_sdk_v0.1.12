/******************************************************************************
 * @file    sys.h
 * @author
 * @version V1.0.0
 * @date    19-07-2013
 * @brief   functin relative of sys 
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */


/**
 * @addtogroup SYSTEM
 * @{
 * @defgroup sys sys.h
 * @{
 */
 
#ifndef __SYS_H__
#define __SYS_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

/**
 * @brief	系统滴答时钟初始化
 * @param	无
 * @return 无
 * @note 该函数初始化系统滴答时钟，每1ms进行一次中断
 */
void SysTickInit(void);


/**
 * @brief	关闭系统滴答时钟
 * @param	无
 * @return 无
 * @note 该函数关闭系统滴答时钟
 */
void SysTickDeInit(void);

/**
 * @brief	清除系统时钟中断标识
 * @param	无
 * @return 无
 * @note 在用户代码中重写系统滴答时钟中断函数时，需要首先调用该函数清除中断标识
 */
void SysTimerIntFlagClear(void);

/**
 * @brief	系统软件复位
 * @param	无
 * @return 无
 * @note 执行该函数后，系统将被复位。
 */
void NVIC_SystemReset(void);

/**
 * @brief	系统滴答中断服务函数，该函数为weak属性，可以被重新改写
 * @param	无
 * @return 无
 * @note 在用户代码中重写系统滴答时钟中断函数时，需要首先调用SysTimerIntFlagClear
 */
__attribute__((weak)) void SystickInterrupt(void);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__SYS_H__

/**
 * @}
 * @}
 */

