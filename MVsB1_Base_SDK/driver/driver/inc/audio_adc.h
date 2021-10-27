/**
 *******************************************************************************
 * @file    audio_adc.h
 * @brief	模拟∑Δ调制A/D转换器（ASDM）驱动程序接口
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2017-04-26 13:27:11$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *******************************************************************************
 */

/**
 * @addtogroup AUDIO_ADC
 * @{
 * @defgroup audio_adc audio_adc.h
 * @{
 */

#ifndef __AUDIO_ADC_H__
#define __AUDIO_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

////////////////////////////////////////////
//     Gain Volume Table for PGA(dB)      //
////////////////////////////////////////////
//	Vol // Gain of Mic  // Gain of LineIn  // Gain of fm  //
//  00	//     21.14	  //      13.25      //      44.31  //
//  01	//     19.76	  //      12.26      //      42.99  //
//  02	//     18.29	  //      11.37      //      41.84  //
//  03	//     17.04	  //      10.31      //      40.82  //
//  04	//     15.94	  //      9.365      //      39.73  //
//  05	//     14.67	  //      8.313      //      38.77  //
//  06	//     13.56	  //      7.198      //      37.69  //
//  07	//     12.12	  //      6.21       //      36.73  //
//  08	//     10.89    //      5.323      //      35.59  //
//  09	//     9.48	    //      4.519      //      34.57  //
//  10	//     7.98	    //      3.436      //      33.66  //
//  11	//     6.48	    //      2.4737     //      32.84  //
//  12	//     5.19	    //      1.6074     //      31.72  //
//  13	//     4.07	    //      0.5723     //      30.72  //
//  14	//     2.78	    //      -0.352     //      29.69  //
//  15	//     1.52	    //      -1.32      //      28.76  //
//  16	//     0.42     //      -2.249     //      27.92  //
//  17	//     -0.86    //      -3.248     //      26.77  //
//  18	//     -1.98    //      -4.143     //      25.75  //
//  19	//     -3.19    //      -5.082     //      24.83  //
//  20	//     -4.46    //      -6.045     //      23.71  //
//  21	//     -5.57    //      -7.014     //      22.71  //
//  22	//     -6.85    //      -7.885     //      21.58  //
//  23	//     -8.1     //      -8.76      //      20.56  //
//  24	//     -9.3     //      -9.71      //      19.44  //
//  25	//     -10.56   //      -10.69     //      18.43  //
//  26	//     -11.82   //      -11.58     //      17.5   //
//  27	//     -13.08   //      -12.49     //      16.64  //
//  28	//     -14.42   //      -13.52     //      15.64  //
//  29	//     -15.7    //      -14.43     //      14.65  //
//  30	//     -17      //      -15.34     //      13.62  //
//  31	//     -18.29   //      -16.3      //      12.66  //
//  32	//                                         11.75  //
//  33	//                                         10.617 //
//  34	//                                         9.55   //
//  35	//                                         8.536  //
//  36	//                                         7.567  //
//  37	//                                         6.632  //
//  38	//                                         5.577  //
//  39	//                                         4.55   //
//  40	//                                         3.545  //
//  41	//                                         2.553  //
//  42	//                                         1.5678 //
//  43	//                                         0.5834 //
//  44	//                                         -0.407 //
//  45	//                                         -1.41  //
//  46	//                                         -2.432 //
//  47	//                                         -3.481 //
//  48	//                                         -4.409 //
//  49	//                                         -5.371 //
//  50	//                                         -6.375 //
//  51	//                                         -7.43  //
//  52	//                                         -8.55  //
//  53	//                                         -9.44  //
//  54	//                                         -10.39 //
//  55	//                                         -11.4  //
//  56	//                                         -12.36 //
//  57	//                                         -13.41 //
//  58	//                                         -14.47 //
//  59	//                                         -15.39 //
//  60	//                                         -16.4  //
//  61	//                                         -17.5  //
//  62	//                                         -18.51 //
//  63	//                                         -19.17 //
////////////////////////////////////////////

/**
 * ADC 模块
 */
