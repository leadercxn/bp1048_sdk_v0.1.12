/**
 **************************************************************************************
 * @file    i2c.h
 * @brief   I2c Module API
 *
 * @author  Jerry
 * @version V1.0.0
 *
 * $Created: 2015-03-09 13:11:35$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
/**
 * @addtogroup I2C
 * @{
 * @defgroup i2c i2c.h
 * @{
 */

#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


#define	I2C_WRITE	        0x00    //I2C write command    
#define	I2C_READ	        0x01    //I2C read command

/**
 * i2c define
 */
 
 /*
typedef enum 
{
	ENABLE					= 1,
	DISABLE					= 0
}I2C_CONFIG;


*/
/**
 * i2c mode define
 */
typedef enum 
{
	MASTER					= 1,
	SLAVE					= 0
}I2C_CONFIG_MODE;

/**
 * i2c port define
 */
typedef enum 
{
	I2C_PORT_A5_A6			= 0,
	I2C_PORT_A24_A25		= 1,
	I2C_PORT_A30_A31		= 2,
	I2C_PORT_B4_B5			= 3,
}I2C_PORT_MODE;

/**
 * i2c state define
 */
typedef enum 
{
	I2C_OK       			= 0x00,
	I2C_ERROR    			= 0x01,
	I2C_BUSY     			= 0x02,
	I2C_TIMEOUT  			= 0x03
}I2C_StatusTypeDef;

/**
 * err state define
 */
typedef enum 
{
	ERROR_OK       			= 0x00,
	ERROR_BUSY    			= 0x01,
	ERROR_TIMEOUT    		= 0x02,
	ERROR_NOACK				= 0x03,
	ERROR_UNMATCH			= 0x04,
	ERROR_SLAVEMODE			= 0x05,
	ERROR_DIV_OVERSTEP		= 0x06
}I2C_ErrorState;


/**
 * i2c���ʼ��㹫ʽ:
 * PLLģʽ��: i2c_rate = OSC-PLL/(SYS_CLK_DIV_NUM+1)/(SYS_CLK_DIV_NUM1+1)/(APB_CLK_DIV_NUM+1)/I2C_CLK_DIV
 * RCģʽ��: i2c_rate = OSC-RC/(SYS_CLK_DIV_NUM+1)/(SYS_CLK_DIV_NUM1+1)/(APB_CLK_DIV_NUM+1)/I2C_CLK_DIV
 * Ĭ������: OSC-PLL = 288M
 *           OSC-RC = 12M
 *           SYS_CLK_DIV_NUM=0
 *           SYS_CLK_DIV_NUM1=1
 *           APB_CLK_DIV_NUM = 7
 * �ο�����:
 *     (1)I2C_CLK_DIV_48(0x28) i2c_rate = 375KHz
 *     (2)I2C_CLK_DIV_80(0x2C) i2c_rate = 225KHz
 * ע: i2c���ʲ�Ҫ����400K
 * i2c ��Ƶ�Ⱥ����ò�����ϵ������:
 ******************************************************************************
 ****** Div | Param **** Div  | Param **** Div | Param **** Div  | Param ******
 ****** 30  | 0x00  **** 288  | 0x10  **** 22  | 0x20  **** 160  | 0x30  ******
 ****** 32  | 0x01  **** 320  | 0x11  **** 24  | 0x21  **** 192  | 0x31  ******
 ****** 36  | 0x02  **** 384  | 0x12  **** 26  | 0x22  **** 224  | 0x32  ******
 ****** 42  | 0x03  **** 480  | 0x13  **** 28  | 0x23  **** 256  | 0x33  ******
 ****** 48  | 0x04  **** 576  | 0x14  **** 32  | 0x24  **** 320  | 0x34  ******
 ****** 52  | 0x05  **** 640  | 0x15  **** 36  | 0x25  **** 384  | 0x35  ******
 ****** 60  | 0x06  **** 768  | 0x16  **** 40  | 0x26  **** 448  | 0x36  ******
 ****** 72  | 0x07  **** 960  | 0x17  **** 44  | 0x27  **** 512  | 0x37  ******
 ****** 80  | 0x08  **** 1152 | 0x18  **** 48  | 0x28  **** 640  | 0x38  ******
 ****** 88  | 0x09  **** 1280 | 0x19  **** 56  | 0x29  **** 768  | 0x39  ******
 ****** 104 | 0x0A  **** 1536 | 0x1A  **** 64  | 0x2A  **** 896  | 0x3A  ******
 ****** 128 | 0x0B  **** 1920 | 0x1B  **** 72  | 0x2B  **** 1024 | 0x3B  ******
 ****** 144 | 0x0C  **** 2304 | 0x1C  **** 80  | 0x2C  **** 1280 | 0x3C  ******
 ****** 160 | 0x0D  **** 2560 | 0x1D  **** 96  | 0x2D  **** 1536 | 0x3D  ******
 ****** 192 | 0x0E  **** 3072 | 0x1E  **** 112 | 0x2E  **** 1792 | 0x3E  ******
 ****** 240 | 0x0F  **** 3840 | 0x1F  **** 128 | 0x2F  **** 2048 | 0x3F  ******
 ******************************************************************************
 */

