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
	uint32_t FirstFileIndex;//�����һ�������ļ���ֵ
	uint32_t prior_files_amount;//������ǰ�ļ��� ֮ǰ�ļ�������
	uint32_t prior_folder_amount;//������ǰ�ļ��е�֮ǰ�ļ���������
	uint32_t song_folder_amount;//������Ч�ļ����ļ�������
} ff_acc_node;

#define FILE_NUM_LEN_INVALID	0xffff //FileNumLen ȷ���ļ��в��������ļ��ţ���δɨ�����ͳ�ơ�
typedef struct /*__attribute__ ((packed))*/ _ff_dir
{
	DIR		FatDir;//fatfs dir�ṹ��
	uint16_t FolderIndex;//�ļ��кš� Ӧ�ò�����ֵ���ɼ������ṩ��0:δ���ã�����:������Ч,Len����
	uint16_t FirstFileIndex;//�����ļ��е�һ������֮ǰ�����������ļ� ������1��ʼ������
	uint16_t FileNumLen; //ͬ�����������0:û�и�����FolderIndexΪ��ʱ�����壬0xffff:��δ��ֵ��
} ff_dir;//�ļ��к͸������� ,��1��ʼ���� ����0��ֵ���� ,������prior_files_amount num_dir�ȸ���


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

//Ӧ�ò�˶��ļ���Ϣ������Ӧ��:�ϵ�����ļ��˶�
typedef void (*ff_file_callback)(DIR *dir, FILINFO *finfo, ff_acc_node *acc_node);
//����ֵΪTRUEʱ��Ԥ��������˴��ļ��У�����Ӧ��:���������¼���ļ���
typedef bool (*ff_folder_callback)(FILINFO *finfo);
//����ֵΪTRUEʱ��Ԥ������ǿ�з��أ�����Ӧ��:scan��������Ҫ�л�ģʽ
typedef bool (*ff_scan_break_callback)(void);

//�����ûص������͹���ʱ��NULL���ӿ�Ԥ�����ٶ�
//void ffpresearch_init(FATFS *fs, ff_file_callback fcb, ff_folder_callback dcb);
void ffpresearch_init(FATFS *fs, ff_file_callback fcb, ff_folder_callback dcb, void *ram_ptr);
void ffpresearch_deinit(void);
FRESULT f_scan_vol(char *volume);

FRESULT f_open_by_num(char *volume, ff_dir *dirhandle, FIL *filehandle, uint32_t file_num, char *file_long_name);


FRESULT f_opendir_by_num(char *volume, ff_dir *dirhandle, uint32_t dir_num, char *dir_long_name);


UINT f_file_real_quantity(void);
UINT f_dir_real_quantity(void);
UINT f_dir_with_song_real_quantity(void);//�ܵ���ЧĿ¼��Ŀ
UINT f_dir_with_song_real_quantity_cur(void);//��ǰ����ЧĿ¼��


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
