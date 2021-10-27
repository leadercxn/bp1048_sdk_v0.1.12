/*******************************************************
 *         MVSilicon Audio Effects Parameters
 *
 *                All Right Reserved
 *******************************************************/

#ifndef __AUDIO_EFFECT_H__
#define __AUDIO_EFFECT_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include "audio_adc.h"
#include "app_config.h"
#include "bt_config.h"

//****************************************************************************************
//				   音效功能配置	   
//1.山景音效算法均由软件实现，因此开启相关音效时，需要考虑系统资源消耗情况；
//    具体消耗情况与ACPWorkbench中的资源消耗状态栏对应,或由MCPSStart，MCPSStop函数获取;
//2.默认音效参数在audio_effect.h中；
//3.不建议关闭如下音效宏(关闭后，ACPWorkbench工具上对应的的音效会不显示，且导出的xxxx.c中数组的大小会改变；
//4.调音参数更新方法:
//    1)在线调音后，可由ACPWorkbench工具直接下载到flash中?
//    2)或导出并替换sdk中effect_parameter的xxxx.c;

#ifdef CFG_FUNC_MIC_KARAOKE_EN
#define  CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN		 (1)
#define  CFG_AUDIO_EFFECT_MUSIC_SILENCE_DECTOR_EN        (1)
#define  CFG_AUDIO_EFFECT_MUSIC_3D_EN                    (1)
#define  CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN               (1)//此音效只适用于BP1048P2或BP1064型号芯片
#define  CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN          (1)
#define  CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN  (1)
#define  CFG_AUDIO_EFFECT_MUSIC_DELAY_EN	             (1)
#define  CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN	 (1)
#define  CFG_AUDIO_EFFECT_MUSIC_EXCITER_EN	             (1)
#if BT_HFP_SUPPORT == ENABLE
#define  CFG_AUDIO_EFFECT_MIC_AEC_EN	                 (1)//目前只支持8K,16K采样率
#else
#define  CFG_AUDIO_EFFECT_MIC_AEC_EN	                 (0)//目前只支持8K,16K采样率
#endif

#define  CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN	     (1)
#define  CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN            (1)
#define  CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN          (1)
#define  CFG_AUDIO_EFFECT_MIC_SILENCE_DECTOR_EN          (1)
#define  CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN			 (1)
#define  CFG_AUDIO_EFFECT_MIC_AUTO_TUNE_EN				 (1)
#define  CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_EN			 (1)
#define  CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN		 (1)
#define  CFG_AUDIO_EFFECT_MIC_ECHO_EN					 (1)