typedef enum _ADC_MODULE
{
    ADC0_MODULE,
    ADC1_MODULE

} ADC_MODULE;

typedef enum _ADC_CHANNEL
{
    CHANNEL_LEFT,
    CHANNEL_RIGHT

} ADC_CHANNEL;

typedef enum _ADC_DMIC_DOWN_SAMPLLE_RATE
{
    DOWN_SR_64,
	DOWN_SR_128

} ADC_DMIC_DSR;

typedef enum AUDIO_ADC_INPUT
{
  LINEIN_NONE,				//none，用于关闭当前PGA下的channel选择
  LINEIN1_LEFT,
  LINEIN1_RIGHT,

  LINEIN2_LEFT,
  LINEIN2_RIGHT,

  LINEIN3_LEFT_OR_MIC1,
  LINEIN3_RIGHT_OR_MIC2,

  LINEIN4_LEFT,//GPIO B1, INPUT_FMIN1_LEFT
  LINEIN4_RIGHT,//GPIO B0, INPUT_FMIN1_RIGHT

  LINEIN5_LEFT,//GPIO B3, INPUT_FMIN2_LEFT
  LINEIN5_RIGHT,//GPIO B2, INPUT_FMIN2_RIGHT
} AUDIO_ADC_INPUT;

/**
 * AGC 模式使能选择
 */
typedef enum _AGC_CHANNEL
{
    AGC_DISABLE	        = 0x00,       /**<禁止AGC功能 */
    AGC_RIGHT_ONLY      = 0x01,       /**<仅右通道使能AGC功能*/
    AGC_LEFT_ONLY       = 0x02,       /**<仅左通道使能AGC功能*/
    AGC_STEREO_OPEN     = 0x03        /**<左右双通道使能AGC功能*/

} AGC_CHANNEL;

/**
 * @brief  ADC 模块使能（总开关）
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return 无
 */
void AudioADC_Enable(ADC_MODULE ADCModule);

/**
 * @brief  所有ADC 模块使能（总开关）
 * @param  无
 * @return 无
 * @note 用于3路以上ADC时，保证各个ADC采样的相位相同。
 * @note 调用该函数的地方需要注意关闭中断,防止函数内部执行时被中断打断
 */
void AudioADC_AllModuleEnable(void);

/**
 * @brief  ADC 模块关闭
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return 无
 */
void AudioADC_Disable(ADC_MODULE ADCModule);

/**
 * @brief  ADC 模块左右通道使能选择
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  IsLeftEn     TRUE,左通道能使; FALSE,左通道关闭
 * @param  IsRightEn    TRUE,右通道能使; FALSE,右通道关闭
 * @return 无
 */
void AudioADC_LREnable(ADC_MODULE ADCModule, bool IsLeftEn, bool IsRightEn);

/**
 * @brief  ADC 模块是否交换左右通道输入
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  IsSwap       TRUE,左右通道交换输入; FALSE,左右通道正常输入
 * @return 无
 */
void AudioADC_ChannelSwap(ADC_MODULE ADCModule, bool IsSwap);

/**
 * @brief  ADC 模块高通滤波器截止频率参数配置
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  Coefficient  滤波器系数，12bit位宽，默认值0xFFE。
 *   @arg  Coefficient = 0xFFE  48k采样率下在20Hz处衰减-1.5db。
 *   @arg  Coefficient = 0xFFC  48k采样率下在40Hz处衰减-1.5db。
 *   @arg  Coefficient = 0xFFD  32k采样率下在40Hz处衰减-1.5db。
 * @return 无
 * @Note 该函数必须在AudioADC_Enable()函数调用之后
 */
void AudioADC_HighPassFilterConfig(ADC_MODULE ADCModule, uint16_t Coefficient);

/**
 * @brief  ADC 模块高通滤波是否使能（去除直流偏量）
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  IsHpfEn      TRUE,能使高通滤波器，FALSE,关闭高通滤波器
 * @return 无
 */
void AudioADC_HighPassFilterSet(ADC_MODULE ADCModule, bool IsHpfEn);

