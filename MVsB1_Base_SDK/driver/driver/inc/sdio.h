/**
 *****************************************************************************
 * @file     sdio.h
 * @author   Owen
 * @version  V1.0.0
 * @date     2017-05-15
 * @brief    declare sdio controller driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */
/**
 * @addtogroup SDIO
 * @{
 * @defgroup sdio sdio.h
 * @{
 */

#ifndef __SDIO_H__
#define __SDIO_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

#define SD_SECTOR_SIZE			512

#define NO_ERR                  (0)
#define SEND_CMD_TIME_OUT_ERR   (1)
/**
 * SDIO BUS MODE
 * In terms of operating supply voltage, two types of SD Memory Cards are defined:
 * SD Memory Cards which supports initialization/identification process with a range of 2.0-3.6v
 * and operating voltage within this range as defined in the CSD register.
 * SDLV Memory Cards - Low Voltage SD Memory Cards, that can be operate in voltage range of
 * 1.6-3.6V. The SDLV Memory Cards will be labeled differently then SD Memory Cards.
 */
#define CMD0_GO_IDLE_STATE				0   /**<reset command and sets each card into Idle State regardless of the current card state*/
#define CMD1_SEND_OP_COND				1	/**<in idle state, only CMD1,ACMD41 and CMD58 are legal host CMD,response R1*/
#define	CMD2_ALL_SEND_CID				2	/**<to each card to get its unique card identification (CID) number*/
#define	CMD3_SEND_RELATIVE_ADDR			3   /**<asks the card to publish a new relative card address*/
#define	CMD4_SET_DSR					4   /**<programs their DSP registers*/
#define	CMD7_SELECT_DESELECT_CARD		7   /**<used to select one card and put it into transfer states */
#define	CMD8_SEND_IF_COND				8	/**<get the range of the voltage of the card support*/
#define	CMD9_SEND_CSD					9
#define	CMD10_SEND_CID					10
#define	CMD12_STOP_TRANSMISSION			12
#define	CMD13_SEND_STATUS				13
#define	CMD15_GO_INACTIVE_STATE			15

/**<block oriented read commands (class 2)*/
#define	CMD16_SET_BLOCKLEN				16
#define	CMD17_READ_SINGLE_BLOCK			17
#define	CMD18_READ_MULTIPLE_BLOCK		18

/**<block oriented write commands (class 4)*/
#define	CMD24_WRITE_BLOCK				24
#define	CMD25_WRITE_MULTIPLE_BLOCK		25
#define	CMD27_PROGRAM_CSD				27

/**<block oriented write protection commands (class 6)*/
#define	CMD28_SET_WRITE_PROT			28
#define	CMD29_CLR_WRITE_PROT			29
#define	CMD30_SEND_WRITE_PROT			30

/**<erase commands (class 5)*/
#define	CMD32_ERASE_WR_BLK_START		32
#define	CMD33_ERASE_WR_BLK_END			33
#define	CMD38_ERASE						38

/**<lock card (class 7)*/
#define	CMD42_LOCK_UNLOCK				42

/**<application specific commands (class 8)*/
#define	CMD55_APP_CMD					55
#define	CMD56_GEN_CMD					56

/**<define ACMD index (ACMD6 ~ ACMD51)*/
/**<application specific commands used/reserved by SD memory card*/
#define	ACMD6_SET_BUS_WIDTH				70	//6  + 64
#define	ACMD13_SD_STATUS				77	//13 + 64
#define	ACMD22_SEND_NUM_WR_BLOCKS		86	//22 + 64
#define	ACMD23_SET_WR_BLK_ERASE_COUNT	87	//23 + 64
#define	ACMD41_SD_SEND_OP_COND			105	//41 + 64             /**<The response to ACMD41 is the operation condition register of the card*/
#define	ACMD42_SET_CLR_CARD_DETECT		106	//42 + 64
#define	ACMD51_SEND_SCR					115	//51 + 64

typedef enum
{
	SDIO_RESP_TYPE_R0 = 0,
	SDIO_RESP_TYPE_R1,
	SDIO_RESP_TYPE_R1B,
	SDIO_RESP_TYPE_R2,
	SDIO_RESP_TYPE_R3,
	SDIO_RESP_TYPE_R6,
	SDIO_RESP_TYPE_R7
}SDIO_RESP_TYPE;

typedef enum
{
	SDIO_INT_CMD = 0,
	SDIO_INT_DAT,
	SDIO_INT_MULTI_BLOCK_DONE,
}SDIO_INT_TYPE;

typedef enum
{
	SDIO_DIR_TX = 0,
	SDIO_DIR_RX,
}SDIO_DIR;

extern const uint8_t CmdRespType[128];



/**
 * @brief  发送完成后自动关闭时钟使能
 * @param  NONE
 * @return NONE
 * @note
 */
void SDIO_AutoKillTXClkEnable(void);

/**
 * @brief  发送完成后自动关闭时钟不使能
 * @param  NONE
 * @return NONE
 * @note
 */
void SDIO_AutoKillTXClkDisable(void);

/**
 * @brief  接收完成后自动关闭时钟使能
 * @param  NONE
 * @return NONE
 * @note
 */
void SDIO_AutoKillRXClkEnable(void);

/**
 * @brief  接收完成后自动关闭时钟不使能
 * @param  NONE
 * @return NONE
 * @note
 */
void SDIO_AutoKillRXClkDisable(void);

/**
 * @brief  byte模式使能
 * @param  NONE
 * @return NONE
 * @note   按字节传输周期读写缓冲
 */
void SDIO_ByteModeEnable(void);

/**
 * @brief  Word模式使能
 * @param  NONE
 * @return NONE
 * @note   按Word/4字节传输周期读写缓冲
 */
