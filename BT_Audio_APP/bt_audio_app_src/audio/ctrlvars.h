/**
 **************************************************************************************
 * @file    ctrlvars.h
 * @brief   Control Variables Definition
 * 
 * @author  Aissen Li
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
#ifndef __CTRLVARS_H__
#define __CTRLVARS_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include <stdint.h>
#include "audio_effect.h"
#include "audio_effect_library.h"
#include "app_config.h"
#include "bt_config.h"
#include "audio_core_api.h"
#include "blue_aec.h"
#include "vocalcut.h"

#define  CTRL_VARS_SYNC_WORD            (0x43564347)

#define  MIN_VOLUME                     (0)
#define  MAX_VOLUME                     (63)
#define  VOLUME_COUNT                   (MAX_VOLUME - MIN_VOLUME + 1)
#define  MIN_BASS_TREB_GAIN             (0)
#define  MAX_BASS_TREB_GAIN             (15)
#define  MIN_MIC_DIG_STEP               (0)
#define  MAX_MIC_DIG_STEP               (32)
#define  MIN_MIC_REVB_STEP              (0)
#define  MAX_MIC_REVB_STEP              (32)
#define  MIN_MUSIC_DIG_STEP             (0)
#define  MAX_MUSIC_DIG_STEP             (32)
#define  BASS_TREB_GAIN_STEP            (1)
#define  MIN_AUTOTUNE_STEP              (0)
#define  MAX_AUTOTUNE_STEP              (11)

#define AEC_SIZE		    			(sizeof(BlueAECContext))
#define PCM_DELAY_SIZE					(sizeof(PCMDelay))
#define EXCITER_SIZE					(sizeof(ExciterContext))
#define EXPANDER_SIZE					(sizeof(ExpanderContext))
#define AUTOTUNE_SIZE					(sizeof(AutoTuneContext))
#define DRC_SIZE						(sizeof(DRCContext))
#define ECHO_SIZE						(sizeof(EchoContext))
#define ECHO_S_SIZE 					(22100)
#define FREQSHIFTER_SIZE				(sizeof(FreqShifterContext))
#define HOWLING_SIZE					(sizeof(HowlingContext))
#define SILENCE_SIZE					(sizeof(SilenceDetectorContext))
#define PS_SIZE							(sizeof(PSContext))
#define PS_PRO_SIZE		    			(sizeof(PitchShifterProContext))
#define REVERB_SIZE						(sizeof(ReverbContext))
#define PLATE_REVERB_SIZE				(sizeof(PlateReverbContext))
#define REVERB_PRO_SIZE     			MAX_REVERB_PRO_CONTEXT_SIZE_44100
#define REVERB_PRO_32K_SIZE 			MAX_REVERB_PRO_CONTEXT_SIZE_32000
#define VOICECHANGER_SIZE				(sizeof(VoiceChangerContext))
#define VOICECHANGER_PRO_SIZE			(sizeof(VoiceChangerProContext))
#define EQ_SIZE							(sizeof(EQContext))
#define VOCALCUT_SIZE	    			(sizeof(VocalCutContext))
#define VOCALREMOVE_SIZE				(sizeof(VocalRemoverContext))
#define THREED_SIZE         			(sizeof(ThreeDContext))
#define THREED_PLUS_SIZE         		(sizeof(ThreeDPlusContext))
#define VB_SIZE             			(sizeof(VBContext))
#define VBCLASSIC_SIZE      			(sizeof(VBClassicContext))
#define CHORUS_SIZE         			(sizeof(ChorusContext))+4
#define STEREO_WIDEN_SIZE         		(sizeof(StereoWidenerContext))
#define PINGPONG_SIZE         		    (sizeof(PingPongContext))
#define AUTOWAH_SIZE         		    (sizeof(AutoWahContext))

//-------only for print audio effect ram malloc---------------//
enum
{
    EFF_AUTO_TUNE=0,
    EFF_DC_BLOCK,
    EFF_DRC,
    EFF_ECHO,
    EFF_EQ,
    EFF_EXPAND,
    EFF_FREQ_SHIFTER,
    EFF_HOWLING_DETOR,
    EFF_NOISE_GATE,
    EFF_PITCH,
    EFF_REVERB,
    EFF_SILENCE_DETOR,
    EFF_THREED,
    EFF_VB,
    EFF_VOICE_CHANGE,
    EFF_GAIN,
    EFF_VOCALCUT,
    EFF_PLATE_REVERB,
    EFF_REVERB_PRO,
    EFF_VOICE_CHANGE_PRO,
    EFF_PHASE_CTL,
    EFF_VOCALREMOVE,
    EFF_PITCH_PRO,
    EFF_VBCLASSIC,
    EFF_PCM_DELAY,
    EFF_EXCITER,
    EFF_AEC,
    EFF_CHORUS,
	EFF_STEREO_WIDEN,
	EFF_PINGPONG,
	EFF_AUTOWAH,
};

#define  CFG_I2S0_OUT_EN 1
#define  CFG_I2S0_IN_EN  1
#define  CFG_I2S1_OUT_EN 1
#define  CFG_I2S1_IN_EN  1
#define  CFG_USB_OUT_EN              1
#define  CFG_USB_OUT_STEREO_EN       1
#define  CFG_SUPPORT_USB_VOLUME_SET  0
	
typedef enum _EFFECT_MODE
{
    EFFECT_MODE_NORMAL=10,
	EFFECT_MODE_ECHO, 
	EFFECT_MODE_REVERB,
	EFFECT_MODE_ECHO_REVERB,
    EFFECT_MODE_PITCH_SHIFTER,    
    EFFECT_MODE_VOICE_CHANGER,   
    EFFECT_MODE_AUTO_TUNE, 
    EFFECT_MODE_BOY,
    EFFECT_MODE_GIRL,
    EFFECT_MODE_KTV,
    EFFECT_MODE_BaoYin,
    EFFECT_MODE_HunXiang,    
    EFFECT_MODE_DianYin,
    EFFECT_MODE_MoYin,
    EFFECT_MODE_HanMai,
    EFFECT_MODE_NanBianNv,
    EFFECT_MODE_NvBianNan,
    EFFECT_MODE_WaWaYin,
    EFFECT_MODE_YuanSheng,
    EFFECT_MODE_HFP_AEC,
    //User can add other effect mode
} EFFECT_MODE;

typedef enum _REMIND_TYPE
{
    REMIND_TYPE_KEY,
	REMIND_TYPE_BACKGROUND, 

} REMIND_TYPE;

typedef enum _EQ_MODE
{
    EQ_MODE_FLAT,
	EQ_MODE_CLASSIC,	
	EQ_MODE_POP,
	EQ_MODE_ROCK,
	EQ_MODE_JAZZ,
	EQ_MODE_VOCAL_BOOST,
} EQ_MODE;


#define ANA_INPUT_CH_NONE    0
#define ANA_INPUT_CH_LINEIN1 1
#define ANA_INPUT_CH_LINEIN2 2
#define ANA_INPUT_CH_LINEIN3 3
#define ANA_INPUT_CH_LINEIN4 4
#define ANA_INPUT_CH_LINEIN5 5

typedef struct
{
	uint8_t       eff_mode;
	const uint8_t *EffectParamas;
	uint16_t	  len;
	char          *name;
	  
}AUDIO_EFF_PARAMAS;

//--------audio effect unit define-----------------------------------//
#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
typedef struct __PcmDelayUnit
{
	PCMDelay            *ct;
	uint32_t  			enable;
	int32_t             delay;
	int32_t             max_delay;
	int32_t    			delay_samples;
	int32_t 		 	max_delay_samples;
	int32_t         	high_quality;
	uint8_t          	*s_buf;
	uint32_t   		    param_cnt;
	uint8_t             channel;

} PcmDelayUnit;

#endif
typedef struct __ExciterUnit
{
	ExciterContext      *ct;
	uint32_t  			enable;
	int32_t 			f_cut;
	int32_t 			dry;
	int32_t 			wet;
	uint32_t   		    param_cnt;
	uint8_t             channel;

} ExciterUnit;

typedef struct __AutoTuneUnit
{
	AutoTuneContext *ct;
	uint32_t 		 enable;
	uint32_t 		 key;
	uint32_t  		 snap_mode;
	uint32_t    	 param_cnt;
	uint32_t         pitch_accuracy;
	uint8_t          channel;
} AutoTuneUnit;

typedef struct __DRCUnit
{
	DRCContext 		*ct;
	uint32_t 		 enable;
	int32_t 		 fc;
	uint32_t 		 mode;
	uint32_t 		 q[2];
	int32_t  		 threshold[3];
	int32_t  		 ratio[3];
	int32_t  		 attack_tc[3];
	int32_t  		 release_tc[3];
	int32_t  		 pregain1;
	int32_t  		 pregain2;
	uint32_t    	 param_cnt;
	uint8_t          channel;

} DRCUnit;

typedef struct __EchoUnit
{
	EchoContext 	*ct;
	uint8_t			*s_buf;
	uint32_t 		 enable;
	int32_t  		 fc;
	int32_t  		 attenuation;
	int32_t  		 delay;
	int32_t          delay_bakup;
	int32_t  		 max_delay;
	int32_t  		 delay_samples;
	int32_t 		 max_delay_samples;
	int32_t  		 direct_sound;
	uint32_t     	 param_cnt;
	uint8_t          channel;

} EchoUnit;

typedef struct __ExpanderUnit
{
	ExpanderContext *ct;
	uint32_t  		 enable;
	int32_t   		 threshold;
	int32_t   		 ratio;
	int32_t   		 attack_time;
	int32_t   		 release_time;
	uint32_t   		 param_cnt;
	uint8_t          channel;

} ExpanderUnit;


typedef struct __FreqShifterUnit
{
	FreqShifterContext *ct;
	uint32_t  			enable;
	int32_t    			deltaf;
	uint8_t             fade_step;
	uint8_t             auto_on_flag;
	uint32_t   		    param_cnt;
	uint8_t             channel;

} FreqShifterUnit;


typedef struct __HowlingDectorUnit
{
	HowlingContext 		*ct;
	uint32_t        	 enable;
	int32_t 			 suppression_mode;
	uint32_t   		     param_cnt;
	uint8_t              channel;

} HowlingDectorUnit;

typedef struct __SilenceDetectorUnit
{
	SilenceDetectorContext *ct;
	uint32_t        	   enable;
	uint32_t        	   level;
	uint8_t                channel;

} SilenceDetectorUnit;

typedef struct __VocalCutUnit
{
	VocalCutContext *ct;
	uint32_t 	   enable;
	int32_t 	   wetdrymix;
	uint32_t 	   vocal_cut_en;
	uint8_t        channel;

} VocalCutUnit;

typedef struct __VocalRemoveUnit
{
	VocalRemoverContext   *ct;
	uint32_t 	          enable;
	int32_t 	          lower_freq;
	int32_t 	          higher_freq;
	int32_t 	          step_size;
	uint32_t              vocal_remove_en;
	uint8_t        		  channel;
} VocalRemoveUnit;

typedef struct __ChorusUnit
{
	ChorusContext *ct;
	uint32_t 			 enable;
	int32_t 			 delay_length;
	int32_t 		     mod_depth;
	uint32_t   	 		 mod_rate;
	uint32_t   	 		 feedback;
	uint32_t   	 		 dry;
	uint32_t   	 		 wet;
	uint8_t              channel;

} ChorusUnit;

typedef struct __PitchShifterUnit
{
	PSContext 			*ct;
	uint32_t 			 enable;
	int32_t  			 semitone_steps;
	uint32_t   		     param_cnt;
	uint8_t              channel;

} PitchShifterUnit;
#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
typedef struct __PitchShifterProUnit
{
	PitchShifterProContext 			*ct;
	uint32_t 			 enable;
	int32_t  			 semitone_steps;
	uint32_t   		     param_cnt;
	uint8_t              channel;

} PitchShifterProUnit;
#endif
typedef struct __ReverbUnit
{
	ReverbContext	 *ct;
	uint32_t 		 enable;
	int32_t 		 dry_scale;
	int32_t 		 wet_scale;
	int32_t 		 width_scale;
	int32_t 		 roomsize_scale;
	int32_t 		 damping_scale;
	uint32_t         mono;
	uint32_t   		 param_cnt;
	uint8_t          channel;

} ReverbUnit;

typedef struct __PlateReverbUnit
{
	PlateReverbContext	 *ct;
	uint32_t 		 enable;
	int32_t highcut_freq;
	int32_t modulation_en;
	int32_t predelay; 
	int32_t diffusion;
	int32_t decay; 
	int32_t damping;
	int32_t wetdrymix;
	uint8_t channel;

} PlateReverbUnit;

#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
typedef struct __ReverbProUnit
{
	uint8_t *ct; 
	uint32_t enable;	
	uint32_t enable_bak;
	int32_t dry;
	int32_t wet;
	int32_t erwet;
	int32_t erfactor;
	int32_t erwidth;
	int32_t ertolate;
	int32_t rt60;
	int32_t delay;
	int32_t width;
	int32_t wander;
	int32_t spin;
	int32_t inputlpf;
	int32_t damplpf;
	int32_t basslpf;
	int32_t bassb;
	int32_t outputlpf;
	int32_t sample_rate;
	uint8_t channel;
} ReverbProUnit;
#endif
#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
typedef struct __ThreeDUnit
{
	ThreeDContext *ct;
	uint32_t 	   enable;
	int32_t  	   intensity;
	uint32_t   	   param_cnt;
	uint32_t 	   three_d_en;
	uint8_t        channel;

} ThreeDUnit;
#endif
#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
typedef struct __ThreeDPlusUnit
{
	ThreeDPlusContext *ct;
	uint32_t 	   enable;
	int32_t  	   intensity;
	uint32_t   	   param_cnt;
	uint32_t 	   three_d_en;
	uint8_t        channel;

} ThreeDPlusUnit;
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
typedef struct __VBUnit
{
	VBContext 	 *ct;
	uint32_t 	  enable;
	int32_t 	  f_cut;
	int32_t 	  intensity;
	int32_t 	  enhanced;
	uint32_t   	  param_cnt;
	uint32_t 	  vb_en;
	uint32_t      vb_en_bak;
	uint8_t       channel;

} VBUnit;
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
typedef struct __VBClassicUnit
{
	VBClassicContext 	 *ct;
	uint32_t 	  enable;
	int32_t 	  f_cut;
	int32_t 	  intensity;
	uint32_t   	  param_cnt;
	uint32_t 	  vb_en;
	uint8_t       channel;

} VBClassicUnit;
#endif
typedef struct __StereoWindenUnit
{
	StereoWidenerContext *ct;
	uint16_t 			 enable;
	int32_t 			 shaping;
	uint8_t              channel;

} StereoWindenUnit;
typedef struct __VoiceChangerUnit
{
	VoiceChangerContext *ct;
	uint32_t 			 enable;
	int32_t 			 pitch_ratio;
	int32_t 		     formant_ratio;
	uint32_t   	 		 param_cnt;
	uint8_t              channel;

} VoiceChangerUnit;
#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
typedef struct __VoiceChangerProUnit
{
	VoiceChangerProContext *ct;
	uint32_t 			 enable;
	uint32_t 			 enable_bak;
	int32_t 			 pitch_ratio;
	int32_t 		     formant_ratio;
	uint32_t   	 		 param_cnt;
	uint8_t              channel;

} VoiceChangerProUnit;
#endif
typedef struct __AecUnit
{
	BlueAECContext *ct;
	uint32_t 			 enable;
	uint8_t				 param_cnt;
	uint8_t				 param1_name_len;
	char				 param1_name[9];
	uint8_t 			 param1_state;
	uint16_t 			 param1_value_min;
	uint16_t 			 param1_value_max;
	uint16_t 			 es_level;
	uint8_t				 param2_name_len;
	char				 param2_name[10];
	uint8_t 			 param2_state;
	uint16_t 			 param2_value_min;
	uint16_t 			 param2_value_max;
	uint16_t 			 ns_level;

} AecUnit;

typedef struct __FilterParams
{
	uint32_t 	enable;
	int32_t		type;           /**< filter type, @see EQ_FILTER_TYPE_SET                               */
	uint32_t	f0;             /**< center frequency (peak) or mid-point frequency (shelf)             */
	int32_t		Q;              /**< quality factor (peak) or slope (shelf), format: Q6.10              */
	int32_t		gain;           /**< Gain in dB, format: Q8.8 */
} FilterParams;