/**
 * @brief  ADC 模块清除寄存器和内存中的数值
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return 无
 */
void AudioADC_Clear(ADC_MODULE ADCModule);

/**
 * @brief  ADC 模块采样率配置
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  SampleRate   ADC采样率值，9种采样率
 * @return 无
 */
void AudioADC_SampleRateSet(ADC_MODULE ADCModule, uint32_t SampleRate);

/**
 * @brief  获取ADC 模块采样率配置值
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return 当前采样率配置值
 */
uint32_t AudioADC_SampleRateGet(ADC_MODULE ADCModule);

/**
 * @brief  ADC 模块淡入淡出时间设置
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  FadeTime     淡入淡出时间, 单位:Ms
 * @return 无
 * @Note   建议时间为10Ms，时间不能设置为0，如果想关闭淡入淡出功能请调用函数AudioADC_FadeDisable();
 */
void AudioADC_FadeTimeSet(ADC_MODULE ADCModule, uint8_t FadeTime);

/**
 * @brief  ADC 模块淡入淡出功能使能
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return 无
 */
void AudioADC_FadeEnable(ADC_MODULE ADCModule);

/**
 * @brief  ADC 模块淡入淡出功能禁用
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return 无
 */
void AudioADC_FadeDisable(ADC_MODULE ADCModule);

/**
 * @brief  ADC 模块数字静音控制，左右通道分别独立控制
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  LeftMuteEn   TRUE,左通道静音使能; FALSE,左通道取消静音
 * @param  RightMuteEn  TRUE,右通道静音使能; FALSE,右通道取消静音
 * @return 无
 * @Note   该函数内部不带延时，配置硬件寄存器之后立即退出。如果需要等待静音动作完成，则要在函数外部做延时
 */
void AudioADC_DigitalMute(ADC_MODULE ADCModule, bool LeftMuteEn, bool RightMuteEn);

/**
 * @brief  ADC 模块软件静音控制，左右通道分别独立控制
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  LeftMuteEn   TRUE,左通道静音使能; FALSE,左通道取消静音
 * @param  RightMuteEn  TRUE,右通道静音使能; FALSE,右通道取消静音
 * @return 无
 * @Note   该函数内部带延时，如果是静音操作，会延时等待音量降低到0之后退出该函数。
 */
void AudioADC_SoftMute(ADC_MODULE ADCModule, bool LeftMuteEn, bool RightMuteEn);

/**
 * @brief  ADC 模块音量设置
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  LeftVol      左声道音量值，0x00:静音, 0x001:-72dB, 0xFFF:0dB
 * @param  RightVol     右声道音量值，0x00:静音, 0x001:-72dB, 0xFFF:0dB
 * @return 无
 */
void AudioADC_VolSet(ADC_MODULE ADCModule, uint16_t LeftVol, uint16_t RightVol);


/**
 * @brief  ADC 模块音量设置（左右声道独立设置）
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  ChannelSel   左右声道选择，0x00: 无；0x1：左声道；0x2：右声道
 * @param  Vol     		音量值，0x00:静音, 0x001:-72dB, 0xFFF:0dB、
 * @note   当ChannelSel为3时，同时选择了左右声道，此时Vol配置对左右声道均有效，左右声道音量值一致
 * @return 无
 */
void AudioADC_VolSetChannel(ADC_MODULE ADCModule, ADC_CHANNEL ChannelSel, uint16_t Vol);

/**
 * @brief  ADC 模块音量获取
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  *LeftVol      左声道音量值，0x00:静音, 0x001:-72dB, 0xFFF:0dB
 * @param  *RightVol     右声道音量值，0x00:静音, 0x001:-72dB, 0xFFF:0dB
 * @return 无
 */
void AudioADC_VolGet(ADC_MODULE ADCModule, uint16_t* LeftVol, uint16_t* RightVol);

/**
 * @brief  ADC 模块模拟部分上电初始化
 * @param  无
 * @return 无
 * @note 如果DAC模块工作，上电先做DAC上电操作，否则会有pop声
 */
void AudioADC_AnaInit(void);