#define  CFG_AUDIO_EFFECT_MIC_REVERB_EN					 (1)
#define  CFG_AUDIO_EFFECT_MIC_PLATE_REVERB_EN            (1)
#define  CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN              (1)
#define  CFG_AUDIO_EFFECT_MUSIC_DRC_EN				 	 (1)
#define  CFG_AUDIO_EFFECT_MIC_DRC_EN				 	 (1)
#define  CFG_AUDIO_EFFECT_REC_DRC_EN					 (1)
#define  CFG_AUDIO_EFFECT_MUSIC_PRE_EQ_EN				 (1)
#define  CFG_AUDIO_EFFECT_MUSIC_OUT_EQ_EN				 (1)
#define  CFG_AUDIO_EFFECT_MIC_PRE_EQ_EN				     (1)
#define  CFG_AUDIO_EFFECT_MIC_BYPASS_EQ_EN				 (1)
#define  CFG_AUDIO_EFFECT_MIC_ECHO_EQ_EN				 (1)
#define  CFG_AUDIO_EFFECT_MIC_REVERB_EQ_EN			     (1)
#define  CFG_AUDIO_EFFECT_MIC_OUT_EQ_EN					 (1)
#define  CFG_AUDIO_EFFECT_REC_EQ_EN						 (1)
#define  CFG_AUDIO_EFFECT_AUX_GAIN_CONTROL_EN			 (1)
#define  CFG_AUDIO_EFFECT_MIC_BYPASS_GAIN_CONTROL_EN     (1)
#define  CFG_AUDIO_EFFECT_MIC_ECHO_GAIN_CONTROL_EN	 	 (1)
#define  CFG_AUDIO_EFFECT_MIC_REVERB_GAIN_CONTROL_EN 	 (1)
#define  CFG_AUDIO_EFFECT_MIC_OUT_GAIN_CONTROL_EN	 	 (1)
#define  CFG_AUDIO_EFFECT_REC_BYPASS_GAIN_CONTROL_EN	 (1)
#define  CFG_AUDIO_EFFECT_REC_EFFECT_GAIN_CONTROL_EN	 (1)
#define  CFG_AUDIO_EFFECT_REC_AUX_GAIN_CONTROL_EN		 (1)
#define  CFG_AUDIO_EFFECT_REC_REMIND_GAIN_CONTROL_EN	 (1)
#define  CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN		 (1)
#define  CFG_AUDIO_EFFECT_REMIND_KEY_GAIN_CONTROL_EN	 (1)
#define  CFG_AUDIO_EFFECT_REMIND_EFFECT_GAIN_CONTROL_EN	 (1)
#define  CFG_AUDIO_EFFECT_I2S_IN_GAIN_CONTROL_EN	     (1)
#define  CFG_AUDIO_EFFECT_BT_IN_GAIN_CONTROL_EN	         (1)
#define  CFG_AUDIO_EFFECT_USB_IN_GAIN_CONTROL_EN	     (1)
#define  CFG_AUDIO_EFFECT_SPDIF_IN_GAIN_CONTROL_EN	     (1)
#define  CFG_AUDIO_EFFECT_VOCAL_CUT_EN                   (1)
#define  CFG_AUDIO_EFFECT_VOCAL_REMOVE_EN                (1)
#define  CFG_AUDIO_EFFECT_CHORUS_EN                      (1)
#define  CFG_AUDIO_EFFECT_PHASE_EN                       (1)
#define  CFG_AUDIO_EFFECT_STEREO_WIDEN_EN                (1)
#define  CFG_AUDIO_EFFECT_AUTOWAH_EN                     (1)
#define  CFG_AUDIO_EFFECT_PINGPONG_EN                    (1)
#define  CFG_AUDIO_EFFECT_GUITAR_EQ_EN                   (0)
#define  CFG_AUDIO_EFFECT_GUITAR_ECHO_EN                 (0)
#define  CFG_AUDIO_EFFECT_GUITAR_GAIN_EN                 (0)
#else
#define  CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN		 (1)
#define  CFG_AUDIO_EFFECT_MUSIC_SILENCE_DECTOR_EN        (1)
#define  CFG_AUDIO_EFFECT_MUSIC_3D_EN                    (1)
#define  CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN               (1)//此音效只适用于BP1048P2或BP1064型号芯片
#define  CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN          (1)
#define  CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN            (1)
#define  CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN  (1)
#define  CFG_AUDIO_EFFECT_MUSIC_DELAY_EN	             (1)
#define  CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN	 (1)
#define  CFG_AUDIO_EFFECT_MUSIC_EXCITER_EN	             (1)
#define  CFG_AUDIO_EFFECT_MIC_AEC_EN	                 (1)//目前只支持8K,16K采样率
#define  CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN	     (0)
#define  CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN            (0)
#define  CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN          (0)
#define  CFG_AUDIO_EFFECT_MIC_SILENCE_DECTOR_EN          (0)
#define  CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN			 (0)
#define  CFG_AUDIO_EFFECT_MIC_AUTO_TUNE_EN				 (0)
#define  CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_EN			 (0)
#define  CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN		 (0)
#define  CFG_AUDIO_EFFECT_MIC_ECHO_EN					 (0)
#define  CFG_AUDIO_EFFECT_MIC_REVERB_EN					 (0)
#define  CFG_AUDIO_EFFECT_MIC_PLATE_REVERB_EN            (0)
#define  CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN              (0)
#define  CFG_AUDIO_EFFECT_MUSIC_DRC_EN				 	 (1)
#define  CFG_AUDIO_EFFECT_MIC_DRC_EN				 	 (0)
#define  CFG_AUDIO_EFFECT_REC_DRC_EN					 (1)
#define  CFG_AUDIO_EFFECT_MUSIC_PRE_EQ_EN				 (1)
#define  CFG_AUDIO_EFFECT_MUSIC_OUT_EQ_EN				 (1)
#define  CFG_AUDIO_EFFECT_MIC_PRE_EQ_EN				     (0)
#define  CFG_AUDIO_EFFECT_MIC_BYPASS_EQ_EN				 (0)
#define  CFG_AUDIO_EFFECT_MIC_ECHO_EQ_EN				 (0)
#define  CFG_AUDIO_EFFECT_MIC_REVERB_EQ_EN			     (0)
#define  CFG_AUDIO_EFFECT_MIC_OUT_EQ_EN					 (1)
#define  CFG_AUDIO_EFFECT_REC_EQ_EN						 (1)
#define  CFG_AUDIO_EFFECT_AUX_GAIN_CONTROL_EN			 (1)
#define  CFG_AUDIO_EFFECT_MIC_BYPASS_GAIN_CONTROL_EN     (0)
#define  CFG_AUDIO_EFFECT_MIC_ECHO_GAIN_CONTROL_EN	 	 (0)
#define  CFG_AUDIO_EFFECT_MIC_REVERB_GAIN_CONTROL_EN 	 (0)
#define  CFG_AUDIO_EFFECT_MIC_OUT_GAIN_CONTROL_EN	 	 (0)
#define  CFG_AUDIO_EFFECT_REC_BYPASS_GAIN_CONTROL_EN	 (0)
#define  CFG_AUDIO_EFFECT_REC_EFFECT_GAIN_CONTROL_EN	 (0)
#define  CFG_AUDIO_EFFECT_REC_AUX_GAIN_CONTROL_EN		 (0)
#define  CFG_AUDIO_EFFECT_REC_REMIND_GAIN_CONTROL_EN	 (0)
#define  CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN		 (0)
#define  CFG_AUDIO_EFFECT_REMIND_KEY_GAIN_CONTROL_EN	 (0)
#define  CFG_AUDIO_EFFECT_REMIND_EFFECT_GAIN_CONTROL_EN	 (0)
#define  CFG_AUDIO_EFFECT_I2S_IN_GAIN_CONTROL_EN	     (1)
#define  CFG_AUDIO_EFFECT_BT_IN_GAIN_CONTROL_EN	         (1)
#define  CFG_AUDIO_EFFECT_USB_IN_GAIN_CONTROL_EN	     (1)
#define  CFG_AUDIO_EFFECT_SPDIF_IN_GAIN_CONTROL_EN	     (1)
#define  CFG_AUDIO_EFFECT_VOCAL_CUT_EN                   (0)
#define  CFG_AUDIO_EFFECT_VOCAL_REMOVE_EN                (0)
#define  CFG_AUDIO_EFFECT_CHORUS_EN                      (0)
#define  CFG_AUDIO_EFFECT_PHASE_EN                       (1)
#define  CFG_AUDIO_EFFECT_STEREO_WIDEN_EN                (1)
#endif

/***********     Audio effects software parameter settings   **************/

#define CFG_MUSIC_NOISE_SUPPRESSOR_THRESHOLD            (  -65*100)  //dB
#define CFG_MUSIC_NOISE_SUPPRESSOR_RATIO                (    3)
#define CFG_MUSIC_NOISE_SUPPRESSOR_ATTACK_TIME          (    5)  //ms
#define CFG_MUSIC_NOISE_SUPPRESSOR_RELEASE_TIME         (  160)  //ms

#define CFG_MIC_NOISE_SUPPRESSOR_THRESHOLD              (  -40*100)  //dB
#define CFG_MIC_NOISE_SUPPRESSOR_RATIO                  (    3)
#define CFG_MIC_NOISE_SUPPRESSOR_ATTACK_TIME            (    5)  //ms
#define CFG_MIC_NOISE_SUPPRESSOR_RELEASE_TIME           (  160)  //ms

#define CFG_MUSIC_PARAM_VB_CUTOFF_FREQ                  (  100)  //unit in Hz
#define CFG_MUSIC_PARAM_VB_INTENSITY                    (   35)  //unit in percent
#define CFG_MUSIC_PARAM_VB_ENHANCED                     (    1)

