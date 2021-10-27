#ifndef __SEG_LED_DISP__
#define __SEG_LED_DISP__

#include "type.h"
#include "app_config.h"
#include "gpio.h"

#ifdef DISP_DEV_7_LED

//7脚LED屏的驱动程序，扫描算法如下：
 
//7脚LED屏，习惯上将7个管脚的序号称为1脚--7脚
//为了方便写代码，本模块中将7个脚序号改为0-6
//COM阶段分别为COM0--COM6
//SEG段码分别为SEG0--SEG6
 
//扫描算法
 
//COM高选中: 输出高电平
//COM非选中: 高阻态
 
//SEG亮: 使能下拉电流源
//SEG灭: 高阻态
 
//COM0: 1脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态 
//COM1: 2脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态
//COM2: 3脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态
//COM3: 4脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态
//COM4: 5脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态
//COM5: 6脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态
//COM6: 7脚做COM输出高电平, 其他6个脚做SEG, 亮的笔画SEG使能下拉电流源，灭的笔画SEG设置为高阻态

#define MAX_LED_PIN_NUM		7	//LED 引脚数

//LED 屏引脚与GPIO 连接定义

//7只脚段码LED屏示例
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


//LED 引脚对应 GPIO 初始化
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

//LED 各个引脚使用前要先禁流

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

//设置LED 各个引脚输出高电平
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


//设置LED 各个引脚恒流源输入
#define LED_PIN1_IN_ON	GPIO_PortAPulldownSet(LED_PIN1_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN2_IN_ON	GPIO_PortAPulldownSet(LED_PIN2_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN3_IN_ON	GPIO_PortAPulldownSet(LED_PIN3_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN4_IN_ON	GPIO_PortAPulldownSet(LED_PIN4_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN5_IN_ON	GPIO_PortAPulldownSet(LED_PIN5_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN6_IN_ON	GPIO_PortAPulldownSet(LED_PIN6_BIT , GPIO_PortA_PULLDOWN_2MA6)

#define LED_PIN7_IN_ON	GPIO_PortAPulldownSet(LED_PIN7_BIT , GPIO_PortA_PULLDOWN_2MA6)


//禁止LED 各个引脚电流输入
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

