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
#include "adc.h"
#include "adc.h"
#include "delay.h"
#include "otg_device_hcd.h"
#include "efuse.h"
#include "audio_adc.h"
#include "spdif.h"

////////////////////BP10 System DeepSleep///////////////////////////////////////////
//����GPIO����Դ
void SystermGPIOWakeupConfig(PWR_SYSWAKEUP_SOURCE_SEL source,PWR_WAKEUP_GPIO_SEL gpio,PWR_SYSWAKEUP_SOURCE_EDGE_SEL edge)
{
	if(gpio < 32)
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   (1 << gpio));
		GPIO_RegOneBitClear(GPIO_A_OE, (1 << gpio));
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			GPIO_RegOneBitSet(GPIO_A_PU, (1 << gpio));//��ΪоƬ��GPIO���ڲ�����������,ѡ���½��ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitClear(GPIO_A_PD, (1 << gpio));
		}
		else if( edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			GPIO_RegOneBitClear(GPIO_A_PU, (1 << gpio));//��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitSet(GPIO_A_PD, (1 << gpio));
		}
	}
	else if(gpio < 41)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   (1 << (gpio - 32)));
		GPIO_RegOneBitClear(GPIO_B_OE, (1 << (gpio - 32)));
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			GPIO_RegOneBitSet(GPIO_B_PU, (1 << (gpio - 32)));//��ΪоƬ��GPIO���ڲ�����������,ѡ���½��ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitClear(GPIO_B_PD, (1 << (gpio - 32)));
		}
		else if( edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			GPIO_RegOneBitClear(GPIO_B_PU, (1 << (gpio - 32)));//��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitSet(GPIO_B_PD, (1 << (gpio - 32)));
		}
	}
	else if(gpio == 41)
	{
//		GPIO_RegOneBitSet(GPIO_C_IE,   (1 << (gpio - 32)));
//		GPIO_RegOneBitClear(GPIO_C_OE, (1 << (gpio - 32)));
//		REG_BKUP_ACCESS_PROTECT = 0xca;
//		REG_BKUP_ACCESS_PROTECT = 0x5c;
		BACKUP_WriteEnable();
//		SREG_BKUP_GPIO_C0_CTRL.GPIO_C0_REG_IE = 1;
//		SREG_BKUP_GPIO_C0_CTRL.GPIO_C0_REG_OE = 0;
		BACKUP_C0RegSet(BKUP_GPIO_C0_REG_IE_OFF, TRUE);
		BACKUP_C0RegSet(BKUP_GPIO_C0_REG_OE_OFF, FALSE);
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
//			SREG_BKUP_GPIO_C0_CTRL.GPIO_C0_REG_PU = 1;
//			SREG_BKUP_GPIO_C0_CTRL.GPIO_C0_REG_PD = 0;
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PU_OFF, TRUE);
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PD_OFF, FALSE);
		}
		else if( edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
//			SREG_BKUP_GPIO_C0_CTRL.GPIO_C0_REG_PU = 0;//��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
//			SREG_BKUP_GPIO_C0_CTRL.GPIO_C0_REG_PD = 1;
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PU_OFF, FALSE);//��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PD_OFF, TRUE);
		}
		BACKUP_WriteDisable();
	}

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(source, gpio, edge);
	Power_WakeupEnable(source);

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();
}

static uint32_t time = 0;
static uint32_t rtc_alarm =0;
void SystermRTCWakeupConfig(uint32_t alarm)
{
	RTC_ClockSrcSel(RC_32K);
	RTC_IntDisable();
	RTC_IntFlagClear();
	RTC_WakeupDisable();

	time = 0;
	RTC_SecSet(time);
	rtc_alarm = alarm;
	RTC_SecAlarmSet(rtc_alarm);

	RTC_WakeupEnable();
	RTC_IntEnable();

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	NVIC_EnableIRQ(Rtc_IRQn);
	NVIC_SetPriority(Rtc_IRQn, 1);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE7_RTC, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE7_RTC);
}

