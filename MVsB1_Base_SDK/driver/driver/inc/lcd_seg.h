/**
 *****************************************************************************
 * @file:			lcd_seg.h
 * @author			Taowen
 * @version			V1.0.0
 * @data			14-June-2019
 * @Brief			LCD Seg driver header file
 ******************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2019 MVSilicon </center></h2>
 */
 
/**
* @addtogroup 驱动
* @{
* @defgroup LCDSeg LCDSeg
* @{
*/
 
#ifndef __LCD_SEG_H__
#define __LCD_SEG_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

typedef enum __LCD_BIAS_MODE{
	LCD_BIAS_ONE_THIRD = 1, /**<LCD reference voltage selection : 1/3 */
	LCD_BIAS_ONE_FOURTH ,   /**<LCD reference voltage selection : 1/4  */

}LCD_BIAS_MODE;

typedef enum __LCD_DRIVE_CURRENT_SEL{
	LCD_DRIVE_CURRENT_LOW = 1, /**<LCD drive current selection : low drive current */
	LCD_DRIVE_CURRENT_HIGH,    /**<LCD drive current selection : high drive current */

}LCD_DRIVE_CURRENT_SEL;

typedef enum __LCD_VOLTAGE_MODE{
	LCD_VDD33_FIFTEEN_NINETEENTH = 3,/**<LCD top voltage selection: VDD33 * 15/19 */
	LCD_VDD33_FIFTEEN_EIGHTEENTH,/**<LCD top voltage selection: VDD33 * 15/18 */
	LCD_VDD33_FIFTEEN_SEVENTEENTH,/**<LCD top voltage selection: VDD33 * 15/17 */
	LCD_VDD33_FIFTEEN_SIXTEENTH,/**<LCD top voltage selection: VDD33 * 15/16 */
	LCD_VDD33,/**<LCD top voltage selection: VDD33 * 1 (default val) */

}LCD_VOLTAGE_MODE;

typedef enum __LCD_CHANNEL{
	LCD0_A5 = (1<<5),
	LCD1_A6 = (1<<6),
	LCD2_A7 = (1<<7),
	LCD3_A8 = (1<<8),
	LCD4_A9 = (1<<9),
	LCD5_A10 = (1<<10),
	LCD6_A11 = (1<<11),
	LCD7_A12 = (1<<12),
	LCD8_A13 = (1<<13),
	LCD9_A14 = (1<<14),
	LCD10_A15 = (1<<15),
	LCD11_A16 = (1<<16),
	LCD12_A17 = (1<<17),
	LCD13_A23 = (1<<23),
	LCD14_A24 = (1<<24),
	LCD15_A25 = (1<<25),

}LCD_CHANNEL;

typedef enum __LCD_CTRL{
	LCD_DISABLE = 0,
	LCD_HIGH_Z = 1,
	LCD_BIAS0_0V = 2,
	LCD_BIAS0_ONE_THIRDS = 3,
	LCD_BIAS0_TWO_THIRDS = 4,
	LCD_BIAS0_THREE_THIRDS = 5,
	LCD_BIAS1_0V = 6,
	LCD_BIAS1_QUARTER = 7,
	LCD_BIAS1_TWO_QUARTERS = 8,
	LCD_BIAS1_THREE_QUARTERS = 9,
	LCD_BIAS1_FOUR_QUARTERS = 10,

}LCD_CTRL;

typedef struct __LCD_SEG_DATA
{
	LCD_CHANNEL SegPort;/**<11111111111 */
	LCD_CHANNEL ComPort; /**<11111111111 */
} LCD_SEG_DATA;

