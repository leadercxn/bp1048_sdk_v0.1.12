/**
 **************************************************************************************
 * @file    Browser.c
 * @brief   
 *
 * @author  Castle/BKD
 * @version V0.01
 *
 * $Created: 2019-9-17
 * 
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include "type.h"
#include "irqn.h"
#include "app_config.h"
#include "app_message.h"
#include "chip_info.h"
#include "dac.h"
#include "gpio.h"
#include "dma.h"
#include "dac.h"
#include "audio_adc.h"
#include "rtos_api.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "debug.h"
#include "audio_effect.h"
#include "main_task.h"
#include "audio_core_api.h"
#include "media_play_mode.h"
#include "decoder_service.h"
#include "audio_core_service.h"
#include "media_play_api.h"
#include "mode_switch_api.h"
#include "remind_sound_service.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "ctrlvars.h"
#include "audio_vol.h"
#include "timeout.h"
#include "ffpresearch.h"
#include "browser_tree.h"
#include "media_play_api.h"

extern char current_vol[8];
#ifdef FUNC_BROWSER_TREE_EN
static void ExitBrowserPlayMode(void);
extern MEDIA_PLAYER* gpMediaPlayer;
brows_tree_cnxt* g_brows_cnxt=NULL;
static BrowserStep BrowserProcess=Browser_None;
//static BrowserPlayState sBrowserPlayState=Browser_Play_None;//0:not play 1:normal play 2:background play
static uint8_t sShowGuiTime=0;
static uint32_t sTotalFileInOneLayer=0;
#define SAVE_INUM_BOTTOM(p,n)                       \
do                                                  \
{                                                   \
	p->item_num = (n == GUI_ROW_CNT_MAX+1)?(GUI_ROW_CNT_MAX):(n);     \
	if(n < GUI_ROW_CNT_MAX + 1)                               \
		p->is_bottom = TRUE;                            \
	else                                              \
		p->is_bottom = FALSE;                           \
}while(0)

static void brows_cnxt_init(brows_tree_cnxt *cnxt, char *volume)
{
	int i;
	memset(cnxt, 0x00, sizeof(brows_tree_cnxt));
	for(i=0; i<TREE_DEPTH-1; i++)
	{
		cnxt->llist[i].up_num = 1;
		cnxt->llist[i].down_num = GUI_ROW_CNT_MAX;
		cnxt->llist[i].is_bottom = FALSE;
	}
	cnxt->volume = volume;
	f_opendir(&(cnxt->llist[cnxt->depth].father), cnxt->volume);
}

static void brows_tree_node_init(dir_tree_node_cnxt *node)
{
	memset(node, 0x00, sizeof(dir_tree_node_cnxt));
	node->up_num = 1;
	node->down_num = GUI_ROW_CNT_MAX;
}

static void show_brows_tree_gui(brows_tree_cnxt *cnxt)
{
	uint8_t i;
	
	sShowGuiTime=4;
	dir_tree_node_cnxt *p = &(cnxt->llist[cnxt->depth]);
	APP_DBG("depth=%d, ptr_num=%ld\n", cnxt->depth, p->ptr_num);
	APP_DBG("------------------------GUI------------------------------\n");
	if(p->item_num==0)
		{
		APP_DBG("!!! No audio file for play ,Please press return key\n");
		APP_DBG("!!! No audio file for play ,Please press return key\n");
		}
	for(i=0; i < p->item_num; i++)
	{
		p->tree[i].long_name[FF_LFN_BUF_TREE_BROWSER-1]=0;
		APP_DBG("%s %s %s\n", (i==p->ptr_num)?(">>"):("  "), (p->tree[i].type==ENTRY_TYPE_FILE)?("SONG  "):("FOLDER"), p->tree[i].long_name);//
	}
	APP_DBG("------------------------GUI------------------------------\n");
}

static void brows_flash_screen(brows_tree_cnxt *cnxt)
{
	uint16_t found_num = 0;
	dir_tree_node_cnxt *p = &(cnxt->llist[cnxt->depth]);
	//uint32_t file_num,folder_num;

	f_scan_dir_by_tree(&(p->father), p->up_num, p->down_num + 1, &(p->tree[0]),FF_LFN_BUF_TREE_BROWSER,&found_num);
	//f_count_item_by_tree(&(p->father),&file_num,&folder_num);
	//APP_DBG("FIRST file num=%d,folder num=%d\n",file_num,folder_num);
	
	APP_DBG("Scan: %ld->%ld=%d\n", p->up_num, p->down_num + 1, found_num);
	p->item_num = (found_num == GUI_ROW_CNT_MAX+1)?(GUI_ROW_CNT_MAX):(found_num);
	if(found_num < GUI_ROW_CNT_MAX + 1)
		p->is_bottom = TRUE;

	show_brows_tree_gui(cnxt);
}

static void brows_move_up(brows_tree_cnxt *cnxt)
{
	uint16_t found_num = 0;
	dir_tree_node_cnxt *p = &(cnxt->llist[cnxt->depth]);
	// DBG("monitor: %d-->%d,%d\n", cnxt->dir_w.up_num, cnxt->dir_w.down_num, cnxt->dir_w.ptr_num);
	if(p->ptr_num == 0)
	{
		if(p->up_num > 1)
		{
			p->up_num--;
			p->down_num--;
			if(f_scan_dir_by_tree(&(p->father), p->up_num, p->down_num + 1, &(p->tree[0]),FF_LFN_BUF_TREE_BROWSER,&found_num) == FR_OK)
			{
				SAVE_INUM_BOTTOM(p,found_num);
			}
		}
		else
		{
			if(sTotalFileInOneLayer>GUI_ROW_CNT_MAX)
			{
				p->ptr_num=GUI_ROW_CNT_MAX-1;
				p->up_num=sTotalFileInOneLayer+1-GUI_ROW_CNT_MAX;
				p->down_num=sTotalFileInOneLayer;
				if(f_scan_dir_by_tree(&(p->father), p->up_num, p->down_num + 1, &(p->tree[0]),FF_LFN_BUF_TREE_BROWSER,&found_num) == FR_OK)
				{
					SAVE_INUM_BOTTOM(p,found_num);
				}
			}
			else
			{
				p->ptr_num=sTotalFileInOneLayer-1;
			}
		}
	}
	else
	{
		p->ptr_num --;
	}
	show_brows_tree_gui(cnxt);
}

/*
static void brows_move_pageup(brows_tree_cnxt *cnxt)
{
	uint16_t found_num = 0;
	dir_tree_node_cnxt *p = &(cnxt->llist[cnxt->depth]);
	// APP_DBG("monitor: %d-->%d,%d\n", cnxt->dir_w.up_num, cnxt->dir_w.down_num, cnxt->dir_w.ptr_num);
	if(p->up_num > GUI_ROW_CNT_MAX)
	{
		p->up_num -= GUI_ROW_CNT_MAX;
		p->down_num -= GUI_ROW_CNT_MAX;
	}
	else
	{
		p->up_num = 1;
		p->down_num = GUI_ROW_CNT_MAX;
	}

	if(f_scan_dir_by_tree(&(p->father), p->up_num, p->down_num + 1, &(p->tree[0]),FF_LFN_BUF_TREE_BROWSER,&found_num,) == FR_OK)
	{
		SAVE_INUM_BOTTOM(p, found_num);
		p->ptr_num = 0;
	}
	show_brows_tree_gui(cnxt);
}

*/
static void brows_move_down(brows_tree_cnxt *cnxt)
{
	uint16_t found_num = 0;
	dir_tree_node_cnxt *p = &(cnxt->llist[cnxt->depth]);
	// APP_DBG("monitor: %d-->%d,%d\n", cnxt->dir_w.up_num, cnxt->dir_w.down_num, cnxt->dir_w.ptr_num);
	if(p->ptr_num < p->item_num - 1)
	{
		p->ptr_num++;
	}
	else
	{
		if(!p->is_bottom)
		{
			p->up_num++;
			p->down_num++;
			if(f_scan_dir_by_tree(&(p->father), p->up_num, p->down_num + 1, &(p->tree[0]),FF_LFN_BUF_TREE_BROWSER,&found_num) == FR_OK)
			{
				SAVE_INUM_BOTTOM(p, found_num);
			}
			else
			{
				p->up_num--;
				p->down_num--;
			}
		}
        else
            {
            
                p->ptr_num=0;
                p->up_num=1;
                p->down_num=GUI_ROW_CNT_MAX;
                if(f_scan_dir_by_tree(&(p->father), p->up_num, p->down_num + 1, &(p->tree[0]),FF_LFN_BUF_TREE_BROWSER,&found_num) == FR_OK)
			{
				SAVE_INUM_BOTTOM(p, found_num);
			}
            
            }
	}
	show_brows_tree_gui(cnxt);

}
/*
static void brows_move_pagedown(brows_tree_cnxt *cnxt)
{
	uint16_t found_num = 0;
	dir_tree_node_cnxt *p = &(cnxt->llist[cnxt->depth]);
	// APP_DBG("monitor: %d-->%d,%d\n", cnxt->dir_w.up_num, cnxt->dir_w.down_num, cnxt->dir_w.ptr_num);

	if(!p->is_bottom)
	{
		p->up_num += GUI_ROW_CNT_MAX;
		p->down_num += GUI_ROW_CNT_MAX;
		if(f_scan_dir_by_tree(&(p->father), p->up_num, p->down_num + 1, &(p->tree[0]), FF_LFN_BUF_TREE_BROWSER,&found_num) == FR_OK)
		{
			SAVE_INUM_BOTTOM(p, found_num);
			p->ptr_num = 0;
		}
	}
	show_brows_tree_gui(cnxt);
}
*/

