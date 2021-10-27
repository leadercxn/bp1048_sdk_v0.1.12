/**
 *******************************************************************************
 * @file    dac.h
 * @author  Sean
 * @version V1.0.0
 * @date    2017/04/21
 * @brief
 *******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2014 MVSilicon </center></h2>
 */

/**
 * @addtogroup DAC
 * @{
 * @defgroup dac dac.h
 * @{
 */

#ifndef __DAC_H__
#define __DAC_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * DAC Module Select
 */
typedef enum _DAC_MODULE
{
	DAC0 = 0,
	DAC1 = 1,
	ALL = 2		//DAC0 and DAC1 all on
}DAC_MODULE;

typedef enum _SCRAMBLE_MODULE
{
	DWA = 0,
	RANDOM_DWA = 1,
	POS_NEG = 2

}SCRAMBLE_MODULE;

typedef enum _DOUT_MODE
{
    MODE0 = 0,// L_o=L_i;R_o=R_i;
    MODE1 = 1,// L_o=L_i;R_o=L_i;
    MODE2 = 2,// L_o=R_i;R_o=L_i;
    MODE3 = 3,// L_o=(L_i+R_i)/2;R_o=(L_i+R_i)/2;
    MODE4 = 4 // L_o=(L_i+R_i)/2;R_o=-(L_i+R_i)/2;
}DOUT_MODE;

typedef enum _SCRAMBLE2_FILTER_WIDTH
{
	FILTER_7_BIT = 0,
	FILTER_8_BIT = 1,
	FILTER_9_BIT = 2,
	FILTER_10_BIT = 3,
	FILTER_11_BIT = 4,
	FILTER_12_BIT = 5,
}SCRAMBLE2_FILTER_WIDTH;

typedef enum _SCRAMBLE2_OPT
{
	OPT0 = 0,//when input is all 0's or all 1's, filter continuous calc; each one path filter is overflow, all path filter is cleared
	OPT1 = 1,//when input is all 0's or all 1's, filter continuous calc; each one path filter is overflow, only the overflow path filter is cleared
	OPT2 = 2,//when input is all 0's or all 1's, filter is hold the current value
	OPT3 = 3,//when input is all 0's, all 1's or only one path is 1, use random select to replace the R2 DWA, all filter value hold current value when input is other value, return back to R2 DWA
}SCRAMBLE2_OPT;

typedef enum _DOUT_WIDTH
{
	WIDTH_16_BIT = 0,
	WIDTH_24_BIT_1 = 2,
	WIDTH_24_BIT_2 = 3,
}DOUT_WIDTH;

typedef enum _EDGE
{
	POSEDGE = 0,
	NEGEDGE = 1
}EDGE;

/**
 * @brief  Audio DAC module enable
 * @param  DACModule which AudioDac you will use
 * @return NONE
 */
void AudioDAC_Enable(DAC_MODULE DACModule);

/**
 * @brief  Audio DAC module disable
 * @param  DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_Disable(DAC_MODULE DACModule);

/**
 * @brief  Audio DAC module dither enable
 * @param  DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_DitherEnable(DAC_MODULE DACModule);

/**
 * @brief  Audio DAC module dither disable
 * @param  DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_DitherDisable(DAC_MODULE DACModule);

/**
 * @brief  Audio DAC module scramble enable
 * @param  DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_ScrambleEnable(DAC_MODULE DACModule);

/**
 * @brief  Audio DAC module scramble disable
 * @param  DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_ScrambleDisable(DAC_MODULE DACModule);

/**
 * @brief  Audio DAC pause,stop ask data from DMA
 * @param  DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_Pause(DAC_MODULE DACModule);

/**
 * @brief  Audio DAC disable pause, go on asking data from DMA
 * @param  DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_Run(DAC_MODULE DACModule);


/**
 * @brief  Audio DAC output mode set
 * @param  DoutOut 	MODE0 = 0,// L_o=L_i;R_o=R_i;
					MODE1 = 1,// L_o=R_i;R_o=L_i;
					MODE2 = 2,// L_o=(L_i+R_i)/2;R_o=(L_i+R_i)/2;
					MODE3 = 3// L_o=(L_i+R_i)/2;R_o=-(L_i+R_i)/2;
 * @param DoutOut
 * @param DoutWidth
 * @return NONE
 */
void AudioDAC_DoutModeSet(DAC_MODULE DACModule, DOUT_MODE DoutOut, DOUT_WIDTH DoutWidth);// 不改

