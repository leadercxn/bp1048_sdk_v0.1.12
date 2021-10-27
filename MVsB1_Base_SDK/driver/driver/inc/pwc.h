/**
 **************************************************************************************
 * @file    pwc.h
 * @brief   Pulse Width Capture (Reuse with General Timer 3&4 ) API
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
 * @addtogroup PWC
 * @{
 * @defgroup pwc pwc.h
 * @{
 */

#ifndef __PWC_H__
#define __PWC_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "timer.h"

#define PWC_MAX_TIMESCALE   0xffff
    
//PWC ���ͨ��GPIO���ù�ϵ
    
#define PWC_IO_TOTAL		40
    


/**
 * PWM IO Mode select definition
 */ 
typedef enum __PWC_IO_MODE
{
    PWC_IO_MODE_NONE = 0,    //��ԭΪGPIO
    PWC_IO_MODE_IN  = 1     //ֱ�����   
}PWC_IO_MODE;
    


/**
 * @brief  PWC���뼫��
 */
typedef enum __PWC_POLARITY
{
    PWC_POLARITY_BOTH     = 1,  /**< ˫���ز��� */
    PWC_POLARITY_RAISING  = 2,  /**< �����ز��� */
    PWC_POLARITY_FALLING  = 3   /**< �½��ز��� */

} PWC_POLARITY;

/**
 * @brief  PWC����ģʽ
 */
typedef enum _PWC_CAPTURE_MODE
{
	PWC_CAPTURE_CONTINUES = 0,	/**< �������� */

	PWC_CAPTURE_ONCE = 1		/**< ֻ����һ�� */

} PWC_CAPTURE_MODE;

/**
 * @brief  PWC�����˲�������ʱ�ӷ�Ƶ��
 */
typedef enum __PWC_FILTER_CLK_DIV
{   
    FILTER_CLK_DIV1 = 0,//����Ƶ,����ϵͳʱ�ӣ���Fsys
    FILTER_CLK_DIV2,    //ϵͳʱ�ӵ�2��Ƶ���� Fsys / 2 
    FILTER_CLK_DIV3,    //ϵͳʱ�ӵ�4��Ƶ���� Fsys / 4
    FILTER_CLK_DIV4,    //ϵͳʱ�ӵ�8��Ƶ���� Fsys / 8
    
}PWC_FILTER_CLK_DIV;


/**
 * @brief  PWC����Ŷ���
 */
typedef enum __PWC_ERROR_CODE
{
    PWC_ERROR_INVALID_PWC_INDEX = -128,		//��ʱ����������
	PWC_ERROR_INVALID_PWC_CH,				//PWCͨ��ѡ�����
    PWC_ERROR_INVALID_PWC_POLARITY,			//���뼫��ѡ�����
    PWC_ERROR_OK = 0
}PWC_ERROR_CODE;


/** 
 * @brief  PWC��ʼ���ṹ�嶨��
 */  
typedef struct __PWC_StructInit
{
    uint8_t  SingleGet;          //��������� 1 -- ֻ��ȡ1�Σ� 0 -- ������ȡ
    
    uint16_t TimeScale;          //PWC����������(ʱ�ӷ�Ƶϵ��)��ȡֵ��Χ[1,65535], �����Բ����ķ�Χ��1/Fsys * TimeScale  ~  65535/Fsys * TimeScale��
    
    uint8_t  DMAReqEnable;       //�Ƿ�ʹ��DMA����ȡ�����ݰ���MEM�У�����ȡֵ�� 1 -- ʹ��DMA���� 0 -- ����DMA����
    
    uint8_t  FilterTime;         //�˲�ʱ�䣬��Χ��1/Fpwc ~ 128/Fpwc
    
    uint8_t  Polarity;           //�����ԣ�����ȡֵ��Χ����Ե����Ե�� �����ص������أ��½��ص��½��أ���ϸ��� #PWC_POLARITY

    uint8_t  PwcSourceSelect;    //0: gpio  1:pwm

    uint8_t  PwcOpt;           	 //0: first valid edge not generate dma req  1: first valid edge generate dma req
          
    uint8_t  MasterModeSel;      //000: Reset - timer3_ug is used as trigger out
     							 //001: Update - the update event is used as trigger out
   								 //010: PWM - pwm_o1 is used as trigger out.
    uint8_t  MasterSlaveSel;     //0: trigger input is no action
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
    uint8_t  TriggerInSrc;

    uint8_t  CcxmAndCccUpdataSel;  //0: the timerX_ccxm and timerX_ccc only update when timerX_ug or timerX_com set
    							   //1:  the timerX_ccxm and timerX_ccc is update when timerX_ug or timerX_com set, or posedge of trigger input happen

}PWC_StructInit;  

/**
 * @brief PWC IOCTROL ����
 */
