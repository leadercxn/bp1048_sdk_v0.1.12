/**
*************************************************************************************
* @file	    audio_effect_library.h
* @brief	Audio Effect Library
*
* @author	ZHAO Ying (Alfred)
* @version	@see AUDIO_EFFECT_LIBRARY_VERSION
*
* &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
*************************************************************************************
*/

#ifndef __AUDIO_EFFECT_LIBRARY_H__
#define __AUDIO_EFFECT_LIBRARY_H__

// audio effect headers
#include "auto_tune.h"
#include "auto_wah.h"
#include "chorus.h"
#include "dc_blocker.h"
#include "drc.h"
#include "echo.h"
#include "eq.h"
#include "exciter.h"
#include "expander.h"
#include "freqshifter.h"
#include "howling_suppressor.h"
#include "noise_gate.h"
#include "pcm_delay.h"
#include "pingpong.h"
#include "pitch_shifter.h"
#include "pitch_shifter_pro.h"
#include "plate_reverb.h"
#include "reverb.h"
#include "reverb_pro.h"
#include "silence_detector.h"
#include "stereo_widener.h"
#include "three_d.h"
#include "three_d_plus.h"
#include "virtual_bass.h"
#include "virtual_bass_classic.h"
#include "vocal_remover.h"
#include "voice_changer.h"
#include "voice_changer_pro.h"



// audio effect library version
#define AUDIO_EFFECT_LIBRARY_VERSION "1.35.1"

// audio effect versions (Info below is for convenience only. In the event of any contradiction, the version info in effect header shall prevail.)
#define EFFECT_VERSION_AUTO_TUNE                    "1.3.0"
#define EFFECT_VERSION_AUTO_WAH                     "1.0.0"
#define EFFECT_VERSION_CHORUS                       "1.1.1"
#define EFFECT_VERSION_DC_BLOCKER                   "1.1.1"
#define EFFECT_VERSION_DYNAMIC_RANGE_COMPRESSOR     "3.0.3"
#define EFFECT_VERSION_ECHO                         "1.9.0"
#define EFFECT_VERSION_EQ                           "8.0.0"
#define EFFECT_VERSION_EXCITER				        "1.1.2"
#define EFFECT_VERSION_NOISE_SUPPRESSOR             "1.2.0"
#define EFFECT_VERSION_FREQUENCY_SHIFTER            "1.6.3"
#define EFFECT_VERSION_HOWLING_SUPPRESSOR           "2.0.1"
#define EFFECT_VERSION_NOISE_GATE                   "1.1.3"
#define EFFECT_VERSION_DELAY                        "2.0.0"
#define EFFECT_VERSION_PINGPONG						"1.4.1"
#define EFFECT_VERSION_PITCH_SHIFTER                "1.7.6"
#define EFFECT_VERSION_PITCH_SHIFTER_PRO            "2.1.1"
#define EFFECT_VERSION_PLATE_REVERB                 "1.2.6"
#define EFFECT_VERSION_REVERB                       "1.9.2"
#define EFFECT_VERSION_REVERB_PRO                   "1.2.3"
#define EFFECT_VERSION_SILENCE_DETECTOR             "1.1.1"
#define EFFECT_VERSION_STEREO_WIDENER               "1.2.0"
#define EFFECT_VERSION_3D                           "3.3.0"
#define EFFECT_VERSION_3D_PLUS                      "1.0.1"
#define EFFECT_VERSION_VIRTUAL_BASS                 "4.1.4"
#define EFFECT_VERSION_VIRTUAL_BASS_CLASSIC         "3.16.5"
#define EFFECT_VERSION_VOCAL_REMOVER                "1.3.2"
#define EFFECT_VERSION_VOICE_CHANGER                "1.6.3"
#define EFFECT_VERSION_VOICE_CHANGER_PRO            "2.3.1"


#endif // __AUDIO_EFFECT_LIBRARY_H__