typedef struct __EQUnit
{
	EQContext 		*ct;
	uint32_t         enable;
	uint32_t 		 pregain;
	uint32_t 		 filter_count;
	FilterParams	 eq_params[10];
	EQFilterParams  *filter_params;
	uint32_t   		 param_cnt;
	uint8_t          channel;

} EQUnit;

typedef struct __PingPongUnit
{
	PingPongContext  *ct;
	uint16_t         enable;
	int16_t          attenuation;
	int32_t          delay;
	int32_t  		 max_delay;
	int32_t          high_quality;
	int32_t          wetdrymix;
	int32_t          delay_samples;
	int32_t 		 max_delay_samples;
	uint8_t          *s;
	uint8_t          channel;
} PingPongUnit;

typedef struct __AutoWahUnit
{
	AutoWahContext  *ct;
	uint16_t         enable;
	uint16_t         modulation_rate;
	uint16_t         min_frequency;
	uint16_t         max_frequency;
	uint16_t         depth;
	uint16_t 		 dry;
	uint16_t 		 wet;
} AutoWahUnit;

typedef struct __PhaseControlUnit
{
	uint32_t      enable;
	uint32_t      phase_difference;
	uint8_t       channel;
} PhaseControlUnit;

typedef struct __GainControlUnit
{
	uint32_t      enable;
	uint32_t      mute;
	uint32_t      gain;
	uint8_t       channel;
} GainControlUnit;