/**
 * @brief  ADC 模块模拟部分去初始化
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return 无
 */
void AudioADC_AnaDeInit(ADC_MODULE ADCModule);

/**
 * @brief  ADC PAG通道选择
 * @param  ADCModule	0,ADC0模块; 1,ADC1模块
 * @param  ChannelSel   0,Left;	1:Right
 * @param  InputSel 	PGA输入通路选择。具体见AUDIO_ADC_INPUT枚举值含义
 * @note   参数3选择时需要和参数1和参数2对应，需要和datasheet对照
 * @return 无
 */
void AudioADC_PGASel(ADC_MODULE ADCModule, ADC_CHANNEL ChannelSel, AUDIO_ADC_INPUT InputSel);

/**
 * @brief  ADC PAG增益配置
 * @param  ADCModule	0,ADC0模块; 1,ADC1模块
 * @param  ChannelSel   0,Left;	1:Right
 * @param  InputSel 	InputSel PGA输入通路选择。具体见AUDIO_ADC_INPUT枚举值含义
 * @param  Gain 		PGA增益设置。配置范围【0-63】。
 * @param  GainBoostSel 0:0dB; 1:9dB; 2:18dB; 3:27dB。该参数只有选择input为mic时才有用。
 * @note   参数3选择时需要和参数1和参数2对应，需要和datasheet对照。
 * @note   参数4具体值对应的实际dB数请参考本文件头的增益表，不同input同样的值对应的参数值不同。
 * @note   参数5只有在input为MIC的时候才有效
 * @return 无
 */
void AudioADC_PGAGainSet(ADC_MODULE ADCModule, ADC_CHANNEL ChannelSel, AUDIO_ADC_INPUT InputSel, uint16_t Gain, uint8_t GainBoostSel);

/**
 * @brief  ADC 模块Mic Bias1电压使能
 * @param  IsMicBiasEn  MIC Bias1使能
 * @return 无
 */
void AudioADC_MicBias1Enable(bool IsMicBiasEn);

/**
 * @brief  ADC模块PGA模块模式选择
 * @param  LeftMode:  	左通道模式选择   0，PGA模块差分输入    1，PGA模块单端输入      默认为单端输入
 * @param  RightMode:  	右通道模式选择   0， PGA模块差分输入    1，PGA模块单端输入     默认为单端输入
 * @note 差分模式只有ADC0模块支持。
 * @return 无
 */
void AudioADC_PGAMode(uint8_t LeftMode, uint8_t RightMode);

/**
 * @brief  PGA模块差分输入电压增益控制
 * @param  LeftGainSel:  	左通道增益选择.0:0dB;1:6dB;2:10dB;3:15dB
 * @param  RightGainSel: 	右通道增益选择.0:0dB;1:6dB;2:10dB;3:15dB
 * @note 差分模式只有ADC0模块支持
 * @return 无
 */
void AudioADC_PGADiffGainSel(uint8_t leftdifGainSel, uint8_t rightdifGainSel);

/**
 * @brief  AGC模块PGA模块过零检测使能
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  IsLeftEn:  	左通道使能
 * @param  IsRightEn: 	右通道使能
 * @return 无
 */
void AudioADC_PGAZeroCrossEnable(ADC_MODULE ADCModule, bool IsLeftEn, bool IsRightEn);

/**
 * @brief  ASDM模块使能DMIC功能
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return 无
 */
void AudioADC_DMICEnable(ADC_MODULE ADCModule);

/**
 * @brief  ASDM模块禁能DMIC功能
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return 无
 */
void AudioADC_DMICDisable(ADC_MODULE ADCModule);

/**
 * @brief  DMIC降采样率倍数选择
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  DownSampleRate    DOWN_SR_64,64倍降采样; DOWN_SR_128,128倍降采样
 * @return 无
 */
void AudioADC_DMICDownSampleSel(ADC_MODULE ADCModule, ADC_DMIC_DSR DownSampleRate);

/**
 * @brief  ADC 模块AGC模块通道选择
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  IsLeftEn  	左通道使能
 * @param  IsRightEn	右通道使能
 * @return 无
 */
