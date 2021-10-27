/**
 *******************************************************************************
 * @file    ffpresearch.h
 * @author  Castle
 * @version V0.1.0
 * @date    24-August-2017
 * @brief
 * @maintainer Castle
 *******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */
/**
* @addtogroup FatFs
* @{
* @defgroup ffpresearch ffpresearch.h
* @{
*/
//#include "app_config.h"

#ifndef _FFPRESEARCH_H_

//#define FAST_LINE_BROWSER

#define FF_LFN_BUF_TREE_BROWSER 60
#define _FFPRESEARCH_H_

//#define FLASH_STORE_PNODE

#define MAX_DIR_STACK_DEPTH 9
#define MAX_ACC_NODE_SIZE 10
#define MAX_ACC_FLB_SIZE 128//can modify


/**
 * Refer to Audio Decoder Type Set
 */
typedef enum _FileType
{
	FILE_TYPE_UNKOWN = 0,
	FILE_TYPE_FLAC,
	FILE_TYPE_APE,
	FILE_TYPE_WAV,
	FILE_TYPE_AIF,
	FILE_TYPE_AMR,
	FILE_TYPE_AAC,
	FILE_TYPE_MP3,
	FILE_TYPE_WMA,
	FILE_TYPE_SBC,
	FILE_TYPE_MSBC,
} FileType;

typedef enum _EntryType
{
	ENTRY_TYPE_NONE,
	ENTRY_TYPE_DIR,
	ENTRY_TYPE_FILE,
	ENTRY_TYPE_MAX
} EntryType;

typedef struct /*__attribute__ ((packed))*/ _ff_presearch_node
{
	FATFS*	fs;
	WORD	id;
	DWORD	sclust;
	FSIZE_t	objsize;
	DWORD	dir_sect;
	BYTE*	dir_ptr;
	TCHAR	altname[13];
#if FF_FS_EXFAT
	DWORD	c_ofs;
	BYTE	stat;
#endif
	BYTE	file_type;
} ff_presearch_node;

typedef struct /*__attribute__ ((packed))*/ _ff_acc_node
{
	//llist_head list;//for chain
	bool is_used;
	DIR dirs_stack[MAX_DIR_STACK_DEPTH];
	uint32_t dirs_depth;
	uint32_t FirstFileIndex;//本层第一个歌曲的计数值
	uint32_t prior_files_amount;//包含当前文件的 之前文件总数，
	uint32_t prior_folder_amount;//包含当前文件夹的之前文件夹总数，
	uint32_t song_folder_amount;//包含有效文件的文件夹总数
} ff_acc_node;

#define FILE_NUM_LEN_INVALID	0xffff //FileNumLen 确认文件夹参数、首文件号，但未扫描结束统计。
typedef struct /*__attribute__ ((packed))*/ _ff_dir
{
	DIR		FatDir;//fatfs dir结构，
	uint16_t FolderIndex;//文件夹号。 应用层请勿赋值，由加速器提供。0:未配置，其他:参数有效,Len除外
	uint16_t FirstFileIndex;//本层文件夹第一个歌曲之前总数，含此文件 ，类似1开始计数，
	uint16_t FileNumLen; //同层歌曲数量，0:没有歌曲，FolderIndex为零时无意义，0xffff:尚未赋值。
} ff_dir;//文件夹和辅助参数 ,以1开始计数 避免0初值问题 ,区别于prior_files_amount num_dir等概念


typedef struct _ff_dir_win
{
	ff_dir dir_info;
	char long_name[FF_LFN_BUF_TREE_BROWSER];
	uint16_t valid_file_num;
	uint16_t first_file_index;//the number of all volume of the first file in this dir;
} ff_dir_win;

typedef struct _ff_file_win
{
	FIL file_hdr;
	char long_name[FF_LFN_BUF_TREE_BROWSER];
	DWORD offset;
} ff_file_win;

typedef union _entry_item
{
	DIR dir_hdr;
	FIL file_hdr;
} entry_item;

typedef struct _ff_entry_tree
{
	union _entry
	{
		DIR dir_hdr;
		FIL file_hdr;
	} entry;
	// entry_item entry;
	DWORD offset;
	EntryType type;
	char long_name[FF_LFN_BUF_TREE_BROWSER];//bkd
} ff_entry_tree;


