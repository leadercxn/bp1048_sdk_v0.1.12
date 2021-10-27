/*
 **************************************************************************************
 * @file    sys_gpio.h
 * @brief  
 * 
 * @author   
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

//#include <string.h>
//#include "debug.h"
//#include "ctrlvars.h"
//#include "config_opus.h"
#include "app_config.h"
//#include "audio_effect.h"
#ifndef __SYS_HW_INTERFACE__
#define __SYS_HW_INTERFACE__
//----- prohibit modify------------------------------------------------------//
#define  CONNECT(x, y, z)                                x ## y ## z

#define  CONVERT(x)                                      #x

#define  CONVERT_STRING(x)                               CONVERT(x)

#define  STRING_CONNECT(x, y , z)                        CONNECT(x, y, z)
//--------------------------------------------------------------------------//
#define  CONNECT2(x, y)                                  x ## y

#define  STRING_CONNECT2(x, y)                           CONNECT2(x, y)

//-----------register define-----------------------------------------------//
#define  GPIO                                           GPIO_
#define  GPIE                                           _IE
#define  GPOE                                           _OE
#define  GPPD                                           _PD
#define  GPPU                                           _PU
#define  GPOUT                                          _OUT
#define  GPIN                                           _IN
#define  GPPULLDOWN                                     _PULLDOWN
#define  PORT_INDEX                                     GPIO_INDEX
#define  GPINT                                           _INT

#if 0
//--------------i2s0 hardware define-----------------------------------//

#if I2S0_GROUP == 0///(GPIOA0~A4)

#define  CFG_I2S0_MCLK_PIN								GPIOA0
#define  CFG_I2S0_MCLK_PIN_MUX_SEL						(2)///msater
#define  CFG_I2S0_MCLK_PIN_MUX_SEL_SLAVE				(1)///slaver

#define  CFG_I2S0_LRCLK_PIN								GPIOA1
#define  CFG_I2S0_LRCLK_PIN_MUX_SEL						(4)

#define  CFG_I2S0_BCLK_PIN								GPIOA2
#define  CFG_I2S0_BCLK_PIN_MUX_SEL						(4)

#define  CFG_I2S0_DO_PIN								GPIOA3
#define  CFG_I2S0_DO_PIN_MUX_SEL						(3)

#define  CFG_I2S0_DIN_PIN								GPIOA4
#define  CFG_I2S0_DIN_PIN_MUX_SEL						(3)

#else

#define  CFG_I2S0_MCLK_PIN								GPIOA24
#define  CFG_I2S0_MCLK_PIN_MUX_SEL						(6)///msater
#define  CFG_I2S0_MCLK_PIN_MUX_SEL_SLAVE				(1)///slaver


#define  CFG_I2S0_LRCLK_PIN								GPIOA25
#define  CFG_I2S0_LRCLK_PIN_MUX_SEL						(4)

#define  CFG_I2S0_BCLK_PIN								GPIOA26
#define  CFG_I2S0_BCLK_PIN_MUX_SEL						(4)

#define  CFG_I2S0_DO_PIN								GPIOA27
#define  CFG_I2S0_DO_PIN_MUX_SEL						(6)

#define  CFG_I2S0_DIN_PIN								GPIOA28
#define  CFG_I2S0_DIN_PIN_MUX_SEL						(2)

#endif

//--------------i2s1   hardware define---(GPIOA8~A12)---------//

#define  CFG_I2S1_MCLK_PIN								GPIOA8
#define  CFG_I2S1_MCLK_PIN_MUX_SEL						(2)///msater
#define  CFG_I2S1_MCLK_PIN_MUX_SEL_SLAVE				(1)///slaver

#define  CFG_I2S1_LRCLK_PIN								GPIOA9
#define  CFG_I2S1_LRCLK_PIN_MUX_SEL						(1)

#define  CFG_I2S1_BCLK_PIN								GPIOA10
#define  CFG_I2S1_BCLK_PIN_MUX_SEL						(2)

#define  CFG_I2S1_DO_PIN								GPIOA11
#define  CFG_I2S1_DO_PIN_MUX_SEL						(3)

#define  CFG_I2S1_DIN_PIN								GPIOA12
#define  CFG_I2S1_DIN_PIN_MUX_SEL						(2)


#define  CFG_I2S_BIT_LENGTH						        I2S_LENGTH_16BITS
//--------------spdif hardware define-----------------------------------//
#define  CFG_SPDIF0_IN_PIN                              GPIOA25
#define  CFG_SPDIF0_IN_IE								GPIO_A_IE
#define  CFG_SPDIF0_IN_OE								GPIO_A_OE
#define  CFG_SPDIF0_IN_ANA_EN                           GPIO_A_ANA_EN
#define  CFG_SPDIF0_IN_PIN_MUX_SEL						(GPIO_INDEX25)
#define  CFG_SPDIF0_IN_PIN_MUX						    (12)
#define  CFG_SPDIF0_IN_ANA_SEL                          (SPDIF_ANA_INPUT_A25)

#define  CFG_SPDIF1_IN_PIN                              GPIOA28
#define  CFG_SPDIF1_IN_PU								GPIO_A_PU
#define  CFG_SPDIF1_IN_IE								GPIO_A_IE
#define  CFG_SPDIF1_IN_OE								GPIO_A_OE
#define  CFG_SPDIF1_IN_ANA_EN                           GPIO_A_ANA_EN
#define  CFG_SPDIF1_IN_PIN_MUX_SEL						(GPIO_INDEX28)
#define  CFG_SPDIF1_IN_PIN_MUX						    (11)
#define  CFG_SPDIF1_IN_ANA_SEL                          (SPDIF_ANA_INPUT_A28)

#define  CFG_SPDIF2_IN_PIN                              GPIOA26
#define  CFG_SPDIF2_IN_IE								GPIO_A_IE
#define  CFG_SPDIF2_IN_OE								GPIO_A_OE
#define  CFG_SPDIF2_IN_ANA_EN                           GPIO_A_ANA_EN
#define  CFG_SPDIF2_IN_PIN_MUX_SEL						(GPIO_INDEX26)
#define  CFG_SPDIF2_IN_PIN_MUX						    (12)
#define  CFG_SPDIF2_IN_ANA_SEL                          (SPDIF_ANA_INPUT_A26)


#define  CFG_SPDIF3_IN_PIN                              GPIOA27
#define  CFG_SPDIF3_IN_IE								GPIO_A_IE
#define  CFG_SPDIF3_IN_OE								GPIO_A_OE
#define  CFG_SPDIF3_IN_ANA_EN                           GPIO_A_ANA_EN
#define  CFG_SPDIF3_IN_PIN_MUX_SEL						(GPIO_INDEX27)
#define  CFG_SPDIF3_IN_PIN_MUX						    (12)
#define  CFG_SPDIF3_IN_ANA_SEL                          (SPDIF_ANA_INPUT_A27)

#define  CFG_SPDIF0_OUT_PIN								GPIOA27
#define  CFG_SPDIF0_OUT_MUX_SEL						    (8)

#define  CFG_SPDIF1_OUT_PIN								GPIOA27
#define  CFG_SPDIF1_OUT_MUX_SEL						    (8)

//--------------UART hardware define-----------------------------------//
#if (SLAVE_UART_IO == SLAVE_UART0_N0)
#define  CFG_UART_SLAVE_TX_PIN			 GPIOA13
#define  CFG_UART_SLAVE_TX_PIN_MUX_SEL	 (2)
#define  CFG_UART_SLAVE_RX_PIN			 GPIOA14
#define  CFG_UART_SLAVE_RX_PIN_MUX_SEL	 (1)	
#elif  (SLAVE_UART_IO == SLAVE_UART1_N0)
#define  CFG_UART_SLAVE_TX_PIN			 GPIOB0
#define  CFG_UART_SLAVE_TX_PIN_MUX_SEL	 (4)
#define  CFG_UART_SLAVE_RX_PIN			 GPIOB1
#define  CFG_UART_SLAVE_RX_PIN_MUX_SEL	 (1)	
#elif  (SLAVE_UART_IO == SLAVE_UART1_N1)
#define  CFG_UART_SLAVE_TX_PIN			 GPIOB2
#define  CFG_UART_SLAVE_TX_PIN_MUX_SEL	 (3)
#define  CFG_UART_SLAVE_RX_PIN			 GPIOB3
#define  CFG_UART_SLAVE_RX_PIN_MUX_SEL	 (1)
#elif  (SLAVE_UART_IO == SLAVE_UART1_N2)
#define  CFG_UART_SLAVE_TX_PIN			 GPIOB7
#define  CFG_UART_SLAVE_TX_PIN_MUX_SEL	 (3)
#define  CFG_UART_SLAVE_RX_PIN			 GPIOB6
#define  CFG_UART_SLAVE_RX_PIN_MUX_SEL	 (1)
#endif


//--------------ADC hardware define-----------------------------------//
#define  CFG_ADC_CHANNEL1                   			ADC_CHANNEL_GPIOA17
#define  CFG_ADC_CHANNEL1_PIN_BANK						'A'
#define  CFG_ADC_CHANNEL1_PIN							BIT(17)
#define  CFG_ADC_CHANNEL2                   			ADC_CHANNEL_GPIOA18
#define  CFG_ADC_CHANNEL2_PIN_BANK						'A'
#define  CFG_ADC_CHANNEL2_PIN							BIT(18)
#define  CFG_ADC_CHANNEL3                   			ADC_CHANNEL_GPIOA24
#define  CFG_ADC_CHANNEL3_PIN_BANK						'A'
#define  CFG_ADC_CHANNEL3_PIN  							BIT(24)
#define  CFG_ADC_CHANNEL4                   			ADC_CHANNEL_GPIOA25
#define  CFG_ADC_CHANNEL4_PIN_BANK						'A'
#define  CFG_ADC_CHANNEL4_PIN							BIT(25)
#define  CFG_ADC_CHANNEL5                   			ADC_CHANNEL_GPIOA26
#define  CFG_ADC_CHANNEL5_PIN_BANK						'A'
#define  CFG_ADC_CHANNEL5_PIN 							BIT(26)
#define  CFG_ADC_CHANNEL6                   			ADC_CHANNEL_GPIOA27
#define  CFG_ADC_CHANNEL6_PIN_BANK						'A'
#define  CFG_ADC_CHANNEL6_PIN							BIT(27)
#define  CFG_ADC_CHANNEL7                   			ADC_CHANNEL_GPIOA23
#define  CFG_ADC_CHANNEL7_PIN_BANK						'A'
#define  CFG_ADC_CHANNEL7_PIN							BIT(23)
#define  CFG_ADC_CHANNEL8                   			ADC_CHANNEL_GPIOA28
#define  CFG_ADC_CHANNEL8_PIN_BANK						'A'
#define  CFG_ADC_CHANNEL8_PIN							BIT(28)
#define  CFG_ADC_CHANNEL9                   			ADC_CHANNEL_GPIOA27
#define  CFG_ADC_CHANNEL9_PIN_BANK						'A'
#define  CFG_ADC_CHANNEL9_PIN							BIT(29)
#define  CFG_ADC_CHANNEL10                  			ADC_CHANNEL_GPIOA28
#define  CFG_ADC_CHANNEL10_PIN_BANK						'A'
#define  CFG_ADC_CHANNEL10_PIN							BIT(30)
#define  CFG_ADC_CHANNEL11                 			    ADC_CHANNEL_GPIOA31
#define  CFG_ADC_CHANNEL11_PIN_BANK						'A'
#define  CFG_ADC_CHANNEL11_PIN							BIT(31)

#define  ADC_LEVEL_CHANNEL_3_ANA_EN				((CFG_ADC_CHANNEL3_PIN_BANK - 'A') * 14 + GPIO_A_ANA_EN)
#define  ADC_LEVEL_CHANNEL_4_ANA_EN				((CFG_ADC_CHANNEL4_PIN_BANK - 'A') * 14 + GPIO_A_ANA_EN)
#define  ADC_LEVEL_CHANNEL_5_ANA_EN				((CFG_ADC_CHANNEL5_PIN_BANK - 'A') * 14 + GPIO_A_ANA_EN)
#define  ADC_LEVEL_CHANNEL_6_ANA_EN				((CFG_ADC_CHANNEL6_PIN_BANK - 'A') * 14 + GPIO_A_ANA_EN)
#define  ADC_LEVEL_CHANNEL_7_ANA_EN				((CFG_ADC_CHANNEL7_PIN_BANK - 'A') * 14 + GPIO_A_ANA_EN)
#define  ADC_LEVEL_CHANNEL_8_ANA_EN				((CFG_ADC_CHANNEL8_PIN_BANK - 'A') * 14 + GPIO_A_ANA_EN)
#define  ADC_LEVEL_CHANNEL_9_ANA_EN				((CFG_ADC_CHANNEL9_PIN_BANK - 'A') * 14 + GPIO_A_ANA_EN)
#define  ADC_LEVEL_CHANNEL_10_ANA_EN			((CFG_ADC_CHANNEL10_PIN_BANK- 'A') * 14 + GPIO_A_ANA_EN)
//-----chipset infomation------------------------------------------------//
#if CFG_CHIP_ID_AP8264A
#define CHIPSET_I "AP8264A"
#elif CFG_CHIP_ID_AP8264A2
#define CHIPSET_I "AP8264A2"
#elif CFG_CHIP_ID_AP8264P2
#define CHIPSET_I "AP8264P2"
#elif CFG_CHIP_ID_AP8248A2
#define CHIPSET_I "AP8248A2"
#elif CFG_CHIP_ID_AP8248P2
#define CHIPSET_I "AP8248P2"
#elif CFG_CHIP_ID_AP8224B2
#define CHIPSET_I "AP8224B2"
#elif CFG_CHIP_ID_AP8224C2
#define CHIPSET_I "AP8224C2"
#elif CFG_CHIP_ID_AP8224P2
#define CHIPSET_I "AP8224P2"
#elif CFG_CHIP_ID_AP8232A
#define CHIPSET_I "AP8232A"
#elif CFG_CHIP_ID_AP8232A2
#define CHIPSET_I "AP8232A2"
#else
#define CHIPSET_I "Chipset Unknown"
#error "Chipst Unknown,Please Select!!"
#endif
//------ usb,uart debug parameter--------------------------------------//
#if     (CFG_COMMUNICATION_METHID == 1)

        #pragma  message "uart 1 debug effect!!"
		#define  CFG_COMMUNICATION_BY_UART				 (1)
		#define  CFG_COMMUNICATION_BY_USB				 (0)
		#define	 CFG_UART_COMMUNICATION_TX_PIN			 GPIOB2
		#define  CFG_UART_COMMUNICATION_TX_PIN_MUX_SEL	 (3)
		#define  CFG_UART_COMMUNICATION_RX_PIN			 GPIOB3
		#define  CFG_UART_COMMUNICATION_RX_PIN_MUX_SEL	 (1)

#elif  (CFG_COMMUNICATION_METHID == 2)

        #pragma  message "uart 2 debug effect!!"
		#define  CFG_COMMUNICATION_BY_UART				 (1)
		#define  CFG_COMMUNICATION_BY_USB				 (0)
		#define	 CFG_UART_COMMUNICATION_TX_PIN			 GPIOB7
		#define  CFG_UART_COMMUNICATION_TX_PIN_MUX_SEL	 (3)
		#define  CFG_UART_COMMUNICATION_RX_PIN			 GPIOB6
		#define  CFG_UART_COMMUNICATION_RX_PIN_MUX_SEL	 (1)

#elif  (CFG_COMMUNICATION_METHID == 3)

        #pragma  message "usb  debug effect!!"
		#define  CFG_COMMUNICATION_BY_UART				 (0)
		#define  CFG_COMMUNICATION_BY_USB				 (1)

#elif  (CFG_COMMUNICATION_METHID == 4)
        #pragma  message "uart 2 + usb debug effect!!"
		#define  CFG_COMMUNICATION_BY_UART				 (1)
		#define  CFG_COMMUNICATION_BY_USB				 (1)
		#define	 CFG_UART_COMMUNICATION_TX_PIN			 GPIOB7
		#define  CFG_UART_COMMUNICATION_TX_PIN_MUX_SEL	 (3)
		#define  CFG_UART_COMMUNICATION_RX_PIN			 GPIOB6
		#define  CFG_UART_COMMUNICATION_RX_PIN_MUX_SEL	 (1)
#else

#define  CFG_COMMUNICATION_BY_UART						 (0)
#define  CFG_COMMUNICATION_BY_USB				         (0)


#endif

//-------------------------------------------------------------------------------//
#if CFG_USB_EN

     #if (CFG_USB_MODE == AUDIO)//USB AUDIO上传+下传功能
     #define  CFG_USB_IN_EN                                   (1)//USB AUDIO下传功能选择
     #define  CFG_USB_OUT_EN                                  (1)//USB AUDIO上传功能选择
     #pragma  message "USB MODE = AUDIO!"

     #elif (CFG_USB_MODE == AUDIO_HID)//USB AUDIO上传+下传+HID功能
     #define  CFG_USB_IN_EN                                   (1)//USB AUDIO下传功能选择
     #define  CFG_USB_OUT_EN                                  (1)//USB AUDIO上传功能选择
     #pragma  message "USB MODE = AUDIO_HID!"

     #elif (CFG_USB_MODE == AUDIO_HID_REMOTE)//USB AUDIO上传+下传+HID功能
     #define  CFG_USB_IN_EN                                   (1)//USB AUDIO下传功能选择
     #define  CFG_USB_OUT_EN                                  (1)//USB AUDIO上传功能选择
     #pragma  message "USB MODE = AUDIO_HID_REMOTE!"

     #elif (CFG_USB_MODE == AUDIO_GAME_HEADSET)//USB AUDIO上传+下传+HID功能
     #define  CFG_USB_IN_EN                                   (1)//USB AUDIO下传功能选择
     #define  CFG_USB_OUT_EN                                  (1)//USB AUDIO上传功能选择
     #pragma  message "USB MODE = AUDIO_GAME_HEADSET!"

     #elif (CFG_USB_MODE == HID)//只有USB HID功能
     #define  CFG_USB_IN_EN                                   (0)//USB AUDIO下传功能选择
     #define  CFG_USB_OUT_EN                                  (0)//USB AUDIO上传功能选择

     #undef  CFG_USB_OUT_STEREO_EN                             //是否支持USB立体声上传
     #undef  CFG_USB_RESAMPLE_EN                               //是否要求支持USB上传/下传音频转采样
     #undef  CFG_SUPPORT_USB_VOLUME_SET        			       //是否接受上位机音量控制
     #undef  CFG_USB_HW_MICRO_ADJUST                           //硬件采样率微调功能选择（目前只针对USB AUDIO/MIC音频流做处理）

	 #define  CFG_USB_OUT_STEREO_EN                           (0)//是否支持USB立体声数据上传    1=支持  0=不支持
     #define  CFG_USB_RESAMPLE_EN                             (0)//是否要求支持USB上传/下传音频转采样    1=支持  0=不支持
     #define  CFG_SUPPORT_USB_VOLUME_SET        			  (0)//是否接受上位机音量控制    1=支持  0=不支持
     #define  CFG_USB_HW_MICRO_ADJUST                         (0)//硬件采样率微调功能选择（目前只针对USB AUDIO/MIC音频流做处理）
     #pragma  message "USB MODE = HID!"

	 #elif (CFG_USB_MODE == AUDIO_HID_KEY)//只有USB AUDIO上传功能
     #define  CFG_USB_IN_EN                                   (0)//USB AUDIO下传功能选择
     #define  CFG_USB_OUT_EN                                  (1)//USB AUDIO上传功能选择
     #pragma  message "USB MODE = HID!"

     #elif (CFG_USB_MODE == AUDIO_ONLY_MIC)//USB AUDIO上传功能
     #define  CFG_USB_IN_EN                                   (0)//USB AUDIO下传功能选择
     #define  CFG_USB_OUT_EN                                  (1)//USB AUDIO上传功能选择
     #pragma  message "USB MODE = AUDIO ONLY MIC!"

     #else
     #define  CFG_USB_IN_EN                                   (0)//USB AUDIO下传功能选择
     #define  CFG_USB_OUT_EN                                  (0)//USB AUDIO上传功能选择
     #pragma  message "USB MODE = NO!"
     #endif///end of #if (CFG_USB_MODE ==

#else
     #undef  CFG_USB_OUT_STEREO_EN                             //是否支持USB立体声上传
     #undef  CFG_USB_RESAMPLE_EN                               //是否要求支持USB上传/下传音频转采样
     #undef  CFG_SUPPORT_USB_VOLUME_SET        			       //是否接受上位机音量控制
     #undef  CFG_USB_IN_EN                                     //USB AUDIO下传功能选择
     #undef  CFG_USB_OUT_EN                                    //USB AUDIO上传功能选择
     #undef  CFG_USB_HW_MICRO_ADJUST                           //硬件采样率微调功能选择（目前只针对USB AUDIO/MIC音频流做处理）

     #define  CFG_USB_OUT_STEREO_EN                           (0)//是否支持USB立体声上传
     #define  CFG_USB_RESAMPLE_EN                             (0)//是否要求支持USB上传/下传音频转采样
     #define  CFG_SUPPORT_USB_VOLUME_SET        			  (0)//是否接受上位机音量控制
     #define  CFG_USB_IN_EN                                   (0)//USB AUDIO下传功能选择
     #define  CFG_USB_OUT_EN                                  (0)//USB AUDIO上传功能选择
     #define  CFG_USB_HW_MICRO_ADJUST                         (0)//硬件采样率微调功能选择（目前只针对USB AUDIO/MIC音频流做处理）

#endif/////end of #if CFG_USB_EN
     ///调音接口判断////
#if CFG_USB_EN  /////
     #if (CFG_COMMUNICATION_METHID == 1)///
     #error "In USB MODE,Do not Select Uart (B2,B3) !!!!!!!!!!!!"
     #endif
#else
     #if CFG_COMMUNICATION_METHID == 3///usb
     #error "USB not open ,Please define CFG_USB_EN and CFG_USB_MODE!!!!!!!!!!!!"
     #endif

     #if CFG_COMMUNICATION_METHID == 4///usb and uart
     #error "USB not open ,Please define CFG_USB_EN and CFG_USB_MODE!!!!!!!!!!!!"
     #endif

#endif
//------------------------------------------------------------------//
#if CFG_USB_EN
	#if CFG_UPGRADE_PC_EN || CFG_UPGRADE_UDISK_EN
	#define FLASH_BOOT_EN	                            	(1)
	#else
	#define FLASH_BOOT_EN	                            	(0)
	#endif
#else
#define CFG_UPGRADE_PC_EN (0)
#define FLASH_BOOT_EN	                            	(0)
#endif

//----hdmi hardware define---only GPIOA27,GPIOA23,GPIOAB2,GPIOAB0-----------//
#if CFG_HDMI_IN_EN

#if (HDMI_CEC_PIN ==WAKEUP_GPIOA27)
    #define HDMI_CEC_IE     GPIO_A_IE
    #define HDMI_CEC_OE     GPIO_A_OE
    #define HDMI_CEC_PU     GPIO_A_PU
    #define HDMI_CEC_PD     GPIO_A_PD
    #define HDMI_CEC_OUT    GPIO_A_OUT
    #define HDMI_CEC_IN     GPIO_A_IN
#elif (HDMI_CEC_PIN ==WAKEUP_GPIOA23)
    #define HDMI_CEC_IE     GPIO_A_IE
    #define HDMI_CEC_OE     GPIO_A_OE
    #define HDMI_CEC_PU     GPIO_A_PU
    #define HDMI_CEC_PD     GPIO_A_PD
    #define HDMI_CEC_OUT    GPIO_A_OUT
    #define HDMI_CEC_IN     GPIO_A_IN
#else
    #define HDMI_CEC_IE     GPIO_B_IE
    #define HDMI_CEC_OE     GPIO_B_OE
    #define HDMI_CEC_PU     GPIO_B_PU
    #define HDMI_CEC_PD     GPIO_B_PD
    #define HDMI_CEC_OUT    GPIO_B_OUT
    #define HDMI_CEC_IN     GPIO_B_IN
#endif

#define HDMI_CEC_RECOVERY_OUT_IO()		GPIO_RegOneBitClear(HDMI_CEC_IE, BIT(HDMI_CEC_PIN)),\
										GPIO_RegOneBitSet(HDMI_CEC_OE, BIT(HDMI_CEC_PIN)),\
										GPIO_RegOneBitClear(HDMI_CEC_OUT, BIT(HDMI_CEC_PIN))

#define HDMI_CEC_RECOVERY_IN_IO()		GPIO_RegOneBitSet(HDMI_CEC_IE, BIT(HDMI_CEC_PIN)),\
										GPIO_RegOneBitClear(HDMI_CEC_OE, BIT(HDMI_CEC_PIN))

#define HDMI_CEC_RECOVERY_RECV_IO()		GPIO_RegOneBitClear(HDMI_CEC_OE, BIT(HDMI_CEC_PIN)),\
										GPIO_RegOneBitClear(HDMI_CEC_IE, BIT(HDMI_CEC_PIN))

#define HDMI_CEC_PULLUP_RECV_IO()	    GPIO_RegOneBitSet(HDMI_CEC_PU, BIT(HDMI_CEC_PIN)),\
										GPIO_RegOneBitClear(HDMI_CEC_PD, BIT(HDMI_CEC_PIN))

#define HDMI_CEC_LEVEL()			    GPIO_RegOneBitGet(HDMI_CEC_IN, BIT(HDMI_CEC_PIN))

#define	TIMER3_PWC_DATA_ADDR			(0x4002F034)
#define TIMER3_PWM_DUTY_ADDR			(0x4002F024)

#define HDMI_CEC_RECV_TIMER_ID	        TIMER3
#define HDMI_CEC_RECV_DMA_ID	        PERIPHERAL_ID_TIMER3
#define HDMI_CEC_RECV_DMA_ADDR	        TIMER3_PWC_DATA_ADDR

#define HDMI_CEC_SEND_TIMER_ID	        TIMER3
#define HDMI_CEC_SEND_DMA_ID	        PERIPHERAL_ID_TIMER3
#define HDMI_CEC_SEND_DMA_ADDR	        TIMER3_PWM_DUTY_ADDR

#define HDMI_CEC_ARBITRATION_TIMER_ID   TIMER5
#define HDMI_CEC_ARBITRATION_TIMER_IRQ  Timer5_IRQn
#define HDMI_CEC_ARBITRATION_TIMER_FUNC Timer5Interrupt

//---HPD 接口定义-----------------------------------------------//
#define HPD_IE            STRING_CONNECT(GPIO,HDMI_HPD_PORT,GPIE)
#define HPD_OE            STRING_CONNECT(GPIO,HDMI_HPD_PORT,GPOE)
#define HPD_PU            STRING_CONNECT(GPIO,HDMI_HPD_PORT,GPPU)
#define HPD_PD            STRING_CONNECT(GPIO,HDMI_HPD_PORT,GPPD)
#define HPD_IN            STRING_CONNECT(GPIO,HDMI_HPD_PORT,GPIN)
#define HPD_OUT           STRING_CONNECT(GPIO,HDMI_HPD_PORT,GPOUT)
#define HPD_INT           STRING_CONNECT(GPIO,HDMI_HPD_PORT,GPINT)


#define HDMI_HPD_CHECK_STATUS_IO	   	HPD_INT
#define HDMI_HPD_CHECK_INI_IO		   	HPD_INT
#define HDMI_HPD_CHECK_STATUS_IO_PIN   	HDMI_HPD_PIN
#define HDMI_HPD_CHECK_IO_INIT()	   	GPIO_RegOneBitClear(HPD_PU, HDMI_HPD_PIN),\
										GPIO_RegOneBitSet(HPD_PD, HDMI_HPD_PIN),\
										GPIO_RegOneBitClear(HPD_OE, HDMI_HPD_PIN),\
										GPIO_RegOneBitSet(HPD_IE, HDMI_HPD_PIN),\
										GPIO_INTEnable(HPD_INT, HDMI_HPD_PIN, GPIO_EDGE_TRIGGER),\
										GPIO_INTFlagClear(HPD_INT, HDMI_HPD_PIN)
									
#endif
#endif//end of if CFG_HDMI_IN_EN

//--------------------------------------------------------------------------//
#endif //end of __SYS_HW_INTERFACE__
