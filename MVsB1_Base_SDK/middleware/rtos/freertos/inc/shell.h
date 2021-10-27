
/**
* @addtogroup FreeRTOS
* @{
* @defgroup shell shell.h
* @{
*/

#ifndef MV_SHELL_H
#define MV_SHELL_H

typedef void (*shell_cmd_func) ( int32_t, char ** );
typedef struct CLICmds_st
{
        const char           *Cmd;
        shell_cmd_func       CmdHandler;
        const char           *CmdUsage;
} shell_cmds;

//call this funtion after uart init
extern void shell_init(void);

//APP must creat new task
extern void mv_shell_task(void * param);

#endif
