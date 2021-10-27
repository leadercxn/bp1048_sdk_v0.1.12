#ifndef __SEG_LED_DISP__
#define __SEG_LED_DISP__

#include "type.h"
#include "app_config.h"
#include "gpio.h"

#ifdef DISP_DEV_7_LED

//7��LED������������ɨ���㷨���£�
 
//7��LED����ϰ���Ͻ�7���ܽŵ���ų�Ϊ1��--7��
//Ϊ�˷���д���룬��ģ���н�7������Ÿ�Ϊ0-6
//COM�׶ηֱ�ΪCOM0--COM6
//SEG����ֱ�ΪSEG0--SEG6
 
//ɨ���㷨
 
//COM��ѡ��: ����ߵ�ƽ
//COM��ѡ��: ����̬
 
//SEG��: ʹ����������Դ
//SEG��: ����̬
 
//COM0: 1����COM����ߵ�ƽ, ����6������SEG, ���ıʻ�SEGʹ����������Դ����ıʻ�SEG����Ϊ����̬ 
//COM1: 2����COM����ߵ�ƽ, ����6������SEG, ���ıʻ�SEGʹ����������Դ����ıʻ�SEG����Ϊ����̬
//COM2: 3����COM����ߵ�ƽ, ����6������SEG, ���ıʻ�SEGʹ����������Դ����ıʻ�SEG����Ϊ����̬
//COM3: 4����COM����ߵ�ƽ, ����6������SEG, ���ıʻ�SEGʹ����������Դ����ıʻ�SEG����Ϊ����̬
//COM4: 5����COM����ߵ�ƽ, ����6������SEG, ���ıʻ�SEGʹ����������Դ����ıʻ�SEG����Ϊ����̬
//COM5: 6����COM����ߵ�ƽ, ����6������SEG, ���ıʻ�SEGʹ����������Դ����ıʻ�SEG����Ϊ����̬
//COM6: 7����COM����ߵ�ƽ, ����6������SEG, ���ıʻ�SEGʹ����������Դ����ıʻ�SEG����Ϊ����̬

#define MAX_LED_PIN_NUM		7	//LED ������

//LED ��������GPIO ���Ӷ���

//7ֻ�Ŷ���LED��ʾ��
#ifdef BP1064A2_DEMO
	#define LED_PIN1_PORT_PU 		GPIO_A_PU
	#define LED_PIN1_PORT_PD 		GPIO_A_PD
	#define LED_PIN1_PORT_OE 		GPIO_A_OE
	#define LED_PIN1_PORT_IE 		GPIO_A_IE
	#define LED_PIN1_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN1_BIT			GPIOA14
#else
	#define LED_PIN1_PORT_PU 		GPIO_A_PU
	#define LED_PIN1_PORT_PD 		GPIO_A_PD
	#define LED_PIN1_PORT_OE 		GPIO_A_OE
	#define LED_PIN1_PORT_IE 		GPIO_A_IE
	#define LED_PIN1_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN1_BIT			GPIOA10
#endif

#ifdef BP1064A2_DEMO
    #define LED_PIN2_PORT_PU 		GPIO_A_PU
	#define LED_PIN2_PORT_PD 		GPIO_A_PD
	#define LED_PIN2_PORT_OE 		GPIO_A_OE
	#define LED_PIN2_PORT_IE 		GPIO_A_IE
	#define LED_PIN2_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN2_BIT			GPIOA13
#else
    #define LED_PIN2_PORT_PU 		GPIO_A_PU
	#define LED_PIN2_PORT_PD 		GPIO_A_PD
	#define LED_PIN2_PORT_OE 		GPIO_A_OE
	#define LED_PIN2_PORT_IE 		GPIO_A_IE
	#define LED_PIN2_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN2_BIT			GPIOA7
#endif

#ifdef BP1064A2_DEMO
	#define LED_PIN3_PORT_PU 		GPIO_A_PU
	#define LED_PIN3_PORT_PD 		GPIO_A_PD
	#define LED_PIN3_PORT_OE 		GPIO_A_OE
	#define LED_PIN3_PORT_IE 		GPIO_A_IE
	#define LED_PIN3_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN3_BIT			GPIOA12