/**
 * @brief      LCD的模拟配置
 *
 * @param[in]  LcdBiasModeSelect	LCD_BIAS_ONE_THIRD: LCD reference voltage select 1/3;
 *                                  LCD_BIAS_ONE_FOURTH: LCD reference voltage select 1/4;
 *
 * @param[in]  LcdDriveCurrentSelect	LCD_DRIVE_CURRENT_LOW: LCD drive current selection : low drive current;
 *                                      LCD_DRIVE_CURRENT_HIGH: LCD drive current selection : high drive current.
 *
 * @param[in]  LcdVoltageSelection		LCD_VDD33_FIFTEEN_NINETEENTH: LCD top voltage selection: VDD33 * 15/19;
 *                                      LCD_VDD33_FIFTEEN_EIGHTEENTH: LCD top voltage selection: VDD33 * 15/18;
 *                                      LCD_VDD33_FIFTEEN_SEVENTEENTH: LCD top voltage selection: VDD33 * 15/17;
 *                                      LCD_VDD33_FIFTEEN_SIXTEENTH: LCD top voltage selection: VDD33 * 15/16;
 *                                      LCD_VDD33: LCD top voltage selection: VDD33 * 1 (default val).
 *
 * @return     TRUE: 设置成功
 *             FALSE: 设置失败
 */
bool LCD_AnalogConfig(LCD_BIAS_MODE LcdBiasModeSelect, LCD_DRIVE_CURRENT_SEL LcdDriveCurrentSelect, LCD_VOLTAGE_MODE LcdVoltageSelection );

/**
 * @brief      LCD的端口初始化配置
 *
 * @param[in]  SegPort	例如： LCD0_A5表示将GPIOA5复用为LCD0，且用作seg口
 *
 * @param[in]  ComPort	例如： LCD1_A6表示将GPIOA6复用为LCD1，且用作com口
 *
 * @return     TRUE: 设置成功
 *             FALSE: 设置失败
 */
bool LCD_SegInit(LCD_CHANNEL SegPort, LCD_CHANNEL ComPort);

/**
 * @brief      LCD的端口禁能
 *
 * @param[in]  SegPort	例如： LCD0_A5表示将GPIOA5复用为LCD0，且用作seg口
 *
 * @param[in]  ComPort	例如： LCD1_A6表示将GPIOA6复用为LCD1，且用作com口
 *
 * @return     TRUE: 设置成功
 *             FALSE: 设置失败
 */
bool LCD_SegDisable(LCD_CHANNEL SegPort, LCD_CHANNEL ComPort);

/**
 * @brief      LCD的端口使能
 *
 * @param[in]  SegPort	例如： LCD0_A5表示将GPIOA5复用为LCD0，且用作seg口
 *
 * @param[in]  ComPort	例如： LCD1_A6表示将GPIOA6复用为LCD1，且用作com口
 *
 * @return     TRUE: 设置成功
 *             FALSE: 设置失败
 */
bool LCD_SegEnable(LCD_CHANNEL SegPort, LCD_CHANNEL ComPort);

/**
 * @brief      LCD数据刷新
 *
 * @param[in]  SegPort	例如： LCD0_A5表示将GPIOA5复用为LCD0，且用作seg口
 *
 * @param[in]  SegCtrl	例如：LCD_BIAS0_THREE_THIRDS表示3/3 (LCD_BIAS_ONE_THIRD模式);
 *                          LCD_BIAS1_THREE_QUARTERS表示 3/4 (LCD_BIAS_ONE_FOURTH)
 * @param[in]  ComPort	例如： LCD1_A6表示将GPIOA6复用为LCD1，且用作com口
 *
 * @param[in]  ComCtrl	例如：LCD_BIAS0_THREE_THIRDS表示3/3 (LCD_BIAS_ONE_THIRD模式);
 *                          LCD_BIAS1_THREE_QUARTERS表示 3/4 (LCD_BIAS_ONE_FOURTH)
 * @return     TRUE: 设置成功
 *             FALSE: 设置失败
 */
bool LCD_SegData(LCD_CHANNEL SegPort, LCD_CTRL SegCtrl, LCD_CHANNEL ComPort, LCD_CTRL ComCtrl);




#ifdef  __cplusplus
}
#endif//__cplusplus

#endif

/**
 * @}
 * @}
 */
