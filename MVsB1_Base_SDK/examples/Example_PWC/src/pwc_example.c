/**
 **************************************************************************************
 * @file    PWC_example.c
 *
 * @author  Bruce
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include <nds32_intrinsic.h>
#include "type.h"
#include "irqn.h"
#include "debug.h" 
#include "timer.h"
#include "gpio.h"
#include "uarts.h"
#include "timeout.h"
#include "pwm.h"
#include "delay.h"
#include "uarts_interface.h"
#include "clk.h"
#include "pwc.h"
#include "dma.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "remap.h"
#include "chip_info.h"

static uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,		//0
	255,//PERIPHERAL_ID_SPIS_TX,			//1
	0,//PERIPHERAL_ID_TIMER3,			//2
	255,//PERIPHERAL_ID_SDIO_RX,			//3
	255,//PERIPHERAL_ID_SDIO_TX,			//4
	255,//PERIPHERAL_ID_UART0_RX,			//5
	255,//PERIPHERAL_ID_TIMER1,				//6
	255,//PERIPHERAL_ID_TIMER2,				//7
	255,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TX需使用同一通道
	255,//PERIPHERAL_ID_SDPIF_TX,			//9
	255,//PERIPHERAL_ID_SPIM_RX,			//10
	255,//PERIPHERAL_ID_SPIM_TX,			//11
	255,//PERIPHERAL_ID_UART0_TX,			//12
	255,//PERIPHERAL_ID_UART1_RX,			//13
	255,//PERIPHERAL_ID_UART1_TX,			//14
	255,//PERIPHERAL_ID_TIMER4,				//15
	255,//PERIPHERAL_ID_TIMER5,				//16
	255,//PERIPHERAL_ID_TIMER6,				//17
	255,//PERIPHERAL_ID_AUDIO_ADC0_RX,		//18
	255,//PERIPHERAL_ID_AUDIO_ADC1_RX,		//19
	255,//PERIPHERAL_ID_AUDIO_DAC0_TX,		//20
	255,//PERIPHERAL_ID_AUDIO_DAC1_TX,		//21
	255,//PERIPHERAL_ID_I2S0_RX,			//22
	255,//PERIPHERAL_ID_I2S0_TX,			//23
	255,//PERIPHERAL_ID_I2S1_RX,			//24
	255,//PERIPHERAL_ID_I2S1_TX,			//25
	255,//PERIPHERAL_ID_PPWM,				//26
	255,//PERIPHERAL_ID_ADC,     			//27
	255,//PERIPHERAL_ID_SOFTWARE,			//28
};

#define UART_TX_IO	1
#define UART_RX_IO  1

extern void SysTickInit(void);

char GetChar(void)
{
	uint8_t		ch = 0;
	while(ch == 0)
	{
		UARTS_Recv(UART_RX_IO, &ch, 1,10);
	}
	return ch;
}

static void PWC_GpioConfigInit(TIMER_INDEX TimerIdx)
{
    if(TimerIdx == TIMER3)
    {
        GPIO_PortAModeSet(GPIOA13,0);
        GPIO_RegOneBitSet(GPIO_A_IE,GPIO_INDEX13);
        PWC_GpioConfig(TIMER3,13);

        DBG("PWC INPUT: A13\n");
    }

    if(TimerIdx == TIMER4)
    {

        GPIO_RegOneBitSet(GPIO_A_IE,GPIO_INDEX13);
        PWC_GpioConfig(TIMER4,13);
        DBG("PWC INPUT: A13\n");
    }

    if(TimerIdx == TIMER5)
    {

        GPIO_RegOneBitSet(GPIO_A_IE,GPIO_INDEX13);
        PWC_GpioConfig(TIMER5,13);
        DBG("PWC INPUT: A13\n");
    }

    if(TimerIdx == TIMER6)
    {

        GPIO_RegOneBitSet(GPIO_A_IE,GPIO_INDEX13);
        PWC_GpioConfig(TIMER6,13);
        DBG("PWC INPUT: A13\n");
    }
}

static void PWM_GpioConfigInit(TIMER_INDEX TimerIdx)
{
    if(TimerIdx == TIMER3)
    {
        PWM_GpioConfig(TIMER3_PWM_A0_A8_A22_A27, 1, PWM_IO_MODE_OUT);
        DBG("PWM Init OUTPUT: A8\n");
    }

    if(TimerIdx == TIMER4)
    {
        PWM_GpioConfig(TIMER4_PWM_A1_A9_A23_A28, 0, PWM_IO_MODE_OUT);
        DBG("PWM Init OUTPUT: A1\n");
    }

    if(TimerIdx == TIMER5)
    {
        PWM_GpioConfig(TIMER5_PWM_A10_A24_B0, 0, PWM_IO_MODE_OUT);
        DBG("PWM Init OUTPUT: A10\n");
    }

    if(TimerIdx == TIMER6)
    {
        PWM_GpioConfig(TIMER6_PWM_A11_A25_B1, 0, PWM_IO_MODE_OUT);
        DBG("PWM Init OUTPUT: A11\n");
    }
}

__attribute__((section(".driver.isr"))) void Timer3Interrupt(void)
{
    uint32_t PWCValue;
    uint32_t ret;

    ret = Timer_InterruptFlagGet(TIMER3,PWC1_CAP_DATA_INTERRUPT_SRC|\
    									PWC_OVER_RUN_INTERRUPT_SRC  | UPDATE_INTERRUPT_SRC);

    if(ret & PWC1_CAP_DATA_INTERRUPT_SRC)
    {
    	PWCValue = PWC_IOCTRL(TIMER3, PWC_DATA_GET, NULL);//读比较值，自动清CAP中断
    	DBG("\n> Get value1 : %d\n", (int)PWCValue);
    }

    if(ret & PWC_OVER_RUN_INTERRUPT_SRC)//捕获定时器溢出
	{
		Timer_InterruptFlagClear(TIMER3,PWC_OVER_RUN_INTERRUPT_SRC);	  //清中断标志
		DBG("\nTimer3 PWC_OVER_RUN\n");
	}
}

void PwcInterruptMode(bool IsSingle)
{
    TIMER_INDEX TimerIdx = TIMER3;
    uint8_t     PolarIdx = PWC_POLARITY_RAISING;

    TIMER_INDEX PWMIdx;

    uint32_t    PwcTimeScale  = 12000;
    uint32_t    PWMFreqDiv = 120000000;


    PWC_StructInit  PWCParam;
    PWM_StructInit  PWMParam;

    uint8_t IrqTbl[4] = {0, 0, Timer3_IRQn, Timer4_IRQn};

    uint8_t  Buf[10];

	DBG("\n> Timer%d, PolarIdx: %d,Time Scale: %d,FreqDiv: %d\n", TimerIdx+1, PolarIdx,(int)PwcTimeScale,(int)PWMFreqDiv);
	DBG("-----------------PWC Mode:Interrupt--------------------\n");
	DBG("Examp Menu:\n");
	DBG("x: exit \n");
	DBG("s: Start\n");
	DBG("p: Pause\n");
	DBG("r: Resume\n");
	DBG("-------------------------------------------------------\n");

    //中断方式

	NVIC_DisableIRQ(Timer3_IRQn);
	NVIC_DisableIRQ(Timer4_IRQn);

	NVIC_EnableIRQ(IrqTbl[TimerIdx]);
	PWMIdx = (TimerIdx == TIMER3? TIMER4:TIMER3);

	PWC_GpioConfigInit(TimerIdx);//PWC复用GPIO配置
	PWM_GpioConfigInit(PWMIdx);//PWM复用GPIO配置

	//PWC参数配置
	PWCParam.Polarity        = PolarIdx;
	PWCParam.SingleGet       = IsSingle;//IsSingle;
	PWCParam.DMAReqEnable    = 0;
	PWCParam.TimeScale       = PwcTimeScale;//PWC的捕获理论值 = PWMFreqDiv / PwcTimeScale
	PWCParam.FilterTime      = 3;

	//PWM参数配置
	PWMParam.CounterMode       = PWM_COUNTER_MODE_UP;
	PWMParam.OutputType        = PWM_OUTPUT_SINGLE_1;
	PWMParam.DMAReqEnable      = 0;
	PWMParam.FreqDiv           = PWMFreqDiv;//1s
	PWMParam.Duty              = 50;

	PWM_Config(PWMIdx, &PWMParam);
	PWM_Enable(PWMIdx);

	PWC_Config(TimerIdx, &PWCParam);
	Timer_InterrputSrcEnable(TimerIdx, PWC1_CAP_DATA_INTERRUPT_SRC);
//	PWC_Enable(TimerIdx);

	//将PWM输出口与PWC捕获输入口联接，PWC就可捕获PWM的脉冲宽度
	while(1)
	{
		Buf[0] = 0x0;
		UARTS_Recv(UART_RX_IO, Buf, 1,100);
		if(Buf[0] == 'x')
		{
			DBG("x\n");
			PWC_Disable(TimerIdx);
			break;
		}
		if(Buf[0] == 's')
		{
			DBG("Start\n");
			PWC_Enable(TimerIdx);
		}
		if(Buf[0] == 'p')
		{
			DBG("Pause\n");
			Timer_Pause(TimerIdx, 1);
		}
		if(Buf[0] == 'r')
		{
			DBG("Resume\n");
			Timer_Pause(TimerIdx, 0);
		}
	}
}

void PwcSingleOrContinueMode(bool IsSingle)
{
    TIMER_INDEX TimerIdx = TIMER3;
    uint8_t     PolarIdx = PWC_POLARITY_RAISING;
    uint8_t     TimeScaleIdx = 2;
    uint8_t     PWMFreqIdx   = 2;
    TIMER_INDEX PWMIdx;

    uint32_t    TimeScaleTbl[]  = {1, 10, 100, 1000};
    uint32_t    PWMFreqTbl[4][3] = {
                                    {20,       800,       4000},
                                    {400,      4000,      80000},
                                    {100000,   4000000,   800000},
                                    {200000,   8000000,   50000000},
                                   };

    PWC_StructInit  PWCParam;
    PWM_StructInit  PWMParam;

    uint32_t PWCValue;
    uint8_t  Buf[10];

    //查询方式
	DBG("\n> Timer%d, PolarIdx: %d,Time Scale: %d,FreqDiv: %d\n", TimerIdx+1,PolarIdx, (int)TimeScaleTbl[TimeScaleIdx],(int)PWMFreqTbl[TimeScaleIdx][PWMFreqIdx]);
	PWMIdx = (TimerIdx == TIMER3? TIMER4:TIMER3);

	//PWC复用GPIO配置
	PWC_GpioConfigInit(TimerIdx);

	//PWM复用GPIO配置
	PWM_GpioConfigInit(PWMIdx);

	//PWC参数配置
	PWCParam.Polarity        = PolarIdx;
	PWCParam.SingleGet       = IsSingle;
	PWCParam.DMAReqEnable    = 0;
	PWCParam.TimeScale       = TimeScaleTbl[TimeScaleIdx];
	PWCParam.FilterTime      = 3;

	//PWM参数配置
	PWMParam.CounterMode        = PWM_COUNTER_MODE_UP;
	PWMParam.OutputType         = PWM_OUTPUT_SINGLE_1;
	PWMParam.DMAReqEnable       = 0;
	PWMParam.FreqDiv            = PWMFreqTbl[TimeScaleIdx][PWMFreqIdx];
	PWMParam.Duty               = 50;

	PWM_Config(PWMIdx,&PWMParam);
	PWM_Enable(PWMIdx);

	PWC_Config(TimerIdx, &PWCParam);
	PWC_Enable(TimerIdx);

	DBG("-----------------PWC Mode:Check------------------------\n");
	DBG("Examp Menu:\n");
	DBG("x: exit \n");
	DBG("s: Start\n");
	DBG("p: Pause\n");
	DBG("r: Resume\n");
	DBG("-------------------------------------------------------\n");

	//将PWM输出口与PWC捕获口相联接，PWC就可捕获PWM的脉冲宽度
	while(1)
	{
		if(PWC_IOCTRL(TimerIdx,PWC_DONE_STATUS_GET,NULL))
		{
			PWCValue = PWC_IOCTRL(TimerIdx, PWC_DATA_GET, NULL);
			DBG("\n> Get value : %d\n", (int)PWCValue);
		}
		Buf[0] = 0x0;
		UARTS_Recv(UART_RX_IO, Buf, 1,100);
		if(Buf[0] == 'x')
		{
			DBG("x\n");
			PWC_Disable(TimerIdx);
			break;
		}
		if(Buf[0] == 's')
		{
			DBG("Start\n");
			PWC_Enable(TimerIdx);
		}
		if(Buf[0] == 'p')
		{
			DBG("Pause\n");
			Timer_Pause(TimerIdx, 1);
		}
		if(Buf[0] == 'r')
		{
			DBG("Resume\n");
			Timer_Pause(TimerIdx, 0);
		}
	}
}

uint32_t pwc_dma_buf[100];
#define     ADR_TIMER3_CAP1 (0x4002C034)
void PwcDMAMode()
{
    TIMER_INDEX TimerIdx = TIMER3;
    uint8_t     PolarIdx = PWC_POLARITY_RAISING;
    uint8_t     TimeScaleIdx = 2;
    uint8_t     PWMFreqIdx   = 2;
    TIMER_INDEX PWMIdx;

    uint32_t    TimeScaleTbl[]  = {1, 10, 100, 1000};
    uint32_t    PWMFreqTbl[4][3] = {
                                    {20,       800,       4000},
                                    {400,      4000,      80000},
                                    {100000,   4000000,   800000},
                                    {200000,   8000000,   50000000},
                                   };

    PWC_StructInit  PWCParam;
    PWM_StructInit  PWMParam;
    DMA_CONFIG    DMAParam;

    //uint32_t PWCValue;
    uint8_t  Buf[10];

    //查询方式
	DBG("\n> Timer%d, PolarIdx: %d,Time Scale: %d,FreqDiv: %d\n", TimerIdx+1,PolarIdx, (int)TimeScaleTbl[TimeScaleIdx],(int)PWMFreqTbl[TimeScaleIdx][PWMFreqIdx]);
	PWMIdx = (TimerIdx == TIMER3? TIMER4:TIMER3);

	//PWC复用GPIO配置
	PWC_GpioConfigInit(TimerIdx);

	//PWM复用GPIO配置
	PWM_GpioConfigInit(PWMIdx);

	//PWC参数配置
	PWCParam.Polarity        = PolarIdx;
	PWCParam.SingleGet       = 0;
	PWCParam.DMAReqEnable    = 1;
	PWCParam.TimeScale       = TimeScaleTbl[TimeScaleIdx];
	PWCParam.FilterTime      = 3;

	//PWC DMA
	DMAParam.Dir = DMA_CHANNEL_DIR_PERI2MEM;
	DMAParam.Mode = DMA_CIRCULAR_MODE;
	DMAParam.SrcAddress = ADR_TIMER3_CAP1;
	DMAParam.SrcDataWidth = DMA_SRC_DWIDTH_WORD;
	DMAParam.SrcAddrIncremental = DMA_DST_AINCR_NO;
	DMAParam.DstAddress = (uint32_t)pwc_dma_buf;;
	DMAParam.DstDataWidth = DMA_SRC_DWIDTH_WORD;
	DMAParam.DstAddrIncremental = DMA_DST_AINCR_DST_WIDTH;
	DMAParam.BufferLen = sizeof(pwc_dma_buf);
	DMA_TimerConfig(PERIPHERAL_ID_TIMER3, &DMAParam);
	DMA_ChannelEnable(PERIPHERAL_ID_TIMER3);
    DMA_CircularWritePtrSet(PERIPHERAL_ID_TIMER4, DMAParam.BufferLen + 10);//将读指针指向buffer外

	//PWM参数配置
	PWMParam.CounterMode        = PWM_COUNTER_MODE_UP;
	PWMParam.OutputType         = PWM_OUTPUT_SINGLE_1;
	PWMParam.DMAReqEnable       = 0;
	PWMParam.FreqDiv            = PWMFreqTbl[TimeScaleIdx][PWMFreqIdx];
	PWMParam.Duty               = 50;

	PWM_Config(PWMIdx,&PWMParam);
	PWM_Enable(PWMIdx);

	PWC_Config(TimerIdx, &PWCParam);
	PWC_Enable(TimerIdx);

	DBG("-----------------PWC Mode:Check------------------------\n");
	DBG("Examp Menu:\n");
	DBG("x: exit \n");
	DBG("s: Start\n");
	DBG("p: Pause\n");
	DBG("r: Resume\n");
	DBG("-------------------------------------------------------\n");

	//将PWM输出口与PWC捕获口相联接，PWC就可捕获PWM的脉冲宽度
	while(1)
	{
		if(PWC_IOCTRL(TimerIdx,PWC_DONE_STATUS_GET,NULL))
		{
			//PWCValue = PWC_IOCTRL(TimerIdx, PWC_DATA_GET, NULL);
			//DBG("\n> Get value : %d\n", (int)PWCValue);
			DBG("\n> Get value : %ld\n", pwc_dma_buf[0]);
		}
		Buf[0] = 0x0;
		UARTS_Recv(UART_RX_IO, Buf, 1,100);
		if(Buf[0] == 'x')
		{
			DBG("x\n");
			PWC_Disable(TimerIdx);
			break;
		}
		if(Buf[0] == 's')
		{
			DBG("Start\n");
			PWC_Enable(TimerIdx);
		}
		if(Buf[0] == 'p')
		{
			DBG("Pause\n");
			Timer_Pause(TimerIdx, 1);
		}
		if(Buf[0] == 'r')
		{
			DBG("Resume\n");
			Timer_Pause(TimerIdx, 0);
		}
	}
}

int main(void)
{
    uint8_t recvBuf = 0;

    Chip_Init(1);
    WDG_Disable();
	//使用外挂24MHz晶振，锁定PLL到288MHz，此时CPU频率288MHz,系统频率144MHz
	Clock_Config(1, 24000000);
	Clock_PllLock(288000);

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_UARTClkSelect(PLL_CLK_MODE);

	//Remap_AddrRemapSet(0, 0, 0x20010000, 64);
	Remap_InitTcm(FLASH_ADDR, TCM_SIZE);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);
	Clock_UARTClkSelect(PLL_CLK_MODE);
	/********************GPIOA25---->TXD***************/
	/********************GPIOA24---->RXD***************/
	GPIO_PortAModeSet(GPIOA24, 1);//Rx, A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx, A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);

	DBG("\nBuilt time: %s %s\n",__TIME__,__DATE__);

	SysTickInit();//如果有用到timeout接口函数，需要调用SysTickInit

	GIE_ENABLE();

	DBG("-------------------------------------------------------\n");
	DBG("\t\tPWC Example\n");
	DBG("Example Menu:\n");
	DBG("1: PWC Single mode example\n");
	DBG("2: PWC Continue mode example\n");
	DBG("3: PWC Interrupt example\n");
	DBG("4: PWC DMA example\n");
	DBG("-------------------------------------------------------\n");

	while(1)
	{
		recvBuf = 0;
		recvBuf = GetChar();

		if(recvBuf == '1')
		{
			PwcSingleOrContinueMode(1);//single mode
		}
		else if(recvBuf == '2')
		{
			PwcSingleOrContinueMode(0);//Continue mode
		}
		else if(recvBuf == '3')
		{
			PwcInterruptMode(0);//
		}
		else if(recvBuf == '4')
		{
			DMA_ChannelAllocTableSet(DmaChannelMap);
			PwcDMAMode();
		}
	}
}