//char tlname[FF_LFN_BUF];
//FIL f_test;
//ff_dir d_test;
static uint8_t brows_form_path_chain(brows_tree_cnxt *cnxt, DWORD *pchain)
{
	int i;//, j;
	dir_tree_node_cnxt *p = &(cnxt->llist[cnxt->depth]);

	//path stack offset chain
	for(i = 1; i < cnxt->depth + 1; i++)
	{
		dir_tree_node_cnxt *qq = &(cnxt->llist[i]);
		// APP_DBG("%08X  ", qq->fher_offset);
		pchain[i-1] = qq->fher_offset;
	}
	// APP_DBG("%08X \n", p->tree[p->ptr_num].offset);
	pchain[cnxt->depth] = p->tree[p->ptr_num].offset;//file's offset
	APP_DBG("depth=%d, ", cnxt->depth+1);
	for(i=0; i<cnxt->depth+1; i++)
	{
		APP_DBG("%08lX  ", pchain[i]);
	}
	APP_DBG("\n");
	return cnxt->depth+1;
}

static bool brows_enter(brows_tree_cnxt *cnxt)
{
	uint16_t found_num = 0;
	dir_tree_node_cnxt *p = &(cnxt->llist[cnxt->depth]);
	uint32_t file_num,folder_num;
	if(p->tree[p->ptr_num].type == ENTRY_TYPE_DIR)
	{
		if(cnxt->depth < TREE_DEPTH-1)
		{
			dir_tree_node_cnxt *q = &(cnxt->llist[cnxt->depth + 1]);
			brows_tree_node_init(q);
			memcpy(&(q->father), &(p->tree[p->ptr_num].entry.dir_hdr), sizeof(DIR));
			q->fher_offset = p->tree[p->ptr_num].offset;
			cnxt->depth++;

			if(f_scan_dir_by_tree(&(q->father), q->up_num, q->down_num + 1, &(q->tree[0]),FF_LFN_BUF_TREE_BROWSER,&found_num) == FR_OK)
			{
				f_count_item_by_tree(&(q->father),&file_num,&folder_num);
                sTotalFileInOneLayer=file_num+folder_num;
				//APP_DBG("Here file num=%d,folder num=%d Total=%d\n",file_num,folder_num,sTotalFileInOneLayer);
				SAVE_INUM_BOTTOM(q, found_num);
				q->ptr_num = 0;
			}
			
		}
		show_brows_tree_gui(cnxt);
	}
	else if(p->tree[p->ptr_num].type == ENTRY_TYPE_FILE)// here exit browser after play song
	{
		uint8_t chain_len;
		uint32_t file_num;
		DWORD path_chain[TREE_DEPTH];

		FIL *f_hdr = &(p->tree[p->ptr_num].entry.file_hdr);
		// APP_DBG("name: %s\n", p->tree[p->ptr_num].long_name);
		chain_len = brows_form_path_chain(cnxt, path_chain);
		//APP_DBG("\nnode index: %d\n", search_acc_by_path_chain(path_chain, cnxt->depth+1));
		// APP_DBG("file clust=%X\n", f_hdr->obj.sclust);
		if(f_get_file_num_by_path_chain(cnxt->volume, path_chain, chain_len, f_hdr, &file_num) == FR_OK)
		{
			APP_DBG("file number: %ld\n", file_num);
			
			gpMediaPlayer->CurFileIndex=file_num;
			MediaPlayerBrowserEnter();
			// if(f_open_by_num(cnxt->volume, &d_test, &f_test, file_num, tlname) == FR_OK)
			// {
			// 	APP_DBG("file name: %s\n", tlname);
			// }
		}

		ExitBrowserPlayMode();
		
	}

	return TRUE;
}

