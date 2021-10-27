/**
 **************************************************************************************
 * @file    timer.h
 * @brief   Timer (Basic Timer 1 & 2, General Timer 3 & 4 & 5 & 6 ) API
 *
 * @author  Grayson Chen
 * @version V1.0.0
 *
 * $Created: 2017-09-28 10:19:30$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
 /*
 ======================================================================================
                       ##### ���ʹ�ö�ʱ����صĺ���API�ӿ� #####
 ======================================================================================
   [..]����������ΪB1Xϵ��оƬ�Ķ�ʱ���ṩ�˺����ӿ�
       ��Щ�������չ��ܣ�����������3��ͷ�ļ��У�������ͬһ��ͨ����3�ֹ���Ϊ�����ϵ��
       
       ��##��������ʱ���ܣ�timer.h 
       
            (++)ѡ��һ����ʱ�������ö�ʱʱ���Լ���ʱģʽ   --  Timer_Start
            (++)����ͨ����ȡ�жϱ�־���ж϶�ʱ�Ƿ��ѵ�      --  Timer_InterruptFlagGet
            (++)�ڶ�ʱ�����й����У���ʱ��ѡ���Ƿ���ͣ      --  Timer_Pause
            (++)�ڵ���ģʽ�£�����ѡ���Ƿ�֧����ͣ             --  Timer_SetHaltEnabled
            (++)��ȡʣ��ʱ��                                                   --  Timer_GetResidualTime
            
 **************************************************************************************
 */

/**
 * @addtogroup TIMER
 * @{
 * @defgroup timer timer.h
 * @{
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
//��Ϊ���ֵ�͵�ǰϵͳ����Ƶ���йأ�timer��ʹ��pll/aupll/rcʱ�ӣ������趨�̶�ֵ���Ը�Ϊ�����ж�̬����
//��ʽ��65535*65535/ClockFreqGet(TimerIndex) / 1000000
//#define  TIMER_MAX_TIMEOUT_VALUE_IN_US  (23860201)      // �������õ�����ֵ microsecond(us) = 65535 * 65535 / 180

/**
 * @brief   ��ʱ��������
 * @note    BP10��6����ʱ��������TIMER1�Ѿ�����Systick����
 */
typedef enum __TIMER_INDEX
{
    TIMER1 = 0,
    TIMER2 = 1,
    TIMER3,
    TIMER4,
    TIMER5,
    TIMER6,
} TIMER_INDEX;

/**
 * @brief   ��ʱ���ж��ź�Դѡ��
 */
typedef enum __TIMER_INTERRUPT_SRC
{
    UPDATE_INTERRUPT_SRC = 1,          //��ʱ�������ж�
    PWC_OVER_RUN_INTERRUPT_SRC = 2,    //PWC����ж�
    PWC1_CAP_DATA_INTERRUPT_SRC = 4,   //PWC�����ж�
    PWM1_COMP_INTERRUPT_SRC = 64,      //PWM�Ƚ��ж�
    PWC1_OVER_CAP_INTERRUPT_SRC = 1024 //PWC�ظ������ж�
} TIMER_INTERRUPT_SRC;

/**
 * @brief   ��ʱ�����ô������Ͷ���
 */
typedef enum __TIMER_ERROR_CODE
{
    TIMER_ERROR_INVALID_TIMER_INDEX = -128,
    TIMER_INTERRUPT_SRC_SEL,
    TIMER_ERROR_OK = 0
}TIMER_ERROR_CODE;

typedef struct __TIMER_CTRL2
{
    uint8_t MasterModeSel   :3;
    uint8_t MasterSlaveSel  :1;
    uint8_t TriggerInSrc    :2;
    uint8_t CcxmAndCccUpdataSel :1;
}TIMER_CTRL2;   

/**
 * @brief      ��ʱ��ʱ��Դѡ��
 *
 * @param[in]  TimerIndex        ��ʱ������, ֻ֧��TIMER3��TIMER4
 * @param[in]  rcClock           1=RCʱ��(12M),0=ϵͳʱ��(Ĭ��ϵͳʱ��)
 *
 * @return     �����, ��� #TIMER_ERROR_CODE
 */
//TIMER_ERROR_CODE Timer_ClockSel(TIMER_INDEX TimerIndex, bool rcClock);
////////////////////////////////////////////////////////////////////////////////////
//
//                      ���������ڶ�ʱ���ܵĺ����ӿ�
//
////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief      ���ö�ʱ���������ú�����λΪus
 *
 * @param[in]  TimerIndex        ��ʱ������, ��� #TIMER_INDEX
 * @param[in]  usec              ��ʱ����, ��λ��΢��(us)
 * @param[in]  SingleShot        0: ����ģʽ����ʱ֮�󣬶�ʱ��������װ�ؼ���ֵ.
 *                               1: ����ģʽ, ��ʱ֮�󣬶�ʱ��ֹͣ������
 * @param[in]  TimerArgs         MasterModeSel: 000: Reset - timer3_ug is used as trigger out
 * 												001: Update - the update event is used as trigger out
												010: PWM - pwm_o1 is used as trigger out.
 *                               MasterSlaveSel: 0: trigger input is no action
 *                               				 1: the counter is reset by the posedge of trigger input
 *                               TriggerInSrc��
 *                               	Timer3:
 *                               				00: Internal trigger 0 (itr0), Timer4
 *                               				01: Internal trigger 1 (itr1), Timer5
 *                               				10: Internal trigger 2 (itr2), Timer6
 *                               	Timer4:
 *                               				00: Internal trigger 0 (itr0), Timer3
 *                               				01: Internal trigger 1 (itr1), Timer5
 *                               				10: Internal trigger 2 (itr2), Timer6
 *                               	Timer5:
 *                               				00: Internal trigger 0 (itr0), Timer3
 *                               				01: Internal trigger 1 (itr1), Timer4
 *                               				10: Internal trigger 2 (itr2), Timer6
 *                               	Timer6:
 *                               				00: Internal trigger 0 (itr0), Timer3
 *                               				01: Internal trigger 1 (itr1), Timer4
 *                               				10: Internal trigger 2 (itr2), Timer5
 *                               CcxmAndCccUpdataSel��0: the timerX_ccxm and timerX_ccc only update when timerX_ug or timerX_com set
 *                               					  1:  the timerX_ccxm and timerX_ccc is update when timerX_ug or timerX_com set, or posedge of trigger input happen
 * @return     �����, ��� #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_ConfigMasterSlave(TIMER_INDEX TimerIndex, uint32_t  usec, bool SingleShot,TIMER_CTRL2* TimerArgs);

