/******************************************************************************
 * @file    gpio.h
 * @author  Taowen 
 * @version V1.0.1
 * @date    2019-5-31
 * @brief   gpio register description and function declare
 * @TypicalAppication
 *   Gpio can be configured as general io pin also be reused for other pin.
 *   They are configured as general io pin using the following functions:
 *   1.Digital output:  
	--Disable input function: Clr GPIO_A_IE, GPIO_B_IE
		--Enable Output function: Set GPIO_A_OE, GPIO_B_OE
		--Strong or week output:  Set or clr GPIO_A_OUTDS, GPIO_B_OUTDS
		--Pull up or down function: Set or clr {PU,PD} 
		--strong or week pull: Set or clr GPIO_A_DS, GPIO_B_DS
		--Output High level or Low level: Set or clr GPIO_A_OUT, GPIO_B_OUT 
		
	 2.Digital input:
		--Disable output function: Clr GPIO_A_OE, GPIO_B_OE
		--Enable input function: Set GPIO_A_IE, GPIO_B_IE
		--Pull up or down function: Set or clr {PU,PD} 
		--strong or week pull: Set or clr GPIO_A_DS, GPIO_B_DS
		--Read input status
	 
	 3.Analog input:
		--Clr PU: Clr GPIO_A_PU, GPIO_B_PU
		--Set PD: Set GPIO_A_PD, GPIO_B_PD
		--Clr OE: Clr GPIO_A_OE, GPIO_B_OE
		--Clr IE: Clr GPIO_A_IE, GPIO_B_IE
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 * 
 */

/**
 * @addtogroup GPIO
 * @{
 * @defgroup gpio gpio.h
 * @{
 */
 
#ifndef __GPIO_H__
#define	__GPIO_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include "type.h"
#include "timer.h"

#define     RESTORE_TO_GENERAL_IO     			   			(0xFF)  /**<this mcaro will be used as a para value to restore the multiplex io to general io*/

/**
 * RegIndex for Bank A
 */
#define		GPIO_A_START						(0x00)

#define		GPIO_A_IN							(0x00 + GPIO_A_START)	/**<GPIOA input data register macro*/
#define		GPIO_A_OUT							(0x01 + GPIO_A_START)	/**<GPIOA output data register macro*/
#define     GPIO_A_SET							(0x02 + GPIO_A_START)
#define     GPIO_A_CLR							(0x03 + GPIO_A_START)
#define     GPIO_A_TGL							(0x04 + GPIO_A_START)
#define		GPIO_A_IE							(0x05 + GPIO_A_START)	/**<GPIOA input enable register macro.0: disable, 1: enable*/
#define		GPIO_A_OE							(0x06 + GPIO_A_START)	/**<GPIOA output enable register macro.0: disable, 1: enable*/
#define		GPIO_A_ANA_EN						(0x07 + GPIO_A_START)
#define		GPIO_A_PU							(0x08 + GPIO_A_START)	/**<GPIOA pull up register macro*/
#define		GPIO_A_PD							(0x09 + GPIO_A_START)	/**<GPIOA pull down register macro.{pu,pd} 2'b00: no pull-up, no pull-down, 2'b01: pull-down, 2'b10: pull-up, 2'b11: no pull-up, no pull-down*/
#define		GPIO_A_CORE_OUT_MASK				(0x0a + GPIO_A_START)
#define		GPIO_A_DMA_OUT_MASK					(0x0b + GPIO_A_START)
#define     GPIO_A_OUT_MASK                     GPIO_A_CORE_OUT_MASK

#define		GPIO_A_END							GPIO_A_DMA_OUT_MASK

/**
 * RegIndex for Bank B
 */


#define		GPIO_B_START						(GPIO_A_END + 1)

#define		GPIO_B_IN							(0x00 + GPIO_B_START)	/**<GPIOB input data register macro*/
#define		GPIO_B_OUT							(0x01 + GPIO_B_START)	/**<GPIOB output data register macro*/
#define     GPIO_B_SET							(0x02 + GPIO_B_START)
#define     GPIO_B_CLR							(0x03 + GPIO_B_START)
#define     GPIO_B_TGL							(0x04 + GPIO_B_START)
#define		GPIO_B_IE							(0x05 + GPIO_B_START)	/**<GPIOB input enable register macro.0: disable, 1: enable*/
#define		GPIO_B_OE							(0x06 + GPIO_B_START)	/**<GPIOB output enable register macro.0: disable, 1: enable*/
#define		GPIO_B_ANA_EN						(0x07 + GPIO_B_START)
#define		GPIO_B_PU							(0x08 + GPIO_B_START)	/**<GPIOB pull up register macro*/
#define		GPIO_B_PD							(0x09 + GPIO_B_START)	/**<GPIOB pull down register macro.{pu,pd} 2'b00: no pull-up, no pull-down, 2'b01: pull-down, 2'b10: pull-up, 2'b11: no pull-up, no pull-down*/
#define		GPIO_B_CORE_OUT_MASK				(0x0a + GPIO_B_START)
#define		GPIO_B_DMA_OUT_MASK					(0x0b + GPIO_B_START)
#define     GPIO_B_OUT_MASK                     GPIO_B_CORE_OUT_MASK

#define		GPIO_B_END							GPIO_B_DMA_OUT_MASK

/**
 * RegIndex for others
 */
#define		GPIO_GLB_INTE						(GPIO_B_END + 1)
/**
 * RegIndex for separate interrupt of GPIO Bank A
 */
#define		GPIO_A_INT			        		(GPIO_B_END + 2)

#define		GPIO_A_SEP_INTE						(0x00 + GPIO_A_INT)//  GPIOA[31:0] separate interrupt enable
#define		GPIO_A_SEP_NTRIG					(0x01 + GPIO_A_INT)
#define		GPIO_A_SEP_PTRIG					(0x02 + GPIO_A_INT)
#define		GPIO_A_SEP_INTC						(0x03 + GPIO_A_INT)
#define		GPIO_A_SEP_INTS						(0x04 + GPIO_A_INT)

#define		GPIO_A_INT_END						GPIO_A_SEP_INTS

/**
 * RegIndex for separate interrupt of GPIO Bank B
 */
#define		GPIO_B_INT					(GPIO_A_INT_END + 1)

#define		GPIO_B_SEP_INTE						(0x00 + GPIO_B_INT)	//  GPIOB[7:0] separate interrupt enable
#define		GPIO_B_SEP_NTRIG					(0x01 + GPIO_B_INT)
#define		GPIO_B_SEP_PTRIG					(0x02 + GPIO_B_INT)
#define		GPIO_B_SEP_INTC						(0x03 + GPIO_B_INT)
#define		GPIO_B_SEP_INTS						(0x04 + GPIO_B_INT)

