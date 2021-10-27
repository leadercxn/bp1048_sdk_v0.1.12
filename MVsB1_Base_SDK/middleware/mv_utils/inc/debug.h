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
*����Ϊ��ӡ������Ϣ�Ľӿ�,��������:
*1.��ӡǰ׺�ӿ� : 
*  1)�����Ҫ�ڴ�ӡʱ���ģ��ǰ׺ ,����� APP_DBG(),
*  2)��Ҫ�ر�ĳģ��ĵ�����Ϣ,������ע������ XXX_MODULE_DEBUG�ĺ�
*	��ģ���,�Ǹ������¹��� �����ֵ� : 
*	������Ϊ����9��ģ��,
*	����7��ģ����AppsĿ¼:��Ϊmedia play,bt,hdmi_in,Main_task,usb_audio_mode,waiting_modeģ��
*	��������ģ����:Deviceģ�顢Servicesģ��
*	ʣ��ͳһ���ڣ�DEBģ��
*
*2.����ӡǰ׺�ӿ� : ���� DBG()
*************************************************************/




#define	DEVICE_MODULE_DEBUG			//DEVICEģ���ӡ������Ϣ����
#define	SERVICE_MODULE_DEBUG		//SERVICEģ���ӡ������Ϣ����
#define	MEDIA_MODULE_DEBUG			//MEDIAģ���ӡ������Ϣ����
#define	BT_MODULE_DEBUG				//BTģ���ӡ������Ϣ����
#define	MAINTSK_MODULE_DEBUG		//MAINTSKģ���ӡ������Ϣ����
#define	USBAUDIO_MODULE_DEBUG		//USBAUDIOģ���ӡ������Ϣ����
#define	WAITING_MODULE_DEBUG		//WAITINGģ���ӡ������Ϣ����



uint8_t DBG_Global(char * str,char **fmt, ...);

#define	APP_DBG(format, ...)	do{char *fmt=format;if(DBG_Global(__FILE__,&fmt, ##__VA_ARGS__))printf(fmt, ##__VA_ARGS__);}while(0)
#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define	OTG_DBG(format, ...)		//printf(format, ##__VA_ARGS__)
#define	BT_DBG(format, ...)			printf(format, ##__VA_ARGS__)//do{printf("[BT] "); printf(format, ##__VA_ARGS__);}while(0)



int DbgUartInit(int Which, unsigned int BaudRate, unsigned char DatumBits, unsigned char Parity, unsigned char StopBits);



/************************** TOGGLE DEBUG**********************************
 *
 * Ϊ�˷������ʱ��debug�ļ��ṩIO toggle����
 * 1. LED_IO_TOGGLE �˿ں���������б���IO��ʵ��api��0~n ��Ҫʹ�ã�
 * 2��OS_INT_TOGGLE Os������ж�ʱ��Ƭ �۲⣬��task���ֱ�Ϊ��ţ�led0~x���ж�����֮����Ҫ�ȿ���LED_IO_TOGGLE
 *
 *************************************************************/

/**OS�����л����ж� ʱIO toggle���ߵ�ƽ����ִ��ʱ��Ƭ**/
//#define OS_INT_TOGGLE

/**��Ҫtoggle��task Name,���ò���,�������ȼ��ߵ���ǰ��˳�� ��ӦLED 0��LED 1....**/
#define DBG_TASK_LIST	{{"AudioCore"}, {"Decoder"}, {"Device"},  {"IDLE"}}
//uint8_t dbgtasklist[][configMAX_TASK_NAME_LEN];
void DbgTaskTGL_set();
void DbgTaskTGL_clr();


/*********************************************
 * �ж�toggle��ڣ���Ҫ����crt0.S��OS_Trap_Int_Comm ����������

 	 ԭ��������:
 	jral $r1

 	�޸�Ϊ:
 	pushm $r0,$r1
 	jal   OS_dbg_int_in
 	popm $r0,$r1
 	push $r0
	jral $r1
 	pop  $r0
 	jal   OS_dbg_int_out
 ********************************************/
/**�ж�toggle��IO(LED_PORT_LIST) ���� OS ��ź���,����DBG_INT_ID, ������OS�ж���һ��IO**/
#define DBG_INT_ID				18 //Int18ΪBT INT0ΪOS �����μ�crt0.S

void OS_dbg_int_in(uint32_t int_num);
void OS_dbg_int_out(uint32_t int_num);



//#define LED_IO_TOGGLE //ʹ��IO toggle����  led���߷ֹ۲� ��� LedPortInit()


/**����IO,�����ڳ���ִ��ʱ����ӻ�����׼SDK��ʹ��**/
/**����ߵ͵�ƽ�������أ��½��أ���ת��led�ȣ�1~4�˿������ã������Σ�led��������Ч��ƽ��**/
/**����LedPortInit��ʼ���˿����á�**/
#define LED_ON_LEVEL	1//�����߸ߵ�ƽ �趨��
/**������Ҫ������Ӧ�˿ں����,led 0~n-1��ע��˿ڸ��ù��**/
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
void LedOn(uint8_t Index);//1~4 �Ƿ���Чȡ����LEDx_PORT�Ƿ���
void LedOff(uint8_t Index);
void LedToggle(uint8_t Index);
void LedPortRise(uint8_t Index);
void LedPortDown(uint8_t Index);
bool LedPortGet(uint8_t Index);//TRUE:�ߵ�ƽ��FALSE:�͵�ƽ��index��Чʱ Ĭ��FALSE

#ifdef __cplusplus
}
#endif//__cplusplus

/**
 * @}
 * @}
 */
 
#endif //__DBG_H__ 

