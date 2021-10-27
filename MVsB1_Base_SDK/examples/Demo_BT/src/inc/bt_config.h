
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

#define ENABLE						TRUE
#define DISABLE						FALSE

/*****************************************************************
 *
 * BLE config
 *
 */
#define BLE_SUPPORT					ENABLE

/*****************************************************************
 *
 * Bluetooth stack common config
 *
 */

//蓝牙名称支持中文,需要使用URL编码
//BLE的名称修改在ble广播数据中体现(ble_app_func.c)
#define BT_NAME						"BP10_BT PLAY DEMO"
#define BT_NAME_SIZE				40//最大支持name size,不能修改
#define BT_ADDR_SIZE				6

#define BT_TRIM						0x16

#define BT_PINCODE					"0000"

#define BT_TX_POWER_LEVEL			22	//23:max tx power level(max:8db / -2db step)

//inquiry scan params
#define BT_INQUIRYSCAN_INTERVAL		0x100	//default:0x1000
#define BT_INQUIRYSCAN_WINDOW		0x12	//default:0x12

//page scan params
#define BT_PAGESCAN_INTERVAL		0x1000	//default:0x1000
#define BT_PAGESCAN_WINDOW			0x12	//default:0x12

//page timeout
//time:4096*0.625=2.56s
#define BT_PAGE_TIMEOUT				0x1000	//default:0x2000

/**
 * 以下宏请勿随意修改，否则会引起编译错误
 * 注：A2DP和AVRCP是标配，必须要ENABLE
 */
#define BT_A2DP_SUPPORT				ENABLE
#if (BT_A2DP_SUPPORT == ENABLE)
#define BT_AVRCP_SUPPORT			ENABLE	//a2dp and avrcp must be enable at the same time
#define BT_HFP_SUPPORT				DISABLE
#define BT_SPP_SUPPORT				ENABLE
#else
#define BT_AVRCP_SUPPORT			DISABLE
#define BT_HFP_SUPPORT				DISABLE
#define BT_SPP_SUPPORT				DISABLE
#endif

//在非蓝牙模式下,播放音乐自动切换到播放模式
//#define BT_AUTO_ENTER_PLAY_MODE

/*****************************************************************
 *
 * A2DP config
 *
 */
#if BT_A2DP_SUPPORT == ENABLE

#include "../../src/bluetooth/inc/bt_a2dp_api.h"

#define BT_A2DP_AUDIO_DATA				A2DP_AUDIO_DATA_PCM

#endif /* BT_A2DP_SUPPORT == ENABLE */

/*****************************************************************
 *
 * AVRCP config
 *
 */
#if BT_AVRCP_SUPPORT == ENABLE

#include "../../src/bluetooth/inc/bt_avrcp_api.h"

/*
 * 建议开启高级AVRCP,关闭后可能会导致A2DP播放状态的更新异常
 */
#define BT_AVRCP_ADVANCED				ENABLE

#if (BT_AVRCP_ADVANCED == ENABLE)
/*
 * If it doesn't support Advanced AVRCP, TG side will be ignored
 * 音量同步功能需要开启该宏开关
 */
#define BT_AVRCP_VOLUME_SYNC			DISABLE

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

#else

#define BT_AVRCP_VOLUME_SYNC			DISABLE

#define BT_AVRCP_SONG_TRACK_INFOR		DISABLE

#define BT_AVRCP_SONG_PLAY_STATE		DISABLE

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

#include "../../src/bluetooth/inc/bt_hfp_api.h"

//DISABLE: only cvsd
//ENABLE: cvsd + msbc
#define BT_HFP_SUPPORT_WBS				ENABLE

/*
 * If it doesn't support WBS, only PCM format data can be
 * transfered to application.
 */
#define BT_HFP_AUDIO_DATA				HFP_AUDIO_DATA_mSBC


/*
 * 通话相关配置
 */
//AEC相关参数配置 (MIC gain, AGC, DAC gain, 降噪参数)
#define BT_HFP_AEC_ENABLE

//MIC无运放,使用如下参数(参考)
#define BT_HFP_MIC_PGA_GAIN				2  //ADC PGA GAIN +18db(0~31, 0:max, 31:min)
#define BT_HFP_MIC_DIGIT_GAIN			30000
#define BT_HFP_INPUT_DIGIT_GAIN			2000