#define		GPIO_B_INT_END						GPIO_B_SEP_INTS


#define		GPIO_A_PULLDOWN0					(0x01 + GPIO_B_INT_END)	/**<GPIOA pull down0 current register macro.0: 0mA pull-down current, 1: 1.3mA pull-down current, 2: 2.6mA pull-down current, 3: 3.9mA pull-down current*/
#define		GPIO_A_PULLDOWN1					(0x01 + GPIO_A_PULLDOWN0)/**<GPIOA pull down1 current register macro.0: 0mA pull-down current, 1: 1.3mA pull-down current, 2: 2.6mA pull-down current, 3: 3.9mA pull-down current*/

#define		GPIO_A_OUTDS					(0x01 + GPIO_A_PULLDOWN1)


#define		GPIO_B_PULLDOWN					(GPIO_A_OUTDS+1)
#define	 	GPIO_B_OUTDS					(GPIO_B_PULLDOWN+1)

#define		GPIO_C_START						(GPIO_B_OUTDS+1)
#define		GPIO_C_SEP_INTE						(0+GPIO_C_START)
#define		GPIO_C_SEP_NTRIG					(1+GPIO_C_START)
#define		GPIO_C_SEP_PTRIG					(2+GPIO_C_START)
#define		GPIO_C_SEP_INTC						(3+GPIO_C_START)
#define		GPIO_C_SEP_INTS						(4+GPIO_C_START)
#define		GPIO_C_REG_I						(5+GPIO_C_START)

#define		GPIO_FSHC_REG_START					(GPIO_C_REG_I+1)
#define		GPIO_FSHC_SCK_CTRL				(0+GPIO_FSHC_REG_START)
#define		GPIO_FSHC_CS_CTRL				(1+GPIO_FSHC_REG_START)
#define		GPIO_FSHC_MOSI_CTRL				(2+GPIO_FSHC_REG_START)
#define		GPIO_FSHC_MISO_CTRL				(3+GPIO_FSHC_REG_START)
#define		GPIO_FSHC_WP_CTRL				(4+GPIO_FSHC_REG_START)
#define		GPIO_FSHC_HOLD_CTRL				(5+GPIO_FSHC_REG_START)

#define		GPIO_GTIMER_SUB_CTRL				(1+GPIO_FSHC_HOLD_CTRL)


#define		LCD_CTRL_START					(1+GPIO_GTIMER_SUB_CTRL)

#define		LCD_ANALOG_CTRL					(0+LCD_CTRL_START)
#define		GPIO_LCD_CTRL_LOAD				(1+LCD_ANALOG_CTRL)
#define		GPIO_LCD_CTRL_REG0				(2+LCD_ANALOG_CTRL)
#define		GPIO_LCD_CTRL_REG1				(3+LCD_ANALOG_CTRL)

#define		BTDM_GPIO_DEBUG					(1+GPIO_LCD_CTRL_REG1)


#define	    GPIO_A_INT_FLAG	                 GPIO_A_SEP_INTS//GPIO_A_SEP_INTS
#define	    GPIO_B_INT_FLAG	                 GPIO_B_SEP_INTS//GPIO_B_SEP_INTS

//#define     GPIO_RESET_MISC_ADR              0x40020000
//#define     GPIO_RESET_MASK                  GPIO_RESET_MISC_ADR
//0: use sys_rst_n
//1: use por_rst_n, only reset gpio register at power on reset 


#define REG_RST_WDG_SOFT_ENABLE_ADR 0x40022008
//system software reset, 1: in reset, 0: out reset
//it will reset all system


/**
 * Define GPIO interrupt trigger type
 */
typedef enum __GPIO_TriggerType{
	GPIO_POS_EDGE_TRIGGER = 1,  /**<GPIO interrupt mode:posedge trigger, you can use it with function GPIO_INTEnable as the third para*/
	GPIO_NEG_EDGE_TRIGGER = 2,  /**<GPIO interrupt mode:negedge trigger, you can use it with function GPIO_INTEnable as the third para*/
	GPIO_EDGE_TRIGGER = 3,      /**<GPIO interrupt mode:edge trigger, you can use it with function GPIO_INTEnable as the third para*/
}GPIO_TriggerType;

/**
 * Define GPIO output Mask Type
 */
typedef enum __GPIO_OUTMaskType{
	GPIO_OUTMASK_CORE= (1<<0),
	GPIO_OUTMASK_DMA = (1<<1),

}GPIO_OUTMaskType;

typedef enum __GPIO_INDEX{
	GPIO_INDEX0 = (1<<0),                        /**<GPIO bit0 macro*/
	GPIO_INDEX1 = (1<<1),                        /**<GPIO bit1 macro*/
	GPIO_INDEX2 = (1<<2),                        /**<GPIO bit2 macro*/
	GPIO_INDEX3 = (1<<3),                        /**<GPIO bit3 macro*/
	GPIO_INDEX4 = (1<<4),                        /**<GPIO bit4 macro*/
	GPIO_INDEX5 = (1<<5),                        /**<GPIO bit5 macro*/
	GPIO_INDEX6 = (1<<6),                        /**<GPIO bit6 macro*/
	GPIO_INDEX7 = (1<<7),                        /**<GPIO bit7 macro*/
	GPIO_INDEX8 = (1<<8),                        /**<GPIO bit8 macro*/
	GPIO_INDEX9 = (1<<9),                        /**<GPIO bit9 macro*/
	GPIO_INDEX10 = (1<<10),                      /**<GPIO bit10 macro*/
	GPIO_INDEX11 = (1<<11),                      /**<GPIO bit11 macro*/
	GPIO_INDEX12 = (1<<12),                      /**<GPIO bit12 macro*/
	GPIO_INDEX13 = (1<<13),                      /**<GPIO bit13 macro*/
	GPIO_INDEX14 = (1<<14),                      /**<GPIO bit14 macro*/
	GPIO_INDEX15 = (1<<15),                      /**<GPIO bit15 macro*/
	GPIO_INDEX16 = (1<<16),                      /**<GPIO bit16 macro*/
	GPIO_INDEX17 = (1<<17),                      /**<GPIO bit17 macro*/
	GPIO_INDEX18 = (1<<18),                      /**<GPIO bit18 macro*/
	GPIO_INDEX19 = (1<<19),                      /**<GPIO bit19 macro*/
	GPIO_INDEX20 = (1<<20),                      /**<GPIO bit20 macro*/
	GPIO_INDEX21 = (1<<21),                      /**<GPIO bit21 macro*/
	GPIO_INDEX22 = (1<<22),                      /**<GPIO bit22 macro*/
	GPIO_INDEX23 = (1<<23),                      /**<GPIO bit23 macro*/
	GPIO_INDEX24 = (1<<24),                      /**<GPIO bit24 macro*/
	GPIO_INDEX25 = (1<<25),                      /**<GPIO bit25 macro*/
	GPIO_INDEX26 = (1<<26),                      /**<GPIO bit26 macro*/
	GPIO_INDEX27 = (1<<27),                      /**<GPIO bit27 macro*/
	GPIO_INDEX28 = (1<<28),                      /**<GPIO bit28 macro*/
	GPIO_INDEX29 = (1<<29),                      /**<GPIO bit29 macro*/
	GPIO_INDEX30 = (1<<30),                      /**<GPIO bit30 macro*/
	GPIO_INDEX31 = (1<<31),                      /**<GPIO bit31 macro*/
}GPIO_INDEX;