void SDIO_ByteModeDisable(void);

/**
 * @brief  连续block读写开启
 * @param  NONE
 * @return NONE
 * @note   寄存器配合
 */
void SDIO_MultiBlockEnable(void);

/**
 * @brief  连续block读写关闭
 * @param  NONE
 * @return NONE
 * @note   寄存器配合
 */
void SDIO_MultiBlockDisable(void);

/**
 * @brief  sys时钟到sdio时钟分频设置
 * @param  ClkDiv分频系数sys_clk/(ClkDiv+1)     (sys时钟为144M情况下)1:72MHz,2:48MHz
 * @return NONE
 * @note：   REG_SDIO_CLK_DIV_NUM宽度为4bit，传入值不得小于1即取值范围为1-15，上电被默认为2
 */
void SDIO_SysToSdioDivSet(uint8_t ClkDiv);

/**
 * @brief   sdio时钟分频设置
 * @param   ClkDiv分频系数    输出时钟频率为sys_clk/(REG_SDIO_CLK_DIV_NUM+1)/(2^(ClkDiv+1))
 * @return  NONE
 * @note    REG_SDIO_CLK_DIV_NUM即sys始终到SDIO时钟的分频系数（SDIO_SysToSdioDivSet设置）
 */
void SDIO_ClkSet(uint8_t ClkDiv);

/**
 * @brief  使能sdio时钟
 * @return NONE
 * @note
 */
void SDIO_ClkEnable(void);

/**
 * @brief  关闭sdio时钟
 * @return NONE
 * @note
 */
void SDIO_ClkDisable(void);

/**
 * @brief  Sdio 控制器初始化
 * @param  NONE
 * @return NONE
 * @note
 */
void SDIO_Init(void);

/**
 * @brief  SDIO发送命令
 * @param  SDIO需要发送的命令编号
 * @param  对于命令的参数
 * @return 无
 * @note
 */
void SDIO_CmdStart(uint8_t Cmd, uint32_t Param);

/**
 * @brief  检查SDIO发送命令是否完成
 * @param  无
 * @return 发送命令成功返回NO_ERR，错误返回SEND_CMD_TIME_OUT_ERR
 * @note
 */
bool SDIO_CmdIsDone(void);

/**
 * @brief  停止SDIO发送
 * @param  无
 * @return 无
 * @note
 */
void SDIO_CmdStop(void);

/**
 * @brief  获取命令的返回的相应
 * @param  buffer指针
 * @param  相应类型
 * @return NONE
 */
bool SDIO_CmdRespGet(uint8_t* RespBuf, uint8_t RespType);

/**
 * @brief  开启/关闭 命令发送完成后，Data由busy到idle，产生中断
 * @param  Enable 1：开启中断，0关闭中断
 * @return NONE
 */
void SDIO_CmdDoneCheckBusy(uint32_t Enable);



/**
 * @brief  Sdio SingleBlock模式下数据收发配置
 * @param  Direction:0 发送数据，Direction:1 接收数据
 * @param  BlockSize:单个Block长度，长度不超过4092
 * @return NONE
 * @note
 */
void SDIO_SingleBlockConfig(SDIO_DIR Direction,uint32_t BlockSize);

/**
 * @brief  Sdio MultiBlock模式下收发数据配置
 * @param  Direction:0 发送数据，Direction:1 接收数据
 * @param  BlockSize:Block的长度
 * @param  BlockCount:Block的个数
 * @return NONE
 * @note
 */
void SDIO_MultiBlockConfig(SDIO_DIR Direction,uint32_t BlockSize,uint32_t BlockCount);

/**
 * @brief  数据收发开启与关闭
 * @param  EN 0关闭数据传输  1开启数据传输
 * @return NONE
 * @note
 */
void SDIO_DataTransfer(bool EN);

/**
 * @brief  数据收发是否完成
 * @param  NONE
 * @return 1:完成，0未完成
 * @note
 */
bool SDIO_DataIsDone(void);

/**
 * @brief  检查连续block读写结束
 * @param  NONE
 * @return 1:传输完成，0：没有完成
 * @note
 */
bool SDIO_MultiBlockTransferDone(void);

/**
 * @brief  检查连续读写block的个数
 * @param  NONE
 * @return 传输完成的block个数
 * @note
 */
uint32_t SDIO_MultiBlockTransferBlocks(void);

/**
 * @brief  获取数据传输的状态
 * @param  NONE
 * @return NONE
 * @note
 */
bool SDIO_DataStartStatusGet(void);

/**
 * @brief  获取SDIO是否busy
 * @param  NONE
 * @return 0 is busy, 1 is idle
 * @note
 */
bool SDIO_IsDataLineBusy(void);

/**
 * @brief  使能SDIO中断
 * @param  中断类型
 * @return
 * @note
 */
void SDIO_InterruptEnable(SDIO_INT_TYPE IntType);

/**
 * @brief  关闭SDIO中断
 * @param  中断类型
 * @return
 * @note
 */
void SDIO_InterruptDisable(SDIO_INT_TYPE IntType);

/**
 * @brief  获取SDIO中断标志
 * @param  中断类型
 * @return 中断标志
 * @note
 */
bool SDIO_InterruptFlagGet(SDIO_INT_TYPE IntType);

/**
 * @brief  清除SDIO中断标志
 * @param  中断类型
 * @return NONE
 * @note
 */
void SDIO_InterruptFlagClear(SDIO_INT_TYPE IntType);

/**
 * @brief  清除自动停止时钟状态位
 * @param  NONE
 * @return NONE
 */
void SDIO_ClearClkHalt(void);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__SDIO_H__

/**
 * @}
 * @}
 */

