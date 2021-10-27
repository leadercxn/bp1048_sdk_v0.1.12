
///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: bt_config.h
//  maintainer: keke
///////////////////////////////////////////////////////////////////////////////

#ifndef __BT_DEVICE_CFG_H__
#define __BT_DEVICE_CFG_H__
#include "type.h"
#include "app_config.h"

#define ENABLE						TRUE
#define DISABLE						FALSE

/*****************************************************************
 *
 * BLE config
 *
 */
#define BLE_SUPPORT					DISABLE

/*****************************************************************
 *
 * Bluetooth stack common config
 *
 */

//��������ע������:
//1.��������֧������,��Ҫʹ��URL����
//2.BLE�������޸���ble�㲥����������(ble_app_func.c)
//3.SDK���������ϵ���flash�ж�ȡ,����ʹ�ù̶�������,���Ʋ���bt_app_func.c��LoadBtConfigurationParams�������޸Ļ�ȡ��ʽ
#define BT_NAME						"BP10_BT"
//#include "bt_name.h"
#define BT_NAME_SIZE				40//���֧��name size,�����޸�
#define BT_ADDR_SIZE				6

#define BT_TRIM						0x18 //trim��Χ:0x05~0x1d

#define BT_SIMPLEPAIRING_FLAG		ENABLE //0:use pin code; 1:simple pairing
#define BT_PINCODE_LEN				4//���16bytes
#define BT_PINCODE					"0000"

//sniff �������
//#define BT_SNIFF_ENABLE


#define	BT_SNIFF_HOSC_CLK 0
#define	BT_SNIFF_RC_CLK 1
#define	BT_SNIFF_LOSC_CLK 2

#define BT_SNIFF_CLK_SEL BT_SNIFF_HOSC_CLK

/* Rf Tx Power Range 
{   level  dbm
	[23] = 8,
	[22] = 6,
	[21] = 4,
	[20] = 2,
	[19] = 0,
	[18] = -2,
	[17] = -4,
	[16] = -6,
	[15] = -8,
	[14] = -10,
	[13] = -13,
	[12] = -15,
	[11] = -17,
	[10] = -19,
	[9]  = -21,
	[8]  = -23,
	[7]  = -25,
	[6]  = -27,
	[5]  = -29,
	[4]  = -31,
	[3]  = -33,
	[2]  = -35,
	[1]  = -37,
	[0]  = -39,
}
*/
#define BT_TX_POWER_LEVEL			19	//defined 0dbm
#define BT_PAGE_TX_POWER_LEVEL		16  //�����������书��

//inquiry scan params
#define BT_INQUIRYSCAN_INTERVAL		0x100	//default:0x1000
#define BT_INQUIRYSCAN_WINDOW		0x12	//default:0x12

//page scan params
#define BT_PAGESCAN_INTERVAL		0x1000	//default:0x1000
#define BT_PAGESCAN_WINDOW			0x12	//default:0x12

//page timeout(ms)
#define BT_PAGE_TIMEOUT				5120	//default:5.12s

/**
 * ���º����������޸ģ����������������
 * ע��A2DP��AVRCP�Ǳ��䣬����ҪENABLE
 */
#define BT_A2DP_SUPPORT				ENABLE
#define BT_AVRCP_SUPPORT			ENABLE	//a2dp and avrcp must be enable at the same time
#if CFG_RES_MIC_SELECT
#define BT_HFP_SUPPORT				ENABLE
#endif
#define BT_SPP_SUPPORT				ENABLE
#define BT_HID_SUPPORT				DISABLE
#define BT_MFI_SUPPORT				DISABLE
#define BT_OBEX_SUPPORT				DISABLE
#define BT_PBAP_SUPPORT				DISABLE


#ifdef	CFG_AI_ENCODE_EN

	#if(BLE_SUPPORT != (1==1))
		#error	"Please enable BLE!!!"
	#endif//BLE CHECK

	#if(BT_SPP_SUPPORT != (1==1))
		#error	"Please enable SPP!!!"
	#endif//BLE CHECK

#endif	//CFG_XIAOAI_AI_EN

//�ڷ�����ģʽ��,���������Զ��л�������ģʽ
//#define BT_AUTO_ENTER_PLAY_MODE

/*****************************************************************
 *
 * A2DP config
 *
 */
#if BT_A2DP_SUPPORT == ENABLE

#include "bt_a2dp_api.h"

//Note:����AAC,��Ҫͬ����������������USE_AAC_DECODER(app_config.h)
//#define BT_AUDIO_AAC_ENABLE

#endif /* BT_A2DP_SUPPORT == ENABLE */

/*****************************************************************
 *
 * AVRCP config
 *
 */
#if BT_AVRCP_SUPPORT == ENABLE

#include "bt_avrcp_api.h"

/*
 * ���鿪���߼�AVRCP,�رպ���ܻᵼ��A2DP����״̬�ĸ����쳣
 */