typedef enum __GPIO_TimerRemapIndex{
	GPIO_TimerRemapIndexA0 = 0,                        /**<GPIO Timer Remap IndexA0 macro*/
	GPIO_TimerRemapIndexA1 = 1,                        /**<GPIO Timer Remap IndexA1 macro*/
	GPIO_TimerRemapIndexA2 = 2,                        /**<GPIO Timer Remap IndexA2 macro*/
	GPIO_TimerRemapIndexA3 = 3,                        /**<GPIO Timer Remap IndexA3 macro*/
	GPIO_TimerRemapIndexA4 = 4,                        /**<GPIO Timer Remap IndexA4 macro*/
	GPIO_TimerRemapIndexA5 = 5,                        /**<GPIO Timer Remap IndexA5 macro*/
	GPIO_TimerRemapIndexA6 = 6,                        /**<GPIO Timer Remap IndexA6 macro*/
	GPIO_TimerRemapIndexA7 = 7,                        /**<GPIO Timer Remap IndexA7 macro*/
	GPIO_TimerRemapIndexA8 = 8,                        /**<GPIO Timer Remap IndexA8 macro*/
	GPIO_TimerRemapIndexA9 = 9,                        /**<GPIO Timer Remap IndexA9 macro*/
	GPIO_TimerRemapIndexA10 = 10,                      /**<GPIO Timer Remap IndexA10 macro*/
	GPIO_TimerRemapIndexA11 = 11,                      /**<GPIO Timer Remap IndexA11 macro*/
	GPIO_TimerRemapIndexA12 = 12,                      /**<GPIO Timer Remap IndexA12 macro*/
	GPIO_TimerRemapIndexA13 = 13,                      /**<GPIO Timer Remap IndexA13 macro*/
	GPIO_TimerRemapIndexA14 = 14,                      /**<GPIO Timer Remap IndexA14 macro*/
	GPIO_TimerRemapIndexA15 = 15,                      /**<GPIO Timer Remap IndexA15 macro*/
	GPIO_TimerRemapIndexA16 = 16,                      /**<GPIO Timer Remap IndexA16 macro*/
	GPIO_TimerRemapIndexA17 = 17,                      /**<GPIO Timer Remap IndexA17 macro*/
	GPIO_TimerRemapIndexA18 = 18,                      /**<GPIO Timer Remap IndexA18 macro*/
	GPIO_TimerRemapIndexA19 = 19,                      /**<GPIO Timer Remap IndexA19 macro*/
	GPIO_TimerRemapIndexA20 = 20,                      /**<GPIO Timer Remap IndexA20 macro*/
	GPIO_TimerRemapIndexA21 = 21,                      /**<GPIO Timer Remap IndexA21 macro*/
	GPIO_TimerRemapIndexA22 = 22,                      /**<GPIO Timer Remap IndexA22 macro*/
	GPIO_TimerRemapIndexA23 = 23,                      /**<GPIO Timer Remap IndexA23 macro*/
	GPIO_TimerRemapIndexA24 = 24,                      /**<GPIO Timer Remap IndexA24 macro*/
	GPIO_TimerRemapIndexA25 = 25,                      /**<GPIO Timer Remap IndexA25 macro*/
	GPIO_TimerRemapIndexA26 = 26,                      /**<GPIO Timer Remap IndexA26 macro*/
	GPIO_TimerRemapIndexA27 = 27,                      /**<GPIO Timer Remap IndexA27 macro*/
	GPIO_TimerRemapIndexA28 = 28,                      /**<GPIO Timer Remap IndexA28 macro*/
	GPIO_TimerRemapIndexA29 = 29,                      /**<GPIO Timer Remap IndexA29 macro*/
	GPIO_TimerRemapIndexA30 = 30,                      /**<GPIO Timer Remap IndexA30 macro*/
	GPIO_TimerRemapIndexA31 = 31,                      /**<GPIO Timer Remap IndexA31 macro*/
	GPIO_TimerRemapIndexB0 = 32,                        /**<GPIO Timer Remap IndexB0 macro*/
	GPIO_TimerRemapIndexB1 = 33,                        /**<GPIO Timer Remap IndexB1 macro*/
	GPIO_TimerRemapIndexB2 = 34,                        /**<GPIO Timer Remap IndexB2 macro*/
	GPIO_TimerRemapIndexB3 = 35,                        /**<GPIO Timer Remap IndexB3 macro*/
	GPIO_TimerRemapIndexB4 = 36,                        /**<GPIO Timer Remap IndexB4 macro*/
	GPIO_TimerRemapIndexB5 = 37,                        /**<GPIO Timer Remap IndexB5 macro*/
	GPIO_TimerRemapIndexB6 = 38,                        /**<GPIO Timer Remap IndexB6 macro*/
	GPIO_TimerRemapIndexB7 = 39,                        /**<GPIO Timer Remap IndexB7 macro*/
}GPIO_TimerRemapIndex;

typedef enum __GPIO_PortA_PULLDOWN_MODE{
	GPIO_PortA_PULLDOWN_CLOSE = 0,           /**<GPIO PortA PULLDOWN Mode:0mA(bit:A0~A31) macro*/
	GPIO_PortA_PULLDOWN_1MA3 = 1,          /**<GPIO PortA PULLDOWN Mode:1.3mA(bit:A5~A17;A24~A25;A30~A31) macro*/
	GPIO_PortA_PULLDOWN_2MA6 = 2,          /**<GPIO PortA PULLDOWN Mode:2.6mA(bit:A0~A31) macro*/
	GPIO_PortA_PULLDOWN_3MA9 = 3,          /**<GPIO PortA PULLDOWN Mode:3.9mA(bit:A5~A17;A24~A25;A30~A31) macro*/
}GPIO_PortA_PULLDOWN_MODE;

