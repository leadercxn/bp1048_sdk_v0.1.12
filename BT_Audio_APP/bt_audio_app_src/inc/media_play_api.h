/**
 **************************************************************************************
 * @file    media_service_api.h
 * @brief   
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2017-3-17 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __MEDIA_SERVICE_H__
#define __MEDIA_SERVICE_H__

#include "type.h"
#include "app_config.h"
#include "ff.h"
#include "audio_decoder_api.h"
#include "decoder_service.h"
#include "ffpresearch.h"
#ifdef CFG_FUNC_LRC_EN
#include "lrc.h"
#endif
#ifdef CFG_FUNC_RECORD_FLASHFS
#include "file.h"
#endif
// Define device ID in system.
typedef	enum _DEV_ID
{
    DEV_ID_NONE,
    DEV_ID_USB,
    DEV_ID_SD,
    DEV_ID_FLASHFS,
} DEV_ID;

typedef enum _PLAYER_STATE
{
    PLAYER_STATE_IDLE = 0,    // 空闲
    PLAYER_STATE_PLAYING,     // 播放
    PLAYER_STATE_PAUSE,       // 暂停
    PLAYER_STATE_FF,          // 快进中
    PLAYER_STATE_FB,          // 快退中
    PLAYER_STATE_STOP,
} PLAYER_STATE;


typedef enum _AUDIO_PLAYER_PLAY_MODE_
{
	PLAY_MODE_REPEAT_ALL,     // 全盘顺序循环播放
	PLAY_MODE_RANDOM_ALL,     // 全盘随机播放
	PLAY_MODE_REPEAT_ONE,     // 单曲循环播放
	PLAY_MODE_REPEAT_FOLDER,  // 文件夹顺序循环播放

	PLAY_MODE_RANDOM_FOLDER,  // 文件夹随机播放
#ifdef FUNC_BROWSER_PARALLEL_EN
	PLAY_MODE_BROWSER,
#endif
	PLAY_MODE_PREVIEW_PLAY,   // 预览播放（在文件浏览中，选中歌曲，可预览播放几秒钟）
	PLAY_MODE_REPEAT_OFF,
	PLAY_MODE_SUM,
} AUDIO_PLAYER_MODE;

#ifdef FUNC_SPECIFY_FOLDER_PLAY_EN
typedef enum _STORY_PLAY_SELECT
{
    STORY_PLAY_CURRENT = 1,    
    STORY_PLAY_NEXT,     
    
} STORY_PLAY_SELECT;
#endif
///////////////////////////////////////////////////////
// 复读模式的数据结构，后续系统设置有单独的复读模式设置(复读时长，循环次数)
#define REPEAT_CLOSED  0 // 复读关闭
#define REPEAT_A_SETED 1 // 已设置复读起点
#define REPEAT_OPENED  2 // 复读中
#define REPEAT_OPENED_PAUSE 3 //复读中但暂停

typedef struct _REPEAT_AB_
{
	//考虑到实际应用，0xFFFF大约也有18个小时，starttime和times不太可能用到uint32_t，所以改成uint16_t类型
	uint16_t StartTime; // 起始时间(S)，播放时，选择复读模式后，设置起点
	uint16_t Times;      // 复读时间长度(S)，如果不通过设置结束时间的方式操作，最终的复读时间以此为准，可预设值
	//uint8_t LoopCount;  // 循环次数，可预先设置(EPROM)，默认3次
	uint8_t RepeatFlag; // 当前是否复读模式(0 - 复读关闭, 1 - 已设置起点, 2 - 已设置终点，并开始复读)
} REPEAT_AB;

///////////////////////////////////////////////////////
typedef struct _MEDIA_PLAYER_CONTROL_
{
	uint8_t		CurPlayState;		// 播放器状态 (uint8_t)
    uint8_t		CurPlayMode;		// 当前播放循环模式
	bool		SongSwitchFlag;		// 切换歌曲的方向，默认0为下一首
    
    uint32_t	CurPlayTime;		// 播放时间, ms

	FATFS		gFatfs_u;   		//U盘/* File system object */
	FATFS 		gFatfs_sd;   		//SD卡/* File system object */

	FIL			PlayerFile;         // 当前播放文件
	uint16_t	CurFileIndex;		// 当前文件号，则代表全盘ID，
	uint16_t	TotalFileSumInDisk;	// 全盘中的文件总数
	
	ff_dir		PlayerFolder;		// 文件夹句柄，由加速器返回参数，应用不要赋值。
	uint16_t	CurFolderIndex;		// 需要打开的文件夹全盘ID号  //注:改为当前文件所在文件夹 的有效文件夹序号
	uint16_t	ValidFolderSumInDisk;//全盘中有效的文件夹总数，过滤空文件夹(不包含歌曲文件的文件夹)

	DecoderState DecoderSync;		//解码状态登记