static void brows_return(brows_tree_cnxt *cnxt)
{
	uint32_t file_num,folder_num;
	if(cnxt->depth > 0)
	{
		cnxt->depth--;
	}
	dir_tree_node_cnxt *p = &(cnxt->llist[cnxt->depth]);
	f_count_item_by_tree(&(p->father),&file_num,&folder_num);
	sTotalFileInOneLayer=file_num+folder_num;

	show_brows_tree_gui(cnxt);
}


static void BrowserClearScreen(void)//user defifine by himself,here only for demo
{

	APP_DBG("clear file and folder gui \n");
 	
}
static void EnterBrowserPlayMode(void)
{
	//BrowserVarDefaultSet();
	//ShowBrowserHomeGui();
	dir_tree_node_cnxt *p = NULL;
	uint32_t file_num,folder_num;

	g_brows_cnxt = (brows_tree_cnxt*)osPortMallocFromEnd(sizeof(brows_tree_cnxt));//
	if(g_brows_cnxt!=NULL)
		{
		memset(g_brows_cnxt, 0, sizeof(brows_tree_cnxt));
		}
	else
		{
		APP_DBG("browser memory not enough\n");
		return;
		}
    
	
        brows_cnxt_init(g_brows_cnxt, current_vol);
        p = &(g_brows_cnxt->llist[g_brows_cnxt->depth]);
        f_count_item_by_tree(&(p->father),&file_num,&folder_num);
        sTotalFileInOneLayer=file_num+folder_num;
		//APP_DBG("Here file num=%d,folder num=%d Total=%d\n",file_num,folder_num,sTotalFileInOneLayer);
        brows_flash_screen(g_brows_cnxt);
	//SetMediaPlayMode(PLAY_MODE_BROWSER);
	BrowserProcess=Browser_FolderAndFile;

}
static void ExitBrowserPlayMode(void)
{
	sShowGuiTime=0;
	BrowserClearScreen();
	BrowserProcess=Browser_None;						
	//sBrowserPlayState=Browser_Play_None;
	//SetMediaPlayMode(PLAY_MODE_REPEAT_ALL);

	osPortFree(g_brows_cnxt);
	g_brows_cnxt = NULL;
}

