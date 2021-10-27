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

//PWM ���ͨ��GPIO���ù�ϵ
#define   TIMER3_PWM_A0_A8_A22_A27   	0    //TIMER3��PWM���ſɸ�����A0, A8, A22����A27
#define   TIMER4_PWM_A1_A9_A23_A28   	1    //TIMER4��PWM���ſɸ�����A1, A9, A23����A28
#define   TIMER5_PWM_A10_A24_B0         2    //TIMER5��PWM���ſɸ�����A10, A24, B0
#define   TIMER6_PWM_A11_A25_B1         3    //TIMER6��PWM���ſɸ�����A11, A25, B1


/**
 * PWM IO Mode select definition
 */ 
typedef enum __PWM_IO_MODE
{
    PWM_IO_MODE_NONE = 0,    //��ԭΪGPIO
    PWM_IO_MODE_OUT  = 1,    //ֱ�����
    PWM_IO_MODE_PD1  = 2,    //PWM����2.4mA���
    
}PWM_IO_MODE;


/**
 * @brief  PWM������Ͷ���  
 */
typedef enum __PWM_OUTPUT_TYPE
{
    PWM_OUTPUT_FORCE_LOW    = 4,      //ǿ������͵�ƽ
    PWM_OUTPUT_FORCE_HIGH   = 5,      //ǿ������ߵ�ƽ
    
    PWM_OUTPUT_SINGLE_1     = 6,      //��׼�����DutyΪ������ռ�ձ�,�����COUNT UP MODE���������Ϊ�ȸߺ�ͣ������COUNT DOWN MODE,�������Ϊ�ȵͺ��
    PWM_OUTPUT_SINGLE_2     = 7,      //��׼�����DutyΪ������ռ�ձ�,�����COUNT UP MODE���������Ϊ�ȵͺ�ߣ������COUNT DOWN MODE,�������Ϊ�ȸߺ��

    PWM_OUTPUT_ONE_PULSE	= 8	      //���һ����������,B1Xû�д˹��ܣ�����ѡ�������ǰ�ڴ���
}PWM_OUTPUT_TYPE;

/**
 * @brief  PWM����ģʽ 
 */
typedef enum __PWM_COUNTER_MODE
{
    PWM_COUNTER_MODE_DOWN = 0,        //�Ӹߵ��ͼ�����PWM���������֮���IDLEֵ�Ǹߵ�ƽ
    PWM_COUNTER_MODE_UP,              //�ӵ͵��߼�����PWM���������֮���IDLEֵ�ǵ͵�ƽ
    PWM_COUNTER_MODE_CENTER_ALIGNED1, //�ȴӵͼӵ��ߣ��ٴӸ߼����ͣ�ֻ�������ʱ�����ж�
    PWM_COUNTER_MODE_CENTER_ALIGNED2, //�ȴӵͼӵ��ߣ��ٴӸ߼����ͣ�ֻ�������ʱ�����ж�
    PWM_COUNTER_MODE_CENTER_ALIGNED3  //�ȴӵͼӵ��ߣ��ٴӸ߼����ͣ���������������ʱ�������ж�
}PWM_COUNTER_MODE;


/**
 * @brief  PWM����ģʽ���жϻ�DMA
 */
typedef enum _PWM_DMA_INTERRUPT_MODE
{
	PWM_REQ_INTERRUPT_MODE = 0,  /**< �ж�ģʽ  */

	PWM_REQ_DMA_MODE             /**< DMAģʽ  */

} PWM_DMA_INTERRUPT_MODE;


/**
 * @brief  PWM���ô������Ͷ���  
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
    uint8_t     CounterMode;        //PWM����ģʽ������ȡֵ��Χ #PWM_COUNTER_MODE

    uint8_t     OutputType;         //PWM�������,����ȡֵΪ#PWM_OUTPUT_TYPE

    uint32_t    FreqDiv;            //PWMƵ����ϵͳʱ�ӵķ�Ƶ�ȣ�ȡֵ��Χ[1, PWM_MAX_FREQ_DIV_VALUE]

    uint16_t    Duty;               //ռ�ձ�,ȡֵ��Χ[0~100]��Ӧռ�ձ�Ϊ[0%~100%]

    bool        DMAReqEnable;       //�Ƿ�ʹ��DMA��Mem����ռ�ձȵ������У�����ȡֵ�� 1 -- ʹ��DMA���� 0 -- ����DMA����

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
 * @brief PWM IOCTRL ����
 */
typedef enum _PWM_IOCTRL_CMD
{
	//update and Select cmd
	OUTPUT_SOFTWARE_UPDATE 		= 0x1,	    /**< TIMERx���update�Ĵ���  */

	OUTPUT_TYPE_UPDATE 			= 0x2,		/**< #PWM_OUTPUT_TYPE  */

	OUTPUT_FREQUENCY_UPDATE 	= 0x10,	    /**< Ƶ�ʸ���  */

	OUTPUT_DUTY_UPDATE 			= 0x20,		/**< ռ�ձȸ���  */

} PWM_IOCTRL_CMD;
/**
 * @brief PWM IOCTRL ����
 */
