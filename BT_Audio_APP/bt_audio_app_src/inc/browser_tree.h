#ifndef _BROWSER_H_

#if defined(FUNC_BROWSER_TREE_EN)||defined(FUNC_SPECIFY_FOLDER_PLAY_EN)

#define _BROWSER_H_

#include "type.h"
#include "ff.h"
#include "ffpresearch.h"

//#define GUI_ROW_CNT_MAX 5
#ifndef FUNC_BROWSER_TREE_EN
#define TREE_DEPTH 1
#else
#define TREE_DEPTH MAX_DIR_STACK_DEPTH
#endif

typedef struct _dir_win_cnxt
{
	uint32_t up_num; //from 1 to N
	uint32_t down_num;
	uint32_t ptr_num;
	uint32_t item_num;
	bool is_bottom;
	DIR father;
	DWORD fher_offset;
	ff_entry_tree tree[GUI_ROW_CNT_MAX];//*2
} dir_tree_node_cnxt;

typedef struct _brows_win_cnxt
{
	dir_tree_node_cnxt llist[TREE_DEPTH];
	uint8_t depth;
	char *volume;
} brows_tree_cnxt;

typedef enum
{
  CODEC_MSG_BEGINNING,
  CODEC_MSG_DECODER_START,
  CODEC_MSG_DECODER_STOP,
  CODEC_MSG_DECODER_NEXT,
  CODEC_MSG_DECODER_PREV,
  CODEC_MSG_MAX
} codec_msg;


typedef enum
{

Browser_None=0,
Browser_FolderAndFile,

}BrowserStep;

typedef enum
{

Browser_Play_None=0,
Browser_Play_Normal,
Browser_Play_BackGround,

}BrowserPlayState;

typedef enum
{

ERGE=0,
GUSHI,
GUOXUE,
YINGYU,

}FolderPlayType;

typedef struct _StoryFolderInfo
{
	uint32_t IndexInFolder;// curremt song play number 
	uint32_t OffsetInFolder[2];// [0]:current song [1]:next song
	uint32_t FolderTotalSongs;
	
} StoryFolderInfo;



#define PLAY_MODE_ORDER			1
#define PLAY_MODE_REVERSED	2
#define PLAY_MODE_REPEAT		3
#define PLAY_MODE_ONCE			4

#define CYCLE_MODE_ALL			1
#define CYCLE_MODE_FOLDER		2
#define CYCLE_MODE_REPEAT		3//repeat one

void BrowserMsgProcess(uint16_t msgId);
//void EnterBrowserPlayMode(void);
//void ExitBrowserPlayMode(void);
uint8_t GetShowGuiTime(void);
void DecShowGuiTime(void);
void PlaySpecifyFolder(FolderPlayType folder_type);
void StoryVarInit(void);
bool PlayStorySongByIndex(void);
uint32_t GetSongNumberCurrent(void);
void DecSongNumber(void);
void ScanAllFolderToGetTotalSongs(void);
uint32_t GetTotalSongsInFolder(void);

uint32_t GetFolderStr(void);
#endif
#endif