uint8_t GetShowGuiTime(void)
{
	return sShowGuiTime;
}
void DecShowGuiTime(void)
{
	if(sShowGuiTime)
		sShowGuiTime--;
}

/*
BrowserPlayState GetBrowserPlay_state(void)//0:not play 1:normal play 2:background play
{
	return sBrowserPlayState;
}
*/

void BrowserMsgProcess(uint16_t msgId)
{
	switch(BrowserProcess)
		{
		case Browser_None:
			{
			if(msgId==MSG_BROWSE)
				{
				APP_DBG("MSG_MEDIA_PLAY_BROWER_SWITCH\n");
				EnterBrowserPlayMode();
				}
			}
		break;
		case Browser_FolderAndFile:
			{
			switch(msgId)
				{
				case MSG_MEDIA_PLAY_BROWER_UP:
				brows_move_up(g_brows_cnxt);

				break;
				case MSG_MEDIA_PLAY_BROWER_DN:
				brows_move_down(g_brows_cnxt);
				break;
				case MSG_MEDIA_PLAY_BROWER_ENTER:
				brows_enter(g_brows_cnxt);
				break;
				case MSG_MEDIA_PLAY_BROWER_RETURN:
				brows_return(g_brows_cnxt);
				break;
				case MSG_BROWSE:
				ExitBrowserPlayMode();	
				break;
				
				}
			}
		default:
			break;
	}	
}

#endif



#ifdef FUNC_SPECIFY_FOLDER_PLAY_EN
//const char dir_gushi[20]="¹ÊÊÂ";
#define DIR_ERGE       "¶ù¸è"
#define DIR_GUSHI     "¹ÊÊÂ"
#define DIR_GUOXUE  "¹úÑ§"
#define DIR_YINGYU   "Ó¢Óï"
#define FOLDER_TYPE_MAX 4
uint8_t FolderStr[FOLDER_TYPE_MAX][10]={{"¶ù¸è"},{"¹ÊÊÂ"},{"¹úÑ§"},{"Ó¢Óï"}};
StoryFolderInfo StoryInformation[FOLDER_TYPE_MAX];