#define CFG_MUSIC_PARAM_VB_CLASSIC_CUTOFF_FREQ          (  100)  //unit in Hz
#define CFG_MUSIC_PARAM_VB_CLASSIC_INTENSITY            (   35)  //unit in percent

#define CFG_MUSIC_PARAM_3D_DEPTH                        (   50)  //unit in percent

#define CFG_MUSIC_PARAM_3D_PLUS_DEPTH                   (   50)  //unit in percent

#define CFG_MUSIC_PITCH_SHIFTER_PRO_SEMITONE_STEP       (   70)  //unit in cents
#define CFG_MUSIC_PITCH_SHIFTER_PRO_FRAME_SIZE          (  512)  //unit in sample

#define CFG_MIC_PITCH_SHIFTER_SEMITONE_STEP             (   70)  //unit in cents
#define CFG_MIC_PITCH_SHIFTER_FRAME_SIZE                (  512)  //unit in sample

#define CFG_FREQ_SHIFT_HZ                               (    5)  //unit: Hz
#define CFG_HOWLING_MODE                                (    2)  //0: No suppression, 1: Precise, 2: Massive


#define CFG_AUDIO_EFFECT_AUX_PREGAIN            	    ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_MIC_BYPASS_PREGAIN             ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_MIC_ECHO_PREGAIN       	    ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_MIC_REVERB_PREGAIN     	    ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_MIC_OUT_PREGAIN        	    ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REC_BYPASS_PREGAIN             ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REC_EFFECT_PREGAIN             ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REC_AUX_PREGAIN                ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REC_REMIND_PREGAIN             ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REC_OUT_PREGAIN                ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REMIND_KEY_PREGAIN             ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REMIND_EFFECT_PREGAIN          ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_I2S_IN_PREGAIN                 ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_BT_IN_PREGAIN                  ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_USB_IN_PREGAIN                 ( 4096)  //  0.000 dB
#define CFG_AUDIO_EFFECT_SPDIF_IN_PREGAIN               ( 4096)  //  0.000 dB


#define CFG_AUDIO_EFFECT_AUX_PREGAIN_MUTE       	    (    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_MIC_BYPASS_PREGAIN_MUTE    	(    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_MIC_ECHO_MUTE       	    	(    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_MIC_REVERB_PREGAIN_MUTE    	(    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_MIC_OUT_PREGAIN_MUTE       	(    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REC_BYPASS_PREGAIN_MUTE        (    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REC_EFFECT_PREGAIN_MUTE        (    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REC_AUX_PREGAIN_MUTE       	(    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REC_REMIND_PREGAIN_MUTE       	(    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REC_OUT_PREGAIN_MUTE       	(    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REMIND_KEY_PREGAIN_MUTE       	(    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_REMIND_EFFECT_PREGAIN_MUTE     (    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_I2S_IN_PREGAIN_MUTE            (    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_BT_IN_PREGAIN_MUTE             (    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_USB_IN_PREGAIN_MUTE            (    0)  //  0.000 dB
#define CFG_AUDIO_EFFECT_SPDIF_IN_PREGAIN_MUTE          (    0)  //  0.000 dB

//---Gain channel :  1= MONO  2= STEREO---------------//
#define CFG_AUDIO_EFFECT_AUX_CHANNEL       	            (    2)  //
#define CFG_AUDIO_EFFECT_MIC_BYPASS_CHANNEL    	        (    2)  //
#define CFG_AUDIO_EFFECT_MIC_ECHO_CHANNEL       	    (    2)  //
#define CFG_AUDIO_EFFECT_MIC_REVERB_CHANNEL    	        (    2)  //
#define CFG_AUDIO_EFFECT_MIC_OUT_CHANNEL       	        (    2)  //
#define CFG_AUDIO_EFFECT_REC_BYPASS_CHANNEL             (    2)  //
#define CFG_AUDIO_EFFECT_REC_EFFECT_CHANNEL             (    2)  //
#define CFG_AUDIO_EFFECT_REC_AUX_CHANNEL       	        (    2)  //
#define CFG_AUDIO_EFFECT_REC_REMIND_CHANNEL       	    (    2)  //
#define CFG_AUDIO_EFFECT_REC_OUT_CHANNEL      	        (    2)  //
#define CFG_AUDIO_EFFECT_REMIND_KEY_CHANNEL       	    (    2)  //
#define CFG_AUDIO_EFFECT_REMIND_EFFECT_CHANNEL          (    2)  //
#define CFG_AUDIO_EFFECT_I2S_IN_CHANNEL                 (    2)  //
#define CFG_AUDIO_EFFECT_BT_IN_CHANNEL                  (    2)  //
#define CFG_AUDIO_EFFECT_USB_IN_CHANNEL                 (    2)  //
#define CFG_AUDIO_EFFECT_SPDIF_IN_CHANNEL               (    2)  //

#define CFG_MIC_AEC_ES_LEVEL                            (   0)   //0: off, 1: minimum suppression, ..., 5: maximum suppression. 
#define CFG_MIC_AEC_NS_LEVEL                            (   0)   //0: off, 1: minimum suppression, ..., 5: maximum suppression. 

#define CFG_MUSIC_PCM_DELAY                             ( 50 )

#define CFG_MUSIC_EXCITER_FCUT                          ( 1000 )
#define CFG_MUSIC_EXCITER_DRY                           ( 100 )  
#define CFG_MUSIC_EXCITER_WET                           ( 100 ) 

#define CFG_MUSIC_VOCALCUT_WETDRYMIX                    ( 100 )

#define CFG_MUSIC_VOCALREMOVE_LOW_FREQ                  ( 200 )
#define CFG_MUSIC_VOCALREMOVE_HIGH_FREQ                 ( 15000 )
#define CFG_MUSIC_VOCALREMOVE_STEP_SIZE                 ( 256 )

#define CFG_CHORUS_DELAY_LENGTH                         ( 13 )
#define CFG_CHORUS_MODULATION_DEPTH                     ( 3 )
#define CFG_CHORUS_MODULATION_RATE                      ( 10 )
#define CFG_CHORUS_FEEDBACK                             ( 30 )
#define CFG_CHORUS_DRY                                  ( 90 )
#define CFG_CHORUS_WET                                  ( 60 )