#define BT_AVRCP_ADVANCED				ENABLE

#if (BT_AVRCP_ADVANCED == ENABLE)
/*
 * If it doesn't support Advanced AVRCP, TG side will be ignored
 * ����ͬ��������Ҫ�����ú꿪��
 */
#define BT_AVRCP_VOLUME_SYNC			ENABLE

/*
 * If it doesn't support Advanced AVRCP, TG side will be ignored
 * ������ͬ�����ܶ��õ�AVRCP TG
 * player application setting and value������ͬ���궨�忪��һ��(eg:EQ/repeat mode/shuffle/scan configuration)
 */
#define BT_AVRCP_PLAYER_SETTING			DISABLE

/*
 * If it doesn't support Advanced AVRCP, song play state will be ignored
 * ��������ʱ��
 */
#define BT_AVRCP_SONG_PLAY_STATE		DISABLE

/*
 * If it doesn't support Advanced AVRCP, song track infor will be ignored
 * ����ID3��Ϣ����
 * ������Ϣ����������ʱ������ȡ,���BT_AVRCP_SONG_PLAY_STATEͬ������
 */
#define BT_AVRCP_SONG_TRACK_INFOR		DISABLE

/*
 * AVRCP BROWSER����ʹ��,����ʹ��libBtStack_AvrcpBrws.a���ļ�
 * ��Ҫ! BB_EM_SIZE����Ҫ����Ϊ(20*1024) -- app_config.h��
 */
#define BT_AVRCP_BROWSER_FUNC			DISABLE

#endif

/*
 * AVRCP���ӳɹ����Զ����Ÿ���
 */
#define BT_AUTO_PLAY_MUSIC				DISABLE

#endif /* BT_AVRCP_SUPPORT == ENABLE */

/*****************************************************************
 *
 * HFP config
 *
 */
#if BT_HFP_SUPPORT == ENABLE

#include "bt_hfp_api.h"

//DISABLE: only cvsd
//ENABLE: cvsd + msbc
#define BT_HFP_SUPPORT_WBS				ENABLE

/*
 * If it doesn't support WBS, only PCM format data can be
 * transfered to application.
 */
#define BT_HFP_AUDIO_DATA				HFP_AUDIO_DATA_mSBC

//��ص���ͬ��(������Ҫ�� CFG_FUNC_POWER_MONITOR_EN ����)
//#define BT_HFP_BATTERY_SYNC

//����HFP���ӣ���ʹ�ܽ���HFP���ģʽ(K��ģʽ��ͨ��ģʽ)
//��Ӧ��������ĳЩ��Ҫ����HFPЭ�飬���ǲ���ҪHFP��ع��ܵĳ���
//#define BT_HFP_MODE_DISABLE

#ifndef BT_HFP_MODE_DISABLE
//K�蹦�ܺ꿪�� (ǰ�᣺ʹ��ͨ��ģʽ��ع���)
//ע:�ù���Ϊƻ���ֻ�����ȫ��K���K��������趨
//#define BT_RECORD_FUNC_ENABLE
#endif

/*
 * ͨ���������
 */
//AEC��ز������� (MIC gain, AGC, DAC gain, �������)
#define BT_HFP_AEC_ENABLE
//#define BT_REMOTE_AEC_DISABLE			//�ر��ֻ���AEC

//MIC���˷�,ʹ�����²���(�ο�)
#define BT_HFP_MIC_PGA_GAIN				15  //ADC PGA GAIN +18db(0~31, 0:max, 31:min)
#define BT_HFP_MIC_DIGIT_GAIN			4095
#define BT_HFP_INPUT_DIGIT_GAIN			1100

//MIC���˷�,ʹ�����²���(�ο�������)
//#define BT_HFP_MIC_PGA_GAIN				14  //ADC PGA GAIN +2db(0~31, 0:max, 31:min)
//#define BT_HFP_MIC_DIGIT_GAIN				4095
//#define BT_HFP_INPUT_DIGIT_GAIN			4095

#define BT_HFP_AEC_ECHO_LEVEL			4 //Echo suppression level: 0(min)~5(max)
#define BT_HFP_AEC_NOISE_LEVEL			2 //Noise suppression level: 0(min)~5(max)

#define BT_HFP_AEC_MAX_DELAY_BLK		32
#define BT_HFP_AEC_DELAY_BLK			8 //MIC���˷Ųο�ֵ
//#define BT_HFP_AEC_DELAY_BLK			14 //MIC���˷Ųο�ֵ(�ο�������)


//����ͨ��ʱ������ѡ��
#define BT_HFP_CALL_DURATION_DISP

//MIC�������
//#define BT_HFP_MIC_PITCH_SHIFTER_FUNC

//ͨ��������ָ�֮ǰ�Ĳ���״̬
//ע��: �ֻ��Զ��ָ�����״̬������Ҫ�����˺궨�忪��
//#define BT_EXIT_HF_RESUME_PLAY_STATE

