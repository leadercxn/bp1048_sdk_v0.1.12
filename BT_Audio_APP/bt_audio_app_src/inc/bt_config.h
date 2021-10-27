
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

//蓝牙名称注意事项:
//1.蓝牙名称支持中文,需要使用URL编码
//2.BLE的名称修改在ble广播数据中体现(ble_app_func.c)
//3.SDK蓝牙名称上电后从flash中读取,如需使用固定的名称,请移步到bt_app_func.c中LoadBtConfigurationParams函数内修改获取方式
#define BT_NAME						"BP10_BT"
//#include "bt_name.h"
#define BT_NAME_SIZE				40//最大支持name size,不能修改
#define BT_ADDR_SIZE				6

#define BT_TRIM						0x18 //trim范围:0x05~0x1d

#define BT_SIMPLEPAIRING_FLAG		ENABLE //0:use pin code; 1:simple pairing
#define BT_PINCODE_LEN				4//最大16bytes
#define BT_PINCODE					"0000"

//sniff 相关配置
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
#define BT_PAGE_TX_POWER_LEVEL		16  //蓝牙回连发射功率

//inquiry scan params
#define BT_INQUIRYSCAN_INTERVAL		0x100	//default:0x1000
#define BT_INQUIRYSCAN_WINDOW		0x12	//default:0x12

//page scan params
#define BT_PAGESCAN_INTERVAL		0x1000	//default:0x1000
#define BT_PAGESCAN_WINDOW			0x12	//default:0x12

//page timeout(ms)
#define BT_PAGE_TIMEOUT				5120	//default:5.12s

/**
 * 以下宏请勿随意修改，否则会引起编译错误
 * 注：A2DP和AVRCP是标配，必须要ENABLE
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

//在非蓝牙模式下,播放音乐自动切换到播放模式
//#define BT_AUTO_ENTER_PLAY_MODE

/*****************************************************************
 *
 * A2DP config
 *
 */
#if BT_A2DP_SUPPORT == ENABLE

#include "bt_a2dp_api.h"

//Note:开启AAC,需要同步开启解码器类型USE_AAC_DECODER(app_config.h)
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
 * 建议开启高级AVRCP,关闭后可能会导致A2DP播放状态的更新异常
 */
#define BT_AVRCP_ADVANCED				ENABLE

#if (BT_AVRCP_ADVANCED == ENABLE)
/*
 * If it doesn't support Advanced AVRCP, TG side will be ignored
 * 音量同步功能需要开启该宏开关
 */
#define BT_AVRCP_VOLUME_SYNC			ENABLE

/*
 * If it doesn't support Advanced AVRCP, TG side will be ignored
 * 和音量同步功能都用到AVRCP TG
 * player application setting and value和音量同步宏定义开关一致(eg:EQ/repeat mode/shuffle/scan configuration)
 */
#define BT_AVRCP_PLAYER_SETTING			DISABLE

/*
 * If it doesn't support Advanced AVRCP, song play state will be ignored
 * 歌曲播放时间
 */
#define BT_AVRCP_SONG_PLAY_STATE		DISABLE

/*
 * If it doesn't support Advanced AVRCP, song track infor will be ignored
 * 歌曲ID3信息反馈
 * 歌曲信息有依赖播放时间来获取,请和BT_AVRCP_SONG_PLAY_STATE同步开启
 */
#define BT_AVRCP_SONG_TRACK_INFOR		DISABLE

/*
 * AVRCP BROWSER功能使用,必须使用libBtStack_AvrcpBrws.a库文件
 * 重要! BB_EM_SIZE必须要配置为(20*1024) -- app_config.h中
 */
#define BT_AVRCP_BROWSER_FUNC			DISABLE

#endif

/*
 * AVRCP连接成功后，自动播放歌曲
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

//电池电量同步(开启需要和 CFG_FUNC_POWER_MONITOR_EN 关联)
//#define BT_HFP_BATTERY_SYNC

//开启HFP连接，不使能进入HFP相关模式(K歌模式和通话模式)
//该应用适用于某些需要连接HFP协议，但是不需要HFP相关功能的场合
//#define BT_HFP_MODE_DISABLE

#ifndef BT_HFP_MODE_DISABLE
//K歌功能宏开关 (前提：使能通话模式相关功能)
//注:该功能为苹果手机开启全名K歌等K歌软件而设定
//#define BT_RECORD_FUNC_ENABLE
#endif

/*
 * 通话相关配置
 */