#define CFG_MIC_AUTO_TUNE_SNAP                          ( 1 )
#define CFG_MIC_AUTO_TUNE_KEY                           ( 2 )

#define CFG_MIC_VOICE_CHANGER_PITCH_RATIO               (  200)
#define CFG_MIC_VOICE_CHANGER_FORMANT_RATIO             (  150)
#define CFG_MIC_VOICE_CHANGER_PRO_PITCH_RATIO           (  200)
#define CFG_MIC_VOICE_CHANGER_PRO_FORMANT_RATIO         (  150)

#define CFG_MIC_ECHO_DELAY                              ( 200)  //200 ms(10143)  //230 ms
#define CFG_MIC_ECHO_ATTENUATION                        (14636)  //-7 dB
#define CFG_MIC_ECHO_DIRECT_SND_EN                      (    0)
#define CFG_MIC_ECHO_CUTOFF_FREQ                        ( 2000)  //2000 Hz

#define CFG_MIC_REVERB_DRY_SCALE                        (   100)
#define CFG_MIC_REVERB_WET_SCALE                        (  90)
#define CFG_MIC_REVERB_WIDTH_SCALE                      (  90)
#define CFG_MIC_REVERB_ROOM_SIZE                        (   76)
#define CFG_MIC_REVERB_DAMPING                          (   35)

#define CFG_MIC_PLATEREVERB_HIGHCUT_FREQ                ( 20000)  //20000 Hz
#define CFG_MIC_PLATEREVERB_MODULATION_EN               (   1)
#define CFG_MIC_PLATEREVERB_PREDELAY                    (   40)
#define CFG_MIC_PLATEREVERB_DIFFUSION                   (   50)
#define CFG_MIC_PLATEREVERB_DECAY                       (   50)
#define CFG_MIC_PLATEREVERB_DAMPING                     (    5)
#define CFG_MIC_PLATEREVERB_WETDRYMIX                   (   80)

#define CFG_MUSIC_DRC_MODE                              (    0)  //Full Band
#define CFG_MUSIC_DRC_CROSSOVER_FREQ                    (  300)  //unit in Hz
#define CFG_MUSIC_DRC_CROSSOVER_FQ1                     (  724)  //0.707
#define CFG_MUSIC_DRC_CROSSOVER_FQ2                     (  724)  //0.707
#define CFG_MUSIC_DRC_PREGAIN1                          ( 4096)  //0.00dB
#define CFG_MUSIC_DRC_PREGAIN2                          ( 4096)  //0.00dB
#define CFG_MUSIC_DRC_THRESHHOLD1                       (    0)  //     0dB
#define CFG_MUSIC_DRC_RATIO1                            (  100)  //100:1
#define CFG_MUSIC_DRC_ATTACK_TIME1                      (    1)  //unit in ms
#define CFG_MUSIC_DRC_RELEASE_TIME1                     ( 1000)  //unit in ms
#define CFG_MUSIC_DRC_THRESHHOLD2                       (    0)  //     0dB
#define CFG_MUSIC_DRC_RATIO2                            (  100)  //100:1
#define CFG_MUSIC_DRC_ATTACK_TIME2                      (    1)  //unit in ms
#define CFG_MUSIC_DRC_RELEASE_TIME2                     ( 1000)  //unit in ms
#define CFG_MUSIC_DRC_THRESHHOLD3                       (    0)  //     0dB
#define CFG_MUSIC_DRC_RATIO3                            (  100)  //100:1
#define CFG_MUSIC_DRC_ATTACK_TIME3                      (    1)  //unit in ms
#define CFG_MUSIC_DRC_RELEASE_TIME3                     ( 1000)  //unit in ms

#define CFG_MIC_DRC_MODE                                (    0)  //Full Band
#define CFG_MIC_DRC_CROSSOVER_FREQ                      (  300)  //unit in Hz
#define CFG_MIC_DRC_CROSSOVER_FQ1                       (  724)  //0.707
#define CFG_MIC_DRC_CROSSOVER_FQ2                       (  724)  //0.707
#define CFG_MIC_DRC_PREGAIN1                            ( 4096)  //0.00dB
#define CFG_MIC_DRC_PREGAIN2                            ( 4096)  //0.00dB
#define CFG_MIC_DRC_THRESHHOLD1                         (    0)  //     0dB
#define CFG_MIC_DRC_RATIO1                              (  100)  //100:1
#define CFG_MIC_DRC_ATTACK_TIME1                        (    1)  //unit in ms
#define CFG_MIC_DRC_RELEASE_TIME1                       ( 1000)  //unit in ms
#define CFG_MIC_DRC_THRESHHOLD2                         (    0)  //     0dB
#define CFG_MIC_DRC_RATIO2                              (  100)  //100:1
#define CFG_MIC_DRC_ATTACK_TIME2                        (    1)  //unit in ms
#define CFG_MIC_DRC_RELEASE_TIME2                       ( 1000)  //unit in ms
#define CFG_MIC_DRC_THRESHHOLD3                         (    0)  //     0dB
#define CFG_MIC_DRC_RATIO3                              (  100)  //100:1
#define CFG_MIC_DRC_ATTACK_TIME3                        (    1)  //unit in ms
#define CFG_MIC_DRC_RELEASE_TIME3                       ( 1000)  //unit in ms

