
#ifndef __BT_PBAP_API_H_
#define __BT_PBAP_API_H_


enum {
	PBAP_CONNECT_START = 0,
	PBAP_CONNECT_OK,
	PBAP_CONNECT_ERROR,
	PBAP_CLOSE_OK,
	PBAP_CLOSE_START,
	PBAP_DATA_START,//��ʼ���ճ����ݰ�
	PBAP_DATA,//�����ݰ�
	PBAP_DATA_END,//������
	PBAP_DATA_END1,
	PBAP_NOT_ACCEPTABLE,
	PBAP_NOT_FOUND,
	PBAP_DATA_SINGLE,//��������������
	PBAP_PACKET_END,//���ݽ������
};

struct _PbapCallbackInfo {
    uint32_t	length;		// data length
    uint8_t		event;		// PBAP Event
    uint8_t		*buffer;	// pointer to data buffer
};

typedef struct _PbapCallbackInfo PbapCallbackInfo;

typedef void (*PbapCallback)(const PbapCallbackInfo *info);

/**
 * @brief
 *  	Initialize pbap
 *
 * @param 
 *		NONE
 *
 * @return
 *		TURE = SUCCESS
 *
 * @note
 *		This function must be called after BTStackRunInit and before BTStackRun
 */
//bool BTStackInitPbap(PbapCallback Callback);

/**
 * @brief
 *  	pbap connect
 *
 * @param 
 *		NONE
 *
 * @return
 *		1 = connect success
 *		0 = fail
 *
 * @note
 *		This function must be called after BTStackRunInit and before BTStackRun
 */
bool PBAPConnect(void);
//int do_pbap_open(void);
int do_pbap_open(uint8_t linkMode, uint8_t *addr);

/**
 * @brief
 *  	pbap disconnect
 *
 * @param 
 *		NONE
 *
 * @return
 *		NONE
 *
 * @note
 *		NONE
 */
void PBAPDisconnect(void);

void PBAP_PullPhoneBook(uint8_t Sel,uint8_t *buf, uint8_t head, uint8_t continueFlag);

#endif /* __BT_PBAP_API_H_ */ 

