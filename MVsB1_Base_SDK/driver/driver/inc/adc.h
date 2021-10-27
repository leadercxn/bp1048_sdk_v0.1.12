/**
 *******************************************************************************
 * @file    adc.h
 * @brief	SarAdc module driver interface
 
 * @author  Sam
 * @version V1.0.0
 
 * $Created: 2014-12-26 14:01:05$
 * @Copyright (C) 2014, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 ******************************************************************************* 
 */

/**
 * @addtogroup ADC
 * @{
 * @defgroup adc adc.h
 * @{
 */
 
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

typedef enum __ADC_DC_CLK_DIV
{
	CLK_DIV_NONE = 0,
	CLK_DIV_2,
	CLK_DIV_4,
	CLK_DIV_8,
	CLK_DIV_16,
	CLK_DIV_32,
	CLK_DIV_64,
	CLK_DIV_128,
	CLK_DIV_256,
	CLK_DIV_512,
	CLK_DIV_1024,
	CLK_DIV_2048,
	CLK_DIV_4096
	
}ADC_DC_CLK_DIV;


typedef enum __ADC_DC_CHANNEL_SEL
{
	ADC_CHANNEL_VIN = 0,		/**channel 0*/
	ADC_CHANNEL_VBK,			/**channel 1*/
	ADC_CHANNEL_VDD1V2,			/**channel 2*/
	ADC_CHANNEL_VCOM,			/**channel 3*/
	ADC_CHANNEL_GPIOA20_A23,	/**channel 4*/
	ADC_CHANNEL_GPIOA21_A24,	/**channel 5*/
	ADC_CHANNEL_GPIOA22_A25,	/**channel 6*/
	ADC_CHANNEL_GPIOA26,		/**channel 7*/
	ADC_CHANNEL_GPIOA27,		/**channel 8*/
	ADC_CHANNEL_GPIOA28,		/**channel 9*/
	ADC_CHANNEL_GPIOA29,		/**channel 10*/
	ADC_CHANNEL_GPIOA30,		/**channel 11*/
	ADC_CHANNEL_GPIOA31,		/**channel 12*/
	ADC_CHANNEL_GPIOB0,			/**channel 13*/
	ADC_CHANNEL_GPIOB1,			/**channel 14*/
	ADC_CHANNEL_POWERKEY		/**channel 15*/
	
}ADC_DC_CHANNEL_SEL;


typedef enum __ADC_VREF
{
	ADC_VREF_VDD,
	ADC_VREF_VDDA,
	ADC_VREF_Extern //�ⲿ����
	
}ADC_VREF;

typedef enum __ADC_CONV
{
	ADC_CON_SINGLE,
	ADC_CON_CONTINUA
}ADC_CONV;

/**
 * @brief  ADCʹ��
 * @param  ��
 * @return ��
 */
void ADC_Enable(void);

/**
 * @brief  ADC����
 * @param  ��
 * @return ��
 */
void ADC_Disable(void);

/**
 * @brief  ADC PowerKeyͨ��ʹ��
 * @param  ��
 * @return ��
 */
void ADC_PowerkeyChannelEnable(void);

/**
 * @brief  ADC PowerKeyͨ������
 * @param  ��
 * @return ��
 */
void ADC_PowerkeyChannelDisable(void);

/**
 * @brief  ADC����У׼��У׼֮������ADC��������
 * @param  ��
 * @return ��
 */
void ADC_Calibration(void);

/**
 * @brief  ����ADC����ʱ�ӷ�Ƶ
 * @param  Div ��Ƶϵ��
 * @return ��
 */
void ADC_ClockDivSet(ADC_DC_CLK_DIV Div);

/**
 * @brief  ��ȡADC����ʱ�ӷ�Ƶ
 * @param  ��
 * @return ��Ƶϵ��
 */
ADC_DC_CLK_DIV ADC_ClockDivGet(void);

/**
 * @brief  ADC�ο���ѹѡ��
 * @param  Mode �ο���ѹԴѡ��2:Extern Vref; 1:VDDA; 0: 2*VMID
 * @return ��
 */
void ADC_VrefSet(ADC_VREF Mode);

/**
 * @brief  ����ADCת������ģʽ
 * @param  Mode ����ģʽ��0:����ģʽ; 1:����ģʽ�����DMAʹ�ã�ֻ�ܲ���һ��ͨ����
 * @return ��
 */
void ADC_ModeSet(ADC_CONV Mode);

/**
 * @brief  ��ȡADC��������
 * @param  ChannalNum ADC������ͨ����
 * @return ADC��������������[0-4095]
 */
int16_t ADC_SingleModeDataGet(uint32_t ChannalNum);

/**
 * @brief  ADC������������������ģʽ
 * @param  ChannalNum ADC������ͨ����
 * @return ��
 * @note ADC_SingleModeDataStart,ADC_SingleModeDataConvertionState,ADC_SingleModeDataOut3���������ʹ��
 */
void ADC_SingleModeDataStart(uint32_t ChannalNum);

/**
 * @brief  ADC���������Ƿ����
 * @param  ��
 * @return �Ƿ�������
 *  @arg	TRUE �������
 *  @arg	FALSE ����δ���
 * @note ADC_SingleModeDataStart,ADC_SingleModeDataConvertionState,ADC_SingleModeDataOut3���������ʹ��
 */
bool ADC_SingleModeDataConvertionState(void);

/**
 * @brief  ��ȡADC��������
 * @param  ��
 * @return ADC��������������[0-4095]
 * @note ADC_SingleModeDataStart,ADC_SingleModeDataConvertionState,ADC_SingleModeDataOut3���������ʹ��
 */
int16_t ADC_SingleModeDataOut(void);

//����ģʽ�¿���ʹ��DMA
/**
 * @brief  ʹ��ADCת������DMA��ʽ
 * @param  ��
 * @return ��
 * @note  ʹ��ǰ������DMA���õ�ͨ��������DMA��ʽ��ADCֻ�ܵ�ͨ������
 */
void ADC_DMAEnable(void);

/**
 * @brief  ADCת��DMAģʽ����
 * @param  ��
 * @return ��
 * @note  ʹ��ǰ������DMA���õ�ͨ��������DMA��ʽ��ADCֻ�ܵ�ͨ������
 */
void ADC_DMADisable(void);

/**
 * @brief  ADC����������ʽ����
 * @param  ChannalNum ADC������ͨ����
 * @return ��
 */
void ADC_ContinuModeStart(uint32_t ChannalNum);

/**
 * @brief  MCU��ȡADC����������ʽ�������
 * @param  ��
 * @return ADC��������������[0-4095]
 */
uint16_t ADC_ContinuModeDataGet(void);

/**
 * @brief  ADC����������ʽ����
 * @param  ��
 * @return ��
 */
void ADC_ContinuModeStop(void);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __ADC_H__
/**
 * @}
 * @}
 */
 