//-----system var--------------------------//
typedef struct _ControlVariablesContext
{
	//for system control 0x01
	uint16_t            sys_mode;
	uint16_t            sys_reset;
	uint16_t            sys_sample_rate_en;
	uint16_t 		    sys_sample_rate;
	uint16_t            sys_mclk_src_en;
	uint16_t            sys_mclk_src;
	uint16_t            sys_default_set;
	
	//for System status 0x02
    uint16_t 			cpu_mips;
	uint16_t 			UsedRamSize;
	uint16_t 			AutoRefresh;//调音时音效参数发生改变，上位机会自动读取音效数据，1=允许上位读，0=不需要上位机读取
	uint16_t            CpuMaxFreq;
	uint16_t            CpuMaxRamSize;
    
	//for ADC0 PGA      0x03
    uint16_t            pga0_line1_l_en;
    uint16_t            pga0_line1_r_en;
    uint16_t            pga0_line2_l_en;
    uint16_t            pga0_line2_r_en;
    uint16_t            pga0_line4_l_en;
    uint16_t            pga0_line4_r_en;
    uint16_t            pga0_line5_l_en;
    uint16_t            pga0_line5_r_en;

	uint16_t            pga0_line1_pin;
    uint16_t            pga0_line2_pin;
    uint16_t            pga0_line4_pin;
    uint16_t            pga0_line5_pin;
	
    uint16_t            pga0_line1_l_gain;
    uint16_t            pga0_line1_r_gain;
    uint16_t            pga0_line2_l_gain;
    uint16_t            pga0_line2_r_gain;
    uint16_t            pga0_line4_l_gain;
    uint16_t            pga0_line4_r_gain;
    uint16_t            pga0_line5_l_gain;
    uint16_t            pga0_line5_r_gain;
	uint16_t            pga0_diff_mode;
	uint16_t            pga0_diff_l_gain;
	uint16_t            pga0_diff_r_gain;

	uint16_t            pga0_zero_cross;
	
    //for ADC0 DIGITAL  0x04
    uint16_t         	adc0_channel_en;
	uint16_t  			adc0_mute;
    uint16_t  			adc0_dig_l_vol;
    uint16_t  			adc0_dig_r_vol;
	uint16_t  			adc0_sample_rate;
	uint16_t  			adc0_lr_swap;
	uint16_t            adc0_dc_blocker;
	uint16_t            adc0_fade_time;
	uint16_t            adc0_mclk_src;
	uint16_t            adc0_dc_blocker_en;
	
	//for AGC0 ADC0     0x05
	uint16_t            adc0_agc_mode;
	uint16_t            adc0_agc_max_level;
	uint16_t            adc0_agc_target_level;
	uint16_t            adc0_agc_max_gain;
	uint16_t            adc0_agc_min_gain;
	uint16_t            adc0_agc_gainoffset;
	uint16_t            adc0_agc_fram_time;
	uint16_t            adc0_agc_hold_time;
	uint16_t            adc0_agc_attack_time;
	uint16_t            adc0_agc_decay_time;
	uint16_t            adc0_agc_noise_gate_en;
	uint16_t            adc0_agc_noise_threshold;
	uint16_t            adc0_agc_noise_gate_mode;
	uint16_t            adc0_agc_noise_time;
	
	//for ADC1 PGA      0x06 
	uint16_t             line3_l_mic1_en;
	uint16_t             line3_r_mic2_en;
    uint16_t             line3_l_mic1_gain;
	uint16_t             line3_l_mic1_boost;
    uint16_t             line3_r_mic2_gain;	
    uint16_t             line3_r_mic2_boost;	
	uint16_t             mic_or_line3;
	uint16_t             mic1_line3l_pin_en;
	uint16_t             mic2_line3r_pin_en;

	//for ADC1 DIGITAL  0x07
    uint16_t         	adc1_channel_en;
	uint16_t  			adc1_mute;
    uint16_t  			adc1_dig_l_vol;
    uint16_t  			adc1_dig_r_vol;
	uint16_t  			adc1_sample_rate;
	uint16_t  			adc1_lr_swap;
	uint16_t            adc1_dc_blocker;
	uint16_t            adc1_fade_time;
	uint16_t            adc1_mclk_src;
	uint16_t            adc1_dc_blocker_en;

	//for AGC1  ADC1    0x08
	uint16_t            adc1_agc_mode;
	uint16_t            adc1_agc_max_level;
	uint16_t            adc1_agc_target_level;
	uint16_t            adc1_agc_max_gain;
	uint16_t            adc1_agc_min_gain;
	uint16_t            adc1_agc_gainoffset;
	uint16_t            adc1_agc_fram_time;
	uint16_t            adc1_agc_hold_time;
	uint16_t            adc1_agc_attack_time;
	uint16_t            adc1_agc_decay_time;
	uint16_t            adc1_agc_noise_gate_en;
	uint16_t            adc1_agc_noise_threshold;
	uint16_t            adc1_agc_noise_gate_mode;
	uint16_t            adc1_agc_noise_time;

	//for DAC0          0x09
	uint16_t            dac0_en;
	uint16_t            dac0_sample_rate;
	uint16_t            dac0_dig_mute;
	uint16_t            dac0_dig_l_vol;
	uint16_t            dac0_dig_r_vol;
	uint16_t            dac0_dither;
	uint16_t            dac0_scramble;
	uint16_t            dac0_out_mode;
	uint16_t            dac0_pause_en;
	uint16_t            dac0_sample_mode;
	uint16_t            dac0_scf_mute; 
	uint16_t            dac0_fade_time;
	uint16_t            dac0_zeros_number;
	uint16_t            dac0_mclk_src;
	uint16_t            dac0_out_bit_len;
	
	//for DAC1          0x0a
	uint16_t            dac1_en;
	uint16_t            dac1_sample_rate;
	uint16_t            dac1_dig_mute;
	uint16_t            dac1_dig_l_vol;
	uint16_t            dac1_dig_r_vol;
	uint16_t            dac1_dither;
	uint16_t            dac1_scramble;
	uint16_t            dac1_out_mode;
	uint16_t            dac1_pause_en;
	uint16_t            dac1_sample_mode;
	uint16_t            dac1_scf_mute; 
	uint16_t            dac1_fade_time;
	uint16_t            dac1_zeros_number;
	uint16_t            dac1_mclk_src;
	uint16_t            dac1_out_bit_len;

	//for i2s0          0x0b
	uint16_t            i2s0_tx_en;
	uint16_t            i2s0_rx_en;
	uint16_t            i2s0_sample_rate;
	uint16_t            i2s0_mclk_src;
	uint16_t            i2s0_work_mode;
	uint16_t            i2s0_word_len;
	uint16_t            i2s0_stereo_mono;
	uint16_t            i2s0_fade_time;
	uint16_t            i2s0_format;
	uint16_t            i2s0_bclk_invert_en;
	uint16_t            i2s0_lrclk_invert_en;
	
	//for i2s1          0x0c
	uint16_t            i2s1_tx_en;
	uint16_t            i2s1_rx_en;
	uint16_t            i2s1_sample_rate;
	uint16_t            i2s1_mclk_src;
	uint16_t            i2s1_work_mode;
	uint16_t            i2s1_word_len;
	uint16_t            i2s1_stereo_mono;
	uint16_t            i2s1_fade_time;
	uint16_t            i2s1_format;
	uint16_t            i2s1_bclk_invert_en;
	uint16_t            i2s1_lrclk_invert_en;

	//for spdif
	uint16_t  			spdif_en;
	uint16_t  			spdif_sample_rate;
	uint16_t  			spdif_channel_mode;
	uint16_t  			spdif_in_gpio;
	uint16_t  			spdif_lock_status;

    //for Audio Effects
	AecUnit             mic_aec_unit;
	#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
    PcmDelayUnit        music_delay_unit;
	#endif
	ExciterUnit         music_exciter_unit;
    AutoTuneUnit 		auto_tune_unit;
    DRCUnit 	   		music_drc_unit;
	DRCUnit 	   		mic_drc_unit;
    DRCUnit     		rec_drc_unit;
    EchoUnit     		echo_unit;
    EchoUnit            guitar_echo_unit;
    ExpanderUnit		music_expander_unit;
    ExpanderUnit		mic_expander_unit;
	#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
	ThreeDUnit          music_threed_unit;
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
	ThreeDPlusUnit      music_threed_plus_unit;
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
	VBUnit              music_vb_unit;
	#endif
	#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
	VBUnit              rec_vb_unit;
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
	VBClassicUnit       music_vb_classic_unit;
	#endif
    PitchShifterUnit    pitch_shifter_unit;
	#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
	PitchShifterProUnit pitch_shifter_pro_unit;
	#endif
    ReverbUnit  		reverb_unit;
	PlateReverbUnit     plate_reverb_unit;
	#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
	ReverbProUnit       reverb_pro_unit;
	#endif
    VoiceChangerUnit    voice_changer_unit;
	#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
	VoiceChangerProUnit voice_changer_pro_unit;
	#endif
	FreqShifterUnit     freq_shifter_unit;
	HowlingDectorUnit   howling_dector_unit;
	SilenceDetectorUnit MicAudioSdct_unit;
	SilenceDetectorUnit MusicAudioSdct_unit;
	VocalCutUnit        vocal_cut_unit;
	VocalRemoveUnit     vocal_remove_unit;
	ChorusUnit          chorus_unit;
    EQUnit	   			music_pre_eq_unit;
	EQUnit	   			music_out_eq_unit;
    EQUnit	   			mic_pre_eq_unit;
    EQUnit	   			mic_bypass_eq_unit;
    EQUnit	   			mic_echo_eq_unit;
    EQUnit	   			mic_reverb_eq_unit;
    EQUnit	   			mic_out_eq_unit;
    EQUnit	   			rec_eq_unit;
    EQUnit	   			guitar_eq_unit;
	PhaseControlUnit    phase_control_unit;
	StereoWindenUnit    stereo_winden_unit;
	PingPongUnit        ping_pong_unit;
	AutoWahUnit         auto_wah_unit;
	//gain control
    GainControlUnit		aux_gain_control_unit;
    GainControlUnit		mic_bypass_gain_control_unit;
    GainControlUnit		mic_echo_control_unit;
    GainControlUnit		mic_reverb_gain_control_unit;
    GainControlUnit		mic_out_gain_control_unit;
    GainControlUnit		rec_bypass_gain_control_unit;
	GainControlUnit		rec_effect_gain_control_unit;
    GainControlUnit		rec_aux_gain_control_unit;
	GainControlUnit		rec_remind_gain_control_unit;
    GainControlUnit		rec_out_gain_control_unit;
    GainControlUnit		remind_key_gain_control_unit;
    GainControlUnit		remind_effect_gain_control_unit;
	GainControlUnit		i2s_gain_control_unit;
	GainControlUnit		bt_gain_control_unit;
	GainControlUnit		spdif_gain_control_unit;
	GainControlUnit		usb_gain_control_unit;
	GainControlUnit		Fade_gain_control_unit;
	GainControlUnit		rec_usb_out_gain_control_unit;
	GainControlUnit		guitar_gain_control_unit;
	
    //for system define
	uint16_t            sample_rate;
	uint16_t            crypto_en;
	uint32_t            password;
	uint8_t             IsEffectChangedByPcTool;
	uint8_t             adc_mic_channel_num;//for mic
	uint8_t             adc_line_channel_num;///for line ,fm ,bt ,usb ,i2s
	uint8_t             remind_type;
	uint8_t             UsbAudioMute;
	uint8_t             usb_audio_upload_flag;
	uint16_t            SamplesPerFrame;
	uint8_t             audio_effect_init_flag;
	uint16_t            beep_en;
	uint16_t            beepAddr;
	#ifdef CFG_FUNC_SHUNNING_EN
	uint8_t             ShunningMode;
	uint32_t            aux_out_dyn_gain;
	uint32_t            remind_out_dyn_gain;
	#endif
	#ifdef CFG_FUNC_DETECT_MIC_EN
	uint8_t             MicOnlin;
	uint8_t             Mic2Onlin;
	#endif
	#ifdef CFG_FUNC_DETECT_MIC_SEG_EN
	uint8_t             MicSegment;
	#endif
	#ifdef CFG_FUNC_DETECT_PHONE_EN
	uint8_t             EarPhoneOnlin;
	#endif
	uint16_t            UsbAudioVolume;
	uint16_t            UsbMicVolume;
	uint16_t            UsbMicMute;
    //------音效参数的最大值 来自调音参数数据-------------------------------//	
    uint32_t            max_chorus_wet;
    uint32_t            max_plate_reverb_roomsize;
    uint32_t            max_plate_reverb_wetdrymix;
	uint32_t            max_echo_delay;
	uint32_t            max_echo_depth;
	uint32_t 		    max_reverb_wet_scale;
	uint32_t 		    max_reverb_roomsize;
	int32_t			    max_reverb_pro_wet;
    int32_t             max_reverb_pro_erwet;	

}ControlVariablesContext;