typedef enum __GPIO_PortB_PULLDOWN_MODE{
	GPIO_PortB_PULLDOWN_CLOSE = 0,           /**<GPIO PortB PULLDOWN Mode:0mA(bit:B0~B7) macro*/
	GPIO_PortB_PULLDOWN_1MA3 = 1,          /**<GPIO PortB PULLDOWN Mode:1.3mA(bit:B4~B5) macro*/
	GPIO_PortB_PULLDOWN_2MA6 = 2,          /**<GPIO PortB PULLDOWN Mode:2.6mA(bit:B4~B5) macro*/
	GPIO_PortB_PULLDOWN_3MA9 = 3,          /**<GPIO PortB PULLDOWN Mode:3.9mA(bit:B4~B5) macro*/
	GPIO_PortB_PULLDOWN_2MA4 = 4,          /**<GPIO PortB PULLDOWN Mode:2.4mA(bit:B0~B3;B6~B7) macro*/
}GPIO_PortB_PULLDOWN_MODE;

/**
 * Define GPIO output Driving Strength
 */
typedef enum __GPIO_PortA_OUTDS_MODE{
	GPIO_PortA_OUTDS_CLOSE = 0,           /**<GPIO PortA digital output driving strength mode select: Close macro*/
	GPIO_PortA_OUTDS_4MA = 1,          /**<GPIO PortA digital output driving strength mode select:4mA(bit:A0~A4;A18~A23;A26~A29) macro*/
	GPIO_PortA_OUTDS_8MA = 2,          /**<GPIO PortA digital output driving strength mode select:8mA(bit:A0~A31) macro*/
	GPIO_PortA_OUTDS_24MA = 3,          /**<GPIO PortA digital output driving strength mode select:24mA(bit:A5~A17;A24~A25;A30~A31) macro*/
}GPIO_PortA_OUTDS_MODE;

typedef enum __GPIO_PortB_OUTDS_MODE{
	GPIO_PortB_OUTDS_CLOSE = 0,           /**<GPIO PortB digital output driving strength mode select: Close macro*/
	GPIO_PortB_OUTDS_4MA = 1,          /**<GPIO PortB digital output driving strength mode select:4mA(bit:B0~B3;B6~B7) macro*/
	GPIO_PortB_OUTDS_8MA = 2,          /**<GPIO PortB digital output driving strength mode select:8mA(bit:B0~B3;B6~B7) macro*/
	GPIO_PortB_OUTDS_19MA = 3,          /**<GPIO PortB digital output driving strength mode select:19mA(bit:B4~B5) macro*/
	GPIO_PortB_OUTDS_34MA = 4,          /**<GPIO PortB digital output driving strength mode select:34mA(bit:B4~B5) macro*/
}GPIO_PortB_OUTDS_MODE;

