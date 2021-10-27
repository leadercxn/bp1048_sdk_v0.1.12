#include <stdio.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "type.h"
#include "uarts_interface.h"
#include "debug.h"
#include "irqn.h"
#include "FreeRTOS.h"
#include "task.h"
#include "shell.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif
//////////////////////////////////////////////////////////////
//shell功能开关和配置
//shell功能要启用时请打开如下宏 CFG_FUNC_SHELL_EN
//UART1和UART0 二选一，默认使用UART1
//另外注意和系统UART端口使用上是否有冲突
//////////////////////////////////////////////////////////////
//#define CFG_FUNC_SHELL_EN
#ifdef CFG_FUNC_SHELL_EN
#define CFG_PARA_SHELL_COM1
//#define CFG_PARA_SHELL_COM0
#endif

#ifdef CFG_FUNC_SHELL_EN

#define BUFSIZE		32
uint8_t	UARTBuffer[BUFSIZE];
uint32_t	gRxQueueHead = 0;
uint32_t	gRxQueueTail = 0;

#define CMD_ARG_MAX_COUNT	3
#define ONE_ARG_MAX_LEN		10
char cmdbuffer[CMD_ARG_MAX_COUNT][ONE_ARG_MAX_LEN];

//多出ONE_ARG_MAX_LEN,目的有参数之间的间隔' '，还有cmd长度可以大于ONE_ARG_MAX_LEN,因此多出ONE_ARG_MAX_LEN长度预留
uint8_t shellbuffer[(CMD_ARG_MAX_COUNT+1)*ONE_ARG_MAX_LEN];

const int num_tab[10] = {	1000000000,
							100000000,
							10000000,
							1000000,
							100000,
							10000,
							1000,
							100,
							10,
							1};


/**
 * @func        char_to_num
 * @brief       Bcd字符串转换成数值
 * @param       char *buffer：不超过10个数字字符
 * @Output      None
 * @return      int
 * @Others      比如“1234567890” 返回1234567890数值。
 * Record
 */
int char_to_num(char *buffer)
{
    int i;
    int templen;
    int sum = 0;
    templen = strlen(buffer);
    if(templen > 10)
    {
        DBG("CMD ERROR\n");
        return -1;
    }
    for(i=0;i<templen;i++)
    {
        buffer[i] -= 0x30;
        if((buffer[i]>=0)&&(buffer[i]<=9))
        {
        	//DBG("%x",buffer[i]);
        }
        else
        {
        	DBG("ERROR");
            return -1;
        }
    }
    int j = 0;
    for(i=0;i<templen;i++)
    {
        sum += buffer[i]*num_tab[10 - templen + j];
        j++;
    }
    return sum;
}

uint8_t shell_getchar(void)
{
	if(gRxQueueHead != gRxQueueTail)//非空
	{
		unsigned char	val;
		val = UARTBuffer[gRxQueueHead];
		gRxQueueHead = ((gRxQueueHead + 1) % BUFSIZE);
		return val;
	}
	return 0;
}
#ifdef CFG_PARA_SHELL_COM0
void UART0_Interrupt(void)
{
	if(UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXSTAT_GET, 1) & 0x01) { //接收中断
		int i;
		uint8_t Temp[0];
		for(i=0;i<4;i++)
		{
			if(UARTS_RecvByte(UART_PORT0,Temp) == 0)
			{
				break;
			}
			else
			{
				UARTBuffer[gRxQueueTail] = Temp[0];
				if (((gRxQueueTail + 1) % BUFSIZE) != gRxQueueHead)		//queue is not full
				{
					gRxQueueTail = ((gRxQueueTail + 1) % BUFSIZE);			//queue tail++
				}
			}
		}
		UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXINT_CLR, 1);
	}
    if(UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXSTAT_GET, 0) & 0x3C) { //错误中断
        //kprintf("Uart0 RX error(0x%02X)\n", (uint32_t)(UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXSTAT_GET, 0) & 0x3C));
        UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXINT_CLR, 1);
    }

    if(UARTS_IOCTL(UART_PORT0, UART_IOCTL_TXSTAT_GET, 0)) { //发送中断
        UARTS_IOCTL(UART_PORT0, UART_IOCTL_TXINT_CLR, 1);
    }
}