/**
 * @brief      ���ö�ʱ���������ú�����λΪus
 *
 * @param[in]  TimerIndex        ��ʱ������, ��� #TIMER_INDEX
 * @param[in]  usec              ��ʱ����, ��λ��΢��(us)
 * @param[in]  SingleShot        0: ����ģʽ����ʱ֮�󣬶�ʱ��������װ�ؼ���ֵ.
 *                               1: ����ģʽ, ��ʱ֮�󣬶�ʱ��ֹͣ������
 * @return     �����, ��� #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_Config(TIMER_INDEX TimerIndex, uint32_t  usec, bool SingleShot);

/**
 * @brief      ������ʱ��
 *
 * @param[in]  TimerIndex        ��ʱ������, ��� #TIMER_INDEX
 *
 * @return     �����, ��� #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_Start(TIMER_INDEX TimerIndex);

/**
 * @brief      ��ͣ��ʱ��
 *
 * @param[in]  TimerIndex        ��ʱ������, ��� #TIMER_INDEX
 *             IsPause           1:��ͣTimerIndex����    0������ʹ��TimerIndex����
 *
 * @return     �����, ��� #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_Pause(TIMER_INDEX TimerIndex, bool IsPause);

/**
 * @brief      ֻ���¶�ʱ���ķ�Ƶ�Ⱥ��Զ���װֵ
 *
 * @param[in]  TimerIndex        ��ʱ������, ��� #TIMER_INDEX
 * @param[in]  Prescale:         ��ʱ���ķ�Ƶ��
 * @param[in]  AutoReload:       �Զ���װֵ
 *
 * @return     �����, ��� #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_TimerClkFreqUpdate(TIMER_INDEX TimerIndex, uint16_t Prescale, uint16_t AutoReload);

/**
 * @brief      �����ڵ���ģʽ�£���ʱ���Ƿ���ͣ����
 *
 * @param[in]  TimerIndex       ��ʱ������, ��� #TIMER_INDEX
 * @param[in]  HaltEnable       1: �����ڵ���ģʽʱ����ͣʹ��,  
 *                              0: �����ڵ���ģʽʱ��������������ͣ
 *
 * @return    �����, ��� #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE  Timer_SetHaltEnabled(TIMER_INDEX TimerIndex, bool HaltEnable);

/**
 * @brief      ��ȡʣ��ʱ��
 *
 * @param[in]  TimerIndex   ��ʱ������, ��� #TIMER_INDEX
 *
 * @return     ʣ��ʱ��.
 */
uint32_t Timer_GetResidualTime(TIMER_INDEX TimerIndex);

////////////////////////////////////////////////////////////////////////////////////
//
//                      �����Ƕ�ʱ���ж���غ���
//
////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief      ʹ���ж�Դ
 *
 * @param[in]  TimerIndex      ��ʱ������, ��� #TIMER_INDEX
 * @param[in]  IntSrc:         �ж�Դѡ�񣬿���ѡ����
 *
 * @return     �����, ��� #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_InterrputSrcEnable(TIMER_INDEX TimerIndex, TIMER_INTERRUPT_SRC IntSrc);
/**
 * @brief      �ر��ж�Դ
 *
 * @param[in]  TimerIndex      ��ʱ������, ��� #TIMER_INDEX
 * @param[in]  IntSrc:         �ж�Դѡ�񣬿���ѡ����
 *
 * @return     �����, ��� #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_InterrputSrcDisable(TIMER_INDEX TimerIndex, TIMER_INTERRUPT_SRC IntSrc);

/**
 * @brief      ��ȡ��ʱ���жϱ�־
 *
 * @param[in]  TimerIndex    ��ʱ������
 * @param[in]  IntSrc:       �ж�Դѡ�񣬿���ѡ����#TIMER_INTERRUPT_SRC
 *
 * @return     1: �����ж�  0:���жϲ����� TIMER_ERROR_INVALID_TIMER_INDEX--����Ķ�ʱ������
 */
TIMER_ERROR_CODE Timer_InterruptFlagGet(TIMER_INDEX TimerIndex, TIMER_INTERRUPT_SRC IntSrc);

/**
 * @brief      �����ʱ���ж�
 *
 * @param[in]  TimerIndex    ��ʱ������
 * @param[in]  IntSrc:       �ж�Դѡ��ֻ��ѡ��1�� #TIMER_INTERRUPT_SRC
 *
 * @return     �����, ��� #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_InterruptFlagClear(TIMER_INDEX TimerIndex, TIMER_INTERRUPT_SRC IntSrc);

/**
 * @brief      ���ϵͳ��ʱ���жϱ�ʶ
 *
 * @param      ��
 * @return   ��
 */
void SysTimerIntFlagClear(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__TIMER_H__

/**
 * @}
 * @}
 */