#else
	#define LED_PIN3_PORT_PU 		GPIO_A_PU
	#define LED_PIN3_PORT_PD 		GPIO_A_PD
	#define LED_PIN3_PORT_OE 		GPIO_A_OE
	#define LED_PIN3_PORT_IE 		GPIO_A_IE
	#define LED_PIN3_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN3_BIT			GPIOA9
#endif

#ifdef BP1064A2_DEMO
	#define LED_PIN4_PORT_PU 		GPIO_A_PU
	#define LED_PIN4_PORT_PD 		GPIO_A_PD
	#define LED_PIN4_PORT_OE 		GPIO_A_OE
	#define LED_PIN4_PORT_IE 		GPIO_A_IE
	#define LED_PIN4_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN4_BIT			GPIOA11
#else
	#define LED_PIN4_PORT_PU 		GPIO_A_PU
	#define LED_PIN4_PORT_PD 		GPIO_A_PD
	#define LED_PIN4_PORT_OE 		GPIO_A_OE
	#define LED_PIN4_PORT_IE 		GPIO_A_IE
	#define LED_PIN4_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN4_BIT			GPIOA0
#endif

#ifdef BP1064A2_DEMO
    #define LED_PIN5_PORT_PU 		GPIO_A_PU
	#define LED_PIN5_PORT_PD 		GPIO_A_PD
	#define LED_PIN5_PORT_OE 		GPIO_A_OE
	#define LED_PIN5_PORT_IE 		GPIO_A_IE
	#define LED_PIN5_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN5_BIT			GPIOA10
#else
    #define LED_PIN5_PORT_PU 		GPIO_A_PU
	#define LED_PIN5_PORT_PD 		GPIO_A_PD
	#define LED_PIN5_PORT_OE 		GPIO_A_OE
	#define LED_PIN5_PORT_IE 		GPIO_A_IE
	#define LED_PIN5_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN5_BIT			GPIOA6
#endif

#ifdef BP1064A2_DEMO
	#define LED_PIN6_PORT_PU 		GPIO_A_PU
	#define LED_PIN6_PORT_PD 		GPIO_A_PD
	#define LED_PIN6_PORT_OE 		GPIO_A_OE
	#define LED_PIN6_PORT_IE 		GPIO_A_IE
	#define LED_PIN6_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN6_BIT			GPIOA9
#else
	#define LED_PIN6_PORT_PU 		GPIO_A_PU
	#define LED_PIN6_PORT_PD 		GPIO_A_PD
	#define LED_PIN6_PORT_OE 		GPIO_A_OE
	#define LED_PIN6_PORT_IE 		GPIO_A_IE
	#define LED_PIN6_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN6_BIT			GPIOA5
#endif

#ifdef BP1064A2_DEMO
    #define LED_PIN7_PORT_PU 		GPIO_A_PU
	#define LED_PIN7_PORT_PD 		GPIO_A_PD
	#define LED_PIN7_PORT_OE 		GPIO_A_OE
	#define LED_PIN7_PORT_IE 		GPIO_A_IE
	#define LED_PIN7_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN7_BIT			GPIOA8
#else
    #define LED_PIN7_PORT_PU 		GPIO_A_PU
	#define LED_PIN7_PORT_PD 		GPIO_A_PD
	#define LED_PIN7_PORT_OE 		GPIO_A_OE
	#define LED_PIN7_PORT_IE 		GPIO_A_IE
	#define LED_PIN7_PORT_OUT 		GPIO_A_OUT
	#define LED_PIN7_BIT			GPIOA1
#endif