#define CFG_REC_DRC_MODE                                (    0)  //Full Band
#define CFG_REC_DRC_CROSSOVER_FREQ                      (  300)  //unit in Hz
#define CFG_REC_DRC_CROSSOVER_FQ1                       (  724)  //0.707
#define CFG_REC_DRC_CROSSOVER_FQ2                       (  724)  //0.707
#define CFG_REC_DRC_PREGAIN1                            ( 4096)  //0.00dB
#define CFG_REC_DRC_PREGAIN2                            ( 4096)  //0.00dB
#define CFG_REC_DRC_THRESHHOLD1                         (    0)  //     0dB
#define CFG_REC_DRC_RATIO1                              (  100)  //100:1
#define CFG_REC_DRC_ATTACK_TIME1                        (    1)  //unit in ms
#define CFG_REC_DRC_RELEASE_TIME1                       ( 1000)  //unit in ms
#define CFG_REC_DRC_THRESHHOLD2                         (    0)  //     0dB
#define CFG_REC_DRC_RATIO2                              (  100)  //100:1
#define CFG_REC_DRC_ATTACK_TIME2                        (    1)  //unit in ms
#define CFG_REC_DRC_RELEASE_TIME2                       ( 1000)  //unit in ms
#define CFG_REC_DRC_THRESHHOLD3                         (    0)  //     0dB
#define CFG_REC_DRC_RATIO3                              (  100)  //100:1
#define CFG_REC_DRC_ATTACK_TIME3                        (    1)  //unit in ms
#define CFG_REC_DRC_RELEASE_TIME3                       ( 1000)  //unit in ms


#define CFG_MUSIC_EQ_COUNT							    (    4)
#define CFG_MUSIC_EQ_PREGAIN						    (	  0)
#define CFG_MUSIC_EQ_1_TYPE						        EQ_FILTER_TYPE_BAND_PASS
#define CFG_MUSIC_EQ_1_F0							    (  100)
#define CFG_MUSIC_EQ_1_Q							    (	 724)
#define CFG_MUSIC_EQ_1_GAIN						        (  0 )
#define CFG_MUSIC_EQ_2_TYPE						        EQ_FILTER_TYPE_BAND_PASS
#define CFG_MUSIC_EQ_2_F0							    (  1000)
#define CFG_MUSIC_EQ_2_Q							    (	 724)
#define CFG_MUSIC_EQ_2_GAIN						        (  0)
#define CFG_MUSIC_EQ_3_TYPE						        EQ_FILTER_TYPE_BAND_PASS
#define CFG_MUSIC_EQ_3_F0							    (  5000)
#define CFG_MUSIC_EQ_3_Q							    (	 724)
#define CFG_MUSIC_EQ_3_GAIN						        (  0)
#define CFG_MUSIC_EQ_4_TYPE						        EQ_FILTER_TYPE_BAND_PASS
#define CFG_MUSIC_EQ_4_F0							    (  10000)
#define CFG_MUSIC_EQ_4_Q							    (	 724)
#define CFG_MUSIC_EQ_4_GAIN						        (  0)

#define CFG_MIC_BYPASS_EQ_COUNT							(    2)
#define CFG_MIC_BYPASS_EQ_PREGAIN						(	  0)
#define CFG_MIC_BYPASS_EQ_1_TYPE						EQ_FILTER_TYPE_HIGH_PASS
#define CFG_MIC_BYPASS_EQ_1_F0							(  0x0032)
#define CFG_MIC_BYPASS_EQ_1_Q							(	0x02cd)
#define CFG_MIC_BYPASS_EQ_1_GAIN						(  0x0400 )
#define CFG_MIC_BYPASS_EQ_2_TYPE						EQ_FILTER_TYPE_PEAKING
#define CFG_MIC_BYPASS_EQ_2_F0							(  0x1f40)
#define CFG_MIC_BYPASS_EQ_2_Q							(	0x0800)
#define CFG_MIC_BYPASS_EQ_2_GAIN						(  0x043d)

#define CFG_MIC_PRE_EQ_COUNT							(    2)
#define CFG_MIC_PRE_EQ_PREGAIN						    (	  0)
#define CFG_MIC_PRE_EQ_1_TYPE						    EQ_FILTER_TYPE_HIGH_PASS
#define CFG_MIC_PRE_EQ_1_F0							    (  0x0032)
#define CFG_MIC_PRE_EQ_1_Q							    (	0x02cd)
#define CFG_MIC_PRE_EQ_1_GAIN						    (  0x0400 )
#define CFG_MIC_PRE_EQ_2_TYPE						    EQ_FILTER_TYPE_PEAKING
#define CFG_MIC_PRE_EQ_2_F0							    (  0x1f40)
#define CFG_MIC_PRE_EQ_2_Q							    (	0x0800)
#define CFG_MIC_PRE_EQ_2_GAIN						    (  0x043d)

#define CFG_MIC_ECHO_EQ_COUNT							(    2)
#define CFG_MIC_ECHO_EQ_PREGAIN						    (	  0)
#define CFG_MIC_ECHO_EQ_1_TYPE							EQ_FILTER_TYPE_PEAKING
#define CFG_MIC_ECHO_EQ_1_F0							(  3941)
#define CFG_MIC_ECHO_EQ_1_Q							    (	 724)
#define CFG_MIC_ECHO_EQ_1_GAIN							(  1797)
#define CFG_MIC_ECHO_EQ_2_TYPE							EQ_FILTER_TYPE_HIGH_SHELF
#define CFG_MIC_ECHO_EQ_2_F0							(  604)
#define CFG_MIC_ECHO_EQ_2_Q							    (	 724)
#define CFG_MIC_ECHO_EQ_2_GAIN							( -1797)

#define CFG_MIC_REVERB_EQ_COUNT							(    2)
#define CFG_MIC_REVERB_EQ_PREGAIN						(	  0)
#define CFG_MIC_REVERB_EQ_1_TYPE						EQ_FILTER_TYPE_HIGH_PASS
#define CFG_MIC_REVERB_EQ_1_F0							(  0x0064)
#define CFG_MIC_REVERB_EQ_1_Q							(	0x02cd)
#define CFG_MIC_REVERB_EQ_1_GAIN						(  0x0470)
#define CFG_MIC_REVERB_EQ_2_TYPE						EQ_FILTER_TYPE_LOW_PASS
#define CFG_MIC_REVERB_EQ_2_F0							(  0x3e80)
#define CFG_MIC_REVERB_EQ_2_Q							(	0x02cd)
#define CFG_MIC_REVERB_EQ_2_GAIN						(  0x0371)