//MIC有运放,使用如下参数(参考开发板)
//#define BT_HFP_MIC_PGA_GAIN				14  //ADC PGA GAIN +2db(0~31, 0:max, 31:min)
//#define BT_HFP_MIC_DIGIT_GAIN				4095
//#define BT_HFP_INPUT_DIGIT_GAIN			4095

#define BT_HFP_AEC_ECHO_LEVEL			1 //Echo suppression level: 0(min)~5(max)
#define BT_HFP_AEC_NOISE_LEVEL			0 //Noise suppression level: 0(min)~5(max)

#define BT_HFP_AEC_MAX_DELAY_BLK		32
#define BT_HFP_AEC_DELAY_BLK			10 //MIC无运放参考值
//#define BT_HFP_AEC_DELAY_BLK			14 //MIC有运放参考值(参考开发板)


//来电通话时长配置选项
#define BT_HFP_CALL_DURATION_DISP


#endif /* BT_HFP_SUPPORT == ENABLE */


/*****************************************************************
 * ram config
 */
//A2DP+AVRCP+SPP = 24K
//A2DP+AVRCP+SPP+HFP = 26K

#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
#define BT_AVRCP_TG_MEM_SIZE		2*1024
#else
#define BT_AVRCP_TG_MEM_SIZE		0
#endif

//Note:如需SPP,则需要再增加协议栈RAM
#if ((BT_A2DP_SUPPORT == ENABLE)&&(BT_HFP_SUPPORT == ENABLE))
 #define BT_STACK_MEM_SIZE			(23*1024+512+BT_AVRCP_TG_MEM_SIZE)
#elif (BT_A2DP_SUPPORT == ENABLE)
 #define BT_STACK_MEM_SIZE			(22*1024-200+BT_AVRCP_TG_MEM_SIZE)//(20*1024+512+BT_AVRCP_TG_MEM_SIZE)
#endif

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

//预留该功能和对应的函数接口
//#define BT_FAST_POWER_ON_OFF_FUNC						// 快速打开/关闭蓝牙功能

#ifndef CFG_APP_CONFIG
	/*某些需求:在非蓝牙模式下,尽可能释放协议栈/EM/BB相关的资源*/
	/*如开启了HFP,最好开启蓝牙后台运行功能*/
	//#define CFG_BT_BACKGROUND_RUN_EN /*蓝牙后台运行开关*/

/**BB EM参数配置 **/
//注：需要配合MPUConfigTable和SRAM_END_ADDR使用
//现在OS相关的RAM结束地址已经和BB_MPU_START_ADDR关联
//对应关系 0x40:20010000; 0x80:20020000; 0xc0:20030000 ;0x100: 0x20040000; 0x120:0x20048000; 0x130:0x2004c000
//default: addr: 0x20040000;  size: 0x10000;  em_start_addr: 0x100
//EM_SIZE说明:只使用A2DP+AVRCP,EM_SIZE配置为16K能满足需求;
//使用HFP+A2DP+AVRCP,EM_SIZE配置需要为24K;

#if (BT_HFP_SUPPORT == ENABLE)
#define BB_EM_MAP_ADDR			0x80000000
#define BB_EM_START_PARAMS		0x128
#define BB_EM_SIZE				(24*1024)
#define BB_MPU_START_ADDR		(0x20050000 - BB_EM_SIZE)
#else
#define BB_EM_MAP_ADDR			0x80000000

//#define BB_EM_START_PARAMS		(0x138)
//#define BB_EM_SIZE				(8*1024)
//||||||| .r2206
//#define BB_EM_START_PARAMS		0x134
//#define BB_EM_SIZE				(12*1024)
//=======
#define BB_EM_START_PARAMS		0x120//0x138
#define BB_EM_SIZE				(32*1024)//(8*1024)

#define BB_MPU_START_ADDR		(0x20050000 - BB_EM_SIZE)
#endif

//max:32k
/*#define BB_EM_MAP_ADDR			0x80000000
#define BB_EM_START_PARAMS		0x120//0x130//
#define BB_EM_SIZE				(32*1024)//(16*1024)
#define BB_MPU_START_ADDR		(0x20050000 - BB_EM_SIZE)
*/
#endif

#endif /*__BT_DEVICE_CFG_H__*/

