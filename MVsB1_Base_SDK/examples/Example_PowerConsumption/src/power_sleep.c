/**
 **************************************************************************************
 * @file    power_waste_example.c
 * @brief   power_waste example
 *
 * @author  Taowen
 * @version V1.0.0
 *
 * $Created: 2019-06-25 19:17:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdlib.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "spi_flash.h"
#include "remap.h"
#include "irqn.h"
#include "sys.h"
#include "chip_info.h"
#include "watchdog.h"
#include "reset.h"
#include <string.h>
#include "type.h"
#include "irqn.h"
#include "gpio.h"
#include "debug.h"
#include "timer.h"
#include "dma.h"
#include "uarts_interface.h"
#include "clk.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "remap.h"
#include "chip_info.h"
#include "powercontroller.h"
#include "dac.h"
#include "rtc.h"
#include "ir.h"
#include "pwc.h"
#include "backup.h"
#include "otg_device_hcd.h"
#include "efuse.h"
#include "audio_adc.h"
#include "spdif.h"
#include "adc.h"

void AudioADC_PowerDown(void);
////////////////////BP10 System Sleep///////////////////////////////////////////
 void SleepPowerConsumption_Config(void)
 {

 	//Power_DeepSleepLDO12ConfigTest(5, 0);//�˴�����LDO12�ĵ�ѹ

 	OTG_PowerDown();
 	//SREG_ANA_PHY_CTRL.USB_PHY_PWRDN = 1;	//�üĴ���Ĭ����PHY POWER DOWN

 	Efuse_ReadDataDisable(); //////////�ر�EFUSE////////
// 		SREG_EFS_CMD.PGMEN_R = 0;
// 		SREG_EFS_CMD.TRIG_AEN = 0;//����������һ��
// 		SREG_EFS_CMD.RDEN_R = 0;
// 		REG_EFUSE_VDD12_PD = 1;

 	ADC_Disable();
 	//SREG_SARADC_ANA_TRL.EN = 0;//ģ�����
 	///////////////////////////////

 	AudioADC_PowerDown();
// 	SREG_ASDM0_ANA_CTRL0.ASDM0_L_PD = 1;
// 	SREG_ASDM0_ANA_CTRL0.ASDM0_R_PD = 1;
//
// 	SREG_ASDM1_ANA_CTRL0.ASDM1_L_PD = 1;
// 	SREG_ASDM1_ANA_CTRL0.ASDM1_R_PD = 1;
//
// //	SREG_PGA0_R_CTRL.PGA0_R_GAINBOOST_PD=1;//���Ĵ�������
// //	SREG_PGA0_L_CTRL.PGA0_L_GAINBOOST_PD=1;
//
// 	SREG_PGA0_R_CTRL.PGA0_R_PD=1;
// 	SREG_PGA0_L_CTRL.PGA0_L_PD=1;
//
// //	SREG_PGA0_R_CTRL.PGA0_R_CZ_PD=1;//���Ĵ�������
// //	SREG_PGA0_L_CTRL.PGA0_L_CZ_PD=1;
//
// //	SREG_PGA1_R_CTRL.PGA1_R_GAINBOOST_PD=1;//���Ĵ�������
// //	SREG_PGA1_L_CTRL.PGA1_L_GAINBOOST_PD=1;
//
// 	SREG_PGA1_R_CTRL.PGA1_R_PD=1;
// 	SREG_PGA1_L_CTRL.PGA1_L_PD=1;
//
// 	SREG_PGA1_R_CTRL.PGA1_R_CZ_PD=1;
// 	SREG_PGA1_L_CTRL.PGA1_L_CZ_PD=1;
 	/////////////////////


 	AudioADC_VcomConfig(1);//ע�⣬VCOM���DAC�����ص�������2��PowerDown VCOM
// 	SREG_MDAC_VCOM_CTRL.VCOM_PD = 1;
// //	SREG_MDAC_VCOM_CTRL.VCOM_EN_MICBIAS = 0;
 	//////////////////////////////////

 	SPDIF_AnalogModuleDisable();
// 	SREG_SYSC_SPDIF_CTRL.SPDIF_LVL_EN  = 0;
// 	SREG_SYSC_SPDIF_CTRL.SPDIF_A2D_SEL = 0;
 	/////////////////////////////////////////

 	AudioDAC_PowerDown(DAC0);//����������SCF_PD
 	AudioDAC_PowerDown(DAC1);//����������SCF_PD

 	//////RF PD
// 	SREG_SYSC_RF_TEST_CTRL.RF_TEST_SW_PD_EN_SEL = 1;
// 	SREG_SYSC_RF_TEST_CTRL.RF_TEST_SW_PD_EN = 0;
 	//RF_PowerDownBySw();
 	RF_PowerDown();

 	//�ر�GPIOA0~A16��A23~A31��B0~B8�ĸ��ù��ܣ��û�������ѡ��ر�
 	GPIO_PortAModeSet(GPIOA0, 0x0000);
 	GPIO_PortAModeSet(GPIOA1, 0x0000);
 	GPIO_PortAModeSet(GPIOA2, 0x0000);
 	GPIO_PortAModeSet(GPIOA3, 0x0000);
 	GPIO_PortAModeSet(GPIOA4, 0x0000);
 	GPIO_PortAModeSet(GPIOA5, 0x0000);
 	GPIO_PortAModeSet(GPIOA6, 0x0000);
 	GPIO_PortAModeSet(GPIOA7, 0x0000);
 	GPIO_PortAModeSet(GPIOA8, 0x0000);
 	GPIO_PortAModeSet(GPIOA9, 0x0000);
 	GPIO_PortAModeSet(GPIOA10, 0x0000);
 	GPIO_PortAModeSet(GPIOA11, 0x0000);
 	GPIO_PortAModeSet(GPIOA12, 0x0000);
 	GPIO_PortAModeSet(GPIOA13, 0x0000);//A13��A14һ�㸴��ΪUART0
 	GPIO_PortAModeSet(GPIOA14, 0x0000);
 	GPIO_PortAModeSet(GPIOA15, 0x0000);
 	GPIO_PortAModeSet(GPIOA16, 0x0000);
 	GPIO_PortAModeSet(GPIOA17, 0x0000);
 	GPIO_PortAModeSet(GPIOA18, 0x0000);
 	GPIO_PortAModeSet(GPIOA19, 0x0000);
 	GPIO_PortAModeSet(GPIOA20, 0x0000);
 	GPIO_PortAModeSet(GPIOA21, 0x0000);
 	GPIO_PortAModeSet(GPIOA22, 0x0000);
 	GPIO_PortAModeSet(GPIOA23, 0x0000);//A23��A24һ�㸴��ΪPWC��
 	GPIO_PortAModeSet(GPIOA24, 0x0000);
 	GPIO_PortAModeSet(GPIOA25, 0x0000);
 	GPIO_PortAModeSet(GPIOA26, 0x0000);
 	GPIO_PortAModeSet(GPIOA27, 0x0000);
 	GPIO_PortAModeSet(GPIOA28, 0x0000);
 	GPIO_PortAModeSet(GPIOA29, 0x0000);

 #ifdef SW_PORTB  //������B��ζSW���ؿڣ�������A�ڻ�ԭ
 	GPIO_PortAModeSet(GPIOA30, 0x0000);//A30��A31һ�㸴��ΪSW���ص��Կ�
 	GPIO_PortAModeSet(GPIOA31, 0x0000);
 #else
 //	GPIO_PortAModeSet(GPIOA30, 0x0000);//A30��A31һ�㸴��ΪSW���ص��Կ�
 //	GPIO_PortAModeSet(GPIOA31, 0x0000);
 	GPIO_PortBModeSet(GPIOB0, 0x000);//B0��B1һ�㸴��ΪSW���ص��Կ�
 	GPIO_PortBModeSet(GPIOB1, 0x000);
 #endif
 	GPIO_PortBModeSet(GPIOB2, 0x000);//B2��B3һ�㸴��ΪUSB
 	GPIO_PortBModeSet(GPIOB3, 0x000);
 	GPIO_PortBModeSet(GPIOB4, 0x000);
 	GPIO_PortBModeSet(GPIOB5, 0x000);
 	GPIO_PortBModeSet(GPIOB6, 0x000);//B6��B7һ�㸴��ΪUART1
 	GPIO_PortBModeSet(GPIOB7, 0x000);

 	GPIO_RegSet(GPIO_A_IE,0x00000000);
 	GPIO_RegSet(GPIO_A_OE,0x00000000);
 	GPIO_RegSet(GPIO_A_OUTDS,0x00000000);
 	GPIO_RegSet(GPIO_A_PD,0xffffffff);//
 	GPIO_RegSet(GPIO_A_PU,0x00000000);//
 	GPIO_RegSet(GPIO_A_ANA_EN,0x00000000);
 	GPIO_RegSet(GPIO_A_PULLDOWN0,0x00000000);
 	GPIO_RegSet(GPIO_A_PULLDOWN1,0x00000000);
 	GPIO_RegSet(GPIO_B_IE,0x000);
 	GPIO_RegSet(GPIO_B_OE,0x000);
 	GPIO_RegSet(GPIO_B_OUTDS,0x000);
 	GPIO_RegSet(GPIO_B_PD,0xff);//B2��B3������B4,B5����
 	GPIO_RegSet(GPIO_B_PU,0x00);//B0��B1����
 	GPIO_RegSet(GPIO_B_ANA_EN,0x000);
 	GPIO_RegSet(GPIO_B_PULLDOWN,0x000);

 	SpiFlashInit(80000000, MODE_1BIT, 0, 1);

 	Clock_DeepSleepSysClkSelect(RC_CLK_MODE, FSHC_RC_CLK_MODE, 1);

 	Clock_PllClose();//DPLL������2.77mA�Ĺ��ģ�
 	Clock_AUPllClose();//AUPLL������Լ980uA�Ĺ���
 	///////////
 	Clock_HOSCDisable();//����RTCӦ�ò���RTC����ʱ����HOSC���򲻹ر�HOSC����24M����

 	Clock_LOSCDisable(); //����RTCӦ�ò���RTC����ʱ����LOSC���򲻹ر�LOSC����32K����

 	Power_PowerModeConfigTest(1, 1, 1);//B1X��HPMģʽ��LPMģʽ�ڹ�����ûɶ����

 	 __nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_PEND2), NDS32_SR_INT_PEND2);  //���pending
 }

//__attribute__((section(".driver.isr")))void RtcInterrupt(void)
//{
//	//	GPIO_RegOneBitClear(GPIO_A_IE, GPIO_INDEX1);
//	//	GPIO_RegOneBitSet(GPIO_A_OE, GPIO_INDEX1);
//	//	GPIO_RegOneBitSet(GPIO_A_OUT, GPIO_INDEX1);
//	//	GPIO_RegOneBitClear(GPIO_A_IN, GPIO_INDEX1);//�˴�����GPIO_A1��Ϊ�˹۲���û�н����ж�
//	if(RTC_IntFlagGet() == TRUE)
//	{
//		RTC_IntFlagClear();
//	}
//}
//
//uint32_t gpioPinIndex[2] = {0,0};
//__attribute__((section(".driver.isr"))) void GpioInterrupt(void)
//{
//
//	//	GPIO_RegOneBitClear(GPIO_A_IE, GPIO_INDEX0);
//	//	GPIO_RegOneBitSet(GPIO_A_OE, GPIO_INDEX0);
//	//	GPIO_RegOneBitSet(GPIO_A_OUT, GPIO_INDEX0);
//	//	GPIO_RegOneBitClear(GPIO_A_IN, GPIO_INDEX0);//�˴�����GPIO_A0��Ϊ�˹۲���û�н����ж�
//	gpioPinIndex[0] = GPIO_INTFlagGet(GPIO_A_INT_FLAG);
//	if(gpioPinIndex[0])
//	{
//		//DBG("GpioA Int Index:0x%08lx\n",gpioPinIndex[0]);
//		GPIO_INTFlagClear(GPIO_A_INT,gpioPinIndex[0]);
//	}
//
//	gpioPinIndex[1] = GPIO_INTFlagGet(GPIO_B_INT_FLAG);
//	if(gpioPinIndex[1])
//	{
//		//DBG("GpioB Int Index:0x%02lx\n",gpioPinIndex[1]);
//		GPIO_INTFlagClear(GPIO_B_INT,gpioPinIndex[1]);
//	}
//
//}
//
//__attribute__((section(".driver.isr"))) void Timer3Interrupt(void)
//{
//	TIMER_ERROR_CODE IntFlag;
//
//	//	GPIO_RegOneBitClear(GPIO_A_IE, GPIO_INDEX2);
//	//	GPIO_RegOneBitSet(GPIO_A_OE, GPIO_INDEX2);
//	//	GPIO_RegOneBitSet(GPIO_A_OUT, GPIO_INDEX2);
//	//	GPIO_RegOneBitClear(GPIO_A_IN, GPIO_INDEX2);//�˴�����GPIO_A2��Ϊ�˹۲���û�н����ж�
//
//   	Timer_InterruptFlagClear(TIMER3, PWC1_CAP_DATA_INTERRUPT_SRC);
//}