typedef enum
{
	GPIOA0 = (1 << 0),	/** GpioMode
	                    0:gpio default register value
						1:spis_mosi_0(i)
						10:uart0_rxd_0(i)
						11:i2s0_mclk_in_0(i)
						100:btdm_debug[0](io)
						101:rf_bt_dat_i[0](io)
						110:mdac_test[2](io)
						111:sadc_clk(io)
						1000:uart0_txd_2(o)
						1001:i2s0_mclk_out_0(o)
						1010:tim3_pwm_0(o)
						1011:mcu_deepsleep(o)
					    */

	GPIOA1 = (1 << 1),	/** GpioMode
						0:gpio default register value
						1:spis_clk_0(i)
						10:uart0_rxd_2(i)
						11:i2s0_lrck_0(i)/i2s1_lrck_2(o)
						100:i2s0_lrck_0(o)/i2s1_lrck_2(i)
						101:i2s0_lrck_0(i)/i2s1_lrck_2(i)
						110:i2s0_lrck _0(io)
						111:i2s1_lrck_2(io)
						1000:btdm_debug[1](io)
						1001:rf_bt_dat_i[1](io)
						1010:mdac_test[3](io)
						1011:sadc_soc(io)
						1100:uart0_txd_0(o)
						1101:tim4_pwm_0(o)
					    */

	GPIOA2 = (1 << 2),	/** GpioMode
						 0:gpio default register value
						1:uart0_cts_0(i)
						10:i2s0_bclk_0(i)/i2s1_bclk_2(o)
						11:i2s0_bclk_0(o)/i2s1_bclk_2(i)
						100:i2s0_bclk_0(i)/i2s1_bclk_2(i)
						101:i2s0_bclk_0(io)
						110:i2s1_bclk_2(io)
						111:btdm_debug[2](io)
						1000:rf_bt_dat_i[2](io)
						1001:mdac_test[4](io)
						1010:spis_miso_0(o)
						1011:sadc_test[0](o)
					    */

	GPIOA3 = (1 << 3),	/** GpioMode
						0:gpio default register value
						1:spis_cs_0(i)
						10:i2s0_din_2(i)
						11:btdm_debug[3](io)
						100:rf_bt_dat_i[3](io)
						101:mdac_test[5](io)
						110:uart0_rts_0(o)
						111:i2s0_dout_0(o)
						1000:sadc_test[1](o)
					    */

	GPIOA4 = (1 << 4),	/** GpioMode
						0:gpio default register value
 						1:i2s0_din_0(i)
 						10:btdm_debug[4](io)
 						11:rf_bt_dat_i[4](io)
 						100:mdac_test[6](io)
 						101:i2s0_dout_2(o)
 						110:sadc_test[2](o)
					    */

	GPIOA5 = (1 << 5),	/** GpioMode
						0:gpio default register value
						1:uart0_rxd_1(i)
						10:i2c_sda_0(io)
						11:btdm_debug[5](io)
						100:rf_bt_dat_i[5](io)
						101:spim_mosi_0(o)
						110:HOSC_TEST/LCD0
					    */

	GPIOA6 = (1 << 6),	/** GpioMode
						0:gpio default register value
						1:i2c_scl_0(io)
						10:btdm_debug[6](io)
						11:rf_bt_dat_i[6](io)
						100:spim_clk_0(o)
						101:uart0_txd_1(o)
						110:LCD1
					    */

	GPIOA7 = (1 << 7),	/** GpioMode
						0:gpio default register value
						1:spim_miso_0(i)
						10:uart0_cts_1(i)
						11:i2s1_mclk_in_1(i)
						100:rf_bt_dat_i[7](io)
						101:i2s1_mclk_out_1(o)
						110:LCD2
					    */

	GPIOA8 = (1 << 8),	/** GpioMode
						0:gpio default register value
						1:i2s1_lrck_1(io)
						10:rf_bt_dat_i[8](io)
						11:uart0_rts_1(o)
						100:tim3_pwm_1(o)
						101:LCD3
					    */

	GPIOA9 = (1 << 9),	/** GpioMode
						0:gpio default register value
						1:uart1_rxd_0(i)
						10:i2s1_bclk_1(io)
						11:rf_bt_dat_i[9](io)
						100:tim4_pwm_1(o)
						101:LCD4
					    */

	GPIOA10 = (1 << 10),/** GpioMode
						0:gpio default register value
						1:i2s1_din_3(i)
						10:rf_agc_gain[0](i)
						11:uart1_txd_0(o)
						100:i2s1_dout_1(o)
						101:tim5_pwm_0(o)
						110:LCD5
						 */

	GPIOA11 = (1 << 11),/** GpioMode
						0:gpio default register value
 						1:uart1_cts_0(i)
 						10:i2s1_din_1(i)
 						11:rf_agc_gain[1](i)
 						100:i2s1_dout_3(o)
 						101:tim6_pwm_0(o)
 						110:LCD6
						 */

	GPIOA12 = (1 << 12),/** GpioMode
						0:gpio default register value
						1:rf_agc_gain[2](i)
						10:uart1_rts_0(o)
						11:LCD7
						 */

	GPIOA13 = (1 << 13),/** GpioMode
						 0:gpio default register value
						 1:rf_agc_gain[3](i)
						 10:LCD8
						 */

	GPIOA14 = (1 << 14),/** GpioMode
						 0:gpio default register value
						 1:rf_agc_gain[4](i)
						 10:LCD9
						 */

	GPIOA15 = (1 << 15),/** GpioMode
						 0:gpio default register value
						 1:sd_dat_0(io)
						 10:LCD10
						 */

	GPIOA16 = (1 << 16),/** GpioMode
						0:gpio default register value
						1:phy_vpo(i)
						10:sd_clk_0(o)
						11:LCD11
				    	*/

	GPIOA17 = (1 << 17),/** GpioMode
						0:gpio default register value
						1:sd_cmd_0(io)
						10:btdm_debug[7](io)
						11:rf_tx_rx_sel(i)
						100:phy_vmo(i)
						101:LCD12
						 */

	GPIOA18 = (1 << 18),/** GpioMode
						0:gpio default register value
						1:uart1_rxd_1(i)
						10:btdm_debug[8](io)
						11:rf_bw_sel(i)
						100:DM
					    */

	GPIOA19 = (1 << 19),/** GpioMode
						0:gpio default register value
						1:btdm_debug[9](io)
						10:rf_bt_dat_q[0](io)
						11:uart1_txd_1(o)
						100:DP
					    */

	GPIOA20 = (1 << 20),/** GpioMode
						0:gpio default register value
						1:sd_dat_1(io)
						10:spis_mosi_1(i)
						11:i2s0_lrck_1(i)/i2s1_lrck_3(o)
						100:i2s0_lrck_1(o)/i2s1_lrck_3(i)
						101:i2s0_lrck_1(i)/i2s1_lrck_3(i)
						110:i2s0_lrck_1(io)
						111:i2s1_lrck_3(io)
						1000:btdm_debug[10](io)
						1001:rf_bt_dat_q[1](io)
						1010:mdac_test[7](io)
						1011:asdm_test[0](io)
						1100:phy_oen(i)
						1101:spim_mosi_1(o)
						1110:sadc_test[3](o)
						1111:AD0
					    */

	GPIOA21 = (1 << 21),/** GpioMode
						0:gpio default register value
						1:spis_clk_1(i)
						10:i2s0_bclk_1(i)/i2s1_bclk_3(o)
						11:i2s0_bclk_1(o)/i2s1_bclk_3(i)
						100:i2s0_bclk_1(i)/i2s1_bclk_3(i)
						101:i2s0_bclk_1(io)
						110:i2s1_bclk_3(io)
						111:btdm_debug[11](io)
						1000:rf_bt_dat_q[2](io)
						1001:mdac_test[8](io)
						1010:asdm_test[1](io)
						1011:sd_clk_1(o)
						1100:spim_clk_1(o)
						1101:sadc_test[4](o)
						1110:phy_vp(o)
						1111:AD1
					    */

	GPIOA22 = (1 << 22),/** GpioMode
						0:gpio default register value
						1:sd_cmd_1(io)
						10:spim_miso_1(i)
						11:i2s0_din_3(i)
						100:btdm_debug[12](io)
						101:rf_bt_dat_q[3](io)
						110:mdac_test[9](io)
						111:asdm_test[2](io)
						1000:spis_miso_1(o)
						1001:uart1_rts_2(o)
						1010:i2s0_dout_1(o)
						1011:tim3_pwm_2(o)
						1100:sadc_test[5](o)
						1101:phy_vm(o)
						1110:AD2
						*/

	GPIOA23 = (1 << 23),/** GpioMode
						0:gpio default register value
						1:spis_cs_1(i)
						10:uart1_cts_2(i)
						11:i2s0_din_1(i)
						100:btdm_debug[13](io)
						101:rf_bt_dat_q[4](io)
						110:mdac_test[10](io)
						111:asdm_test[3](io)
						1000:i2s0_dout_3(o)
						1001:tim4_pwm_2(o)
						1010:sadc_test[6](o)
						1011:fshc_hold_test(o)
						1100:phy_rcv(o)
						1101:LCD13/AD0
						*/

	GPIOA24 = (1 << 24),/** GpioMode
						0:gpio default register value
						1:uart1_rxd_2(i)
						10:i2c_sda_1(io)
						11:i2s0_mclk_in_1(i)
						100:btdm_debug[14](io)
						101:rf_bt_dat_q[5](io)
						110:mdac_test[11](io)
						111:asdm_test[4](io)
						1000:phy_suspend(i)
						1001:i2s0_mclk_out_1(o)
						1010:tim5_pwm_1(o)
						1011:sadc_test[7](o)
						1100:fshc_miso_test(o)
						1101：LCD14/AD1
						 */

	GPIOA25 = (1 << 25),/** GpioMode
						0:gpio default register value
						1:i2c_scl_1(io)
						10:btdm_debug[15](io)
						11:rf_bt_dat_q[6](io)
						100:mdac_test[12](io)
						101:asdm_test[5](io)
						110:uart1_txd_2(o)
						111:tim6_pwm_1(o)
						1000:sadc_test[8](o)
						1001:fshc_wp_test(o)
						1010:bist_end(o)
						1011:LCD15/AD2
						 */

	GPIOA26 = (1 << 26),/** GpioMode
						0:gpio default register value
						1:btdm_debug[16](io)
						10:rf_bt_dat_q[7](io)
						11:mdac_test[13](io)
						100:asdm_test[6](io)
						101:sadc_test[9](o)
						110:fshc_clk_test(o)
						111:AD3
					    */

	GPIOA27 = (1 << 27),/** GpioMode
						 0:gpio default register value
						1:i2s1_mclk_in_0(i)
						10:btdm_debug[17](io)
						11:rf_bt_dat_q[8](io)
						100:mdac_test[14](io)
						101:asdm_test[7](io)
						110:i2s1_mclk_out_0(o)
						111:tim3_pwm_3(o)
						1000:sadc_test[10](o)
						1001:fshc_mosi_test(o)
						1010:AD4
					    */

	GPIOA28 = (1 << 28),/** GpioMode
						0:gpio default register value
						1:i2s1_lrck_0(io)
						10:spdif_di_0(i)
						11:btdm_debug[18](io)
						100:rf_bt_dat_q[9](io)
						101:mdac_test[15](io)
						110:asdm_test[8](io)
						111:spdif_do_0(o)
						1000:tim4_pwm_3(o)
						1001:sadc_test[11](o)
						1010:AD5/SPDIF_AI_0
						*/

	GPIOA29 = (1 << 29),/** GpioMode
						0:gpio default register value
						1:i2s1_bclk_0(io)
						10:spdif_di_1(i)
						11:ir_0(i)
						100:btdm_debug[19](io)
						101:rf_rcb_clk(i)
						110:mdac_test[16](io)
						111:spdif_do_1(o)
						1000:clk_out_0(o)
						1001:sadc_test[12](o)
						1010:AD6/SPDIF_AI_1
						*/

	GPIOA30 = (1 << 30),/** GpioMode
						0:gpio default register value
						1:i2c_sda_2(io)
						10:i2s1_din_2(i)
						11:spdif_di_2(i)
						100:dmic1_dat(i)
						101:sw_clk_0(i)
						110:i2s1_dout_0(o)
						111:spdif_do_2(o)
						1000:AD7/SPDIF_AI_2
						*/

	GPIOA31 = (1 << 31),/** GpioMode
						0:gpio default register value
						1:i2c_scl_2(io)
						10:i2s1_din_0(i)
						11:spdif_di_3(i)
						100:sw_d_0(io)
						101:i2s1_dout_2(o)
						110:spdif_do_3(o)
						111:dmic1_clk(o)
						1000:AD8/SPDIF_AI_3
						*/
}GPIO_PortA;


