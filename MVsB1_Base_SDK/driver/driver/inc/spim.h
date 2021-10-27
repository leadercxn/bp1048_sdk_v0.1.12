/**
  *****************************************************************************
  * @file:			spim.c
  * @author			Lilu
  * @version		V1.0.0
  * @data			2014-11-20
  * @Brief			SPI Master driver interface
  ******************************************************************************
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
  * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
  */

/**
 * @addtogroup SPIM
 * @{
 * @defgroup spim spim.h
 * @{
 */
 
 
#ifndef __SPIM_H__
#define __SPIM_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


//SPIM时钟=PLL_CLK/(Clock_SPIMClkDivGet())/2^(SREG_SPIM_CTRL.CRAT+1)     REG_SPIM_CLK_DIV_NUM默认等于2
/*注意：此处宏定义均建立在PLL时钟为288M情况下*/
/*如需配置SPIM模块钟为120M(接口时钟为60M)，则需要将PLL配置为240M，同时Clock_SPIMClkDivSet(1),SREG_SPIM_CTRL.CRAT=0
 */
#define	SPIM_CLK_DIV_24M	0x0			/**< SPI master clock Div 30MHz*/
#define	SPIM_CLK_DIV_12M	0x1			/**< SPI master clock Div 15MHz*/
#define	SPIM_CLK_DIV_6M 	0x2			/**< SPI master clock Div 7.5MHz*/
#define	SPIM_CLK_DIV_3M     0x3			/**< SPI master clock Div 3.75MHz*/
#define	SPIM_CLK_DIV_1M5	0x4			/**< SPI master clock Div 1.875MHz*/
#define	SPIM_CLK_DIV_750K	0x5			/**< SPI master clock Div 937.5KHz*/









typedef enum
{
	SPIM_PORT0_A5_A6_A7			= 0,
	SPIM_PORT1_A20_A21_A22		= 1,
}SPIM_PORT_MODE;




/**
 * err code define
 */
typedef enum _SPI_MASTER_ERR_CODE
{
    ERR_SPIM_TIME_OUT = -255,			/**<function execute time out*/
    ERR_SPIM_DATALEN_OUT_OF_RANGE,		/**<data len is out of range < 0*/
    SPIM_NONE_ERR = 0,
} SPI_MASTER_ERR_CODE;





/**
 * @brief
 *		初始化SPIM模块
 * @param	Mode
 *				0 - CPOL = 0 & CPHA = 0, idle为低电平,第一个边沿采样(上升沿)
 *				1 - CPOL = 0 & CPHA = 1, idle为低电平,第二个边沿采样(下降沿)
 *				2 - CPOL = 1 & CPHA = 0, idle为高电平,第一个边沿采样(下降沿)
 *				3 - CPOL = 1 & CPHA = 1, idle为高电平,第一个边沿采样(上升沿)
 * @param	ClkDiv 	分频系数:0 ~ 11
 * 				spi clock = system clock /(Clock_SPIMClkDivGet()) / (2 EXP (ClkDivExp)).
 * @return
 *		无
 * @note	注意时钟模块是否有分频，SPIM模块时钟先经过时钟分频，查看Clock_SPIMClkDivGet()返回值
 */
bool SPIM_Init(uint8_t Mode, uint8_t ClkDiv);

/**
 * @brief
 *		选择SPIM模块功能引脚
 * @param	PortSel    SPIM_PORT0_A5_A6_A7   SPIM_PORT1_A20_A21_A22
 * @return
 *		无
 * @note
 */
SPI_MASTER_ERR_CODE SPIM_IoConfig(uint8_t PortSel);

//--------------------------------MCU MODE-------------------------------------
/**
 * @brief
 *		MCU模式发送数据
 * @param	SendBuf		发送数据首地址
 * @param	Length		发送数据长度（单位：Byte）
 * @return
 *		无
 * @note
 */
void SPIM_Send(uint8_t* SendBuf, uint32_t Length);

/**
 * @brief
 *		MCU模式接收数据
 * @param	RecvBuf		接收数据首地址
 * @param	Length		接收数据长度（单位：Byte）
 * @return
 *		无
 * @note
 */
void SPIM_Recv(uint8_t* RecvBuf, uint32_t Length);



//--------------------------------DMA MODE-------------------------------------
/**
 * @brief
 *		启动DMA模式传输数据，非阻塞
 * @param	Length	数据长度（单位：Byte），最大65536 byte
 * 			TXEN    DMA发送数据开启
 * 			RXEN    DMA接收数据开启
 * @return
 *		无
 * @Note SPIM中断标志置位标志收发数据完成，可使用查询或中断方式
 */
void SPIM_DMAStart(uint32_t Length,bool TXEN,bool RXEN);



//--------------------------------DMA CTL-------------------------------------

/**
 * @brief
 *		中断使能
 * @param	无
 * @return
 *		无
 */
void SPIM_SetIntEn(bool EN);
/**
 * @brief
 *		清除中断标志
 * @param	无
 * @return
 *		无
 */
void SPIM_ClrIntFlag(void);
/**
 * @brief
 *		设置数据发送模式
 * @param	EN （默认为1）
 *      EN=0:数据低位先传输     EN=1：数据高位先传输
 * @return
 *		无
 */
void SPIM_SetMSB(bool EN);
/**
 * @brief
 *		清除所有状态标志
 * @param	无
 * @return
 *		无
 */
void SPIM_ClrAllStatus();
/**
 * @brief
 *		设置SPIM数据传输DMA方式使能
 * @param	EN：  EN=0关DMA传输      EN=1开DMA传输
 * @return
 *		无
 */
void SPIM_SetDmaEn(bool EN);

void SPIM_ClrOverwr();
//--------------------------------DMA STATUS-------------------------------------
/**
 * @brief
 *		获取中断标志
 * @param	无
 * @return
 *		返回中断标志
 */
bool SPIM_GetIntFlag(void);
/**
 * @brief
 *		获取总线当前状态
 * @param	无
 * @return
 *		返回总线当前状态
 */
bool SPIM_GetBusy();
/**
 * @brief
 *		获取MSB设置
 * @param	无
 * @return
 *		返回MSB设置位当前设置
 */
bool SPIM_GetMsb();
/**
 * @brief
 *		获取发送Fifo写溢出标志
 * @param	无
 * @return
 *		返回 0：无溢出  1：有数据溢出
 */
bool SPIM_GetTxOverwr();
/**
 * @brief
 *		获取接收FIFO写溢出标志
 * @param	无
 * @return
 *		返回 0：无溢出  1：有数据溢出
 */
bool SPIM_GetRxOverwr();
#ifdef __cplusplus
}
#endif//__cplusplus



#endif

/**
 * @}
 * @}
 */
