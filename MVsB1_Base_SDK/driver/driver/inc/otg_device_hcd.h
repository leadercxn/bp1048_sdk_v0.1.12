/**
 *****************************************************************************
 * @file     otg_device_hcd.h
 * @author   Owen
 * @version  V1.0.0
 * @date     27-03-2017
 * @brief    device module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */
 
/**
 * @addtogroup OTG
 * @{
 * @defgroup otg_device_hcd otg_device_hcd.h
 * @{
 */
 
#ifndef __OTG_DEVICE_HCD_H__
#define	__OTG_DEVICE_HCD_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"


	
//MV 端点FIFO大小
/*	
EP0,	64//
EP1,	64//bulk
EP2,	128//bulk
EP3,	256//iso
*/

// Max packet size. Fixed, user should not modify.
#define	DEVICE_FS_CONTROL_MPS		64	
#define	DEVICE_FS_INT_IN_MPS		64	
#define	DEVICE_FS_INT_OUT_MPS		64	
#define	DEVICE_FS_BULK_IN_MPS		64	
#define	DEVICE_FS_BULK_OUT_MPS		64	
#define	DEVICE_FS_ISO_IN_MPS		192//only use by ep4 5	
#define	DEVICE_FS_ISO_OUT_MPS_DRV_RECEIVE		288//only use by ep4 5 bkd 


typedef enum _DEVICE_TRANSFER_TYPE
{
    TYPE_SETUP 			= 0xFF,
    TYPE_CONTROL_IN		= 0xFE,
    TYPE_CONTROL_OUT	= 0xFD,
	TYPE_ISO_IN 		= 0x81,
	TYPE_ISO_OUT 		= 0x01,
	TYPE_BULK_IN 		= 0x82,
	TYPE_BULK_OUT 		= 0x02,
	TYPE_INT_IN 		= 0x83,
	TYPE_INT_OUT 		= 0x03,
} DEVICE_TRANSFER_TYPE;




typedef enum _OTG_DEVICE_ERR_CODE
{
	DEVICE_NONE_ERR = 0,
	DEVICE_UNLINK_ERR,
	
	CONTROL_SEND_ERR,
	CONTROL_RCV_ERR,
	CONTROL_SETUP_ERR,
	
	BULK_SEND_ERR,
	BULK_RCV_ERR0,
	BULK_RCV_ERR1,
	BULK_RCV_ERR2,

	INT_SEND_ERR,
	INT_RCV_ERR0,
	INT_RCV_ERR1,
	INT_RCV_ERR2,
}OTG_DEVICE_ERR_CODE;


/**
 * @brief  Device initial
 * @param  NONE
 * @return NONE
 */
void OTG_DeviceInit(void);


/**
 * @brief  Device software connect.
 * @param  NONE
 * @return NONE
 */
void OTG_DeviceConnect(void);


/**
 * @brief  Device software disconnect.
 * @param  NONE
 * @return NONE
 */
void OTG_DeviceDisConnect(void);


/**
 * @brief  返回STALL应答
 * @param  EndpointNum 端点号
 * @return NONE
 */
void OTG_DeviceStallSend(uint8_t EndpointNum);


/**
 * @brief  复位某个端点
 * @param  EndpointNum 端点号
 * @param  EndpointType 端点类型
 * @return NONE
 */
void OTG_DeviceEndpointReset(uint8_t EndpointNum, uint8_t EndpointType);


/**
 * @brief  设置设备地址
 * @brief  总线复位后，设置设备地址为0，后续根据PC端发的SetAddress命令设置指定的设备地址
 * @param  Address 设备地址
 * @return NONE
 */
void OTG_DeviceAddressSet(uint8_t Address);


/**
 * @brief  获取总线上的事件
 * @param  NONE
 * @return 总线事件编码，USB_RESET-总线复位事件，...
 */
uint8_t OTG_DeviceBusEventGet(void);