//AEC相关参数配置 (MIC gain, AGC, DAC gain, 降噪参数)
#define BT_HFP_AEC_ENABLE
//#define BT_REMOTE_AEC_DISABLE			//关闭手机端AEC

//MIC无运放,使用如下参数(参考)
#define BT_HFP_MIC_PGA_GAIN				15  //ADC PGA GAIN +18db(0~31, 0:max, 31:min)
#define BT_HFP_MIC_DIGIT_GAIN			4095
#define BT_HFP_INPUT_DIGIT_GAIN			1100

//MIC有运放,使用如下参数(参考开发板)
//#define BT_HFP_MIC_PGA_GAIN				14  //ADC PGA GAIN +2db(0~31, 0:max, 31:min)
//#define BT_HFP_MIC_DIGIT_GAIN				4095
//#define BT_HFP_INPUT_DIGIT_GAIN			4095

#define BT_HFP_AEC_ECHO_LEVEL			4 //Echo suppression level: 0(min)~5(max)
#define BT_HFP_AEC_NOISE_LEVEL			2 //Noise suppression level: 0(min)~5(max)

#define BT_HFP_AEC_MAX_DELAY_BLK		32
#define BT_HFP_AEC_DELAY_BLK			8 //MIC无运放参考值
//#define BT_HFP_AEC_DELAY_BLK			14 //MIC有运放参考值(参考开发板)


//来电通话时长配置选项
#define BT_HFP_CALL_DURATION_DISP

//MIC变调处理
//#define BT_HFP_MIC_PITCH_SHIFTER_FUNC

//通话结束后恢复之前的播放状态
//注意: 手机自动恢复播放状态，不需要开启此宏定义开关
//#define BT_EXIT_HF_RESUME_PLAY_STATE

#endif /* BT_HFP_SUPPORT == ENABLE */


/*****************************************************************
 *
 * OBEX config
 *
 */
#if (BT_OBEX_SUPPORT == ENABLE)
//mva通过obex进行升级(升级方式:双bank)
//#define MVA_BT_OBEX_UPDATE_FUNC_SUPPORT
#endif

/*****************************************************************
 * ram config
 */
#define BT_BASE_MEM_SIZE			24*1024

//Note:当项目应用不需要BLE和HFP时,可以使用libBtStack_NoBle_NoHfp.a这个库
//同时BT_BASE_MEM_SIZE可以减少到如下配置,同时app_config.h中,BB_EM_SIZE可以配置为12K
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
#define BT_RECONNECTION_FUNC							// 蓝牙自动重连功能
#ifdef BT_RECONNECTION_FUNC
	#define BT_POWERON_RECONNECTION						// 开机自动重连
	#ifdef BT_POWERON_RECONNECTION
		#define BT_POR_TRY_COUNTS			(7)			// 开机重连尝试次数
		#define BT_POR_INTERNAL_TIME		(3)			// 开机重连每两次间隔时间(in seconds)
	#endif

	#define BT_BB_LOST_RECONNECTION						// BB Lost之后自动重连
	#ifdef BT_BB_LOST_RECONNECTION
		#define BT_BLR_TRY_COUNTS			(90)		// BB Lost 尝试重连次数
		#define BT_BLR_INTERNAL_TIME		(5)			// BB Lost 重连每两次间隔时间(in seconds)
	#endif

#endif

#ifdef CFG_BT_BACKGROUND_RUN_EN
//注意：该宏定义的开启，必须要蓝牙保持后台
#define BT_FAST_POWER_ON_OFF_FUNC						// 快速打开/关闭蓝牙功能
#endif

//手机端清除配对记录,回连时,被手机端拒绝连接,则同步的清理配对记录,下次不再回连;
//#define BT_AUTO_CLEAR_LAST_PAIRING_LIST

#define BT_MATCH_CHECK					0

#endif /*__BT_DEVICE_CFG_H__*/

