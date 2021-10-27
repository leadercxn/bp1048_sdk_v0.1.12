/**
  *****************************************************************************
  * @file:			spdif.h
  * @author			Cecilia
  * @version		V1.0.0
  * @data			2015-7-20
  * @Brief			SPDIF driver interface
  ******************************************************************************
 */
/**
 * @addtogroup SPDIF
 * @{
 * @defgroup spdif spdif.h
 * @{
 */
 
#ifndef __SPDIF_H__
#define __SPDIF_H__
 
#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

typedef enum __SPDIF_ANA_CH_SEL
{
	SPDIF_ANA_INPUT_A28 = 0,

	SPDIF_ANA_INPUT_A29,

	SPDIF_ANA_INPUT_A30,

	SPDIF_ANA_INPUT_A31

} SPDIF_ANA_CH_SEL;

typedef enum __SPDIF_ANA_INPUT_LEVEL
{
	SPDIF_ANA_LEVEL_300mVpp = 0,

	SPDIF_ANA_LEVEL_200mVpp

} SPDIF_ANA_LEVEL;
    
typedef enum __SPDIF_DATA_WORDLTH
{
    SPDIF_WORDLTH_16BIT = 16,
    SPDIF_WORDLTH_20BIT = 20,
    SPDIF_WORDLTH_24BIT = 24
} SPDIF_DATA_WORDLTH;    

/**
 * @brief 20bit和24bit时，数据存放格式
 */
typedef enum _SPDIF_20BIT_24BIT_ALIGN_MODE
{
	SPDIF_HIGH_BITS_ACTIVE = 0,
	SPDIF_LOW_BITS_ACTIVE,
} SPDIF_20BIT_24BIT_ALIGN_MODE;

typedef enum __SPDIF_FLAG_STATUS
{
    PARITY_FLAG_STATUS = 21,
    UNDER_FLAG_STATUS  = 22,
    OVER_FLAG_STATUS   = 23,
    EMPTY_FLAG_STATUS  = 24,
    AEMPTY_FLAG_STATUS = 25,
    FULL_FLAG_STATUS   = 26,
    AFULL_FLAG_STATUS  = 27,
    SYNC_FLAG_STATUS   = 28,
    LOCK_FLAG_STATUS   = 29,
    BEGIN_FLAG_STATUS  = 30
    
} SPDIF_STATUS_TYPE;    

typedef enum __SPDIF_INT_TYPE
{
    PARITY_ERROR_INT   = 21,    //校验错误中断
    UNDER_ERROR_INT    = 22,    //
    OVER_ERROR_INT     = 23,    //
    EMPTY_ERROR_INT    = 24,    //FIFO空中断
    AEMPTY_ERROR_INT   = 25,    //FIFO接近空但不为空中断
    FULL_ERROR_INT     = 26,
    AFULL_ERROR_INT    = 27,
    SYNC_ERROR_INT     = 28,
    LOCK_INT           = 29,
    BEGIN_FRAME_INT    = 30,
    ALL_SPDIF_INT_DISABLE = 31     //以上中断全部关闭

} SPDIF_INT_TYPE;    

typedef enum __SPIDF_CLOCK_SOURCE_INDEX{
	SPIDF_CLK_SOURCE_PLL = 0,	/*设置SPDIF的时钟源为PLL*/
	SPIDF_CLK_SOURCE_AUPLL		/*设置SPDIF的时钟源为AUPLL*/
}SPIDF_CLOCK_SOURCE_INDEX;


/**
 * @brief  使能spdif输入前端电平转换电路
 * @param  ChannelIO： 通道号 详见：#SPDIF_ANA_CH_SEL
 * @param  Level: SPDIF_ANA_LEVEL_300mVpp: 输入最小信号压差支持300mV
 * 				  SPDIF_ANA_LEVEL_200mVpp: 输入最小信号压差支持200mV
 * @return 无
 */
void SPDIF_AnalogModuleEnable(SPDIF_ANA_CH_SEL ChannelIO, SPDIF_ANA_LEVEL Level);


/**
 * @brief  禁能spdif输入前端电平转换电路
 * @param  无
 * @return 无
 */
void SPDIF_AnalogModuleDisable(void);

/**
 * @brief  读取当前SPDIF的采样率
 * @param  无
 * @return 当前采样率
 */
uint32_t SPDIF_SampleRateGet(void);

/**
 * @brief  设置发送时的采样率
 * @param  SpdifSampleRate: 发送采样率
 * @return 无
 */
void SPDIF_SampleRateSet(uint32_t SpdifSampleRate);

/**
 * @brief  将SPDIF设置为发送模式并进行初始化
 * @param  ParitySrc：生成极性元的源选择，0 - 软件配置 1 - 硬件生成
 * @param  IsValidityEn: 有效位是否进行检测，0 - 有效位不检测，全部写入FIFO中， 
                                           1 - 只有有效位正确的写入FIFO中
 * @param  ChannelMoe: 0: 模式0 - SPDIF作为接收模块或发送模块时，均为双声道数据
                       1: 模式1 - SPDIF作为接收模块时，只把左声道的数据写入FIFO中；作为发送模块时，左声道数据正常，右声道数据为左声道拷贝数据
                       2: 模式2 - SPDIF作为接收模块时，同模式1相同；作为发送模块时，左声道数据正常，右声道数据填0
 * @param  SyncFrameDelay: 同步帧在reset之后的第SyncFrameDelay帧之后发送，如果为0，则Reset之后立即发送同步帧
 * @return 无
 */