/**
 * @brief  从控制端点发送数据
 * @param  Buf 数据缓冲区指针
 * @param  Len 数据长度
 * @param  TimeOut超时时间，ms
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceControlSend(uint8_t* Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  从控制端点接收OUT数据
 * @param  Buf 数据缓冲区指针
 * @param  MaxLen 最大数据长度
 * @param  *pTransferLen实际接收到的数据长度
 * @param  TimeOut超时时间，ms
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceControlReceive(uint8_t* Buf, uint32_t MaxLen, uint32_t *pTransferLen, uint32_t TimeOut);


/**
 * @brief  从控制端点接收SETUP数据
 * @param  Buf 数据缓冲区指针
 * @param  MaxLen 最大数据长度，一般SETUP数据包长度都是8字节
 * @param  *pTransferLen实际接收到的数据长度
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceSetupReceive(uint8_t* Buf, uint32_t MaxLen ,uint32_t *pTransferLen);


/**
 * @brief  从BULK IN端点发送数据
 * @param  EpNum 端点号
 * @param  Buf 数据缓冲区指针
 * @param  Len 数据长度
 * @param  TimeOut超时时间，ms
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceBulkSend(uint8_t EpNum, uint8_t* Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  从BULK OUT端点接收数据
 * @param  EpNum 端点号
 * @param  Buf 数据缓冲区指针
 * @param  MaxLen 最大数据长度
 * @param  *pTransferLen实际接收到的数据长度
 * @param  TimeOut超时时间，ms
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceBulkReceive(uint8_t EpNum, uint8_t* Buf, uint16_t MaxLen, uint32_t *pTransferLen ,uint32_t TimeOut);


/**
 * @brief  从ISO IN端点发送数据
 * @param  EpNum 端点号
 * @param  Buf 数据缓冲区指针
 * @param  Len 数据长度，不能大于DEVICE_FS_ISO_IN_MPS字节
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceISOSend(uint8_t EpNum, uint8_t* Buf, uint32_t Len);


/**
 * @brief  从ISO OUT端点接收数据
 * @param  EpNum 端点号
 * @param  Buf 数据缓冲区指针
 * @param  MaxLen 最大数据长度
 * @param  *pTransferLen实际接收到的数据长度
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceISOReceive(uint8_t EpNum, uint8_t* Buf, uint32_t MaxLen ,uint32_t *pTransferLen);


/**
 * @brief  从INT IN端点发送数据
 * @param  EpNum 端点号
 * @param  Buf 数据缓冲区指针
 * @param  Len 数据长度，不能大于DEVICE_FS_BULK_IN_MPS字节
 * @param  TimeOut超时时间，ms
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceInterruptSend(uint8_t EpNum, uint8_t* Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  从INT OUT端点接收数据
 * @param  EpNum 端点号
 * @param  Buf 数据缓冲区指针
 * @param  MaxLen 最大数据长度
 * @param  *pTransferLen实际接收到的数据长度
 * @param  TimeOut超时时间，ms
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceInterruptReceive(uint8_t EpNum, uint8_t* Buf, uint16_t MaxLen, uint32_t *pTransferLen ,uint32_t TimeOut);


/**
 * @brief  DEVICE模式下使能某个端点中断
 * @param  Pipe Pipe指针
 * @param  Func 中断回调函数指针
 * @return NONE
 */
void OTG_EndpointInterruptEnable(uint8_t EpNum, FPCALLBACK Func);


/**
 * @brief  DEVICE模式下禁止某个端点中断
 * @param  EpNum 端点号
 * @return NONE
 */
void OTG_EndpointInterruptDisable(uint8_t EpNum);

/**
 * @brief  OTG PowerDown
 * @param  NONE
 * @return NONE
 */
void OTG_PowerDown(void);

/**
 * @brief  OTG Pause detect & Backup reg &Powerdown
 * @param  NONE
 * @return NONE
 */
void OTG_DeepSleepBackup(void);

/**
 * @brief  OTG resume Reg/PowerOn & Detect
 * @param  NONE
 * @return NONE
 */
void OTG_WakeupResume(void);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__OTG_DEVICE_HCD_H__
/**
 * @}
 * @}
 */