EQFilterParams eq1_filter_buf[10];
EQFilterParams eq2_filter_buf[10];
EQFilterParams eq3_filter_buf[10];
EQFilterParams eq4_filter_buf[10];
EQFilterParams eq5_filter_buf[10];
EQFilterParams eq6_filter_buf[10];
EQFilterParams eq7_filter_buf[10];
EQFilterParams eq8_filter_buf[10];

extern ControlVariablesContext gCtrlVars;

//---system const-----------------------------//
extern const uint32_t SupportSampleRateList[13];
extern const uint8_t AutoTuneKeyTbl[13];
extern const uint8_t AutoTuneSnapTbl[3];
extern const uint8_t MIC_BOOST_LIST[5];
extern const int16_t DeltafTable[8];
extern const AUDIO_EFF_PARAMAS EFFECT_TAB[9];
extern const int32_t DRC_DEFAULT_TABLE[][3];
extern const int32_t EQ_DEFAULT_TABLE[][8];
extern const uint16_t DigVolTab_256[256];
extern const int16_t DigVolTab_64[64];
extern const int16_t DigVolTab_32[32];
extern const int16_t DigVolTab_16[16];
extern const int16_t EchoDlyTab_16[16];
extern const uint8_t ReverbRoomTab[16];
extern const uint8_t PlateReverbRoomTab[16];
extern const int32_t EXPANDER_DEFAULT_TABLE[][2];
extern const int32_t GAIN_CONTROL_TABLE[][16];
extern const int16_t BassTrebGainTable[16];
extern const unsigned char HunXiang[1958];
extern const unsigned char HanMai[1958];
extern const unsigned char DianYin[1958];
extern const unsigned char MoYin[1958];
extern const unsigned char NanBianNv[1958];
extern const unsigned char NvBianNan[1958];
extern const unsigned char WaWaYin[1958];
extern const unsigned char YuanSheng[1958];
extern const unsigned char Music[1074];
extern const unsigned char AECBuf[572];