#define MAX_ACC_RAM_SIZE (sizeof(ff_acc_node)*(MAX_ACC_NODE_SIZE))
#define MAX_ACC_FLB_RAM_SIZE (sizeof(ff_dir_win)*(MAX_ACC_FLB_SIZE))

//应用层核对文件信息，典型应用:断电记忆文件核对
typedef void (*ff_file_callback)(DIR *dir, FILINFO *finfo, ff_acc_node *acc_node);
//返回值为TRUE时，预搜索会过滤此文件夹，典型应用:加速器规避录音文件夹
typedef bool (*ff_folder_callback)(FILINFO *finfo);
//返回值为TRUE时，预搜索会强行返回，典型应用:scan过程中需要切换模式
typedef bool (*ff_scan_break_callback)(void);

//不启用回调检索和过滤时置NULL，加快预搜索速度
//void ffpresearch_init(FATFS *fs, ff_file_callback fcb, ff_folder_callback dcb);
void ffpresearch_init(FATFS *fs, ff_file_callback fcb, ff_folder_callback dcb, void *ram_ptr);
void ffpresearch_deinit(void);
FRESULT f_scan_vol(char *volume);

FRESULT f_open_by_num(char *volume, ff_dir *dirhandle, FIL *filehandle, uint32_t file_num, char *file_long_name);


FRESULT f_opendir_by_num(char *volume, ff_dir *dirhandle, uint32_t dir_num, char *dir_long_name);


UINT f_file_real_quantity(void);
UINT f_dir_real_quantity(void);
UINT f_dir_with_song_real_quantity(void);//总的有效目录数目
UINT f_dir_with_song_real_quantity_cur(void);//当前的有效目录号


FileType get_audio_type(TCHAR *string);

UINT f_file_count_in_dir(ff_dir *dirhandle);

FRESULT f_open_by_num_in_dir(ff_dir *dirhandle, UINT filenum,ff_file_win *file_win , uint8_t name_max_len);
FRESULT f_open_by_name_in_dir(ff_dir *dirhandle, FIL *filehandle, char *file_long_name);

FRESULT f_scan_dir_by_win(char *volume, uint32_t start_num, uint32_t stop_num, ff_dir_win *dir_win, uint8_t name_max_len, uint16_t *ret_len);
FRESULT f_scan_file_in_dir_by_win(ff_dir *dirhandle, uint32_t start_num, uint32_t stop_num, ff_file_win *file_win, uint8_t name_max_len, uint16_t *ret_len);
FRESULT f_find_next_in_dir(ff_dir *dirhandle, DWORD offset, ff_file_win *file_win, uint8_t name_max_len);
FRESULT f_find_prev_in_dir(ff_dir *dirhandle, DWORD offset, ff_file_win *file_win, uint8_t name_max_len);
FRESULT f_find_last_in_dir(ff_dir *dirhandle, DWORD offset, ff_file_win *file_win, uint8_t name_max_len);

FRESULT f_scan_dir_by_tree(DIR *dirhdr, uint32_t start_num, uint32_t stop_num, ff_entry_tree *entry_tree, uint8_t name_max_len, uint16_t *ret_len);
FRESULT f_get_file_num_by_path_chain(char *volume, DWORD *path_chain, uint8_t depth, FIL *file_hdr, uint32_t *file_num);
FRESULT f_get_dir_num_by_path_chain(char *volume, DWORD *path_chain, uint8_t depth, DIR *dir_hdr, uint32_t *dir_num);
FRESULT f_count_item_by_tree(DIR *dirhdr, uint32_t *file_count, uint32_t *dir_count);
uint32_t f_scan_totalfile_in_dir(DIR *dir_folder);

#ifdef FAST_LINE_BROWSER
void f_flb_init(ff_dir_win *ptr, int32_t item_max, int32_t lnf_max);
FRESULT f_scan_dir_by_win_fast(char *volume, uint32_t start_num, uint32_t stop_num, ff_dir_win *dir_win, uint8_t name_max_len, uint16_t *ret_len);
#endif

void ffpresearch_break_callback_set(ff_scan_break_callback bcb);
#endif

/**
 * @}
 * @}
 */