DIR gStory_Dir;
extern uint8_t	file_longname[FF_LFN_BUF + 1];
static FolderPlayType FolderType=ERGE;
extern void MediaPlayerStoryEnter(void);

void StoryVarInit(void)
{
	memset(&StoryInformation[0],0,(sizeof(StoryFolderInfo))*4);
	FolderType=ERGE;
	StoryInformation[ERGE].IndexInFolder=1;
	StoryInformation[ERGE].OffsetInFolder[0]=0;//Current
	StoryInformation[ERGE].OffsetInFolder[1]=0;//next
	
	StoryInformation[GUSHI].IndexInFolder=1;
	StoryInformation[GUSHI].OffsetInFolder[0]=0;
	StoryInformation[GUSHI].OffsetInFolder[1]=0;
	
	StoryInformation[GUOXUE].IndexInFolder=1;
	StoryInformation[GUOXUE].OffsetInFolder[0]=0;
	StoryInformation[GUOXUE].OffsetInFolder[1]=0;
	
	StoryInformation[YINGYU].IndexInFolder=1;
	StoryInformation[YINGYU].OffsetInFolder[0]=0;
	StoryInformation[YINGYU].OffsetInFolder[1]=0;
	
}
void PlaySpecifyFolder(FolderPlayType folder_type)
{
	SetMediaPlayMode(PLAY_MODE_REPEAT_FOLDER);
	MediaPlayerStoryEnter();
	FolderType=folder_type;

}
void FindFileHdrByDir(uint32_t Array_index)// Array_index-0:Current ,1:next
{
        char FilePath[20];
	
        memset(FilePath,0,20);
        strcpy(FilePath, current_vol);
        switch(FolderType)
        {
            case ERGE:
                strcat(FilePath, DIR_ERGE);
            break;
            case GUSHI:
                strcat(FilePath, DIR_GUSHI);
            break;
            case GUOXUE:
                strcat(FilePath, DIR_GUOXUE);
            break;
            case YINGYU:
                strcat(FilePath, DIR_YINGYU);
            break;
            default:
                 strcat(FilePath, DIR_ERGE);
            break;
        }

            {
				
			ff_dir dirhandle;
			ff_file_win file_win;
			//nt32_t file_total_numbers;
			f_opendir(&gStory_Dir, FilePath);
			//le_total_numbers=f_scan_totalfile_in_dir(&gStory_Dir);
			//G("file_total_numbers=%d\n",file_total_numbers);
			
			search_again:
			memset(&dirhandle, 0, sizeof(ff_dir));
			memcpy(&(dirhandle.FatDir),&gStory_Dir,sizeof(DIR));
			if(f_find_next_in_dir(&dirhandle,StoryInformation[FolderType].OffsetInFolder[Array_index],&file_win,FF_LFN_BUF_TREE_BROWSER))
			{
				StoryInformation[FolderType].OffsetInFolder[Array_index]=0;
				StoryInformation[FolderType].IndexInFolder=0;
				vTaskDelay(1);
				APP_DBG("not find or error ,search again ");
				goto search_again;
            }
			memcpy(&gpMediaPlayer->PlayerFile,&file_win.file_hdr,sizeof(FIL));
			memcpy(&file_longname,&file_win.long_name,FF_LFN_BUF_TREE_BROWSER);
			file_win.long_name[59]=0;
			if(Array_index==1)//next song
			{
				StoryInformation[FolderType].OffsetInFolder[0]=StoryInformation[FolderType].OffsetInFolder[1];//Current
				StoryInformation[FolderType].OffsetInFolder[1]=file_win.offset;//next
				StoryInformation[FolderType].IndexInFolder++;
			}
			else
			{
				StoryInformation[FolderType].OffsetInFolder[1]=file_win.offset;//next
			}
			//APP_DBG("hhhhhhhhhhhhhh %s %s bb=%d\n",gpMediaPlayer->PlayerFile.fn,file_longname,file_win.offset);
    
       }


}


