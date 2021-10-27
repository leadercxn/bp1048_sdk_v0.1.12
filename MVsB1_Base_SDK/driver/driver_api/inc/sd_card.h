/**
 *****************************************************************************
 * @file     sd_card.h
 * @author   owen
 * @version  V1.0.0
 * @date     2017-05-15
 * @brief    sd card module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#ifndef __CARD_H__
#define __CARD_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "sdio.h"

#define	SD_BLOCK_SIZE					512 

#define SDIO_A15_A16_A17	0
#define SDIO_A20_A21_A22	1
typedef enum _SDIO_INIT_STATE_
{
	SD_NOINIT,
	SD_CONTROLER_INIT,
	SD_IDLE,
	SD_READY,
	SD_IDENT,
	SD_STANDBY,
	SD_INITED,
} SD_INIT_STATE;


/**
 * SD CARD attribute definition
 */
typedef struct
{
	uint8_t		CardType;
	uint8_t   	MaxTransSpeed;
	SD_INIT_STATE	CardInit;
	bool		IsSDHC;
	uint32_t	BlockNum;
	uint32_t	RCA;//��16λΪRCA����ʹ��RCAʱ��16λ��Ϊ�����ַ����˴���resp��R6�ĵ�16λһ�𱣴��ˣ�ͬO18Bһ�£������ޱ�Ҫ��
} SD_CARD;

/**
 * SD CARD CID definition
 */
typedef struct _SD_CARD_ID
{
	uint8_t		MID;			/**< Manufacturer ID CID[127:120]*/
	uint8_t		OID[2];			/**< OEM/Application ID CID[119:104]*/
	uint8_t		PNM[5];			/**< Product name CID[103:64]*/
	uint8_t		PRV;			/**< Product revision CID[63:56]*/
	uint8_t		PSN[4];			/**< Product serial number CID[55:24]*/
	uint16_t	Rsv : 4;		/**< reserved CID[23:20]*/
	uint16_t	MDT : 12;		/**< Manufacturing date CID[19:8]*/
	uint8_t		CRC : 7;		/**< CRC7 checksum CID[7:1]*/
	uint8_t	NoUse : 1;		/**< not used, always 1  CID[0:0]*/
} SD_CARD_ID;

/**
 * err code definition
 */
typedef enum _SD_CARD_ERR_CODE
{
    CMD_SEND_TIME_OUT_ERR = -255,   /**<cmd send time out*/
    GET_RESPONSE_STATUS_ERR,        /**<get current transfer status err*/
    READ_SD_CARD_TIME_OUT_ERR,      /**<sd card read time out*/
    WRITE_SD_CARD_TIME_OUT_ERR,     /**<sd card write time out*/
    SD_CARD_IS_BUSY_TIME_OUT_ERR,   /**<sd card is busy time out*/
    NOCARD_LINK_ERR,                /**<sd card link err*/
    ACMD41_SEND_ERR,                /**<send ACMD41 err*/
    CMD1_SEND_ERR,                  /**<send CMD1 err*/
    CMD2_SEND_ERR,                  /**<get CID err*/
    ACMD6_SEND_ERR,                 /**<set bus width err*/
    CMD7_SEND_ERR,                  /**<select and deselect card err*/
    CMD9_SEND_ERR,                  /**<get CSD err*/
    CMD12_SEND_ERR,                 /**<stop cmd send err*/
    CMD13_SEND_ERR,                 /**<CMD13 read card status err*/
    CMD16_SEND_ERR,                 /**<set block length err*/
    CMD18_SEND_ERR,                 /**<CMD18 send err*/
    CMD25_SEND_ERR,                 /**<multi block write cmd send err*/
    CMD55_SEND_ERR,                 /**<send cmd55 err*/
    GET_SD_CARD_INFO_ERR,           /**<get sd card info err*/
    BLOCK_NUM_EXCEED_BOUNDARY,      /**<read block exceed boundary err*/
    NONE_ERR = 0,
} SD_CARD_ERR_CODE;


/**
 * @brief  sdio��IO��ʼ��
 * @param  SDIO_A15_A16_A17	, SDIO_A20_A21_A22
 * @return NONE
 * @note
 */
void CardPortInit(uint8_t SdioPort);
/**
 * @brief  sdio��������
 * @param  SDIO_A15_A16_A17	, SDIO_A20_A21_A22
 * @return NONE
 * @note
 */
void SDCardDeinit(uint8_t SdioPort);
/**
 * @brief  SD�����
 * @param  NONE
 * @return SD_CARD_ERR_CODE
 * @note
 */
SD_CARD_ERR_CODE SDCard_Detect(void);

/**
 * @brief  SD����Ϣ��ȡ
 * @param  NONE
 * @return SD����Ϣ
 * @note
 */
SD_CARD* SDCard_GetCardInfo(void);

/**
 * @brief  SD����ʼ��
 * @param  NONE
 * @return SD_CARD_ERR_CODE
 * @note
 */
SD_CARD_ERR_CODE SDCard_Init(void);

/**
 * @brief  SD��������
 * @param  Block ������
 * @param  Buffer ��������ָ��
 * @param  Size ��������
 * @return SD_CARD_ERR_CODE
 * @note
 */
SD_CARD_ERR_CODE SDCard_ReadBlock(uint32_t Block, uint8_t* Buffer,uint8_t Size);

/**
 * @brief  SD��д����
 * @param  Block ������
 * @param  Buffer д������ָ��
 * @param  Size ��������
 * @return SD_CARD_ERR_CODE
 * @note
 */
SD_CARD_ERR_CODE SDCard_WriteBlock(uint32_t Block, uint8_t* Buffer,uint8_t Size);


/**
 * @brief  SD��������ȡ
 * @param  NONE
 * @return SD������
 * @note
 */
uint32_t SDCard_CapacityGet(void);


/**
 * @brief  ��ʼ��SDIO������
 * @param  NONE
 * @return NONE
 * @note   ���ｫ��дģʽĬ�ϳ�ʼ��Ϊ������д��ʽ
 */
void SDCard_ControllerInit(void);

/**
 * @brief  SDIO��������
 * @param  SDIO��Ҫ���͵�������
 * @param  ��������Ĳ���
 * @return ��������ɹ�����NO_ERR�����󷵻�SEND_CMD_TIME_OUT_ERR
 * @note
 */
bool SDIO_CmdSend(uint8_t Cmd, uint32_t Param, uint16_t TimeOut);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__CARD_H__