#define CFG_MIC_OUT_EQ_COUNT							(    2	  )
#define CFG_MIC_OUT_EQ_PREGAIN						    (	  0   )
#define CFG_MIC_OUT_EQ_1_TYPE							EQ_FILTER_TYPE_HIGH_SHELF
#define CFG_MIC_OUT_EQ_1_F0								(  0x07d0)
#define CFG_MIC_OUT_EQ_1_Q								(	0x02cd)
#define CFG_MIC_OUT_EQ_1_GAIN							(  0x02d5)
#define CFG_MIC_OUT_EQ_2_TYPE							EQ_FILTER_TYPE_LOW_SHELF
#define CFG_MIC_OUT_EQ_2_F0							 	(  200   )
#define CFG_MIC_OUT_EQ_2_Q							 	(	0x02cd)
#define CFG_MIC_OUT_EQ_2_GAIN							(  0x02d5)

#define CFG_REC_EQ_COUNT							    (    1)
#define CFG_REC_EQ_PREGAIN						 	    (	  0)
#define CFG_REC_EQ_TYPE							 	    EQ_FILTER_TYPE_LOW_SHELF
#define CFG_REC_EQ_F0								    (  200)
#define CFG_REC_EQ_Q								    (	 724)
#define CFG_REC_EQ_GAIN							 	    ( -768)


/***********     Audio effects hardware parameter settings   **************/

#define CFG_SYS_PLL_CLK_SRC                             (      0) //0=PLL_CLK_1 ,1=PLL_CLK_2 :PLL_CLK_INDEX
#define CFG_SYS_MODE                                    (      0) //0=standby 1=sleep
#define CFG_SYS_RESET                                   (      0) //0=standby 1=reset
#define CFG_SYS_SAMPLE_RATE_EN                          (      1) //1= en 0 = disable
#define CFG_SYS_SAMPLE_RATE                             (      7) //0=8000,1=11025,2=12000,3=16000,4=22050,5=24000,6=32000,7=44100,8=48000,
#define CFG_SYS_MCLK_SRC_EN                             (      1) //1= en 0 = disable
#define CFG_SYS_MCLK_SRC                                (      0) //0= PLL 1 = GPIO_IN 2=OSC_IN

#define CFG_LINE1_L_GAIN                                (     18) //0DB//
#define CFG_LINE1_R_GAIN                                (     18) //0DB//
#define CFG_LINE2_L_GAIN                         		(     18) //0DB//
#define CFG_LINE2_R_GAIN                         		(     18) //0DB//
#define CFG_LINE4_L_GAIN                           		(     21) //0DB//
#define CFG_LINE4_R_GAIN                           		(     21) //0DB//
#define CFG_LINE5_L_GAIN                           		(     21) //0DB//
#define CFG_LINE5_R_GAIN                           		(     21) //0DB//
#define CFG_DIFF_MODE                            		(      0) //0 =L SIG,R SIG   1 = L DIFF IN R SIG,2= L SIG,R DIFF IN, 3=L,R DIFF IN
#define CFG_DIFF_L_GAIN                          		(      0) //0:0dB;1:6dB;2:10dB;3:15dB
#define CFG_DIFF_R_GAIN                          		(      0) //0:0dB;1:6dB;2:10dB;3:15dB
#define CFG_PGA1_ZERO_CROSS                      		(      0) //
#define CFG_LINE1_PIN_TYPE                              (      3)//0：no pin; 1 : line1 left pin; 2 : line1 right pin; 3 : both pin;
#define CFG_LINE2_PIN_TYPE                              (      3)//0：no pin; 1 : line2 left pin; 2 : line2 right pin; 3 : both pin;
#define CFG_LINE4_PIN_TYPE                              (      3)//0：no pin; 1 : line4 left pin; 2 : line4 right pin; 3 : both pin;
#define CFG_LINE5_PIN_TYPE                              (      3)//0：no pin; 1 : line5 left pin; 2 : line5 right pin; 3 : both pin;

#define CFG_ADC0_LR_CHANNEL_EN                   		(      3) //0=LR disable, 1=ADC0 left digital en,2=ADC0 right digital en,3=ADC0 left+rithg digital en,
#define CFG_ADC0_DIG_MUTE_EN                     		(      0) //0=umute 1=ADC1 left mute, 2=ADC1  right mute, 3= ADC1 all mute,
#define CFG_ADC0_LEFT_DIG_VOLUME                 		(   4095) //4096=0dB  0~0x3fff
#define CFG_ADC0_RIGHT_DIG_VOLUME                		(   4095) //4096=0dB  0~0x3fff
#define CFG_ADC0_SAMPLE_RATE                     		(      7)//0=8000,1=11025,2=12000,3=16000,4=22050,5=24000,6=32000,7=44100,8=48000,
#define CFG_ADC0_LEFT_RIGHT_SWAP                 		(      0) //0= disable  1 = enble 
#define CFG_ADC0_HPC                             		(      0) // 
#define CFG_ADC0_FADE_TIME                       		(      0) //0=disable 1~255ms 
#define CFG_ADC0_MCLK_SRC                        		(      0)//0= PLL_CLOCK1 1 = PLL_CLOCK2  2=OSC_IN 3=GPIO_IN0 4=GPIO_IN1

#define CFG_ADC0_AGC_MODE                        		(      0)//0=dis 1=left_en 2= right_en 3= left+right_en
#define CFG_ADC0_AGC_MAX_LEVEL                   		(      0)//0~31
#define CFG_ADC0_AGC_TARGET_LEVEL                		(      0)//0~255
#define CFG_ADC0_AGC_MAX_GAIN                    		(      0)//0~63
#define CFG_ADC0_AGC_MIN_GAIN                    		(      0)//0~63
#define CFG_ADC0_AGC_OFFSET                      		(      0)//0~15
#define CFG_ADC0_AGC_FRAM_TIME                   		(     10)//1~4096ms
#define CFG_ADC0_AGC_HOLD_TIME                   		(     10)//1~4096ms
#define CFG_ADC0_AGC_ATTACK_TIME                 		(     10)//1~4096ms
#define CFG_ADC0_AGC_DECAY_TIME                  		(     10)//1~4096ms
#define CFG_ADC0_AGC_NOISE_GATE_EN               		(      0)//0=disable 1=enable
#define CFG_ADC0_AGC_NOISE_THRESHOLD             		(      5)//0~31
#define CFG_ADC0_AGC_NOISE_GATE_MODE             		(      0)//0=ADC0 output level  1=ADC0 input level
#define CFG_ADC0_AGC_NOISE_GATE_HOLD_TIME        		(      0)//0~4096ms

