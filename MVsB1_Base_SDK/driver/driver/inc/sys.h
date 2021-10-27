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
 * @brief	ϵͳ�δ�ʱ�ӳ�ʼ��
 * @param	��
 * @return ��
 * @note �ú�����ʼ��ϵͳ�δ�ʱ�ӣ�ÿ1ms����һ���ж�
 */
void SysTickInit(void);


/**
 * @brief	�ر�ϵͳ�δ�ʱ��
 * @param	��
 * @return ��
 * @note �ú����ر�ϵͳ�δ�ʱ��
 */
void SysTickDeInit(void);

/**
 * @brief	���ϵͳʱ���жϱ�ʶ
 * @param	��
 * @return ��
 * @note ���û���������дϵͳ�δ�ʱ���жϺ���ʱ����Ҫ���ȵ��øú�������жϱ�ʶ
 */
void SysTimerIntFlagClear(void);

/**
 * @brief	ϵͳ�����λ
 * @param	��
 * @return ��
 * @note ִ�иú�����ϵͳ������λ��
 */
void NVIC_SystemReset(void);

/**
 * @brief	ϵͳ�δ��жϷ��������ú���Ϊweak���ԣ����Ա����¸�д
 * @param	��
 * @return ��
 * @note ���û���������дϵͳ�δ�ʱ���жϺ���ʱ����Ҫ���ȵ���SysTimerIntFlagClear
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