/**
 * @brief     i2c ��ʼ��
 *
 * @param[in] I2cClkParam ��Ƶ��������(��Χ0x00-0x3f)����� # i2c div table
 * @param[in] port      i2cʹ�ö˿�ѡ����� # I2C_PORT_MODE
 * @param[in] OwnAddr   i2c��slave����Ҫ���������ַ
 *                      i2c��master������������
 *
 * @return    state     ��� # I2C_ErrorState
 */
I2C_ErrorState I2C_Init(uint8_t I2cClkParam, I2C_PORT_MODE port, uint8_t OwnAddr);

/**
 * @brief   i2cʹ��
 *
 * @param	��
 *
 * @return  �� 
 */
void I2C_Enable(void);

/**
 * @brief     i2c����
 *
 * @param	��
 *
 * @return  ��  
 */
void I2C_Disable(void);

/**
 * @brief     ����i2c�ж�ʹ��
 *
 * @param[in] flag    ʹ�ܱ�־
 *
 * @return   �� 
 */
void I2C_IntEn(uint8_t flag);

/**
 * @brief     ���i2c�жϱ�־
 *
 * @param[in] ��
 *
 * @return   �� 
 */
void I2C_IntClr(void);

/**
 * @brief     ��ȡi2c�жϱ�־
 *
 * @param[in] ��
 *
 * @return   ��ȡ�жϱ�־�� 
 *			  0�����ж��źŲ���
 *            1�����ж��źŲ���
 */
bool I2C_GetIntFlag(void);

/**
 * @brief     ��ѯi2c����״̬
 *
 * @param[in] ��
 *
 * @return    I2C����״̬��
 *				0��I2C����״̬
 *            	1��I2Cæ״̬
 */
bool I2C_IsBusy(void);

/**
 * @brief     i2cͨѶ��ַ�Ƿ�ƥ��
 *
 * @param[in] ��
 *
 * @return    i2cͨѶ��ַƥ�����
 *				0�� ��ַ��ƥ��
 *	            1�� ��ַƥ��
 */
bool I2C_IsAddressMatched(void);

/**
 * @brief     i2c�ٲ��Ƿ�ʧ
 *
 * @param[in] ��
 *
 * @return    i2c�ٲ������
 *				0:�ٲ�δ��ʧ;1���ٲö�ʧ
 */
bool I2C_IsArbitrationLost(void);

/**
 * @brief     �ȴ�i2c����״̬
 *
 * @param[in] timeout    ��ʱʱ������
 *
 * @return    i2c status ��� I2C_StatusTypeDef
 */