void SystermPowerKeyWakeupConfig(PWR_SYSWAKEUP_SOURCE_SEL source, PWR_SYSWAKEUP_SOURCE_EDGE_SEL edge)
{
	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE6_POWERKEY, 0, edge);
	Power_WakeupEnable(SYSWAKEUP_SOURCE6_POWERKEY);
}

void SystermIRWakeupConfig(IR_MODE_SEL ModeSel, IR_IO_SEL GpioSel, IR_CMD_LEN_SEL CMDLenSel)
{
	IR_Config(ModeSel, GpioSel, CMDLenSel);
	IR_Enable();
	//IR_InterruptEnable();//IR���ѹ��ܲ��迪IR�ж�
	IR_WakeupEnable();

	if(GpioSel == IR_GPIOA29)
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX29);
		GPIO_RegOneBitSet(GPIO_A_IN,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OUT, GPIO_INDEX29);
	}
	else if(GpioSel == IR_GPIOB7)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX7);
		GPIO_RegOneBitSet(GPIO_B_IN,   GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_OUT, GPIO_INDEX7);
	}
	else if(GpioSel == IR_GPIOB6)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX6);
		GPIO_RegOneBitSet(GPIO_B_IN,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OUT, GPIO_INDEX6);
	}

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE9_IR, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE9_IR);
}

void SystermTimer3IntWakeupConfig(TIMER_INTERRUPT_SRC ModeSel)
{
	PWC_StructInit  PWCParam;
   uint8_t     TimeScaleIdx = 2;
   uint32_t    TimeScaleTbl[]  = {1, 10, 100, 1000};

	//SREG_CLK_SEL0.TIMER3_CLK_SEL = 1;//timer3_clk selection:0: sys_clk   1: rc_clk
	 Clock_Timer3ClkSelect(RC_CLK_MODE);//timer3_Interrupt wakeup deepsleep  only can selection SYSTEM_CLK_MODE and  RC_CLK_MODE
	//SREG_CLK_EN0.TIMER3_CLK_EN = 1;
	 Clock_Module1Enable(TIMER3_CLK_EN);

	if( ModeSel == UPDATE_INTERRUPT_SRC )//��ʱ�������ж�
	{
        Timer_InterruptFlagClear(TIMER3, UPDATE_INTERRUPT_SRC);
        Timer_Config(TIMER3, 10*1000*1000, 1);
        Timer_Start(TIMER3);

//			SREG_TIMER3_CTRL.TIMER3_CEN        = 0;
//			SREG_TIMER3_CTRL.TIMER3_OPM		   = 0;
//			SREG_TIMER3_CTRL.TIMER3_UIF_CLR    = 1;
//			SREG_TIMER3_CTRL.TIMER3_HALT_EN	   = 1;
//			SREG_TIMER3_CTRL.TIMER3_INT_EN     = 1;
//
//			REG_TIMER3_ARR = 10*1000;
//			REG_TIMER3_PSC = 12*1000 - 1;//��ʱ��λΪ1mS
//
//			SREG_TIMER3_CTRL.TIMER3_CEN        = TRUE;
//			SREG_TIMER3_CTRL.TIMER3_UG		   = 1;

//			SREG_PWR_PWR_MODE_CTRL.HRC_HALT_IN_DSLP = 0;//����DeepSleepӲ������ر�RC CLK
	}
	else if( ModeSel == PWC1_CAP_DATA_INTERRUPT_SRC)  //PWC�����ж�
	{
///////////////////////////////////PWC�����ж�///////////////////////////////////////////////
        GPIO_RegOneBitSet(GPIO_B_IE,GPIO_INDEX7);
		PWC_GpioConfig(TIMER3, GPIO_TimerRemapIndexB7);//GPIOB7��Ϊ�����źŽ��������ͬʱ������ΪPWC������
//         //PWC��������
//          PWCParam.Polarity        = PWC_POLARITY_BOTH;
//          PWCParam.SingleGet       = PWC_CAPTURE_ONCE;//IsSingle;
//          PWCParam.TimeScale       = 20;
//          PWCParam.FilterTime      = 6;//�˲�ʱ�䣬��Χ��1/Fpwc ~ 128/Fpwc
//          PWCParam.PwcSourceSelect      = 0;//���ù�DMA
      	//PWC��������
      	PWCParam.Polarity        = PWC_POLARITY_BOTH;
      	PWCParam.SingleGet       = 1;//1: IsSingle   0: Continue
      	PWCParam.DMAReqEnable    = 0;
      	PWCParam.TimeScale       = TimeScaleTbl[TimeScaleIdx];
      	PWCParam.FilterTime      = 3;

      	PWC_Config(TIMER3, &PWCParam);//�ú������Ѿ������ж�ʹ��
      	PWC_Enable(TIMER3);

        DBG("PWC INPUT: B7(B7����ΪIR)\n");

  		IR_Config(IR_MODE_SONY, IR_GPIOB7, IR_SONY_12BITS);
  		IR_Enable();
	}

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE10_TIMER3_INT, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE10_TIMER3_INT);

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();


}

