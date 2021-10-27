
#ifndef __BT_PBAP_API_H_
#define __BT_PBAP_API_H_

/**
 * PBAP phone book : select patch
 */
enum {
	PHONE = 0,
	SIM1,
	SIM2,
};

/**
 * PBAP releate event
 */
typedef enum{
	BT_STACK_EVENT_PBAP_NONE = 0,
	BT_STACK_EVENT_PBAP_DISCONNECTING,
	BT_STACK_EVENT_PBAP_DISCONNECT,
	BT_STACK_EVENT_PBAP_CONNECT_ERROR,
	BT_STACK_EVENT_PBAP_CONNECTING,
	BT_STACK_EVENT_PBAP_CONNECTED,
	BT_STACK_EVENT_PBAP_DATA_START,//��ʼ���ճ����ݰ�
	BT_STACK_EVENT_PBAP_DATA,//�����ݰ�
	BT_STACK_EVENT_PBAP_DATA_END,//������
	BT_STACK_EVENT_PBAP_DATA_END1,
	BT_STACK_EVENT_PBAP_NOT_ACCEPTABLE,
	BT_STACK_EVENT_PBAP_NOT_FOUND,
	BT_STACK_EVENT_PBAP_DATA_SINGLE,//��������������
	BT_STACK_EVENT_PBAP_PACKET_END,//���ݽ������
}BT_PBAP_CALLBACK_EVENT;

/*
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
*/
typedef struct _BT_PBAP_CALLBACK_PARAMS {
    uint32_t	length;		// data length
    uint8_t		*buffer;	// pointer to data buffer
}BT_PBAP_CALLBACK_PARAMS;


typedef void (*BTPbapCallbackFunc)(BT_PBAP_CALLBACK_EVENT event, BT_PBAP_CALLBACK_PARAMS * param);

void BtPbapCallback(BT_PBAP_CALLBACK_EVENT event, BT_PBAP_CALLBACK_PARAMS * param);

/**
 * @brief
 *  	pbap connect
 *
 * @param 
 *		addr - the remote address
 *
 * @return
 *		1 = connect success
 *		0 = fail
 *
 * @note
 *		This function must be called after BTStackRunInit and before BTStackRun
 */
bool PBAPConnect(uint8_t* addr);


/**
 * @brief
 *  	pbap disconnect
 *
 * @param 
 *		NONE
 *
 * @return
 *		1 = command send success
 *		0 = fail
 *
 * @note
 *		NONE
 */
bool PBAPDisconnect(void);

/**
 * @brief
 *  	pbap pull phone book
 *
 * @param 
 *		sel: patch(phone,sim1,sim2)
 *		buf: pointer to type info 
 *
 * @return
 *		NONE
 *
 * @note
 *		NONE
 */
void PBAP_PullPhoneBook(uint8_t Sel,uint8_t *buf);

bool PbapAppInit(BTPbapCallbackFunc callback);

//��ȡ��1�绰����Ϣ 
void GetSim1CardPhoneBook(void);

//��ȡ��2�绰����Ϣ 
void GetSim2CardPhoneBook(void);

//��ȡ�ֻ�����绰����Ϣ 
void GetMobilePhoneBook(void);

//��ȡ����绰��Ϣ 
void GetIncomingCallBook(void);

//��ȡ�����绰����Ϣ 
void GetOutgoingCallBook(void);

//��ȡδ�ӵ绰����Ϣ 
void GetMissedCallBook(void);

void GetCombinedCallBook(void);

#endif /* __BT_PBAP_API_H_ */ 