//LED ���Ŷ�Ӧ GPIO ��ʼ��
#define LedPinGpioInit()	GPIO_PortAModeSet(LED_PIN1_BIT,0),\
                            GPIO_PortAModeSet(LED_PIN2_BIT,0),\
	                        GPIO_PortAModeSet(LED_PIN3_BIT,0),\
	                        GPIO_PortAModeSet(LED_PIN4_BIT,0),\
                            GPIO_PortAModeSet(LED_PIN5_BIT,0),\
	                        GPIO_PortAModeSet(LED_PIN6_BIT,0),\
	                        GPIO_PortAModeSet(LED_PIN7_BIT,0),\
							GPIO_PortAOutDsSet(LED_PIN1_BIT,GPIO_PortA_OUTDS_8MA),\
							GPIO_PortAOutDsSet(LED_PIN2_BIT,GPIO_PortA_OUTDS_8MA),\
							GPIO_PortAOutDsSet(LED_PIN3_BIT,GPIO_PortA_OUTDS_8MA),\
							GPIO_PortAOutDsSet(LED_PIN4_BIT,GPIO_PortA_OUTDS_8MA),\
							GPIO_PortAOutDsSet(LED_PIN5_BIT,GPIO_PortA_OUTDS_8MA),\
							GPIO_PortAOutDsSet(LED_PIN6_BIT,GPIO_PortA_OUTDS_8MA),\
							GPIO_PortAOutDsSet(LED_PIN7_BIT,GPIO_PortA_OUTDS_8MA),\
                            GPIO_RegOneBitClear(LED_PIN1_PORT_PU, LED_PIN1_BIT),\
	                        GPIO_RegOneBitClear(LED_PIN2_PORT_PU, LED_PIN2_BIT),\
							GPIO_RegOneBitClear(LED_PIN3_PORT_PU, LED_PIN3_BIT),\
							GPIO_RegOneBitClear(LED_PIN4_PORT_PU, LED_PIN4_BIT),\
	                        GPIO_RegOneBitClear(LED_PIN5_PORT_PU, LED_PIN5_BIT),\
							GPIO_RegOneBitClear(LED_PIN6_PORT_PU, LED_PIN6_BIT),\
							GPIO_RegOneBitClear(LED_PIN7_PORT_PU, LED_PIN7_BIT),\
                            GPIO_RegOneBitClear(LED_PIN1_PORT_PD, LED_PIN1_BIT),\
                            GPIO_RegOneBitClear(LED_PIN2_PORT_PD, LED_PIN2_BIT),\
                            GPIO_RegOneBitClear(LED_PIN3_PORT_PD, LED_PIN3_BIT),\
                            GPIO_RegOneBitClear(LED_PIN4_PORT_PD, LED_PIN4_BIT),\
                            GPIO_RegOneBitClear(LED_PIN5_PORT_PD, LED_PIN5_BIT),\
                            GPIO_RegOneBitClear(LED_PIN6_PORT_PD, LED_PIN6_BIT),\
                            GPIO_RegOneBitClear(LED_PIN7_PORT_PD, LED_PIN7_BIT),\
                            GPIO_RegOneBitClear(LED_PIN1_PORT_OE, LED_PIN1_BIT),\
                            GPIO_RegOneBitClear(LED_PIN2_PORT_OE, LED_PIN2_BIT),\
                            GPIO_RegOneBitClear(LED_PIN3_PORT_OE, LED_PIN3_BIT),\
                            GPIO_RegOneBitClear(LED_PIN4_PORT_OE, LED_PIN4_BIT),\
                            GPIO_RegOneBitClear(LED_PIN5_PORT_OE, LED_PIN5_BIT),\
                            GPIO_RegOneBitClear(LED_PIN6_PORT_OE, LED_PIN6_BIT),\
                            GPIO_RegOneBitClear(LED_PIN7_PORT_OE, LED_PIN7_BIT),\
							GPIO_PortAPulldownSet(LED_PIN1_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN2_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN3_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN4_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN5_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN6_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN7_BIT,0)

//LED ��������ʹ��ǰҪ�Ƚ���

#define LedAllPinGpioInput()   GPIO_RegOneBitClear(LED_PIN1_PORT_OE, LED_PIN1_BIT),\
                            GPIO_RegOneBitClear(LED_PIN2_PORT_OE, LED_PIN2_BIT),\
                            GPIO_RegOneBitClear(LED_PIN3_PORT_OE, LED_PIN3_BIT),\
                            GPIO_RegOneBitClear(LED_PIN4_PORT_OE, LED_PIN4_BIT),\
                            GPIO_RegOneBitClear(LED_PIN5_PORT_OE, LED_PIN5_BIT),\
                            GPIO_RegOneBitClear(LED_PIN6_PORT_OE, LED_PIN6_BIT),\
                            GPIO_RegOneBitClear(LED_PIN7_PORT_OE, LED_PIN7_BIT),\
							GPIO_PortAPulldownSet(LED_PIN1_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN2_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN3_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN4_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN5_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN6_BIT,0),\
							GPIO_PortAPulldownSet(LED_PIN7_BIT,0)