typedef enum _PWC_IOCTRL_CMD
{
	PWC_DATA_GET = 0,                    /**< PWC���ݻ�ȡ */
	PWC_DONE_STATUS_GET,                 /**< PWC�������״̬��ȡ */
	PWC_OVER_CAPTURE_STATUS_GET,         /**< PWC�ظ�����״̬��ȡ */
	PWC_ERR_STATUS_GET,                  /**< PWC����״̬��ȡ */

	PWC_OVER_CAPTURE_STATUS_CLR,         /**< PWC�ظ�����״̬��� */
	PWC_ERR_STATUS_CLR,                  /**< PWC����״̬��� */

    PWC_POLARITY_UPDATE,                 /**< PWC���뼫������ */


} PWC_IOCTRL_CMD;

/**
 * @brief PWC IOCTROL ����
 */
typedef struct _PWC_IOCTRL_ARG
{

	PWC_POLARITY    PWCPolarity;    /**< PWC���뼫�� */

} PWC_IOCTRL_ARG;

/**
 * @brief      ��GPIO����ΪPWC���Ż�ԭΪGPIO����.
 *
 * @param[in]  TimerId  ��ʱ��ѡ��ע�⣺PWCֻ����GTIMER: TIMER3/TIMER4/TIMER5/TIMER6
 * @param[in]  PWCIoSel  gpio pwc input select
						   note:all gpio can be PWC input and should enable IE witch is the selected gpio by software
						   -------------------------------------------------------
						   | 0��gpio_A0  | 1��gpio_A1  | 2��gpio_A2  | 3��gpio_A3 |
						   -------------------------------------------------------
						   | 4��gpio_A4  | 5��gpio_A5  | 6��gpio_A6  | 7��gpio_A7 |
						   -------------------------------------------------------
						   | 8��gpio_A8  | 9��gpio_A9  |10��gpio_A10 |11��gpio_A11|
						   -------------------------------------------------------
						   |12��gpio_A12 |13��gpio_A13 |14��gpio_A14 |15��gpio_A15|
						   -------------------------------------------------------
						   |16��gpio_A16 |17��gpio_A17 |18��gpio_A18 |19��gpio_A19|
						   -------------------------------------------------------
						   |20��gpio_A20 |21��gpio_A21 |22��gpio_A22 |23��gpio_A23|
						   -------------------------------------------------------
						   |24��gpio_A24 |25��gpio_A25 |26��gpio_A26 |27��gpio_A27|
						   -------------------------------------------------------
						   |28��gpio_A28 |29��gpio_A29 |30��gpio_A30 |31��gpio_A31|
						   -------------------------------------------------------
						   |32��gpio_B0  |33��gpio_B1  |34��gpio_B2  |35��gpio_B3 |
						   -------------------------------------------------------
						   |36��gpio_B4  |37��gpio_B5  |38��gpio_B6  |39��gpio_B7 |
						   -------------------------------------------------------
 *
 * @return     PWC_ERROR_CODE
 */
PWC_ERROR_CODE PWC_GpioConfig(TIMER_INDEX TimerId, uint8_t PWCIoSel);



/**
 * @brief     ѡ��һ����ʱ����PWC1ͨ�������ò���
 *
 * @param[in] TimerIdx  ��ʱ�������ţ�ֻ֧��TIMER3��TIMER4
 * @param[in] PWCParam  PWC��ʼ����������ϸ�ο� PWCInfo
 *
 * @return    ����ţ�0 - ��ȷ������Ϊ������ϸ�ο�PWC_ERROR_CODE
 */
PWC_ERROR_CODE PWC_Config(TIMER_INDEX TimerIdx, PWC_StructInit *PWCParam);

/**
 * @brief     ʹ��TimerIdx�µ�PWCͨ�����вɼ�
 *
 * @param[in] TimerIdx  ��ʱ�������ţ�ֻ֧��TIMER3��TIMER4
 *
 * @return    ����ţ�0 - ��ȷ������Ϊ������ϸ�ο�PWC_ERROR_CODE
 */
PWC_ERROR_CODE PWC_Enable(TIMER_INDEX TimerIdx);

/**
 * @brief     �ر�TimerIdx�µ�PWCͨ�����вɼ�
 *
 * @param[in] TimerIdx  ��ʱ�������ţ�ֻ֧��TIMER3��TIMER4
 *
 * @return    ����ţ�0 - ��ȷ������Ϊ������ϸ�ο�PWC_ERROR_CODE
 */
PWC_ERROR_CODE PWC_Disable(TIMER_INDEX TimerIdx);


/**
 * @brief     ��ȡTimerIdx�µ�һ·PWC���������
 *
 * @param[in] TimerIdx  ��ʱ�������ţ�ֻ֧��TIMER3��TIMER4
 *
 * @return    0xffff -- ���������������PWC��ȡ��������
 */
uint32_t PWC_CaptureValueGet(TIMER_INDEX TimerIdx);



/**
 * @brief     TimerIdx�µ�PWC����
 *
 * @param[in] TimerIdx  ��ʱ�������ţ�ֻ֧��TIMER3��TIMER4
 *
 * @return    >=0: ��ȷ��ֵ�� < 0: ���󷵻�ֵ
 */
PWC_ERROR_CODE PWC_IOCTRL(TIMER_INDEX TimerIdx,PWC_IOCTRL_CMD Cmd, PWC_IOCTRL_ARG *Arg);




#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__PWC_H__

/**
 * @}
 * @}
 */

