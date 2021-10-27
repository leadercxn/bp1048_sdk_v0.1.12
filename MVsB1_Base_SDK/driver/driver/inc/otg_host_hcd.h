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
    uint8_t bmRequest;//D7(0:H->D;1:D->H);D6~D5(0:��׼,1:��,2:����,3:rsv)��D4~D0(0:�豸,1:�ӿ�,2:�˵�,3:����,4_31:RSV)
    uint8_t bRequest;//�������
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
 * @brief  ���õ�ַ
 * @param  ��ַ
 * @return NONE
 */
void OTG_HostAddressSet(uint8_t Address);

/**
 * @brief  �ӿ��ƶ˵����һ�ο���д�������������д����,SETUP OUTDATA INDATA����SETUP IN
 * @param  SetupPacket setup����cmd
 * @param  Buf OUT���ݻ�����ָ��
 * @param  Len OUT���ݳ���
 * @param  TimeOut ���䳬ʱֵ����λΪ����
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostControlWrite(USB_CTRL_SETUP_REQUEST SetupPacket, uint8_t *Buf, uint32_t Len, uint32_t TimeOut);

/**
 * @brief  �ӿ��ƶ˵����һ�ο��ƶ����䣬SETUP INDATA OUT
 * @param  SetupPacket setup��������
 * @param  Buf IN���ݻ�����ָ��
 * @param  Len IN���ݳ���
 * @param  TimeOut ���䳬ʱֵ����λΪ����
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostControlRead(USB_CTRL_SETUP_REQUEST SetupPacket, uint8_t *Buf, uint32_t Len, uint32_t TimeOut);

/**
 * @brief  Host��BULK���䷽ʽ����һ�����ݰ�
 * @param  Pipe pipeָ��
 * @param  Buf �������ݻ�����ָ��
 * @param  Len �������ݳ���
 * @param  TimeOut �������ݳ�ʱʱ�䣬��λΪ����
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostBulkWrite(PIPE_INFO* Pipe, uint8_t *Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  Host��BULK���䷽ʽ����һ�����ݰ�
 * @param  Pipe pipeָ��
 * @param  Buf �������ݻ�����ָ��
 * @param  Len �������ݳ���
 * @param  pTransferLen ʵ�ʽ��յ������ݳ���
 * @param  TimeOut �������ݳ�ʱʱ�䣬��λΪ����
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostBulkRead(PIPE_INFO* Pipe, uint8_t* Buf, uint32_t Len, uint32_t *pTransferLen,uint16_t TimeOut);


/**
 * @brief  Host��interrupt���䷽ʽ����һ�����ݰ�
 * @param  Pipe pipeָ��
 * @param  Buf �������ݻ�����ָ��
 * @param  Len �������ݳ���
 * @param  TimeOut �������ݳ�ʱʱ�䣬��λΪ����
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostInterruptWrite(PIPE_INFO* Pipe, uint8_t *Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  Host��interrupt���䷽ʽ����һ�����ݰ�
 * @param  Pipe pipeָ��
 * @param  Buf �������ݻ�����ָ��
 * @param  Len �������ݳ���
 * @param  pTransferLen ʵ�ʽ��յ������ݳ���
 * @param  TimeOut �������ݳ�ʱʱ�䣬��λΪ����
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostInterruptRead(PIPE_INFO* Pipe, uint8_t *Buf, uint32_t Len,uint32_t *pTransferLen, uint32_t TimeOut);



/**
 * @brief  Host��iso���䷽ʽдһ�����ݰ�
 * @param  Pipe pipeָ��
 * @param  Buf �������ݻ�����ָ��
 * @param  Len �������ݳ���
 * @param  TimeOut �������ݳ�ʱʱ�䣬��λΪ����
 * @return OTG_HOST_ERR_CODE
 */
OTG_HOST_ERR_CODE OTG_HostISOWrite(PIPE_INFO* Pipe, uint8_t *Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  Host��iso���䷽ʽ����һ�����ݰ�
 * @param  Pipe pipeָ��
 * @param  Buf �������ݻ�����ָ��
 * @param  Len �������ݳ���
  * @param *pTransferLen ��������ʵ�ʳ���
 * @param  TimeOut �������ݳ�ʱʱ�䣬��λΪ����
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
