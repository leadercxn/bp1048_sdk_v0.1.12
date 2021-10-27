/**
 *****************************************************************************
 * @file     otg_host_hcd.h
 * @author   owen
 * @version  V1.0.0
 * @date     27-03-2017
 * @brief    otg host hardware driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2015 MVSilicon </center></h2>
 */

/**
 * @addtogroup OTG
 * @{
 * @defgroup otg_host_hcd otg_host_hcd.h
 * @{
 */
 
#ifndef __OTG_HOST_HCD_H__
#define	__OTG_HOST_HCD_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
	
#include "type.h"
	
#define	HOST_CONTROL_EP			0
#define	HOST_BULK_IN_EP			1
#define	HOST_BULK_OUT_EP		2
#define	HOST_INT_IN_EP			3
#define	HOST_ISO_OUT_EP			4
#define	HOST_ISO_IN_EP			5

#define	HOST_FS_CONTROL_MPS		64
#define	HOST_FS_INT_IN_MPS		64
#define	HOST_FS_BULK_IN_MPS		64
#define	HOST_FS_BULK_OUT_MPS	64
#define	HOST_FS_ISO_OUT_MPS		192
#define	HOST_FS_ISO_IN_MPS		768

#define ENDPOINT_TYPE_CONTROL	0X00
#define ENDPOINT_TYPE_ISO		0X01
#define ENDPOINT_TYPE_BULK		0X02
#define ENDPOINT_TYPE_INT		0X03	


typedef enum _OTG_HOST_ERR_CODE
{
	HOST_NONE_ERR = 0,
	HOST_UNLINK_ERR,
	
	CONTROL_WRITE_SETUP_ERR,
	CONTROL_WRITE_OUT_ERR,
	CONTROL_WRITE_IN_ERR,
	
	CONTROL_READ_SETUP_ERR,
	CONTROL_READ_OUT_ERR,
	CONTROL_READ_IN_ERR,
	
	BULK_READ_ERR,
	BULK_WRITE_ERR,

	INT_READ_ERR,
	INT_WRITE_ERR,

	ISO_READ_ERR,
	ISO_WRITE_ERR,
}OTG_HOST_ERR_CODE;

	
typedef struct _USB_CTRL_SETUP_REQUEST{
    uint8_t bmRequest;//D7(0:H->D;1:D->H);D6~D5(0:标准,1:类,2:厂商,3:rsv)；D4~D0(0:设备,1:接口,2:端点,3:其他,4_31:RSV)
    uint8_t bRequest;//请求代码
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
}USB_CTRL_SETUP_REQUEST, *PUSB_CTRL_SETUP_REQUEST;


typedef struct _PIPE_INFO
{
	uint8_t	EpNum;//endpoint number
	uint8_t	EpType;//endpoint type
	uint16_t MaxPacketSize;	//max packet size
} PIPE_INFO;

/**
 * @brief  usb host init
 * @param  NONE
 * @return NONE
 */
void OTG_HostControlInit(void);

/**
 * @brief  设置地址
 * @param  地址
 * @return NONE
 */
void OTG_HostAddressSet(uint8_t Address);

/**
 * @brief  从控制端点完成一次控制写传输或者无数据写传输,SETUP OUTDATA INDATA或者SETUP IN
 * @param  SetupPacket setup请求cmd
 * @param  Buf OUT数据缓冲区指针
 * @param  Len OUT数据长度
 * @param  TimeOut 传输超时值，单位为毫秒
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostControlWrite(USB_CTRL_SETUP_REQUEST SetupPacket, uint8_t *Buf, uint32_t Len, uint32_t TimeOut);

/**
 * @brief  从控制端点完成一次控制读传输，SETUP INDATA OUT
 * @param  SetupPacket setup请求数据
 * @param  Buf IN数据缓冲区指针
 * @param  Len IN数据长度
 * @param  TimeOut 传输超时值，单位为毫秒
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostControlRead(USB_CTRL_SETUP_REQUEST SetupPacket, uint8_t *Buf, uint32_t Len, uint32_t TimeOut);

/**
 * @brief  Host以BULK传输方式接收一个数据包
 * @param  Pipe pipe指针
 * @param  Buf 接收数据缓冲区指针
 * @param  Len 接收数据长度
 * @param  TimeOut 接收数据超时时间，单位为毫秒
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostBulkWrite(PIPE_INFO* Pipe, uint8_t *Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  Host以BULK传输方式发送一个数据包
 * @param  Pipe pipe指针
 * @param  Buf 发送数据缓冲区指针
 * @param  Len 发送数据长度
 * @param  pTransferLen 实际接收到的数据长度
 * @param  TimeOut 接收数据超时时间，单位为毫秒
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostBulkRead(PIPE_INFO* Pipe, uint8_t* Buf, uint32_t Len, uint32_t *pTransferLen,uint16_t TimeOut);


/**
 * @brief  Host以interrupt传输方式发送一个数据包
 * @param  Pipe pipe指针
 * @param  Buf 发送数据缓冲区指针
 * @param  Len 发送数据长度
 * @param  TimeOut 接收数据超时时间，单位为毫秒
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostInterruptWrite(PIPE_INFO* Pipe, uint8_t *Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  Host以interrupt传输方式接收一个数据包
 * @param  Pipe pipe指针
 * @param  Buf 发送数据缓冲区指针
 * @param  Len 发送数据长度
 * @param  pTransferLen 实际接收到的数据长度
 * @param  TimeOut 接收数据超时时间，单位为毫秒
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostInterruptRead(PIPE_INFO* Pipe, uint8_t *Buf, uint32_t Len,uint32_t *pTransferLen, uint32_t TimeOut);



/**
 * @brief  Host以iso传输方式写一个数据包
 * @param  Pipe pipe指针
 * @param  Buf 发送数据缓冲区指针
 * @param  Len 发送数据长度
 * @param  TimeOut 接收数据超时时间，单位为毫秒
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostISOWrite(PIPE_INFO* Pipe, uint8_t *Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  Host以iso传输方式接收一个数据包
 * @param  Pipe pipe指针
 * @param  Buf 发送数据缓冲区指针
 * @param  Len 发送数据长度
  * @param *pTransferLen 发送数据实际长度
 * @param  TimeOut 接收数据超时时间，单位为毫秒
 * @return OTG_HOST_ERR_CODE
 */
	
OTG_HOST_ERR_CODE OTG_HostISORead(PIPE_INFO* Pipe, uint8_t *Buf, uint32_t Len, uint32_t *pTransferLen, uint32_t TimeOut);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__OTG_HOST_HCD_H__
/**
 * @}
 * @}
 */