static uint8_t UART_RecvByteVal;
void SystermUART_RXWakeupConfig(UART_PORT_T PortSel)
{

	//(*(volatile unsigned long *)0x40010124) |= (0x1 << 24); //A14 RX
	//(*(volatile unsigned long *)0x400100C4) |= (0x1 << 24); //A14 RX
	if(PortSel == 0)
	 {
		GPIO_PortAModeSet(GPIOA0, 0x2); //A0 UART RX
	 }
	else if (PortSel == 1)
	{
		 GPIO_PortAModeSet(GPIOA9, 0x1);//Rx
	}


    UARTS_Init(PortSel, 115200, 8, 0, 1);

    if(PortSel == UART_PORT0)
    {

        UART0_RecvByte(&UART_RecvByteVal);

    	Power_WakeupSourceClear();
    	Power_WakeupSourceSet(SYSWAKEUP_SOURCE11_UART0_RX, 0, 0);
    	Power_WakeupEnable(SYSWAKEUP_SOURCE11_UART0_RX);
    }
    else if(PortSel == UART_PORT1)
    {

        UART1_RecvByte(&UART_RecvByteVal);

    	Power_WakeupSourceClear();
    	Power_WakeupSourceSet(SYSWAKEUP_SOURCE12_UART1_RX, 0, 0);
    	Power_WakeupEnable(SYSWAKEUP_SOURCE12_UART1_RX);
    }

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();
}

void Systerm_LVD_WakeupConfig(PWR_LVD_Threshold_SEL Lvd_Threshold_Sel)
{
	Power_LVDWakeupConfig(Lvd_Threshold_Sel);

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE8_LVD, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE8_LVD);

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();
}

void Systerm_BT_WakeupConfig(PWR_SYSWAKEUP_SOURCE_SEL SystemSource)
{
	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SystemSource, 0, 0);
	Power_WakeupEnable(SystemSource);

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();
}
void AudioADC_PowerDown(void);
 ////////////////////B1P10��DeepSleep����ʾ��///////////////////////////////////////////
 void DeepSleepPowerConsumption_Config(void)
 {

	//Power_DeepSleepLDO12ConfigTest(FALSE, 0, 0);//�˴�����LDO12�ĵ�ѹ

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

 #ifdef SW_PORTB  //������B��ζSW���ؿڣ�������A��
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
 }