/**
 * @brief  Audio DAC sample edge mode config
 * @param  DACModule which AudioDAC you will use
 * @param  Edge posedge or negedge to sample data
 * @return NONE
 */
void AudioDAC_EdgeSet(DAC_MODULE DACModule,EDGE Edge);

/**
 * @brief  Audio DAC scramble mode config
 * @param  DACModule which AudioDAC you will use
 * @param  ScrambleMode DWA
 *                      RANDOM_DWA
 *                      BUTTERFLY_DWA
 * @return NONE
 */
void AudioDAC_ScrambleModeSet(DAC_MODULE DACModule,SCRAMBLE_MODULE ScrambleMode);

/**
 * @brief  Audio DAC dsm outdis mode config
 * @param  DACModule which AudioDAC you will use
 * @param  mode 0 the DSM out drive the scramble module as normal
 *              1 auto mute the scramble input data  when the continuous N points of the PCM * VOL data values are 0, or the VOL is 0,the N is set by mdac0_zeros_num_sel
 * @return NONE
 */
void AudioDAC_DsmOutdisModeSet(DAC_MODULE DACModule,uint8_t mode);

/**
 * @brief  Audio DAC zero num set
 * @param  DACModule which AudioDAC you will use
 * @param  sel the continuous PCM data as 0, and the 0's number limit
 *             	0: zeros number value: 512
				1: zeros number value: 1024
				2: zeros number value: 2048
				3: zeros number value: 4096
				4: zeros number value: 8192
				5: zeros number value: 16384
				6: zeros number value: 32768
				7: zeros number value: 65535
 * @return NONE
 */
void AudioDAC_ZeroNumSet(DAC_MODULE DACModule,uint8_t sel);


/**
 * @brief  Audio DAC dither pow set
 * @param  DACModule which AudioDAC you will use
 * @param  DitherPow you will set this value to dither pow
 * @return NONE
 */
void AudioDAC_DitherPowSet(DAC_MODULE DACModule,uint32_t DitherPow);

/**
 * @brief  Audio DAC get dither pow
 * @param  DACModule which AudioDAC you will use
 * @return Dither Pow you used
 */
uint32_t AudioDAC_DitherPowGet(DAC_MODULE DACModule);


/**
 * @brief  Audio DAC get sampler rate
 * @param  DACModule which AudioDAC you will use
 * @return Sample Rate you used
 */
uint32_t AudioDAC_SampleRateGet(DAC_MODULE DACModule);

/**
 * @brief  Audio DAC sampler rate set
 * @param  DACModule which AudioDAC you will use
 * @param  SampleRate 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000 are supported.
 * @return NONE
 */
void AudioDAC_SampleRateSet(DAC_MODULE DACModule,uint32_t SampleRate);


/**
 * @brief  Audio DAC reset the reg data and mem data
 * @param  DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_Reset(DAC_MODULE DACModule);

/**
 * @brief  Audio DAC clear mdac_load_error
 * @param  DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_ErrorFlagClear(DAC_MODULE DACModule);

/**
 * @brief  Audio DAC  mdac load error, when load next data ,but data is not ready
 * @param  DACModule which AudioDAC you will use
 * @return  ErrorFlag
 */
bool AudioDAC_ErrorFlagGet(DAC_MODULE DACModule);

/**
 * @brief Audio DAC fade time set
 * @param DACModule which AudioDAC you will use
 * @param FadeTime unit is ms
 * @return NONE
 */
void AudioDAC_FadeTimeSet(DAC_MODULE DACModule,uint8_t FadeTime);

/**
 * @brief Audio DAC fadein fadeout enable
 * @param DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_FadeEnable(DAC_MODULE DACModule);

/**
 * @brief Audio DAC fadein fadeout disable
 * @param DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_FadeDisable(DAC_MODULE DACModule);

/**
 * @brief Audio DAC Digital mute,DAC0 has left channel and right channel,DAC1 has just one channel we use LeftMuteEn to control
 * @param DACModule which AudioDAC you will use
 * @param LeftMuteEn left channel for DAC0,it is valid for DAC1
 * @param RightMuteEn right channel for DAC0,it is invalid for DAC1
 * @return NONE
 */
void AudioDAC_DigitalMute(DAC_MODULE DACModule,bool LeftMuteEn, bool RightMuteEn);