//-----system function--------------------------//
extern EQUnit *eq_unit_aggregate[8];
extern DRCUnit *drc_unit_aggregate[3];
extern EQFilterParams *eq_param_aggregate[8];
extern ExpanderUnit *expander_unit_aggregate[2];
extern GainControlUnit *gain_unit_aggregate[16];

void CtrlVarsInit(void);
void DefaultParamgsInit(void);
void Line3MicPinSet(void);
void GlobalSampeRateSet(void);
void CpuVerification(void);
void GlobalSampeRateSet(void);
void GlobalMclkSet(void);
void UsbLoadAudioMode(uint16_t len,uint8_t *buff);
bool  AudioEffectListJudge(uint16_t len, const uint8_t *buff);
void Communication_Effect_Init(void);
void AudioLineSelSet(void);
void BeepEnable(void);
void Beep(int16_t *OutBuf,uint16_t n);
void AudioAnaChannelSet(int8_t ana_input_ch);
void MicLine3PinSet(void);
void AudioLine3MicSelect(void);
void SamplesFrameUpdataMsg(void);
void EffectUpdataMsg(void);
/***********     Audio effects  function  declaration **************/
#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
void AudioEffectPcmDelayInit(PcmDelayUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
void AudioEffectExciterInit(ExciterUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectAecInit(AecUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectExpanderInit(ExpanderUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectFreqShifterInit(FreqShifterUnit *unit);
void AudioEffectHowlingSuppressorInit(HowlingDectorUnit *unit);
void AudioEffectPitchShifterInit(PitchShifterUnit *unit, uint8_t channel, uint32_t sample_rate);
#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
void AudioEffectPitchShifterProInit(PitchShifterProUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
void AudioEffectAutoTuneInit(AutoTuneUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectVoiceChangerInit(VoiceChangerUnit *unit, uint8_t channel, uint32_t sample_rate);
#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
void AudioEffectVoiceChangerProInit(VoiceChangerProUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
void AudioEffectEchoInit(EchoUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectReverbInit(ReverbUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectPlateReverbInit(PlateReverbUnit *unit, uint8_t channel, uint32_t sample_rate);
#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
void AudioEffectReverbProInit(ReverbProUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
void AudioEffectDRCInit(DRCUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectEQInit(EQUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectSilenceDectorInit(SilenceDetectorUnit *unit,uint8_t channel, uint32_t sample_rate);
void AudioEffectVocalCutInit(VocalCutUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectVocalRemoveInit(VocalRemoveUnit *unit,  uint8_t channel, uint32_t sample_rate);
void AudioEffectChorusInit(ChorusUnit *unit,  uint8_t channel, uint32_t sample_rate);
#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
void AudioEffectThreeDInit(ThreeDUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
void AudioEffectThreeDPlusInit(ThreeDPlusUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
void AudioEffectVBInit(VBUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
void AudioEffectVBClassicInit(VBClassicUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
void AudioEffectPcmDelayConfig(PcmDelayUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
void AudioEffectExciterConfig(ExciterUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectExpanderThresholdConfig(ExpanderUnit *unit);
void AudioEffectExpanderConfig(ExpanderUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectFreqShifterConfig(FreqShifterUnit *unit);
void AudioEffectPitchShifterConfig(PitchShifterUnit *unit);
#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
void AudioEffectPitchShifterProConfig(PitchShifterProUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
void AudioEffectAutoTuneConfig(AutoTuneUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectVoiceChangerConfig(VoiceChangerUnit *unit, uint8_t channel, uint32_t sample_rate);
#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
void AudioEffectVoiceChangerProConfig(VoiceChangerProUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
void AudioEffectEchoConfig(EchoUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectReverbConfig(ReverbUnit *unit);
void AudioEffectPlateReverbConfig(PlateReverbUnit *unit);
void AudioEffectPlateReverbHighcutModulaConfig(PlateReverbUnit *unit, uint8_t channel, uint32_t sample_rate);
#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
void AudioEffectReverProbConfig(ReverbProUnit *unit,uint32_t sample_rate);
#endif
void AudioEffectEQPregainConfig(EQUnit *unit);
void AudioEffectEQFilterConfig(EQUnit *unit, uint32_t sample_rate);
void AudioEffectVocalRemoveConfig(VocalRemoveUnit *unit,  uint8_t channel, uint32_t sample_rate);
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
void AudioEffectVBConfig(VBUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
void AudioEffectVBClassicConfig(VBClassicUnit *unit, uint8_t channel, uint32_t sample_rate);
#endif
void AudioEffectHowlingSuppressorConfig(HowlingDectorUnit *unit);
void AudioEffectDRCConfig(DRCUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectPhaseApply(PhaseControlUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n,  uint8_t channel);
#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
void AudioEffectPcmDelayApply(PcmDelayUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
#endif
void AudioEffectAecApply(AecUnit *unit, int16_t *u_pcm_in, int16_t *d_pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectExciterApply(ExciterUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectExpanderApply(ExpanderUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectFreqShifterApply(FreqShifterUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectHowlingSuppressorApply(HowlingDectorUnit *unit, int16_t *pcm_in, int16_t *pcm_out, int32_t n);
void AudioEffectPregainApply(GainControlUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n, uint8_t channel);
void AudioEffectPitchShifterApply(PitchShifterUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n, uint8_t channel);
#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
void AudioEffectPitchShifterProApply(PitchShifterProUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
#endif
void AudioEffectAutoTuneApply(AutoTuneUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectVoiceChangerApply(VoiceChangerUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
void AudioEffectVoiceChangerProApply(VoiceChangerProUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
#endif
void AudioEffectReverbApply(ReverbUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectPlateReverbApply(PlateReverbUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
void AudioEffectReverbProApply(ReverbProUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
#endif
void AudioEffectEchoApply(EchoUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectDRCApply(DRCUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectEQApply(EQUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n, uint8_t channel);
void AudioEffectSilenceDectorApply(SilenceDetectorUnit *unit, int16_t *pcm_in, int32_t n);
void AudioEffectVocalCutApply(VocalCutUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectVocalRemoveApply(VocalRemoveUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectChorusApply(ChorusUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
void AudioEffectThreeDApply(ThreeDUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
#endif
#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
void AudioEffectThreeDPlusApply(ThreeDPlusUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
void AudioEffectVBApply(VBUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
void AudioEffectVBClassicApply(VBClassicUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
#endif
void AudioEffectStereoWidenerInit(StereoWindenUnit *unit, uint32_t sample_rate);
void AudioEffectStereoWidenerApply(StereoWindenUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectPinPongApply(PingPongUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectPingPongInit(PingPongUnit *unit, uint32_t sample_rate);
void AudioEffectAutoWahInit(AutoWahUnit *unit, uint32_t sample_rate);
void AudioEffectAutoWahApply(AutoWahUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);

void AudioEffectProcess(AudioCoreContext *pAudioCore);
void AudioMusicProcess(AudioCoreContext *pAudioCore);
void AudioBypassProcess(AudioCoreContext *pAudioCore);
void AudioEffectProcessBTRecord(AudioCoreContext *pAudioCore);
void AudioEffectProcessBTHF(AudioCoreContext *pAudioCore);
void AudioNoAppProcess(AudioCoreContext *pAudioCore);

void AudioProcess(void *Buf);
void AudioOutProcess(void);
void AudioEffectsInit(void);
void AudioEffectsDeInit(void);
void AudioEffectsAllDisable(void);
void AudioEffectModeSel(uint8_t mode, uint8_t init_flag);
void GetAudioEffectMaxValue(void);
void ReverbStepSet(uint8_t ReverbStep);
#ifdef CFG_FUNC_MIC_TREB_BASS_EN
void MicBassTrebAjust(int16_t 	BassGain,	int16_t TrebGain);
#endif
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
void MusicBassTrebAjust(int16_t 	BassGain,	int16_t TrebGain);
#endif
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
void LoadEqMode(const uint8_t *buff);
#endif
#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__CTRLVARS_H__