#ifdef CFG_FUNC_RECORD_SD_UDISK
	uint16_t*	RecFileList;		//录音文件夹文件Index排序，第一个最新。最多256.
	DIR 		Dir;
	FILINFO 	FileInfo;
#elif defined(CFG_FUNC_RECORD_FLASHFS)
	MemHandle	FlashFsMemHandle;
	FFILE		*FlashFsFile;
#endif

	uint8_t*    AccRamBlk;        //ACC内存分配指针，分配大小：MAX_ACC_RAM_SIZE;

#ifdef CFG_FUNC_LRC_EN
	FIL			LrcFile;         	// LRC歌词文件
	uint8_t		LrcFlag;			// 歌词显示标志(掉电记忆)
	uint8_t		IsLrcRunning;
	LRC_ROW		LrcRow;
	uint8_t 	ReadBuffer[512];
	LRC_INFO	LrcInfo;
	uint8_t 	TempBuf1[128];
	uint8_t 	TempBuf2[255];
#endif

    REPEAT_AB	RepeatAB;			// 复读模式设置信息

//#ifdef PLAYER_SPECTRUM 				// 频谱显示
//	uint8_t		SpecTrumFlag;		// 频谱打开标识
//	SPECTRUM_INFO SpecTrumInfo;
//#endif
	uint16_t	ErrFileCount;		// 无法正常解码的曲目计数，用户对folder中的曲目都无法解码进行异常退出处理
//	uint16_t	error_times;		// 某些错误连续发生的次数，用户恢复播放
//	uint8_t		IsBPMached;			// 是否FS已匹配了断点信息，主要用于防止保存无效的断点信息(播放模式初始化失败后，需要改标志判断是否保存断点信息)

#ifdef FUNC_SPECIFY_FOLDER_PLAY_EN
	uint8_t		StorySelectPlayFlag;// 1:select curret play 2:next play
	uint8_t 	StoryPlayByIndexFlag;
#endif


} MEDIA_PLAYER;

extern MEDIA_PLAYER* gpMediaPlayer;

//关闭播放文件
void MediaPlayerCloseSongFile(void);
void MediaPlayerNextSong(bool IsSongPlayEnd);
void MediaPlayerPreSong(void);
void MediaPlayerPlayPause(void);
void MediaPlayerStop(void);
bool MediaPlayerInitialize(DEV_ID DeviceIndex, int32_t FileIndex, uint32_t FolderIndex);
void MediaPlayerDeinitialize(void);
void MediaPlayerSwitchPlayMode(uint8_t PlayMode);
//播放器 上一文件夹 设定
void MediaPlayerPreFolder(void);
//播放器 下一文件夹 设定
void MediaPlayerNextFolder(void);
void MediaPlayerTimeUpdate(void);
void MediaPlayerRepeatAB(void);
void MediaPlayerFastForward(void);
void MediaPlayerFastBackward(void);
void MediaPlayerFFFBEnd(void);
void MediaPlayerDecoderStop(void);
void MediaPlayerBackup(void);
bool MediaPlayerSongRefresh(void);
void MediaPlaySetDecoderState(DecoderState State);
DecoderState MediaPlayGetDecoderState(void);
uint8_t GetMediaPlayerState(void);
void SetMediaPlayerState(uint8_t state);
uint8_t GetMediaPlayMode(void);
void SetMediaPlayMode(uint8_t playmode);
void MediaPlayerBrowserEnter(void);
#if defined(FUNC_MATCH_PLAYER_BP) //&& (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN)) 
 void DiskSongSearchBPInit(void);
 void DiskSongSearchBP(DIR *dir, FILINFO *finfo, ff_acc_node *acc_node);
 uint32_t BPDiskSongNumGet(void);
 uint32_t BPDiskSongPlayTimeGet(void);
 uint32_t BPDiskSongPlayModeGet(void);
#endif
void MediaPlayerSongBrowserRefresh(void);
void MediaPlayerRepeatABClear(void);
void MediaPlayerTimerCB(void);
void MediaPlayerRepeatAB(void);


#endif /*__MEDIA_SERVICE_H__*/