#define CFG_LINE3_L_MIC1_EN                             (      1)
#define CFG_LINE3_R_MIC2_EN                             (      1)
#define CFG_LINE3_L_MIC1_GAIN                           (      16) //
#define CFG_LINE3_L_MIC1_BOOST                          (      0) //0= bypass 1 = 0db 2= +6db 3= +12db 4= +20db
#define CFG_LINE3_R_MIC2_GAIN                           (      16) //
#define CFG_LINE3_R_MIC2_BOOST                          (      0) //0= bypass 1 = 0db 2= +6db 3= +12db 4= +20db
#define CFG_PGA1_ZERO_CROSS                      		(      0) //0= disable 1 = adc0_l_en 2= adc0_r_en  3= adc0_l_r_en
#define CFG_MIC_LINE_DISP_TYPE                          (      0)//0= mic1,mic2;1 = line3; 2 = mic1,line3_r;3 = mic2,line3_l
#define CFG_MIC1_LINE3L_PIN_EN                          (      1)//0= no mic1/line3l pin disp, 1= enable mic1/line3l pin disp
#define CFG_MIC2_LINE3R_PIN_EN                          (      1)//0= no mic2/line3r pin disp, 1= enable mic2/line3r pin disp

#define CFG_ADC1_LR_CHANNEL_EN                   		(      3) //0=LR disable, 1=ADC0 left digital en,2=ADC0 right digital en,3=ADC0 left+rithg digital en,	 
#define CFG_ADC1_DIG_MUTE_EN                     		(      0) //0=umute 1=ADC1 left mute, 2=ADC1  right mute, 3= ADC1 all mute,
#define CFG_ADC1_LEFT_DIG_VOLUME                 		(   4095) //4096=0dB  0~0x3fff
#define CFG_ADC1_RIGHT_DIG_VOLUME                		(   4095) //4096=0dB  0~0x3fff
#define CFG_ADC1_SAMPLE_RATE                     		(      7)//0=8000,1=11025,2=12000,3=16000,4=22050,5=24000,6=32000,7=44100,8=48000,
#define CFG_ADC1_LEFT_RIGHT_SWAP                 		(      0) //0= disable  1 = enble 
#define CFG_ADC1_HPC                             		(      0) // 
#define CFG_ADC1_FADE_TIME                       		(      0) //0=disable 1~255ms 
#define CFG_ADC1_MCLK_SRC                        		(      0)//0= PLL_CLOCK1 1 = PLL_CLOCK2  2=OSC_IN 3=GPIO_IN0 4=GPIO_IN1
	 
#define CFG_ADC1_AGC_MODE                        		(      0)//0=dis 1=left_en 2= right_en 3= left+right_en
#define CFG_ADC1_AGC_MAX_LEVEL                   		(      0)//0~31
#define CFG_ADC1_AGC_TARGET_LEVEL                		(      0)//0~255
#define CFG_ADC1_AGC_MAX_GAIN                    		(      0)//0~63
#define CFG_ADC1_AGC_MIN_GAIN                    		(      0)//0~63
#define CFG_ADC1_AGC_OFFSET                      		(      0)//0~15
#define CFG_ADC1_AGC_FRAM_TIME                   		(     10)//1~4096ms
#define CFG_ADC1_AGC_HOLD_TIME                   		(     10)//1~4096ms
#define CFG_ADC1_AGC_ATTACK_TIME                 		(     10)//1~4096ms
#define CFG_ADC1_AGC_DECAY_TIME                  		(     10)//1~4096ms
#define CFG_ADC1_AGC_NOISE_GATE_EN               		(      0)//0=disable 1=enable
#define CFG_ADC1_AGC_NOISE_THRESHOLD             		(      5)//0~31
#define CFG_ADC1_AGC_NOISE_GATE_MODE             		(      0)//0=ADC0 output level  1=ADC0 input level
#define CFG_ADC1_AGC_NOISE_GATE_HOLD_TIME        		(      0)//0~4096ms

#define CFG_DAC0_MODULE_EN                       		(      3)//0=disable, 1=left enable,2=right enable,3=left + right enable,
#define CFG_DAC0_SAMPLE_RATE                     		(      7)//0=8000,1=11025,2=12000,3=16000,4=22050,5=24000,6=32000,7=44100,8=48000,
#define CFG_DAC0_DIG_MUTE_EN                     		(      0)//0=umute 1=left_mute, 2=right_mute 3=all_mute
#define CFG_DAC0_LEFT_DIG_VOLUME                 		(   2053) //4096=0dB  0~0x3fff 
#define CFG_DAC0_RIGHT_DIG_VOLUME                		(   2053) //4096=0dB  0~0x3fff 	 
#define CFG_DAC0_DITHER_EN                       		(      0)//0=disable other=enable
#define CFG_DAC0_SCRAMBLE_EN                     		(      0)//0=disable 1=DWA 2=RANDON_DWA 3=BUTTERFLY_DWA
#define CFG_DAC0_OUT_MODE                        		(      0)//{0:Li->Lo,Ri->Ro} {1:Ri->Lo,Li->Ro} {2:(Li+Ri)/2->Lo,(Li+Ri)/2->Ro}{3:(Li+Ri)/2->Lo,(Li+Ri)/2->Ro,180}
#define CFG_DAC0_PAUSE_EN                        		(      0)//0=PLAY 1=pause
#define CFG_DAC0_SAMPLE_EDGE_MODE                		(      0)//0=posedge 1=negedge
#define CFG_DAC0_SCF_MUTE_MODE                   		(      0)//0=all_unmute, 1=mute scf, left 2=mute scf right, 3=mute all scf
#define CFG_DAC0_FADE_TIME                       		(      0)//0=disable, other=1~255ms
#define CFG_DAC0_ZERO_NUM                        		(      0)//0=512,1=1024,2=2048,3=4096,4=8192,5=16384,6=32768,7=65535
#define CFG_DAC0_MCLK_SRC                        		(      0)//0= PLL_CLOCK1 1 = PLL_CLOCK2  2=OSC_IN 3=GPIO_IN0 4=GPIO_IN1
#define CFG_DAC0_DOUT_WIDTH                      		(      0)//0=16BIT 2= 24BIT_1 3=24BIT_2