I2C_StatusTypeDef I2C_WaitForIdle(uint32_t timeout);

/**
 * @brief     i2c����һ����ʼ�ź�
 * 
 * @param[in] ��
 *
 * @return    ��  
 */
void I2C_SendStart(void);

/**
 * @brief     i2c����һ���ظ���ʼ�ź�
 *
 * @param[in] ��
 *
 * @return    ��
 */
void I2C_SendReStart(void);

/**
 * @brief     i2c����һ��ֹͣ�ź�
 *
 * @param[in] ��
 *
 * @return    ��
 */
void I2C_SendStop(void);

/**
 * @brief     i2c����һ��ACK�ź�
 *
 * @param[in] ��
 *
 * @return    ��
 */
void I2C_SendAck(void);

/**
 * @brief     i2c����һ��NoACK�ź�
 *
 * @param[in] ��
 *
 * @return    ��
 */
void I2C_SendNoAck(void);

/**
 * @brief     i2c����ACK�ź�
 *
 * @param[in] ��
 *
 * @return    i2c����ACK�ź�
 *				0�����յ�ACK�ź�
 *            	1�����յ�NoACK�ź�
 */
bool I2C_ReceiveAcknowledge(void);

/**
 * @brief     i2c����һ�ֽ�����
 *
 * @param[in] value     ��λ: byte
 *
 * @return    ��
 */
void I2C_SendByte(uint8_t value);

/**
 * @brief     i2c����һ�ֽ�����
 *
 * @param[in] ��
 *
 * @return    value     ��λ: byte
 */
uint8_t I2C_ReceiveByte(void);

/*************************** I2C ��ģʽ ****************************/

/**
 * @brief     i2c��ģʽʹ��
 *
 * @param[in] ��
 *
 * @return    ��
 */
void I2C_MasterEnable(void);

/**
 * @brief     i2c��ģʽ����
 *
 * @param[in] ��
 *
 * @return    ��
 */
void I2C_MasterConfig(void);

/**
 * @brief     I2C��ģʽ��,��ʼ���մ��豸���͵�����
 *
 * @param[in] ��
 *
 * @return    ��
 */
void I2C_ReceiveStart(void);


/*************************** I2C ��ģʽ ****************************/

/**
 * @brief     i2c��ģʽʹ��
 *
 * @param[in] ��
 *
 * @return    ��
 */
void I2C_SlaveEnable(void);

/**
 * @brief     i2c��ģʽ����
 *
 * @param[in] ��
 *
 * @return    ��
 */
void I2C_SlaveConfig(void);

/**
 * @brief     i2c��ģʽ����������ֹͣ�¼���Ӧʱ���ȣ��˳�ë�̣�
 *
 * @param[in] value��ȡֵ��ΧΪ0-15
 *
 * @return
 */
void I2C_SlaveWidSet(uint8_t start_value,uint8_t stop_value);

/**
 * @brief     ��ȡi2c����/����״̬
 *
 * @param[in] ��
 *
 * @return    0 ��ģʽ����״̬, ������ģʽ���͵�����
 *            1 ��ģʽ����״̬, �������ݸ���ģʽ
 */
bool I2C_SlaveReadWriteGet(void);

/**
 * @brief     �ͷ�i2c
 *
 * @param[in] ��
 *
 * @return    ��
 */
void I2C_SlaveReleaseI2C(void);

/**
 * @brief     I2C��ģʽ��,�ȴ�д����
 *
 * @param[in] ��
 *
 * @return    i2c state   ��� I2C_ErrorState
 */
I2C_ErrorState I2C_SlaveRequestWrite(void);

/**
 * @brief     I2C��ģʽ��,�ȴ�������
 *
 * @param[in] ��
 *
 * @return    i2c state   ��� I2C_ErrorState
 */
I2C_ErrorState I2C_SlaveRequestRead(void);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__I2C_H__

/**
 * @}
 * @}
 */
