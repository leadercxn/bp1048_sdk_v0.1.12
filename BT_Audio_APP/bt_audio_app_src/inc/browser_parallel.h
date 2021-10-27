#ifndef __BROWSER_H__
#define __BROWSER_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

#ifdef FUNC_BROWSER_PARALLEL_EN

typedef enum
{

Browser_None=0,
Browser_Folder,
Browser_File,

}BrowserStep;

typedef enum
{

Browser_Play_None=0,
Browser_Play_Normal,
Browser_Play_BackGround,

}BrowserPlayState;


//void ShowFolderGui(char *current_vol,uint32_t start_folder_index,uint32_t end_folder_index,uint32_t dir_focusing,bool scan_if);
//void ShowFileGui(ff_dir_win *dir_win,uint32_t start_file_index,uint32_t end_file_index,uint32_t file_focusing,bool scan_if);
BrowserStep GetBrowserStep(void);
BrowserPlayState GetBrowserPlay_state(void);//0:not play 1:normal play 2:background play
void BrowserUp(BrowserStep BrowserProcessTemp);
void BrowserDn(BrowserStep BrowserProcessTemp);
void DecShowGuiTime(void);
void BrowserVarDefaultSet(void);
void BrowserMsgProcess(uint16_t msgId);
uint8_t GetShowGuiTime(void);

void EnterBrowserPlayMode(void);
void ExitBrowserPlayMode(void);

#endif
#ifdef __cplusplus
}
#endif//__cplusplus

#endif




