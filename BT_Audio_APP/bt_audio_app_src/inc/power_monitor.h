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
*					��������
**********************************************************
*/
//
//���ܼ��ӳ�ʼ��
//ʵ��ϵͳ���������еĵ͵�ѹ��⴦���Լ����ó���豸������IO��
//
void PowerMonitorInit(void);

//
//ϵͳ��Դ״̬��غʹ���
//ϵͳ���������LDOIN���ڿ�����ѹ������ϵͳ���������м��LDOIN
//
uint8_t PowerMonitor(void);

//
//��ȡ��ǰ��ص���
//return: level(0-3)
//
PWR_LEVEL PowerLevelGet(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