typedef enum
{
	GPIOB0 = (1 << 0),	/** GpioMode
						0:gpio default register value
						1:sw_clk_1(i)
						10:btdm_debug[20](io)
						11:rf_rcb_din(i)
						100:mdac_test[17](io)
						101:tim5_pwm_2(o)
						110:FM1R/AD9
						 */

	GPIOB1 = (1 << 1),	/** GpioMode
						0:gpio default register value
						1:sw_d_1(io)
						10:btdm_debug[21](io)
						11:mdac_test[18](io)
						100:tim6_pwm_2(o)
						101:rf_rcb_dout(o)
						110:FM1L/AD10
						 */

	GPIOB2 = (1 << 2),	/** GpioMode
						0:gpio default register value
						1:dmic0_dat(i)
						10:btdm_debug[22](io)
						11:rf_rcb_le(i)
						100:mdac_test[0](io)
						101:FM2R
						*/

	GPIOB3 = (1 << 3),	/** GpioMode
						0:gpio default register value
						1:btdm_debug[23](io)
						10:mdac_test[1](io)
						11:dmic0_clk(o)
						100:rf_clk_out(o)
						101:FM2L
						*/

	GPIOB4 = (1 << 4),	/** GpioMode
						0:gpio default register value
						1:i2c_sda_3(io)
						10:ppwm_n(o)
	 	 	 	 	 	*/

	GPIOB5 = (1 << 5),	/** GpioMode
						0:gpio default register value
						1:i2c_scl_3(io)
						10:ppwm_p(o)
	 	 	 	 	 	*/

	GPIOB6 = (1 << 6),	/** GpioMode
						0:gpio default register value
						1:ir_1(i)
						10:clk_out_1(o)
						11:FUSEVDD25
						*/

	GPIOB7 = (1 << 7),	/** GpioMode
						0:gpio default register value
						1:ir_2(i)
						10:clk_out_2(o)
						*/


}GPIO_PortB;

typedef enum _GEN_PIN_FEATURE
{
    DIGITAL_IN = 0,
    DIGITAL_OUT,

} GEN_PIN_FEATURE;

typedef enum _GPIOC0_REG
{
	GPIO_C0_REG_IE = 0,
	GPIO_C0_REG_OE = 1,
	GPIO_C0_REG_DS = 2,
	GPIO_C0_REG_PU = 3,
	GPIO_C0_REG_PD = 4,
	GPIO_C0_REG_PULLDOWN = 5,
} GPIOC0_REG;


typedef enum _GPIOC0_MODE
{
	GPIO_C0_MODE_GPIO = 0,//	4'b0000:gpio default register value
	GPIO_C0_MODE_WAKEUP_IN,//	4'b0001:wkup_in(i)
	GPIO_C0_MODE_OSC_32K,//	4'b0010:osc_32k(o)
	GPIO_C0_MODE_WAKEIP_OUT,//	4'b0011:wkup_out(o)
	GPIO_C0_MODE_PMU_DIS,//	4'b0100:board_pmu_dis(o)
	GPIO_C0_MODE_XM32,//	4'b0101:xM32k(o)
	GPIO_C0_MODE_RC32K,//	4'b0110:rc_32k(o)
	GPIO_C0_MODE_PVD,//	4'b0111:PVD(o)
	GPIO_C0_MODE_CORE_POR_N,//	4'b1000:CORE_POR_N(o)
	GPIO_C0_MODE_POWERKET,//	4'b1001:ONKEY(o)
	GPIO_C0_MODE_SEL_VIN,//	4'b1010:SEL_VIN(o)
}GPIOC0_MODE;

/**************************************************************************************************
                                 通用GPIO设置或获取状态部分
**************************************************************************************************/

//************************* 1、 只设置或读取 1 bit  *****************************//

/**
 * @brief      只置位 1bit
 *
 * @param[in]  RegIndex: GPIO地址索引，例如： GPIO_A_OUT
 * @param[in]  GpioIndex：GPIO bit位，例如：GPIO_INDEX1
 *
 * @return     无
 */
void GPIO_RegOneBitSet(uint32_t	RegIndex, GPIO_INDEX GpioIndex);

