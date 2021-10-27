Example_Backup工程主要演示了powerkey的基本操作

1、硬件环境要求:
	BP10系列开发板，外接串口小板，TX/RX/GND 接至A25/A24/GND 引脚 (波特率 256000)；

3、软件使用说明:
	powerkey可以做如下配置：
	#define POWERKEY_BYPASS                        0
	#define POWERKEY_SOFT                          1
	#define POWERKEY_SLIDE_HIGH_RUN_LOW_POWERDOWN  2
	#define POWERKEY_SLIDE_LOW_RUN_HIGH_POWERDOWN  3
	#define POWERKEY_SLIDE_EDGE                    4
	
	POWERKEY_BYPASS：powerkey功能关闭；
	POWERKEY_SOFT：轻触开关；系统上电对powerkey进行配置；生效后长按轻触开关可以进行powerkey关机和开机；
	POWERKEY_SLIDE_HIGH_RUN_LOW_POWERDOWN：拨动开关；LEVEL触发；系统上电对powerkey进行配置；高电平系统运行低电平系统关机；
	POWERKEY_SLIDE_LOW_RUN_HIGH_POWERDOWN：拨动开关；LEVEL触发；系统上电对powerkey进行配置；低电平系统运行高电平系统关机；
	POWERKEY_SLIDE_EDGE：拨动开关；EDGE触发；系统上电对powerkey进行配置；高脉冲或者低脉冲可以关机和开机；