#else
void UART1_Interrupt(void)
{
	if(UARTS_IOCTL(UART_PORT1, UART_IOCTL_RXSTAT_GET, 1) & 0x01) { //接收中断
		int i;
		uint8_t Temp[0];
		for(i=0;i<4;i++)
		{
			if(UARTS_RecvByte(UART_PORT1,Temp) == 0)
			{
				break;
			}
			else
			{
				UARTBuffer[gRxQueueTail] = Temp[0];
				if (((gRxQueueTail + 1) % BUFSIZE) != gRxQueueHead)		//queue is not full
				{
					gRxQueueTail = ((gRxQueueTail + 1) % BUFSIZE);			//queue tail++
				}
			}
		}
		UARTS_IOCTL(UART_PORT1, UART_IOCTL_RXINT_CLR, 1);
	}
    if(UARTS_IOCTL(UART_PORT1, UART_IOCTL_RXSTAT_GET, 0) & 0x3C) { //错误中断
        //kprintf("Uart0 RX error(0x%02X)\n", (uint32_t)(UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXSTAT_GET, 0) & 0x3C));
        UARTS_IOCTL(UART_PORT1, UART_IOCTL_RXINT_CLR, 1);
    }

    if(UARTS_IOCTL(UART_PORT1, UART_IOCTL_TXSTAT_GET, 0)) { //发送中断
        UARTS_IOCTL(UART_PORT1, UART_IOCTL_TXINT_CLR, 1);
    }
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cmd_test(int argc, char *argv[]);
static void DisplayMemInf(void);
static void DisplayRunTimeStatesInf(void);
static void DisplayTaskInf(void);
static void Displayhelp(void);
extern void clear_net_cfg(void);
extern void creat_wifi_connect_task(void);
extern void edp_task_start();
extern void mqtt_run();
extern void DisplayMcpsInfo(void);
shell_cmds mv_shell[] =
{
    { "test",   				(shell_cmd_func)cmd_test,       			"for test"},
    { "displaytaskinfo",   		(shell_cmd_func)DisplayTaskInf,       		"show task info"},
    { "displayruntimestats",   	(shell_cmd_func)DisplayRunTimeStatesInf,	"show run time states"},
    { "DisplayMemInf",   		(shell_cmd_func)DisplayMemInf,				"show heap memory size"},
#ifdef use_MCPS_ANALYSIS
	{ "DisplayMcpsInf",   		(shell_cmd_func)DisplayMcpsInfo,			"show task mcps info"},
#endif
    { "help",   				(shell_cmd_func)Displayhelp,				"help"},
    { NULL, NULL, NULL },
};

void cmd_test(int argc, char *argv[])
{
	int i;
	for(i=0;i<argc;i++)
	{
		if(i==0)
		{
			DBG("%s\n",argv[i]);
		}
		else
		{
			DBG("%d\n",char_to_num(argv[i]));
		}
	}
}


//static
void DisplayMemInf(void)
{
	DBG("\nRemainingMemoryInBytes: %d\r\n",(int)xPortGetFreeHeapSize());
}

static void DisplayRunTimeStatesInf(void)
{
	uint8_t *buf= pvPortMalloc(4096);
	memset(buf,0,4096);
	vTaskGetRunTimeStats((char*)buf);
	DBG("\nTask           \tAbs Time\tPercent\r\n****************************************\r\n");
	DBG("%s\n",buf);
	vPortFree(buf);
}

static void DisplayTaskInf(void)
{
	uint8_t *buf= pvPortMalloc(4096);
	memset(buf,0,4096);
	vTaskList((char*)buf);
	DBG("\nTask           State  Prio+3  FreeStack(word)  PID\n***************************************************\r\n");
	DBG("%s\n",buf);
	vPortFree(buf);
}


static void Displayhelp(void)
{
	int i=0;
	DBG("\n***************************************************\nmv shell cmd list:\n");
	DBG("\tcmd\t\thelpinfo\t\n");
	while(1)
	{
		if(shellbuffer != NULL  && mv_shell[i].Cmd == NULL)
		{
			break;
		}
		DBG("\t%s\t\t%s\t\n",mv_shell[i].Cmd,mv_shell[i].CmdUsage);
		i++;
	}
	DBG("cmd example:test 123 456\n");
	DBG("***************************************************\n");
}


void cmd_proess()
{
	int i = 0;
	int j = 0;
	uint k = 0;
	int ii = 0;
	int argc=0;
	char *argv[CMD_ARG_MAX_COUNT];
	char temp[ONE_ARG_MAX_LEN];
	while(1)
	{
		if(mv_shell[i].Cmd == NULL)
		{
			if(j==0)
			{
				DBG("未知命令\n");
			}
			return;
		}
		if(memcmp(shellbuffer,mv_shell[i].Cmd,strlen(mv_shell[i].Cmd)) == 0)
		{
			argc=0;
			ii = 0;
			memset(temp,0,sizeof(temp));
			for(k=0;k<strlen((char*)shellbuffer);k++)
			{
				if((shellbuffer[k] == 0x20)||(shellbuffer[k] == 0x0D))
				{
					memcpy(cmdbuffer+argc,temp,sizeof(temp));
					memset(temp,0,sizeof(temp));
					argv[argc] = (char*)(cmdbuffer+argc);
					ii = 0;
					argc++;
					if(argc == CMD_ARG_MAX_COUNT)
					{
						break;
					}
				}
				else
				{
					temp[ii] = shellbuffer[k];
					ii++;
					if(argc >= ONE_ARG_MAX_LEN)
					{
						ii=10;
					}
				}
			}
			(mv_shell[i].CmdHandler)(argc,argv);
			j++;
		}
		i++;
	}
}


void shell_init(void)
{
#ifdef CFG_PARA_SHELL_COM0
    UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXFIFO_CLR, 1);
    UARTS_IOCTL(UART_PORT0, UART_IOCTL_TXFIFO_CLR, 1);
    UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXINT_SET, 1);
    UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXFIFO_TRGRDEP_SET, 0);
    memset(UARTBuffer,0,sizeof(UARTBuffer));
    NVIC_EnableIRQ(UART0_IRQn);
    NVIC_SetPriority(UART0_IRQn,2);
