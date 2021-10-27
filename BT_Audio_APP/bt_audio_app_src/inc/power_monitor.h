//  maintainer: 
#ifndef __POWER_MONITOR_H__
#define __POWER_MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


#define  PWR_MNT_HIGH_V   0
#define  PWR_MNT_OFF_V    1
#define  PWR_MNT_LOW_V    2

typedef enum _PWR_LEVEL
{
	PWR_LEVEL_0 = 0,
	PWR_LEVEL_1,
	PWR_LEVEL_2,
	PWR_LEVEL_3,
	PWR_LEVEL_4		//max
	 
} PWR_LEVEL;

/*
**********************************************************
*					函数声明
**********************************************************
*/
//
//电能监视初始化
//实现系统启动过程中的低电压检测处理，以及配置充电设备接入检测IO等
//
void PowerMonitorInit(void);

//
//系统电源状态监控和处理
//系统启动后如果LDOIN大于开机电压，放在系统正常运行中监测LDOIN
//
uint8_t PowerMonitor(void);

//
//获取当前电池电量
//return: level(0-3)
//
PWR_LEVEL PowerLevelGet(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