#define CFG_DAC1_MODULE_EN                       		(      1)//0=disable, 1=left enable,2=right enable,3=left + right enable,
#define CFG_DAC1_SAMPLE_RATE                     		(      7)//0=8000,1=11025,2=12000,3=16000,4=22050,5=24000,6=32000,7=44100,8=48000,
#define CFG_DAC1_DIG_MUTE_EN                     		(      0)//0=umute 1=left_mute, 2=right_mute 3=all_mute
#define CFG_DAC1_LEFT_DIG_VOLUME                 		(   2053) //4096=0dB  0~0x3fff 
#define CFG_DAC1_RIGHT_DIG_VOLUME                		(   2053) //4096=0dB  0~0x3fff 	 
#define CFG_DAC1_DITHER_EN                       		(      0)//0=disable other=enable
#define CFG_DAC1_SCRAMBLE_EN                     		(      0)//0=disable 1=DWA 2=RANDON_DWA 3=BUTTERFLY_DWA
#define CFG_DAC1_OUT_MODE                        		(      0)//{0:Li->Lo,Ri->Ro} {1:Ri->Lo,Li->Ro} {2:(Li+Ri)/2->Lo,(Li+Ri)/2->Ro}{3:(Li+Ri)/2->Lo,(Li+Ri)/2->Ro,180}
#define CFG_DAC1_PAUSE_EN                        		(      0)//0=PLAY 1=pause
#define CFG_DAC1_SAMPLE_EDGE_MODE                		(      0)//0=posedge 1=negedge
#define CFG_DAC1_SCF_MUTE_MODE                   		(      0)//0=all_unmute, 1=mute scf, left 2=mute scf right, 3=mute all scf
#define CFG_DAC1_FADE_TIME                       		(      0)//0=disable, other=1~255ms
#define CFG_DAC1_ZERO_NUM                        		(      0)//0=512,1=1024,2=2048,3=4096,4=8192,5=16384,6=32768,7=65535
#define CFG_DAC1_MCLK_SRC                        		(      0)//0= PLL_CLOCK1 1 = PLL_CLOCK2  2=OSC_IN 3=GPIO_IN0 4=GPIO_IN1
#define CFG_DAC1_DOUT_WIDTH                      		(      0)//0=16BIT 2= 24BIT_1 3=24BIT_2

#define CFG_I2S0_TX_EN                           		(      0)//0=disable 1=enable
#define CFG_I2S0_RX_EN                           		(      0)//0=disable 1=enable
#define CFG_I2S0_SAMPLE_RATE                     		(      7)//0=8000,1=11025,2=12000,3=16000,4=22050,5=24000,6=32000,7=44100,8=48000,9=88200,10=96000,11=176000,12=196000,
#define CFG_I2S0_MCLK_SRC                        		(      0)//0= PLL_CLOCK1 1 = PLL_CLOCK2  2=OSC_IN 3=GPIO_IN0 4=GPIO_IN1
#define CFG_I2S0_WORK_MODE                       		(      0)//0=master, 1=slaver
#define CFG_I2S0_WORD_LEN                        		(      0)//0=16bit, 1=20bit, 2=24bit,3=32bit,  
#define CFG_I2S0_MONO_EN                         		(      0)//0=disable mono, 1=enable mono
#define CFG_I2S0_FADE_TIME                       		(      0)//0=disable, other=1~255ms
#define CFG_I2S0_FORMAT                          		(      2)//0=I2S_FORMAT_RIGHT, 1=I2S_FORMAT_LEFT,2=I2S_FORMAT_I2S,3=I2S_FORMAT_DSPA,4=I2S_FORMAT_DSPA
#define CFG_I2S0_BCLK_INVERT_EN                  		(      0)//0= disable 1=enable
#define CFG_I2S0_LRCLK_INVERT_EN                 		(      0)//0= disable 1=enable

#define CFG_I2S1_TX_EN                           		(       0)//0=disable 1=enable
#define CFG_I2S1_RX_EN                           		(       0)//0=disable 1=enable
#define CFG_I2S1_SAMPLE_RATE                     		(       7)//0=8000,1=11025,2=12000,3=16000,4=22050,5=24000,6=32000,7=44100,8=48000,9=88200,10=96000,11=176000,12=196000,
#define CFG_I2S1_MCLK_SRC                        		(       0)//0= PLL_CLOCK1 1 = PLL_CLOCK2  2=OSC_IN 3=GPIO_IN0 4=GPIO_IN1
#define CFG_I2S1_WORK_MODE                       		(       0)//0=master, 1=slaver
#define CFG_I2S1_WORD_LEN                        		(       0)//0=16bit, 1=20bit, 2=24bit,3=32bit,  
#define CFG_I2S1_MONO_EN                         		(       0)//0=disable mono, 1=enable mono
#define CFG_I2S1_FADE_TIME                       		(       0)//0=disable, other=1~255ms
#define CFG_I2S1_FORMAT                          		(       2)//0=I2S_FORMAT_RIGHT, 1=I2S_FORMAT_LEFT,2=I2S_FORMAT_I2S,3=I2S_FORMAT_DSPA,4=I2S_FORMAT_DSPA
#define CFG_I2S1_BCLK_INVERT_EN                  		(       0)//0= disable 1=enable
#define CFG_I2S1_LRCLK_INVERT_EN                 		(       0)//0= disable 1=enable

#define CFG_SPDIF_EN                             		(       0)//0= disable, 1=RX enable,2= TX enable
#define CFG_SPDIF_SAMPLE_RATE                    		(       7)///0=8000,1=11025,2=12000,3=16000,4=22050,5=24000,6=32000,7=44100,8=48000,9=88200,10=96000,11=176000,12=192000,
#define CFG_SPDIF_CHANNEL_MODE                   		(       0)//0= in RX and TX:L,R valid,  1=in RX and TX:L valid, 2= in RX, L,R valid:in TX ,R = L ,

void EffectPcmBufClear(uint32_t SampleLen);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__AUDIO_EFFECT_MIC_H__