/**
 * @brief      只清零 1bit
 *
 * @param[in]  RegIndex: GPIO地址索引，例如： GPIO_A_OUT
 * @param[in]  GpioIndex：GPIO bit位，例如：GPIO_INDEX1
 *
 * @return     无
 */
void GPIO_RegOneBitClear(uint8_t RegIndex, GPIO_INDEX GpioIndex);

/**
 * @brief      读取 1bit数据
 *
 * @param[in]  RegIndex: GPIO地址索引，例如： GPIO_A_IN
 * @param[in]  GpioIndex：GPIO bit位，例如：GPIO_INDEX1
 *
 * @return     1 or 0
 */
bool GPIO_RegOneBitGet(uint8_t RegIndex, GPIO_INDEX GpioIndex);


//************************* 2、 设置或读取多 bit  ***************************** //

/**
 * @brief      置位多 bits
 *
 * @param[in]  RegIndex: GPIO地址索引，例如： GPIO_A_OUT
 * @param[in]  mask：GPIO bit位，例如：GPIO_INDEX1
 *
 * @return     无
 */
void GPIO_RegBitsSet(uint8_t RegIndex, GPIO_INDEX GpioIndex);

/**
 * @brief      清除多 bits
 *
 * @param[in]  RegIndex: GPIO地址索引，例如： GPIO_A_OUT
 * @param[in]  GpioIndex bit位，例如：GPIO_INDEX1
 *
 * @return     无
 */
void GPIO_RegBitsClear(uint8_t RegIndex, GPIO_INDEX GpioIndex);


//************************* 3、 设置或读取一组  ***************************** //

/**
 * @brief      设置一组GPIO数据
 *
 * @param[in]  RegIndex: GPIO地址索引，例如： GPIO_A_OUT
 * @param[in]  mask：GPIO bit位，例如：GPIO_INDEX1 | GPIO_INDEX2
 *
 * @return     无
 */
void GPIO_RegSet(uint8_t RegIndex, GPIO_INDEX GpioIndex);

/**
 * @brief      读取一组GPIO数据
 *
 * @param[in]  RegIndex: GPIO地址索引，例如： GPIO_A_OUT
 *
 * @return     GPIO值
 */
uint32_t GPIO_RegGet(uint8_t RegIndex);


//*************************** 4、  GPIO中断  ******************************* //

/**
 * @brief      使能GPIO某一bit的中断功能
 *
 * @param[in]  RegIndex: GPIO中断地址索引，例如： GPIO_A_INT
 * @param[in]  GpioIndex: bit位，例如GPIO_INDEX1
 * @param[in]  TrigType: 触发极性，例如GPIO_NEG_EDGE_TRIGGER触发
 *
 * @return     无
 */
void GPIO_INTEnable(uint8_t RegIndex, GPIO_INDEX GpioIndex, GPIO_TriggerType TrigType);

/**
 * @brief      禁能GPIO某一bit的中断功能
 *
 * @param[in]  RegIndex: GPIO中断地址索引，例如： GPIO_A_INT
 * @param[in]  GpioIndex: bit位，例如GPIO_INDEX1
 *
 * @return     无
 */
void GPIO_INTDisable(uint32_t RegIndex, GPIO_INDEX GpioIndex);

/**
 * @brief      获取GPIO的中断状态
 *
 * @param[in]  RegIndex: GPIO中断地址索引，例如： GPIO_A_INT_FLAG
 *
 * @return     一组GPIO的中断状态
 */
uint32_t GPIO_INTFlagGet(uint32_t RegIndex);

/**
 * @brief      清除某一位GPIO的中断状态
 *
 * @param[in]  RegIndex: GPIO中断地址索引，例如： GPIO_A_INT_FLAG
 * @param[in]  GpioIndex: GPIO bit位,例如GPIO_INDEX1
 * @return     无
 */
void GPIO_INTFlagClear(uint32_t RegIndex, GPIO_INDEX GpioIndex);

//*************************** 5、  GPIO bit mask  ******************************* //
/**
 * @brief    GPIO output mask bit,  take affect when CPU/DMA access A_REG_O/B_REG_O register
 *
 * @param[in]  RegIndex: GPIO中断地址索引，例如： GPIO_A_INT_FLAG
 * @param[in]  OutMask: GPIO_OUTMASK_CORE(take affect when CPU access A_REG_O/B_REG_O register)
 *                      GPIO_OUTMASK_DMA(take affect when DMA access A_REG_O/B_REG_O register)
 * @param[in]  GpioIndex: GPIO bit位,例如GPIO_INDEX1
 *
 * @return     无
 */
bool GPIO_OutMaskSet(uint8_t RegIndex, GPIO_OUTMaskType OutMask, GPIO_INDEX MaskBits);

/**
 * @brief    Reg GPIO output mask clear
 *
 * @param[in]  RegIndex: GPIO中断地址索引，例如： GPIO_A_INT_FLAG
 * @param[in]  OutMask: GPIO_OUTMASK_CORE(take affect when CPU access A_REG_O/B_REG_O register)
 *                      GPIO_OUTMASK_DMA(take affect when DMA access A_REG_O/B_REG_O register)
 *
 * @return     无
 */
bool GPIO_OutMaskClear(uint8_t RegIndex, GPIO_OUTMaskType OutMask);

/**************************************************************************************************
                                 GPIO与其他功能的复用部分
**************************************************************************************************/

/**
 * @brief      设置GPIOA端口复用关系
 *
 * @param[in]  port: GPIO port，例如： GPIOA10
 * @param[in]  GpioMode: GPIO 模式， 0 代表使用为普通GPIO口，其他值参考 GPIO_PortA 注释。
 *
 * @return     无
 */
void GPIO_PortAModeSet(GPIO_PortA Port, uint8_t GpioMode);

/**
 * @brief      设置GPIOB端口复用关系
 *
 * @param[in]  port: GPIO port，例如： GPIOB0
 * @param[in]  GpioMode: GPIO 模式， 0 代表使用为普通GPIO口，其他值参考 GPIO_PortB 注释。
 *
 * @return     无
 */
void GPIO_PortBModeSet(GPIO_PortB Port, uint8_t GpioMode);

/**
 * @brief      获取GPIO模式
 *
 * @param[in]  port: GPIO A port，例如： GPIOA10
 *
 * @return     GpioMode: GPIO 模式， 0 代表使用为普通GPIO口，其他值参考 GpioPort 注释。
 */
uint8_t GPIO_PortAModeGet(GPIO_PortA Port);

/**
 * @brief      获取GPIO模式
 *
 * @param[in]  port: GPIO B port，例如： GPIOB1
 *
 * @return     GpioMode: GPIO 模式， 0 代表使用为普通GPIO口，其他值参考 GpioPort 注释。
 */