//����LED ������������ߵ�ƽ
#define LED_PIN1_OUT_HIGH	GPIO_RegOneBitSet(LED_PIN1_PORT_OE , LED_PIN1_BIT),\
		                    GPIO_RegOneBitSet(LED_PIN1_PORT_OUT , LED_PIN1_BIT)
                      
#define LED_PIN2_OUT_HIGH	GPIO_RegOneBitSet(LED_PIN2_PORT_OE , LED_PIN2_BIT),\
		                    GPIO_RegOneBitSet(LED_PIN2_PORT_OUT , LED_PIN2_BIT)

#define LED_PIN3_OUT_HIGH	GPIO_RegOneBitSet(LED_PIN3_PORT_OE , LED_PIN3_BIT),\
		                    GPIO_RegOneBitSet(LED_PIN3_PORT_OUT , LED_PIN3_BIT)

#define LED_PIN4_OUT_HIGH	GPIO_RegOneBitSet(LED_PIN4_PORT_OE , LED_PIN4_BIT),\
		                    GPIO_RegOneBitSet(LED_PIN4_PORT_OUT , LED_PIN4_BIT)

#define LED_PIN5_OUT_HIGH	GPIO_RegOneBitSet(LED_PIN5_PORT_OE , LED_PIN5_BIT),\
		                    GPIO_RegOneBitSet(LED_PIN5_PORT_OUT , LED_PIN5_BIT)

#define LED_PIN6_OUT_HIGH	GPIO_RegOneBitSet(LED_PIN6_PORT_OE , LED_PIN6_BIT),\
		                    GPIO_RegOneBitSet(LED_PIN6_PORT_OUT , LED_PIN6_BIT)

#define LED_PIN7_OUT_HIGH	GPIO_RegOneBitSet(LED_PIN7_PORT_OE , LED_PIN7_BIT),\
		                    GPIO_RegOneBitSet(LED_PIN7_PORT_OUT , LED_PIN7_BIT)


//����LED �������ź���Դ����
#define LED_PIN1_IN_ON	GPIO_PortAPulldownSet(LED_PIN1_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN2_IN_ON	GPIO_PortAPulldownSet(LED_PIN2_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN3_IN_ON	GPIO_PortAPulldownSet(LED_PIN3_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN4_IN_ON	GPIO_PortAPulldownSet(LED_PIN4_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN5_IN_ON	GPIO_PortAPulldownSet(LED_PIN5_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN6_IN_ON	GPIO_PortAPulldownSet(LED_PIN6_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN7_IN_ON	GPIO_PortAPulldownSet(LED_PIN7_BIT , GPIO_PortA_PULLDOWN_2MA6)


//��ֹLED �������ŵ�������
#define LED_PIN1_IN_OFF	GPIO_PortAPulldownSet(LED_PIN1_BIT , 0)

#define LED_PIN2_IN_OFF	GPIO_PortAPulldownSet(LED_PIN2_BIT , 0)

#define LED_PIN3_IN_OFF	GPIO_PortAPulldownSet(LED_PIN3_BIT , 0)

#define LED_PIN4_IN_OFF	GPIO_PortAPulldownSet(LED_PIN4_BIT , 0)

#define LED_PIN5_IN_OFF	GPIO_PortAPulldownSet(LED_PIN5_BIT , 0)

#define LED_PIN6_IN_OFF	GPIO_PortAPulldownSet(LED_PIN6_BIT , 0)

#define LED_PIN7_IN_OFF	GPIO_PortAPulldownSet(LED_PIN7_BIT , 0)


//void DispLedIcon(ICON_IDX Icon, ICON_STATU Light);
void LedDispDevSymbol(void);
void LedDispRepeat(void);
void LedDispPlayState(void);

#endif

#endif

