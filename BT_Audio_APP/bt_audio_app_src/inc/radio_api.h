/*
 * radio_api.h
 *
 *  Created on: 2018.5.3
 *      Author: litao
 */


#ifndef SRC_INC_RADIO_API_H_
#define SRC_INC_RADIO_API_H_

#include "app_config.h"
#include "timeout.h"

#ifdef CFG_APP_RADIOIN_MODE_EN

#define MAX_RADIO_CHANNEL_NUM    50
#define RADIO_SEEK_PREVIEW_SUPPORT
#define RADIO_DELAY_SWITCH_CHANNEL
#define FM_CHL_SEARCH_TIME_UNIT 100
#define FM_PERVIEW_TIMEOUT 1000

#define FM_AREA_CHINA  0x00 // 00B 87.5~108MHz (US/Europe, China)
#define FM_AREA_JAPAN  0x01 // 01B 76~90MHz (Japan)
#define FM_AREA_RUSSIA 0x02 // 10B 65.8~73MHz (Russia)
#define FM_AREA_OTHER  0x03 // 11B 60~76MHz

#define CHINA_BASE_FREQ  875 // China/US/Europe Ƶ�� ��׼Ƶ��
#define JAPAN_BASE_FREQ  760 // Japan Ƶ�� ��׼Ƶ��
#define RUSSIA_BASE_FREQ 650 // Russia Ƶ�� ��׼Ƶ��
#define OTHER_BASE_FREQ  600 // ����Ƶ�� ��׼Ƶ��

#define FM_CHANNLE_STEP  100 // 100KHZ��FM��̨����

typedef enum _Radio_NAME
{
    RADIO_NONE,
    RADIO_RDA5807P,
    RADIO_SILI4702,
    RADIO_CS2010,
    RADIO_CL6017X,
    RADIO_KT0830E,
    RADIO_AR1000F,
    RADIO_BK1080,
    RADIO_QN8035,
    RADIO_RDA5807H
} RADIO_NAME;

typedef enum _RADIO_STATUS_
{
    RADIO_STATUS_IDLE = 0,  //������
    RADIO_STATUS_SEEKING,   //��̨��
    RADIO_STATUS_SEEKING_UP,   //��̨��
    RADIO_STATUS_SEEKING_DOWN, //��̨��
    RADIO_STATUS_PLAYING,   //������
    RADIO_STATUS_PREVIEW,   //��̨Ԥ��

} RADIO_STATUS;


#define RADIO_DEF_MIN_FREQ_VALUE 875  // FMƵ����С��Χ(US/Europe, China)
#define RADIO_DEF_MAX_FREQ_VALUE 1080 // FMƵ�����Χ(US/Europe, China)

#define RADIO_JAP_MIN_FREQ_VALUE 760  // FMƵ����С��Χ(Japan)
#define RADIO_JAP_MAX_FREQ_VALUE 900  // FM(Ƶ�����ΧJapan)

#define RADIO_RUS_MIN_FREQ_VALUE 658  // FMƵ����С��Χ(Russia)
#define RADIO_RUS_MAX_FREQ_VALUE 730  // FMƵ�����Χ(Russia)

#define RADIO_OTH_MIN_FREQ_VALUE 600  // FMƵ����С��Χ(����)
#define RADIO_OTH_MAX_FREQ_VALUE 760  // FMƵ�����Χ(����)

typedef struct _RADIO_CONTROL_
{
	uint8_t   Channel[MAX_RADIO_CHANNEL_NUM]; // ��̨�б�
	uint16_t  Freq;                           // ��ǰƵ��
	uint16_t  MinFreq;
	uint16_t  MaxFreq;
	uint8_t  ChlCount;                       // ���е�̨����
	uint8_t  CurStaIdx;                      // ��ǰ��̨������
	uint8_t  State;                          // FM Radio��ǰ״̬
	uint8_t  CurFreqArea;                    // ��ǰƵ��(�й����ձ���ŷ��...)

	TIMER TimerHandle;
#ifdef RADIO_DELAY_SWITCH_CHANNEL
	TIMER DelayDoTimer; // ��̨�л��ӳ���תTimer
#endif
	uint16_t InputNumber; // �������ݰ���ѡ̨
	TIMER NumKeyTimer;

} RADIO_CONTROL;

extern RADIO_CONTROL* sRadioControl;
extern RADIO_NAME Radio_Name;	//����FM���ͺ�

bool RadioDeinitialize(void);
bool RadioInitialize(void);
bool RadioControl(void);
void RadioPlay(void);
bool RadioSwitchChannel(uint32_t Msg);

#endif

#endif /* SRC_INC_RADIO_API_H_ */

