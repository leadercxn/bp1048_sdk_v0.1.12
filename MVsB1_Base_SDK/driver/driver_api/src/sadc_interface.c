#include <string.h>
#include "type.h"
#include "adc.h"
#include "clk.h"
#include "backup.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#else
#define CFG_RES_POWERKEY_ADC_EN
#endif
void SarADC_Init(void)
{
	Clock_Module1Enable(ADC_CLK_EN);
	
#ifdef  CFG_RES_POWERKEY_ADC_EN
	//ʹ��PowerKey ����ADC��������
	ADC_PowerkeyChannelEnable();
	BACKUP_VBK22KMode();//PowerKey����������������Ϊ22K
#endif

	ADC_Enable();
	ADC_ClockDivSet(CLK_DIV_128);//30M / 8
	ADC_VrefSet(ADC_VREF_VDDA);//1:VDDA; 0:VDD
	ADC_ModeSet(ADC_CON_SINGLE);

	ADC_Calibration();//�ϵ�У׼һ�μ���
}

//�˴�δ����β���ȡƽ������
//ԭ��ADC������ȡ���������ʽ�������˲���ϵͳ����Ӱ��
//�������״̬����ʽ��Ӧ�ò������
int16_t SarADC_LDOINVolGet(void)
{
	uint32_t DC_Data1;
	uint32_t DC_Data2;

	DC_Data1 = ADC_SingleModeDataGet(ADC_CHANNEL_VIN);
	DC_Data2 = ADC_SingleModeDataGet(ADC_CHANNEL_VDD1V2);
	DC_Data1 = (DC_Data1 * 2 * 1200) / DC_Data2;
	if(DC_Data1 < 3200)
	{
		DC_Data1 += 30;//��ѹ����3.3V֮�󣬲�������ֵƫ��
	}
	//DBG("LDOIN �� %d\n", DC_Data1);

	return (int16_t)DC_Data1;
}