typedef struct _PWM_IOCTRL_ARG
{
	PWM_OUTPUT_TYPE     	   OutputType;         /**< PWM�������  */

	uint32_t    			   FreqDiv;            /**< PWMƵ����ϵͳʱ�ӵķ�Ƶ�ȣ�ȡֵ��Χ[1, PWM_MAX_FREQ_DIV_VALUE]  */

	uint16_t				   Duty;			   /**< ռ�ձ�  */

} PWM_IOCTRL_ARG;
 


/**
 * @brief     ����һ·PWM��GPIO����
 *
 * @param[in] PWMChSel  PWMͨ��ѡ�񣬲����ɼ�#PWM ͨ��GPIO���ù�ϵ�궨��
 * @param[in] PwmIoSel  ���磺��ѡ��TIMER3_PWM_A23_A27_B0_B2��
 *                            PwmIoSel = 0,��PWM������A23��
 *                            PwmIoSel = 1,������A27,
 *                            PwmIoSel = 2,������B0��
 *                            PwmIoSel = 3,������B2��
 * @param[in] PWMMode   PWM���ù�ϵ����ϸ�ο� #PWM_IO_MODE
 *
 * @return    ��
 */
void PWM_GpioConfig(uint8_t PWMChSel, uint8_t PwmIoSel, PWM_IO_MODE PWMMode);




/**
 * @brief     ѡ��һ����ʱ���µ�PWMͨ�������������
 *
 * @param[in] TimerIdx  ��ʱ�������ţ�ֻ֧��TIMER3��TIMER4
 * @param[in] PWMParam  PWM��ʼ����������ϸ�ο� PWMInfo
 * 						Slaveģʽ�£�CounterMode��OutputType������Ҫע�⣺
 * 						OutputType=PWM_OUTPUT_SINGLE_1��CounterMode=PWM_COUNTER_MODE_DOWNʱ��PWM����ʱ����һ��ë�̣�
 * 						����ʹ��OutputType=PWM_OUTPUT_SINGLE_1��CounterMode=PWM_COUNTER_MODE_UP�滻
 *
 * @return    ����ţ�0 - ��ȷ������Ϊ������ϸ�ο�PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_Config(TIMER_INDEX TimerIdx, PWM_StructInit *PWMParam);

/**
 * @brief     ������ʱ��TimerIdx��һ��ͨ��PWM
 *
 * @param[in] TimerIdx  ��ʱ�������ţ�ֻ֧��TIMER3��TIMER4
 *
 * @return    ����ţ�0 - ��ȷ������Ϊ������ϸ�ο�PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_Enable(TIMER_INDEX TimerIdx);

/**
 * @brief     �رն�ʱ��TimerIdx��һ��ͨ��PWM
 *
 * @param[in] TimerIdx  ��ʱ�������ţ�ֻ֧��TIMER3��TIMER4
 *
 * @return    ����ţ�0 - ��ȷ������Ϊ������ϸ�ο�PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_Disable(TIMER_INDEX TimerIdx);

/**
 * @brief     ��ͣ��ʱ��TimerIdx��һ��ͨ��PWM
 *
 * @param[in] TimerIdx  ��ʱ�������ţ���� # TIMER_INDEX
 * @param[in] IsPause   1: PWM��ͣ����������������ƽ  0��PWM�������
 *
 * @return    ����ţ�0 - ��ȷ������Ϊ������ϸ�ο�PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_Pause(TIMER_INDEX TimerIdx, bool IsPause);


/**
 * @brief     TimerIdx�µ�һ·PWM���в���
 *
 * @param[in] TimerIdx  ��ʱ�������ţ���� # TIMER_INDEX
 * @param[in] IOCtrl    IOCtrl����
 * @param[in] Arg       IOCtrl����
 *
 * @return    ����ţ�0 - ��ȷ������Ϊ������ϸ�ο�PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_IOCTRl(TIMER_INDEX TimerIdx, PWM_IOCTRL_CMD Cmd, PWM_IOCTRL_ARG *Arg);




/**
 * @brief      ռ�ձȶ�̬����
 *
 * @param[in]  TimerIdx ��ʱ��������,ֻ֧��TIMER3��TIMER4
 * @param[in]  Duty     0~0xFFFF��Ӧ0~100%
 *
 * @return     ����ţ�0 - ��ȷ������Ϊ������ϸ�ο�PWM_ERROR_CODE
 */
PWM_ERROR_CODE PWM_DutyConfigIQ16(TIMER_INDEX TimerIdx, uint16_t Duty);

/**
 * @brief      ����ʱ��TimerIdxռ�ձȲ�������ת�������ٷֱ�����ת��ʮ����������
 *
 * @param[in]  TimerIdx ��ʱ��������,ֻ֧��TIMER3��TIMER4
 * @param[in]  BufIn    ռ�ձ����������ַ����λ��
 * @param[out] BufOut   ռ�ձ����������ַ��ʮ������
 * @param[in]  Len      BufIn�ĳ���,��λ���ֽ�
 *
 * @return     BufOut   ����
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