void SPDIF_TXInit(uint8_t ParitySrc, uint8_t IsValidityEn, uint8_t ChannelMode, uint8_t SyncFrameDelay);

/**
 * @brief  将SPDIF设置为接收模式并进行初始化
 * @param  ParitySrc：生成极性元的源选择，0 - 软件配置 1 - 硬件生成
 * @param  IsValidityEn: 有效位是否进行检测，0 - 有效位不检测，全部写入FIFO中， 
                                            1 - 只有有效位正确的写入FIFO中
 * @param  ChannelMoe: 0: 模式0 - SPDIF作为接收模块或发送模块时，均为双声道数据
                       1: 模式1 - SPDIF作为接收模块时，只把左声道的数据写入FIFO中；作为发送模块时，左声道数据正常，右声道数据填0
                       2: 模式2 - SPDIF作为接收模块时，同模式1相同；作为发送模块时，左声道数据正常，右声道数据为左声道拷贝数据
 * @return 无
 */
void SPDIF_RXInit(uint8_t ParitySrc, uint8_t IsValidityEn, uint8_t ChannelMode);

/**
 * @brief  使能SPDIF模块
 * @param  无
 * @return 无
 */
void SPDIF_ModuleEnable(void);

/**
 * @brief  禁能SPDIF模块
 * @param  无
 * @return 无
 */
void SPDIF_ModuleDisable(void);

/**
 * @brief  获取SPDIF状态标志位
 * @param  FlagType： SPDIF标志类型，详见 #SPDIF_STATUS_TYPE
 * @return 标志状态
 */
bool SPDIF_FlagStatusGet(SPDIF_STATUS_TYPE FlagType);

// *******************************************************************************************************************
//                                                  以下是中断部分
// *******************************************************************************************************************
/**
 * @brief  开启或关闭SPDIF中断
 * @param  IntType: SPDIF中断类型，详见 #SPDIF_INT_TYPE
 * @param  IsEnable: 0 - 禁能该类型中断，1 - 开启该类型中断
 * @return 无
 */
void SPDIF_SetInt(SPDIF_INT_TYPE IntType, bool IsEnable);

/**
 * @brief  获取中断标志
 * @param  IntType: SPDIF中断类型，详见 #SPDIF_INT_TYPE
 * @return 中断标志
 */
bool SPDIF_GetIntFlg(SPDIF_INT_TYPE IntType);

/**
 * @brief  清除中断标志
 * @param  IntType: SPDIF中断类型，详见 #SPDIF_INT_TYPE
 * @return 无
 */
void SPDIF_ClearIntFlg(SPDIF_INT_TYPE IntType);

// *******************************************************************************************************************
//                                        以下是PCM格式与SPDIF格式之间的转换
// *******************************************************************************************************************

/**
 * @brief  PCM格式数据转SPDIF格式数据
 * @param  PcmBuf：PCM双声道数据格式，LRLR...
 * @param  Length: PCM数据的长度，单位为：Byte
 * @param  SpdifBuf: SPDIF格式数据
 * @param  Wordlth: 数据宽度，详见#SPDIF_DATA_WORDLTH
 * @return SPDIF数据长度，单位：Byte
 */
uint32_t SPDIF_PCMDataToSPDIFData(int32_t *PcmBuf, uint32_t Length, int32_t *SpdifBuf, SPDIF_DATA_WORDLTH Wordlth);

/**
 * @brief  SPDIF格式数据转PCM格式数据
 * @param  SpdifBuf：存放SPDIF数据的首地址
 * @param  Length: SPDIF数据的长度，单位为：Byte
 * @param  PcmBuf: 存放PCM数据的首地址
 * @param  Wordlth: 数据宽度，详见#SPDIF_DATA_WORDLTH
 * @return >0: PCM数据长度，单位：Byte;
 */
int32_t SPDIF_SPDIFDataToPCMData(int32_t *SpdifBuf, uint32_t Length, int32_t *PcmBuf, SPDIF_DATA_WORDLTH Wordlth);

/**
 * @brief  在20bit和24bit时，数据格式选择
 * @param  AlignMode I2S_20BIT_24BIT_ALIGN_MODE
 *
 * @return NONE
 */
void SPDIF_AlignModeSet(SPDIF_20BIT_24BIT_ALIGN_MODE AlignMode);

/**
 * @brief	设置spdif时钟源
 * @param	SourceType 0: PLL, 1: AUPLL
 */
void SPDIF_ClockSourceSelect(SPIDF_CLOCK_SOURCE_INDEX SourceType);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__SPDIF_H__

/**
 * @}
 * @}
 */