bool PlayStorySongByIndex(void)//f_open_by_num_in_dir
{
        char FilePath[20];

        memset(FilePath,0,20);
        strcpy(FilePath, current_vol);
		
		SetMediaPlayMode(PLAY_MODE_REPEAT_FOLDER);
	   switch(FolderType)
            {
                case ERGE:
                    strcat(FilePath, DIR_ERGE);
                break;
                case GUSHI:
                    strcat(FilePath, DIR_GUSHI);
                break;
                case GUOXUE:
                    strcat(FilePath, DIR_GUOXUE);
                break;
                case YINGYU:
                    strcat(FilePath, DIR_YINGYU);
                break;
                default:
                     strcat(FilePath, DIR_ERGE);
                break;
            }

		{
			   
		   ff_dir dirhandle;
		   ff_file_win file_win;
		   f_opendir(&gStory_Dir, FilePath);
		   memset(&dirhandle, 0, sizeof(ff_dir));
		   memcpy(&(dirhandle.FatDir),&gStory_Dir,sizeof(DIR));// get the total NUMber
		   if(StoryInformation[FolderType].IndexInFolder>1)
		   {
			   if(f_open_by_num_in_dir(&dirhandle,StoryInformation[FolderType].IndexInFolder-1,&file_win,FF_LFN_BUF_TREE_BROWSER))// longname have problem
			   		return FALSE;
				StoryInformation[FolderType].OffsetInFolder[0]=file_win.offset;
				APP_DBG("search 1\n");
				//f_opendir(&gStory_Dir, FilePath);
				memset(&dirhandle, 0, sizeof(ff_dir));
				memcpy(&(dirhandle.FatDir),&gStory_Dir,sizeof(DIR));// get the total NUMber
			   
		}
	else
		   	 StoryInformation[FolderType].OffsetInFolder[0]=0;
		   
		   if(f_open_by_num_in_dir(&dirhandle,StoryInformation[FolderType].IndexInFolder,&file_win,FF_LFN_BUF_TREE_BROWSER))// longname have problem
		   		return FALSE;
			  
		   memcpy(&gpMediaPlayer->PlayerFile,&file_win.file_hdr,sizeof(FIL));
		   memcpy(&file_longname,&file_win.long_name,FF_LFN_BUF_TREE_BROWSER);
		   file_win.long_name[59]=0;
		  
		   StoryInformation[FolderType].OffsetInFolder[1]=file_win.offset;//next
		   
		   //APP_DBG("hhhhhhhhhhhhhh %s %s bb=%d\n",gpMediaPlayer->PlayerFile.fn,file_longname,file_win.offset);
			   
		}
    

	return TRUE;
}
void ScanAllFolderToGetTotalSongs(void)
{
	char FilePath[20];
	
	memset(FilePath,0,20);
	strcpy(FilePath, current_vol);
	strcat(FilePath, DIR_ERGE);
	f_opendir(&gStory_Dir, FilePath);
	StoryInformation[ERGE].FolderTotalSongs=f_scan_totalfile_in_dir(&gStory_Dir);

	memset(FilePath,0,20);
	strcpy(FilePath, current_vol);
	strcat(FilePath, DIR_GUSHI);
	f_opendir(&gStory_Dir, FilePath);
	StoryInformation[GUSHI].FolderTotalSongs=f_scan_totalfile_in_dir(&gStory_Dir);

	memset(FilePath,0,20);
	strcpy(FilePath, current_vol);
	strcat(FilePath, DIR_GUOXUE);
	f_opendir(&gStory_Dir, FilePath);
	StoryInformation[GUOXUE].FolderTotalSongs=f_scan_totalfile_in_dir(&gStory_Dir);

	memset(FilePath,0,20);
	strcpy(FilePath, current_vol);
	strcat(FilePath, DIR_YINGYU);
	f_opendir(&gStory_Dir, FilePath);
	StoryInformation[YINGYU].FolderTotalSongs=f_scan_totalfile_in_dir(&gStory_Dir);

	APP_DBG("ERGE = %d GUSHI = %d GUOXUE = %d YINGYU = %d\n",StoryInformation[0].FolderTotalSongs,\
		StoryInformation[1].FolderTotalSongs,StoryInformation[2].FolderTotalSongs,StoryInformation[3].FolderTotalSongs
		);
	
	//le_total_numbers=f_scan_totalfile_in_dir(&gStory_Dir);
	//G("file_total_numbers=%d\n",file_total_numbers);

}

uint32_t GetSongNumberCurrent(void)
{
	return StoryInformation[FolderType].IndexInFolder;
}

uint32_t GetTotalSongsInFolder(void)
{
	return StoryInformation[FolderType].FolderTotalSongs;
}
uint32_t GetFolderStr(void)
{
	return (uint8_t *)(&FolderStr[FolderType][0]);
}
void DecSongNumber(void)
{

if(StoryInformation[FolderType].IndexInFolder<=1)
	StoryInformation[FolderType].IndexInFolder=StoryInformation[FolderType].FolderTotalSongs;
else
	StoryInformation[FolderType].IndexInFolder--;
}


#endif