#endif /* BT_HFP_SUPPORT == ENABLE */


/*****************************************************************
 *
 * OBEX config
 *
 */
#if (BT_OBEX_SUPPORT == ENABLE)
//mvaͨ��obex��������(������ʽ:˫bank)
//#define MVA_BT_OBEX_UPDATE_FUNC_SUPPORT
#endif

/*****************************************************************
 * ram config
 */
#define BT_BASE_MEM_SIZE			24*1024

//Note:����ĿӦ�ò���ҪBLE��HFPʱ,����ʹ��libBtStack_NoBle_NoHfp.a�����
//ͬʱBT_BASE_MEM_SIZE���Լ��ٵ���������,ͬʱapp_config.h��,BB_EM_SIZE��������Ϊ12K
//#define BT_BASE_MEM_SIZE			(20*1024+300)

#if (BLE_SUPPORT == ENABLE)
#define BT_BLE_MEM_SIZE				1150
#else
#define BT_BLE_MEM_SIZE				0
#endif

#ifdef BT_AUDIO_AAC_ENABLE
#define BT_AUDIO_AAC_MEM_SIZE		400
#else
#define BT_AUDIO_AAC_MEM_SIZE		0
#endif

#if ((BT_AVRCP_VOLUME_SYNC == ENABLE)||(BT_AVRCP_PLAYER_SETTING == ENABLE))
#define BT_AVRCP_TG_MEM_SIZE		2310
#else
#define BT_AVRCP_TG_MEM_SIZE		0
#endif

#if (BT_AVRCP_BROWSER_FUNC == ENABLE)
#define BT_AVRCP_BRWS_MEM_SIZE		(21*1024+512)
#else
#define BT_AVRCP_BRWS_MEM_SIZE		0
#endif

#if (BT_HFP_SUPPORT == ENABLE)
#define BT_HFP_MEM_SIZE				1900
#else
#define BT_HFP_MEM_SIZE				0
#endif

#if (BT_SPP_SUPPORT == ENABLE)
#define BT_SPP_MEM_SIZE				700
#else
#define BT_SPP_MEM_SIZE				0
#endif

#if (BT_HID_SUPPORT == ENABLE)
#define BT_HID_MEM_SIZE				1024
#else
#define BT_HID_MEM_SIZE				0
#endif

#if (BT_MFI_SUPPORT == ENABLE)
#define BT_MFI_MEM_SIZE				700
#else
#define BT_MFI_MEM_SIZE				0
#endif

#if (BT_OBEX_SUPPORT == ENABLE)
#define BT_OBEX_MEM_SIZE			400
#else
#define BT_OBEX_MEM_SIZE			0
#endif

#if (BT_PBAP_SUPPORT == ENABLE)
#define BT_PBAP_MEM_SIZE			600
#else
#define BT_PBAP_MEM_SIZE			0
#endif

#define BT_STACK_MEM_SIZE	(BT_BASE_MEM_SIZE + BT_AUDIO_AAC_MEM_SIZE + BT_AVRCP_TG_MEM_SIZE + BT_AVRCP_BRWS_MEM_SIZE +\
							BT_HFP_MEM_SIZE + BT_SPP_MEM_SIZE + BT_BLE_MEM_SIZE + BT_HID_MEM_SIZE + \
							BT_MFI_MEM_SIZE + BT_OBEX_MEM_SIZE + BT_PBAP_MEM_SIZE)


/*****************************************************************
 * bt mode config
 */
#define BT_RECONNECTION_FUNC							// �����Զ���������
#ifdef BT_RECONNECTION_FUNC
	#define BT_POWERON_RECONNECTION						// �����Զ�����
	#ifdef BT_POWERON_RECONNECTION
		#define BT_POR_TRY_COUNTS			(7)			// �����������Դ���
		#define BT_POR_INTERNAL_TIME		(3)			// ��������ÿ���μ��ʱ��(in seconds)
	#endif

	#define BT_BB_LOST_RECONNECTION						// BB Lost֮���Զ�����
	#ifdef BT_BB_LOST_RECONNECTION
		#define BT_BLR_TRY_COUNTS			(90)		// BB Lost ������������
		#define BT_BLR_INTERNAL_TIME		(5)			// BB Lost ����ÿ���μ��ʱ��(in seconds)
	#endif

#endif

#ifdef CFG_BT_BACKGROUND_RUN_EN
//ע�⣺�ú궨��Ŀ���������Ҫ�������ֺ�̨
#define BT_FAST_POWER_ON_OFF_FUNC						// ���ٴ�/�ر���������
#endif

//�ֻ��������Լ�¼,����ʱ,���ֻ��˾ܾ�����,��ͬ����������Լ�¼,�´β��ٻ���;
//#define BT_AUTO_CLEAR_LAST_PAIRING_LIST

#define BT_MATCH_CHECK					0

#endif /*__BT_DEVICE_CFG_H__*/