void AudioADC_AGCChannelSel(ADC_MODULE ADCModule, bool IsLeftEn, bool IsRightEn);

/**
 * @brief  ADC 模块用于整理AGC增益
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  GainOffset 	增益偏移设置。【8 ~ 15】-->【-4dB ~ -0.5dB】;【0 ~ 7】-->【0dB ~ 3.5dB】.
 * @return 无
 */
void AudioADC_AGCGainOffset(ADC_MODULE ADCModule, uint8_t GainOffset);

/**
 * @brief  ADC 模块AGC模块增益最大水平
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  MaxLevel     AGC模块增益最大水平.【0 ~ 31】-->【-3 ~ -34dB】
 * @return 无
 */
void AudioADC_AGCMaxLevel(ADC_MODULE ADCModule, uint8_t MaxLevel);

/**
 * @brief  AGC模块增益目标水平
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  TargetLevel	AGC模块增益目标水平.【0 ~ 31】-->【-3 ~ -34dB】
 * @return 无
 */
void AudioADC_AGCTargetLevel(ADC_MODULE ADCModule, uint8_t TargetLevel);

/**
 * @brief  AGC模模块可调节的最大增益
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  MaxGain		AGC模块最大增益.【0 ~ 63】-->【 39.64 ~ -20.3dB】,step:-0.95dB.
 * @return 无
 */
void AudioADC_AGCMaxGain(ADC_MODULE ADCModule, uint8_t MaxGain);

/**
 * @brief  AGC模模块可调节的最小增益
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  MinGain		AGC模块最小增益.【0 ~ 63】-->【 39.64 ~ -20.3dB】,step:-0.95dB.
 * @return 无
 */
void AudioADC_AGCMinGain(ADC_MODULE ADCModule, uint8_t MinGain);

/**
 * @brief  AGC模块帧时间
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  FrameTime	帧时间设置。 单位：ms。 范围【 1 ~ 4096】
 * @return 无
 */
void AudioADC_AGCFrameTime(ADC_MODULE ADCModule, uint16_t FrameTime);

/**
 * @brief  AGC模块保持时间，开始AGC算法自适应增益之前的保持时间。
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  HoldTime		AGC开始算法之前的保持时间。单位：ms。范围【0*FrameTime ~ 31*FrameTime】
 * @note   HoldTime必须为FrameTimer的整数倍，否则内部也会四舍五入。
 * @return 无
 */
void AudioADC_AGCHoldTime(ADC_MODULE ADCModule, uint32_t HoldTime);

/**
 * @brief  AGC模块当输入信号太大时，AGC增益衰减的步进时间设置。
 * @param  ADCModule    	0,ADC0模块; 1,ADC1模块
 * @param  AttackStepTime 	AGC增益增强的步进时间，单位为ms,范围为1 ~ 4096 ms
 * @return 无
 */
void AudioADC_AGCAttackStepTime(ADC_MODULE ADCModule, uint16_t AttackStepTime);

/**
 * @brief  AGC模块当输入信号太小时，AGC增益增强的步进时间设置。
 * @param  ADCModule    	0,ADC0模块; 1,ADC1模块
 * @param  DecayStepTime 	AGC增益增强的步进时间，单位为ms,范围为1 ~ 4096 ms
 * @return 无
 */
void AudioADC_AGCDecayStepTime(ADC_MODULE ADCModule, uint16_t DecayStepTime);

/**
 * @brief  AGC模块AGC噪声阈值设置
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  NoiseThreshold： 噪音阈值设置,范围：【0 ~ 31】对应值：【-90dB ~ -28dB】，step：2dB
 * 						      默认为 01111,即-60 dB
 * @return 无
 */
void AudioADC_AGCNoiseThreshold(ADC_MODULE ADCModule, uint8_t NoiseThreshold);

/**
 * @brief  AGC模块AGC模块噪声模式选择,
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  NoiseMode：	0: ADC输出数据电平与噪声阈值判断，确定该输入是否为噪声；
 * 						1: ADC输入数据电平与噪声阈值判断，确定该输入是否为噪声。
 * @return 无
 */