#else
    UARTS_IOCTL(UART_PORT1, UART_IOCTL_RXFIFO_CLR, 1);
    UARTS_IOCTL(UART_PORT1, UART_IOCTL_TXFIFO_CLR, 1);
    UARTS_IOCTL(UART_PORT1, UART_IOCTL_RXINT_SET, 1);
    UARTS_IOCTL(UART_PORT1, UART_IOCTL_RXFIFO_TRGRDEP_SET, 0);
    memset(UARTBuffer,0,sizeof(UARTBuffer));
    NVIC_EnableIRQ(UART1_IRQn);
    NVIC_SetPriority(UART1_IRQn,2);
#endif
}


void mv_shell_task(void * param)
{
	uint8_t count = 0;
	uint8_t cmdstr;
	DBG("mv_shell_task START\n");
	while(1)
	{
		cmdstr = shell_getchar();
		if(cmdstr)
		{
			DBG("%c",cmdstr);
			shellbuffer[count] = cmdstr;
			if((cmdstr == 0x0A)||(cmdstr == 0x0D))
			{
				DBG("\n");
				cmd_proess();
				count = 0;
				memset(shellbuffer,0,100);
			}
			else
			{
				count++;
			}
		}
		//vTaskDelay(30);

	}
}

#endif //CFG_FUNC_SHELL_EN