static uint16_t sources  = 0;
void WakeupSourceGet()
{
    uint32_t i=0;
    uint32_t getGpio;
    uint8_t getEdge;
    PWR_SYSWAKEUP_SOURCE_SEL source = 0 ;
    uint16_t PWC_Timer3_CAP_Data;
    uint8_t UART_RecvByteVal;
    bool UART_RecvWakeupFlag = FALSE;

	for(i = 0;i<14;i++)
	{
		source = sources & (1<<i);
		if(source == FALSE)
		{
			continue;
		}

		switch(source)
		{
			case SYSWAKEUP_SOURCE6_POWERKEY:
			getEdge = Power_WakeupEdgeGet(source);
			DBG("PowerKey Wakeup--------------");
			if(getEdge)
			{
				DBG("posedge trigger \n");
			}
			else
			{
				DBG("negedge trigger \n");
			}
			break;

			case SYSWAKEUP_SOURCE7_RTC:
			DBG("RTC Wakeup\n");
			break;

			case SYSWAKEUP_SOURCE8_LVD:
			DBG("LVD Wakeup\n");
			break;

			case SYSWAKEUP_SOURCE9_IR:
			DBG("IR Cmd Wakeup\n");
			break;

			case SYSWAKEUP_SOURCE10_TIMER3_INT:
			DBG("Timer3 Interrupt Wakeup\n");
			if(PWC_IOCTRL(TIMER3,PWC_DONE_STATUS_GET,NULL))
			{
				PWC_Timer3_CAP_Data = PWC_IOCTRL(TIMER3, PWC_DATA_GET, NULL);
				DBG("\n> Get value : %d\n", (int)PWC_Timer3_CAP_Data);
			}
//    		if(SREG_TIMER3_ST.TIMER3_CAP1_DF == 1)
//    		{
//    			PWC_Timer3_CAP_Data = REG_TIMER3_CAP1;
//    			DBG("PWC_Timer3_CAP_Data: %8lx\n", PWC_Timer3_CAP_Data);
//    		}

			break;

			case SYSWAKEUP_SOURCE11_UART0_RX:
			DBG("UART0_RX Wakeup\n");
			UART_RecvWakeupFlag = UART0_RecvByte(&UART_RecvByteVal);
			if( UART_RecvWakeupFlag == TRUE )
			{
				DBG("UART_RecvByteVal : %x\n", UART_RecvByteVal);
			}
			break;

			case SYSWAKEUP_SOURCE12_UART1_RX:
			DBG("UART1_RX Wakeup\n");
			UART_RecvWakeupFlag = UART1_RecvByte(&UART_RecvByteVal);
			if( UART_RecvWakeupFlag == TRUE )
			{
				DBG("UART_RecvByteVal : %x\n", UART_RecvByteVal);
			}
			break;

			case SYSWAKEUP_SOURCE13_BT:
			DBG(" baseband Wakeup\n");
			break;


			default:
			getGpio = Power_WakeupGpioGet(source);
			getEdge = Power_WakeupEdgeGet(source);

			DBG("GPIO Wakeup :    ");
			//DBG("GPIO_SOURCE%d ;",(source - 1));
			DBG("GPIO_SOURCE%d ;",(int)i);
			if(getGpio < 32)
			{
				DBG("GPIO_A%d ;",(int)getGpio);
			}
			else if(getGpio < 41)
			{
				getGpio = getGpio - 32;
				DBG("GPIO_B%d ;",(int)getGpio);
			}
			else if(getGpio == 41)
			{
				getGpio = getGpio - 41;
				DBG("GPIO_C%d ;",(int)getGpio);
			}

			if(getEdge)
			{
				DBG("posedge trigger \n");
			}
			else
			{
				DBG("negedge trigger \n");
			}
			break;
		}
	}
}

__attribute__((section(".driver.isr")))void WakeupInterrupt(void)
{
	if(Power_WakeupSourceGet() != FALSE )
	{
		sources = Power_WakeupSourceGet();
		Power_WakeupSourceClear();
	}
}

//������Ҫ��RTC�жϺ�������������жϱ�־���������
__attribute__((section(".driver.isr")))void RtcInterrupt(void)//RTC���� ���������RTC�ж�
{
	if(RTC_IntFlagGet() == TRUE)
	{
		__udelay(32);//��ΪRTC����32KHzʱ�ӣ�����Ҫ�ȴ�һ��Cycle
		RTC_IntFlagClear();
		//���һ���б�Ҫ��RTC�ж�����������ʱ����һ��Ҫ������жϱ�־֮�����
	}
}

