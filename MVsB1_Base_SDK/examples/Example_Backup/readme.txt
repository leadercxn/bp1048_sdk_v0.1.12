Example_Backup������Ҫ��ʾ��powerkey�Ļ�������

1��Ӳ������Ҫ��:
	BP10ϵ�п����壬��Ӵ���С�壬TX/RX/GND ����A25/A24/GND ���� (������ 256000)��

3�����ʹ��˵��:
	powerkey�������������ã�
	#define POWERKEY_BYPASS                        0
	#define POWERKEY_SOFT                          1
	#define POWERKEY_SLIDE_HIGH_RUN_LOW_POWERDOWN  2
	#define POWERKEY_SLIDE_LOW_RUN_HIGH_POWERDOWN  3
	#define POWERKEY_SLIDE_EDGE                    4
	
	POWERKEY_BYPASS��powerkey���ܹرգ�
	POWERKEY_SOFT���ᴥ���أ�ϵͳ�ϵ��powerkey�������ã���Ч�󳤰��ᴥ���ؿ��Խ���powerkey�ػ��Ϳ�����
	POWERKEY_SLIDE_HIGH_RUN_LOW_POWERDOWN���������أ�LEVEL������ϵͳ�ϵ��powerkey�������ã��ߵ�ƽϵͳ���е͵�ƽϵͳ�ػ���
	POWERKEY_SLIDE_LOW_RUN_HIGH_POWERDOWN���������أ�LEVEL������ϵͳ�ϵ��powerkey�������ã��͵�ƽϵͳ���иߵ�ƽϵͳ�ػ���
	POWERKEY_SLIDE_EDGE���������أ�EDGE������ϵͳ�ϵ��powerkey�������ã���������ߵ�������Թػ��Ϳ�����
