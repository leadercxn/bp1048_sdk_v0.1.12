 /**
 **************************************************************************************
 * @file    mcu_circular_buf.h
 * @brief   MCU management cycle buf
 *
 * @author  Sam
 * @version V1.1.0
 *
 * $Created: 2015-11-02 15:56:11$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef		__MCU_CIRCULAR_BUF_H__
#define		__MCU_CIRCULAR_BUF_H__

/**
 * @addtogroup mv_utils
 * @{
 * @defgroup MCUCricularBuf MCUCricularBuf.h
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

typedef struct __MCU_CIRCULAR_CONTEXT__
{
    uint32_t    R;              //循环buf读指针,byte数
    uint32_t    W;              //循环buf写指针,byte数
    uint32_t    BufDepth;       //循环长度,byte数  
    int8_t*     CircularBuf;    //循环buf指针
} MCU_CIRCULAR_CONTEXT;

extern MCU_CIRCULAR_CONTEXT OTGCircularBuf;

/*
 * 软件循环fifo配置
 */
void MCUCircular_Config(MCU_CIRCULAR_CONTEXT* CircularBuf, void* Buf, uint32_t Len);

/*
 * 获取循环fifo剩余空间长度，单位Byte
 * 注意：读写指针重合，只发生在MCU 循环fifo 初始化或者播空的时候，
 * 系统往fifo中写数据需要控制水位，防止满的情况发生
 */
int32_t MCUCircular_GetSpaceLen(MCU_CIRCULAR_CONTEXT* CircularBuf);

/*
 * 往循环fifo中存放数据
 * 注意：读写指针重合，只发生在MCU 循环fifo 初始化或者播空的时候，
 * 系统往fifo中写数据需要控制水位，防止满的情况发生
 */
void MCUCircular_PutData(MCU_CIRCULAR_CONTEXT* CircularBuf, void* InBuf, uint16_t Len);

/*
 * 往循环fifo中读数据
 */
int32_t MCUCircular_GetData(MCU_CIRCULAR_CONTEXT* CircularBuf, void* OutBuf, uint16_t MaxLen);

/*
 * 获取循环fifo有效数据长度，单位Byte
 */
uint16_t MCUCircular_GetDataLen(MCU_CIRCULAR_CONTEXT* CircularBuf);

/*
 * 循环fifo中丢弃数据
 */
int32_t MCUCircular_AbortData(MCU_CIRCULAR_CONTEXT* CircularBuf, uint16_t MaxLen);


/*
 * 从fifo中读出新的数据 不改变读指针  不改变数据长度
 */
int32_t MCUCircular_ReadData(MCU_CIRCULAR_CONTEXT* CircularBuf, void* OutBuf, uint16_t MaxLen);


typedef struct __MCU_DOUBLE_CIRCULAR_CONTEXT__
{
    uint32_t    R1;              //循环buf读指针,byte数
    uint32_t    R2;              //循环buf第二读指针,byte数，R2位于R1和W之间数据位置。
    uint32_t    W;              //循环buf写指针,byte数
    uint32_t    BufDepth;       //循环长度,byte数
    int8_t*     CircularBuf;    //循环buf指针
} MCU_DOUBLE_CIRCULAR_CONTEXT;


/*
 * 软件循环fifo配置
 */
void MCUDCircular_Config(MCU_DOUBLE_CIRCULAR_CONTEXT* CircularBuf, void* Buf, uint32_t Len);

/*
 * 获取循环fifo剩余空间长度，单位Byte
 * 注意：读写指针重合，只发生在MCU 循环fifo 初始化或者播空的时候，
 * 系统往fifo中写数据需要控制水位，防止满的情况发生
 */
int32_t MCUDCircular_GetSpaceLen(MCU_DOUBLE_CIRCULAR_CONTEXT* CircularBuf);

/*
 * 往循环fifo中存放数据
 * 注意：读写指针重合，只发生在MCU 循环fifo 初始化或者播空的时候，
 * 系统往fifo中写数据需要控制水位，防止满的情况发生
 */
void MCUDCircular_PutData(MCU_DOUBLE_CIRCULAR_CONTEXT* CircularBuf, void* InBuf, uint16_t Len);

/*
 * 往循环fifo中读数据
 */
int32_t MCUDCircular_GetData1(MCU_DOUBLE_CIRCULAR_CONTEXT* CircularBuf, void* OutBuf, uint16_t MaxLen);

/*
 * 获取循环fifo有效数据长度，单位Byte
 */
uint16_t MCUDCircular_GetData1Len(MCU_DOUBLE_CIRCULAR_CONTEXT* CircularBuf);

/*
 * 往循环fifo中读数据  不改变读写指针
 */
int32_t MCUDCircular_ReadData1(MCU_DOUBLE_CIRCULAR_CONTEXT* CircularBuf, void* OutBuf, uint16_t MaxLen);

/*
 * 往循环fifo中读数据
 */
int32_t MCUDCircular_GetData2(MCU_DOUBLE_CIRCULAR_CONTEXT* CircularBuf, void* OutBuf, uint16_t MaxLen);

/*
 * 获取循环fifo有效数据长度，单位Byte
 */
uint16_t MCUDCircular_GetData2Len(MCU_DOUBLE_CIRCULAR_CONTEXT* CircularBuf);





#ifdef __cplusplus
}
#endif//__cplusplus

/**
 * @}
 * @}
 */
#endif
//