uint8_t GPIO_PortBModeGet(GPIO_PortB Port);

/**
 * @brief      gpio pwc input select
 *
 * @param[in]  TimerSel: Timer port select  [Timer3~Timer6](TIMER_INDEX枚举类型)
 * @param[in]  GpioTimerRemapIndex: (GPIO_TimerRemapIndex枚举类型)
 *
 * @return     TRUE:设置成功
 *             FALSE:设置失败
 * @note       all gpio can be PWC input and should enable IE register witch bit is the selected gpio by software
 */
bool GPIO_TimerRemapConfig(TIMER_INDEX TimerSel, GPIO_TimerRemapIndex GpioTimerRemapIndex);


//*************************   GPIO下拉电流源设置  ***************************** //
/**
 * @brief      获取GPIO模式
 *
 * @param[in]  Port: GPIO A port，例如： GPIOA1
 *
 * @param[in]  PullDownModeSel: 下拉电流源的 模式：
 *                                   GPIO_PortA_PULLDOWN_Close: 0mA(bit:A0~A31);
 *                                   GPIO_PortA_PULLDOWN_1MA3: 1.3mA(bit:A5~A17;A24~A25;A30~A31);
 *                                   GPIO_PortA_PULLDOWN_2MA6: 2.6mA(bit:A0~A31);
 *                                   GPIO_PortA_PULLDOWN_3MA9: 3.9mA(bit:A5~A17;A24~A25;A30~A31)
 * @return     TRUE:设置成功
 *             FALSE:设置失败
 */
bool GPIO_PortAPulldownSet( GPIO_PortA Port,  GPIO_PortA_PULLDOWN_MODE PulldownModeSel);

/**
 * @brief      获取GPIO模式
 *
 * @param[in]  Port: GPIO B port，例如： GPIOB1
 *
 * @param[in]  PullDownModeSel: 下拉电流源的 模式：
 *                                   GPIO_PortB_PULLDOWN_Close: 0mA(bit:B0~B7);
 *                                   GPIO_PortB_PULLDOWN_2MA4: 2.4mA(bit:B0~B3;B6~B7);
 *                                   GPIO_PortA_PULLDOWN_1MA3: 1.3mA(bit:B4~B5);
 *                                   GPIO_PortA_PULLDOWN_2MA6: 2.6mA(bit:B4~B5);
 *                                   GPIO_PortA_PULLDOWN_3MA9: 3.9mA(bit:B4~B5)
 * @return     TRUE:设置成功
 *             FALSE:设置失败
 */
bool GPIO_PortBPulldownSet( GPIO_PortB Port,  GPIO_PortB_PULLDOWN_MODE PulldownModeSel);

//*************************   GPIO输出驱动能力设置  ***************************** //
/**
 * @brief      获取GPIO模式
 *
 * @param[in]  PortAGpioIndex: 例如： GPIO_INDEX1(即为GPIO_A1)
 *
 * @param[in]  PortAOutDsModeSel: 输出驱动能力的模式选择：
 *                                   GPIO_PortA_OUTDS_CLOSE : 0mA(bit:A0~A31);
 *                                   GPIO_PortA_OUTDS_4MA : 4mA(bit:A0~A4;A18~A23;A26~A29);
 *                                   GPIO_PortA_OUTDS_8MA : 8mA(bit:A0~A31);
 *                                   GPIO_PortA_OUTDS_24MA : 24mA(bit:A5~A17;A24~A25;A30~A31);
 *
 * @return     TRUE:设置成功
 *             FALSE:设置失败
 */
bool GPIO_PortAOutDsSet( GPIO_INDEX PortAGpioIndex,  GPIO_PortA_OUTDS_MODE PortAOutDsModeSel);

/**
 * @brief      获取GPIO模式
 *
 * @param[in]  PortBGpioIndex: GPIO_INDEX1(即为GPIO_B1)
 *
 * @param[in]  PullDownModeSel: 下拉电流源的 模式：
 *                                   GPIO_PortB_OUTDS_CLOSE : 0mA(bit:B0~B7);
 *                                   GPIO_PortB_OUTDS_4MA : 4mA(bit:B0~B3;B6~B7);
 *                                   GPIO_PortB_OUTDS_8MA : 8mA(bit:B0~B3;B6~B7);
 *                                   GPIO_PortB_OUTDS_19MA : 19mA(bit:B4~B5);
 *                                   GPIO_PortB_OUTDS_34MA : 34mA(bit:B4~B5);
 * @return     TRUE:设置成功
 *             FALSE:设置失败
 */
bool GPIO_PortBOutDsSet( GPIO_INDEX PortBGpioIndex,  GPIO_PortB_OUTDS_MODE PortBOutDsModeSel);

/**
 * @brief      Avoid unwanted reset source to reset gpio register and only reset gpio register at power on reset
 *
 * @param[in]  NONE
 *
 * @return     NONE
 */
void GPIO_RegisterResetMask(void);

/**
 * @brief      GPIOC0配成输入或输出模式
 *
 * @param[in]  pin_feature:  GPIOC0模式：
 * 							DIGITAL_IN  ： 输入模式
 * 							DIGITAL_OUT ： 输出模式
 * @return     NONE
 */
void GPIO_ConfigGpioC0GeneralPin(GEN_PIN_FEATURE pin_feature);

/**
 * @brief      GPIOC0输出高电平
 *
 * @param[in]  NONE
 *
 * @return     NONE
 */
void GPIO_ConfigGpioC0High(void);

/**
 * @brief	   GPIOC0输出低电平
 *
 * @param[in]  NONE
 *
 * @return     NONE
 */
void GPIO_ConfigGpioC0Low(void);

/**
 * @brief      获取GPIOC0电平
 *
 * @param[in]  NONE
 *
 * @return     1:高电平
 *             0:低电平
 */
uint32_t GPIO_GetGpioC0(void);

/**
 * @brief      将相应的寄存器置1
 *
 * @param[in]  RegIndex: GPIOC0寄存器索引
 *
 * @return     无
 */
void GPIO_RegGpioC0Set(GPIOC0_REG RegIndex);

/**
 * @brief      将相应的寄存器清0
 *
 * @param[in]  RegIndex: GPIOC0寄存器索引
 *
 * @return     无
 */
void GPIO_RegGpioC0Clear(GPIOC0_REG RegIndex);

/**
 * @brief      GPIOC0复用关系
 *
 * @param[in]  Mode: 复用关系
 *
 * @return     无
 */
void GPIO_GpioC0ModeSet(GPIOC0_MODE Mode);

#ifdef __cplusplus
}
#endif // __cplusplus 


#endif//__GPIO_H__