/**
 * @brief Audio DAC soft mute,DAC0 has left channel and right channel,DAC1 has just one channel we use LeftMuteEn to control
 * @param DACModule which AudioDAC you will use
 * @param LeftMuteEn left channel for DAC0,it is valid for DAC1
 * @param RightMuteEn right channel for DAC0,it is invalid for DAC1
 * @return NONE
 */
void AudioDAC_SoftMute(DAC_MODULE DACModule,bool LeftMuteEn, bool RightMuteEn);

/**
 * @brief Audio DAC volume set,DAC0 has left channel and right channel,DAC1 has just one channel we use LeftVol to control
 * @param DACModule which AudioDAC you will use
 * @param LeftVol left channel for DAC0,it is valid for DAC1,its scope is form 0 to 0x3fff
 * @param RightVol right channel for DAC0,it is invalid for DAC1,its scope is form 0 to 0x3fff
 * @return NONE
 */
void AudioDAC_VolSet(DAC_MODULE DACModule,uint16_t LeftVol, uint16_t RightVol);

/**
 * @brief Audio DAC volume Get,DAC0 has left channel and right channel,DAC1 has just one channel we use LeftVol to control
 * @param DACModule which AudioDAC you will use
 * @param *LeftVol left channel for DAC0,it is valid for DAC1,its scope is form 0 to 0x3fff
 * @param *RightVol right channel for DAC0,it is invalid for DAC1,its scope is form 0 to 0x3fff
 * @return NONE
 */
void AudioDAC_VolGet(DAC_MODULE DACModule,uint16_t* LeftVol, uint16_t* RightVol);

/**
 * @brief enable DAC clock
 * @param DACModule which AudioDAC you will use
 * @param Enable TRUE for enable, FALSE for disable
 * @return NONE
 */
void AudioDAC_ClkEnable(DAC_MODULE DACModule, bool Enable);

/**
 * @brief function reset for DAC
 * @param DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_FuncReset(DAC_MODULE DACModule);

/**
 * @brief register reset for DAC, this will reset ALL REGISTER including DAC0 and DAC1
 * @param NONE
 * @return NONE
 */
void AudioDAC_RegReset(void);

/**
 * @brief VMID starts charging with programmed current, and power down BIAS
 * @param NONE
 * @return NONE
 */
void AudioDAC_VIMDControl(void);

/**
 * @brief BIAS power on/off control
 * @param OnOff TRUE for BIAS Power on, FALSE for BIAS Power off
 * @return NONE
 */
void AudioDAC_BIASPowerCtrl(bool OnOff);

/**
 * @brief control SCF_VREFP current for left/right channel
 * @param DACModule which AudioDAC you will use
 * @param VrefpL: form 0 to 0x3
 * @param VrefpR: form 0 to 0x3
 * @return NONE
 */
void AudioDAC_IBSelect(DAC_MODULE DACModule, uint8_t VrefpL, uint8_t VrefpR);

/**
 * @brief All Analog power on DAC
 * @param NONE
 * @return TRUE:sucess,FALSE:failure
 */
bool AudioDAC_AllPowerOn(void);


/**
 * @brief Analog power on DAC
 * @param DACModule which AudioDAC you will use
 * @return TRUE:sucess,FALSE:failure
 */
bool AudioDAC_PowerOn(DAC_MODULE DACModule);

/**
 * @brief Analog power down DAC
 * @param DACModule which AudioDAC you will use
 * @return TRUE:sucess,FALSE:failure
 */
bool AudioDAC_PowerDown(DAC_MODULE DACModule);

/**
 * @brief Enable all Analog channel for DAC
 * @param NONE
 * @return NONE
 */
void AudioDAC_AllChannelEnable(void);

/**
 * @brief disable all Analog channel for DAC
 * @param NONE
 * @return NONE
 */
void AudioDAC_AllChannelDisable(void);

/**
 * @brief enable Analog channel for DAC
 * @param DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_ChannelEnable(DAC_MODULE DACModule);

/**
 * @brief disable Analog channel for DAC
 * @param DACModule which AudioDAC you will use
 * @return NONE
 */
void AudioDAC_ChannelDisable(DAC_MODULE DACModule);

/**
 * @brief Mute or Unmute SCF
 * @param DACModule which AudioDAC you will use
 * @param MuteL, channel to mute
 * @param MuteR, channel to mute
 * @return NONE
 */
void AudioDAC_SCFMute(DAC_MODULE DACModule, bool MuteL, bool MuteR);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__DAC_H__

/**
 * @}
 * @}
 */