void AudioADC_AGCNoiseMode(ADC_MODULE ADCModule, uint8_t NoiseMode);

/**
 * @brief  AGC模块AGC模块噪声Gate功能使能
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  NoiseGateEnable： 0:禁止噪声Gate功能; 1:使能噪声Gate功能
 * @return 无
 */
void AudioADC_AGCNoiseGateEnable(ADC_MODULE ADCModule, bool NoiseGateEnable);

/**
 * @brief  AGC模块AGC模块噪声Gate模式选择
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  NoiseGateMode 	0:  当检查到过零信号时，PCM数据mute/unmute
 *							1:  PCM数据立即mute/unmute
 * @return 无
 */
void AudioADC_AGCNoiseGateMode(ADC_MODULE ADCModule, uint8_t NoiseGateMode);

/**
 * @brief  AGC模块AGC模块噪声保持时间设置。
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  NoiseHoldTime	噪声持续NoiseHoldTime后，噪声相关算法开始执行。单位（ms）
 * @return 无
 */
void AudioADC_AGCNoiseHoldTime(ADC_MODULE ADCModule, uint8_t NoiseHoldTime);

/**
 * @brief  AGC模块获取AGC增益
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return AGC增益
 * @note   返回的值并不是AGC的实际增益值，AGC实际增益值与寄存器值
 * 		        之间的转换见AGC实际增益值与寄存器值对应表
 */
uint8_t AudioADC_AGCGainGet(ADC_MODULE ADCModule);

/**
 * @brief  AGC模块获取AGC模块静音信息
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return AGC模块静音标志
 */
uint8_t AudioADC_AGCMuteGet(ADC_MODULE ADCModule);

/**
 * @brief  AGC模块获取AGC模块更新标志位
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @return AGC模块更新标志位
 */
uint8_t AudioADC_AGCUpdateFlagGet(ADC_MODULE ADCModule);

/**
 * @brief  AGC模块清除AGC模块更新标志位
 * @param  ADCModule 	0,ADC0模块; 1,ADC1模块
 * @return 无
 */
void AudioADC_AGCUpdateFlagClear(ADC_MODULE ADCModule);

/**
 * @brief  VCOM使用为Mic供电偏置电源
 * @param  Config 0: VCOM,DAC直推耳机; 1:MicBias; 2:PowerDown VCOM
 * @return 无
 * @note   选择VCOM作为mic供电电源，则不能使用DAC输出免驱动耳机。
 * @note   如果DAC工作，则先配置DAC上电，否则DAC会有上电pop声
 */
void AudioADC_VcomConfig(uint8_t Config);


/**
 * @brief  Dynamic-Element-Matching enable signal
 * @param  ADCModule    0,ADC0模块; 1,ADC1模块
 * @param  IsLeftEn  	左通道使能
 * @param  IsRightEn	右通道使能
 * @return 无
 */
void AudioADC_DynamicElementMatch(ADC_MODULE ADCModule, bool IsLeftEn, bool IsRightEn);


/**
 * @brief  功能重置
 * @param  ADCModule 	0,ADC0模块; 1,ADC1模块
 * @return 无
 */
void AudioADC_FuncReset(ADC_MODULE ADCModule);

/**
 * @brief  寄存器重置
 * @param  无
 * @return 无
 */
void AudioADC_RegReset(void);

/**
 * @brief  查询ADC采样过程是否数据溢出
 * @param  ADCModule 	0,ADC0模块; 1,ADC1模块
 * @return TREU：有溢出过
 */
bool AudioADC_IsOverflow(ADC_MODULE ADCModule);

/**
 * @brief 清除ADC采样过程是否数据溢出标志
 * @param  ADCModule 	0,ADC0模块; 1,ADC1模块
 * @return 无
 */
void AudioADC_OverflowClear(ADC_MODULE ADCModule);

void AudioADC_PowerDown(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__AUDIO_ADC_H__

/**
 * @}
 * @}
 */
