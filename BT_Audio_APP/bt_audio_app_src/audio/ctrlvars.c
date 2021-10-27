/**
 **************************************************************************************
 * @file    ctrlvars.c
 * @brief   Control Variables Definition
 * 
 * @author  Cecilia Wang
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include <stdint.h>
#include "debug.h"
#include "app_config.h"
#include "audio_effect.h"
#include "audio_effect_library.h"
#include "clk.h"
#include "ctrlvars.h"
#include "spi_flash.h"
#include "timeout.h"
#include "delay.h"
#include "breakpoint.h"
#include "nds32_intrinsic.h"


extern uint8_t            effect_sum;
extern uint16_t           effect_list[50];
extern uint32_t           effect_addr[50];
extern uint32_t           eff_addr_compare[50];

ControlVariablesContext gCtrlVars;

const AUDIO_EFF_PARAMAS EFFECT_TAB[9]={
#ifdef CFG_FUNC_MIC_KARAOKE_EN	
	{EFFECT_MODE_HunXiang,         HunXiang,   sizeof(HunXiang),  "HunXiang"},
	{EFFECT_MODE_DianYin,          DianYin,    sizeof(DianYin),   "DianYin"},
	{EFFECT_MODE_HanMai,           HanMai,     sizeof(HanMai),    "HanMai"},
	{EFFECT_MODE_MoYin,            MoYin,      sizeof(MoYin),     "MoYin"},
	{EFFECT_MODE_NanBianNv,        NanBianNv,  sizeof(NanBianNv), "NanBianNv"},
	{EFFECT_MODE_NvBianNan,        NvBianNan,  sizeof(NvBianNan), "NvBianNan"},
	{EFFECT_MODE_WaWaYin,          WaWaYin,    sizeof(WaWaYin),   "WaWaYin"},
	{EFFECT_MODE_YuanSheng,        YuanSheng,  sizeof(YuanSheng), "YuanSheng"},
	{EFFECT_MODE_HFP_AEC,	       AECBuf,     sizeof(AECBuf), 	  "AECBuf"},
#else
	{EFFECT_MODE_NORMAL,           Music,      sizeof(Music),	"Music"},
	{EFFECT_MODE_NORMAL,           Music,      sizeof(Music),	"Music"},
	{EFFECT_MODE_NORMAL,           Music,      sizeof(Music),	"Music"},
	{EFFECT_MODE_NORMAL,           Music,      sizeof(Music),	"Music"},
	{EFFECT_MODE_NORMAL,           Music,      sizeof(Music),	"Music"},
	{EFFECT_MODE_NORMAL,           Music,      sizeof(Music),	"Music"},
	{EFFECT_MODE_NORMAL,           Music,      sizeof(Music),	"Music"},
	{EFFECT_MODE_HFP_AEC,	       AECBuf,     sizeof(AECBuf), 	"AECBuf"},
	{0xff,              (uint8_t *)0x00,	(uint16_t)0x00,		""},
#endif

};


const uint8_t AutoTuneKeyTbl[13] ={ 
 'a','A','b','B','C','d','D','e','E','F','g','G','X',
};
 
const uint8_t AutoTuneSnapTbl[3] ={ 
'n','u','l',
};

const int16_t DeltafTable[8] = {
 -7,-5,-3,3,5,7,0,
};

const uint8_t MIC_BOOST_LIST[5]={
	4,//dis
	0,//0db
	1,//6db
	2,//12db
	3,//20db
};

const uint32_t SupportSampleRateList[13]={
	8000,
    11025,
    12000, 
    16000, 
    22050, 
    24000, 
    32000, 
    44100, 
    48000,
//////i2s//////////////////
    88200,
    96000,
    176400,
    192000,
};

const uint8_t PlateReverbRoomTab[16]={

	0,10,11,13,14,15,20,25,30,35,40,45,48,50,52,70,

};
const uint8_t ReverbRoomTab[16]={
	
	0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,80,

};
	
const int16_t EchoDlyTab_16[16] =
{
    0,		10,	    50,	    100,	120,	140,	160,	180,  
    200,	210,	220,	230,	240,	260,	280,	320,
};

const int16_t DigVolTab_16[16] =
{
	0,  	332,	460,	541,	636,	748,	880,	1035,  
	1218,	1433,	1686,	1984,	2434,	2946,	3500,	4096
};

const int16_t DigVolTab_32[32] =
{
	0,		31,		36,		42,		49,		58,		67,		78,
	91,		107,	125,	147,	173,	204,	240,	282,
	332,	391,	460,	541,	636,	748,	880,	1035,
	1218,	1433,	1686,	1984,	2434,	2946,	3500,	4096
};

const int16_t DigVolTab_64[64] =
{
      0,    3,    4,    4,    5,    5,    6,    6,    7,    8,    9,   10,   12,   13,   15,   16, 
     18,   21,   23,   26,   29,   33,   36,   41,   46,   52,   58,   65,   73,   82,   92,  103, 
    115,  129,  145,  163,  183,  205,  230,  258,  290,  325,  365,  410,  459,  516,  578,  649, 
    728,  817,  917, 1029, 1154, 1295, 1453, 1630, 1829, 2052, 2303, 2584, 2899, 3253, 3650, 4095, 
};

const uint16_t DigVolTab_256[256] =
{
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001,
    0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002,
    0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003,
    0x003, 0x003, 0x003, 0x003, 0x004, 0x004, 0x004, 0x004, 0x004, 0x005, 0x005, 0x005, 0x006, 0x006, 0x006, 0x007,
    0x007, 0x008, 0x008, 0x009, 0x009, 0x00A, 0x00A, 0x00B, 0x00B, 0x00C, 0x00C, 0x00D, 0x00D, 0x00E, 0x00E, 0X00F,
    0x010, 0x011, 0x012, 0x013, 0x014, 0x015, 0x016, 0x017, 0x018, 0x019, 0x01a, 0x01b, 0x01d, 0x01e, 0x01f, 0x021,
    0x022, 0x023, 0x025, 0x027, 0x028, 0x02a, 0x02c, 0x02e, 0x030, 0x032, 0x034, 0x037, 0x039, 0x03c, 0x03e, 0x041,
    0x044, 0x047, 0x04a, 0x04d, 0x051, 0x054, 0x058, 0x05c, 0x060, 0x064, 0x068, 0x06d, 0x072, 0x077, 0x07c, 0x082,
    0x087, 0x08d, 0x093, 0x09a, 0x0a1, 0x0a8, 0x0af, 0x0b7, 0x0bf, 0x0c7, 0x0d0, 0x0d9, 0x0e3, 0x0ed, 0x0f8, 0x102,
    0x10e, 0x11a, 0x126, 0x133, 0x141, 0x14f, 0x15e, 0x16d, 0x17d, 0x18e, 0x1a0, 0x1b2, 0x1c5, 0x1d9, 0x1ee, 0x204,
    0x21a, 0x232, 0x24b, 0x265, 0x280, 0x29c, 0x2ba, 0x2d8, 0x2f9, 0x31a, 0x33d, 0x362, 0x388, 0x3b0, 0x3d9, 0x405,
    0x432, 0x462, 0x493, 0x4c7, 0x4fd, 0x535, 0x570, 0x5ad, 0x5ed, 0x630, 0x676, 0x6bf, 0x70b, 0x75b, 0x7ae, 0x805,
    0x85f, 0x8be, 0x921, 0x988, 0x9f3, 0xa64, 0xad9, 0xb54, 0xbd4, 0xc59, 0xce5, 0xd76, 0xe0e, 0xead, 0xf53, 0xfff
};

const int32_t EQ_DEFAULT_TABLE[][8] =
{
	{CFG_AUDIO_EFFECT_MUSIC_PRE_EQ_EN,	CFG_AUDIO_EFFECT_MUSIC_PRE_EQ_EN,	CFG_AUDIO_EFFECT_MIC_BYPASS_EQ_EN,  CFG_AUDIO_EFFECT_MIC_PRE_EQ_EN,     CFG_AUDIO_EFFECT_MIC_ECHO_EQ_EN,  CFG_AUDIO_EFFECT_MIC_REVERB_EQ_EN, 	CFG_AUDIO_EFFECT_MIC_OUT_EQ_EN, 	CFG_AUDIO_EFFECT_REC_EQ_EN  },
	{CFG_MUSIC_EQ_COUNT,   		  		CFG_MUSIC_EQ_COUNT,   		  		CFG_MIC_BYPASS_EQ_COUNT,   			CFG_MIC_PRE_EQ_COUNT,   			CFG_MIC_ECHO_EQ_COUNT,   		  CFG_MIC_REVERB_EQ_COUNT,   			CFG_MIC_OUT_EQ_COUNT,   		   	CFG_REC_EQ_COUNT			},        //eq count
	{CFG_MUSIC_EQ_PREGAIN,		  		CFG_MUSIC_EQ_PREGAIN,		  		CFG_MIC_BYPASS_EQ_PREGAIN, 			CFG_MIC_PRE_EQ_PREGAIN, 			CFG_MIC_ECHO_EQ_PREGAIN, 		  CFG_MIC_REVERB_EQ_PREGAIN, 			CFG_MIC_OUT_EQ_PREGAIN, 	       	CFG_REC_EQ_PREGAIN			},        //pregain
	{CFG_MUSIC_EQ_1_F0,	  		  		CFG_MUSIC_EQ_1_F0,	  		  		CFG_MIC_BYPASS_EQ_1_F0,				CFG_MIC_PRE_EQ_1_F0,				CFG_MIC_ECHO_EQ_1_F0,    		  CFG_MIC_REVERB_EQ_1_F0,				CFG_MIC_OUT_EQ_1_F0,	  		   	CFG_REC_EQ_F0				},        //idx = 1, f0
	{CFG_MUSIC_EQ_1_Q,		  	  		CFG_MUSIC_EQ_1_Q,		  	  		CFG_MIC_BYPASS_EQ_1_Q,				CFG_MIC_PRE_EQ_1_Q,				    CFG_MIC_ECHO_EQ_1_Q,	  		  CFG_MIC_REVERB_EQ_1_Q, 				CFG_MIC_OUT_EQ_1_Q, 	 		   	CFG_REC_EQ_Q				},
	{CFG_MUSIC_EQ_1_GAIN,	 	  		CFG_MUSIC_EQ_1_GAIN,	 	  		CFG_MIC_BYPASS_EQ_1_GAIN,			CFG_MIC_PRE_EQ_1_GAIN,			    CFG_MIC_ECHO_EQ_1_GAIN,  		  CFG_MIC_REVERB_EQ_1_GAIN,  			CFG_MIC_OUT_EQ_1_GAIN,    		   	CFG_REC_EQ_GAIN				},
	{CFG_MUSIC_EQ_1_TYPE,		  		CFG_MUSIC_EQ_1_TYPE,		  		CFG_MIC_BYPASS_EQ_1_TYPE,			CFG_MIC_PRE_EQ_1_TYPE,			    CFG_MIC_ECHO_EQ_1_TYPE,		      CFG_MIC_REVERB_EQ_1_TYPE,				CFG_MIC_OUT_EQ_1_TYPE,			   	CFG_REC_EQ_TYPE				},
	{CFG_MUSIC_EQ_2_F0,			  		CFG_MUSIC_EQ_2_F0,			  		CFG_MIC_BYPASS_EQ_2_F0,				CFG_MIC_PRE_EQ_2_F0,				CFG_MIC_ECHO_EQ_2_F0,	  		  CFG_MIC_REVERB_EQ_2_F0,				CFG_MIC_OUT_EQ_2_F0, 			   	0					    	},
	{CFG_MUSIC_EQ_2_Q,			  		CFG_MUSIC_EQ_2_Q,			  		CFG_MIC_BYPASS_EQ_2_Q,				CFG_MIC_PRE_EQ_2_Q,				    CFG_MIC_ECHO_EQ_2_Q, 	  		  CFG_MIC_REVERB_EQ_1_Q, 				CFG_MIC_OUT_EQ_2_Q,			   		0					    	},
	{CFG_MUSIC_EQ_2_GAIN,		  		CFG_MUSIC_EQ_2_GAIN,		  		CFG_MIC_BYPASS_EQ_2_GAIN,			CFG_MIC_PRE_EQ_2_GAIN,			    CFG_MIC_ECHO_EQ_2_GAIN,  		  CFG_MIC_REVERB_EQ_2_GAIN,  			CFG_MIC_OUT_EQ_2_GAIN,		   		0					    	},
	{CFG_MUSIC_EQ_2_TYPE,		  		CFG_MUSIC_EQ_2_TYPE,		  		CFG_MIC_BYPASS_EQ_2_TYPE,			CFG_MIC_PRE_EQ_2_TYPE,			    CFG_MIC_ECHO_EQ_2_TYPE,  		  CFG_MIC_REVERB_EQ_2_TYPE,  			CFG_MIC_OUT_EQ_2_TYPE,		   		0					    	},
	{CFG_MUSIC_EQ_3_F0,			  		CFG_MUSIC_EQ_3_F0,			  		0,									0,									0,	  		  					  0,									0, 			   						0					    },
	{CFG_MUSIC_EQ_3_Q,			  		CFG_MUSIC_EQ_3_Q,			  		0,									0,									0, 	  		  					  0, 									0,			   						0					    },
	{CFG_MUSIC_EQ_3_GAIN,		  		CFG_MUSIC_EQ_3_GAIN,		  		0,									0,									0,  		  				      0,  									0,		   							0					    },
	{CFG_MUSIC_EQ_3_TYPE,		  		CFG_MUSIC_EQ_3_TYPE,		  		0,									0,									0,  		  				      0,  									0,		   							0					    },
	{CFG_MUSIC_EQ_4_F0,			  		CFG_MUSIC_EQ_4_F0,			  		0,									0,									0,	  		  				      0,									0, 			   						0					    },
	{CFG_MUSIC_EQ_4_Q,			  		CFG_MUSIC_EQ_4_Q,			  		0,									0,									0, 	  		  				      0, 									0,			   						0					    },
	{CFG_MUSIC_EQ_4_GAIN,		  		CFG_MUSIC_EQ_4_GAIN,		  		0,									0,									0,  		  				      0,  									0,		   							0					    },
	{CFG_MUSIC_EQ_4_TYPE,		  		CFG_MUSIC_EQ_4_TYPE,		  		0,									0,									0,  		  				      0,  									0,		   							0					    },

};

const int32_t DRC_DEFAULT_TABLE[][3] =
{
	{CFG_AUDIO_EFFECT_MUSIC_DRC_EN,		CFG_AUDIO_EFFECT_MIC_DRC_EN,	CFG_AUDIO_EFFECT_REC_DRC_EN	},
	{CFG_MUSIC_DRC_MODE, 				CFG_MIC_DRC_MODE,				CFG_REC_DRC_MODE			},
	{CFG_MUSIC_DRC_CROSSOVER_FREQ, 		CFG_MIC_DRC_CROSSOVER_FREQ,		CFG_REC_DRC_CROSSOVER_FREQ	},
	{CFG_MUSIC_DRC_CROSSOVER_FQ1, 		CFG_MIC_DRC_CROSSOVER_FQ1,		CFG_REC_DRC_CROSSOVER_FQ1	},
	{CFG_MUSIC_DRC_CROSSOVER_FQ2, 		CFG_MIC_DRC_CROSSOVER_FQ2,		CFG_REC_DRC_CROSSOVER_FQ2	},
	{CFG_MUSIC_DRC_PREGAIN1, 			CFG_MIC_DRC_PREGAIN1,			CFG_REC_DRC_PREGAIN1		},
	{CFG_MUSIC_DRC_PREGAIN2, 			CFG_MIC_DRC_PREGAIN2,			CFG_REC_DRC_PREGAIN2		},
	{CFG_MUSIC_DRC_THRESHHOLD1, 		CFG_MIC_DRC_THRESHHOLD1,		CFG_REC_DRC_THRESHHOLD1		},
	{CFG_MUSIC_DRC_RATIO1, 				CFG_MIC_DRC_RATIO1,				CFG_REC_DRC_RATIO1			},
	{CFG_MUSIC_DRC_ATTACK_TIME1, 		CFG_MIC_DRC_ATTACK_TIME1,		CFG_REC_DRC_ATTACK_TIME1	},
	{CFG_MUSIC_DRC_RELEASE_TIME1, 		CFG_MIC_DRC_RELEASE_TIME1,		CFG_REC_DRC_RELEASE_TIME1	},
	{CFG_MUSIC_DRC_THRESHHOLD2, 		CFG_MIC_DRC_THRESHHOLD2,		CFG_REC_DRC_THRESHHOLD2		},
	{CFG_MUSIC_DRC_RATIO2, 				CFG_MIC_DRC_RATIO2,				CFG_REC_DRC_RATIO2			},
	{CFG_MUSIC_DRC_ATTACK_TIME2, 		CFG_MIC_DRC_ATTACK_TIME2,		CFG_REC_DRC_ATTACK_TIME2	},
	{CFG_MUSIC_DRC_RELEASE_TIME2,		CFG_MIC_DRC_RELEASE_TIME2,		CFG_REC_DRC_RELEASE_TIME2	},
	{CFG_MUSIC_DRC_THRESHHOLD3, 		CFG_MIC_DRC_THRESHHOLD3,		CFG_REC_DRC_THRESHHOLD3		},
	{CFG_MUSIC_DRC_RATIO3, 				CFG_MIC_DRC_RATIO3,				CFG_REC_DRC_RATIO3			},
	{CFG_MUSIC_DRC_ATTACK_TIME3, 		CFG_MIC_DRC_ATTACK_TIME3,		CFG_REC_DRC_ATTACK_TIME3	},
	{CFG_MUSIC_DRC_RELEASE_TIME3,		CFG_MIC_DRC_RELEASE_TIME3,		CFG_REC_DRC_RELEASE_TIME3	},
};

const int32_t EXPANDER_DEFAULT_TABLE[][2] =
{
	{CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN,		CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN	},
	{CFG_MUSIC_NOISE_SUPPRESSOR_THRESHOLD, 				CFG_MIC_NOISE_SUPPRESSOR_THRESHOLD			},
	{CFG_MUSIC_NOISE_SUPPRESSOR_RATIO, 					CFG_MIC_NOISE_SUPPRESSOR_RATIO				},
	{CFG_MUSIC_NOISE_SUPPRESSOR_ATTACK_TIME,			CFG_MIC_NOISE_SUPPRESSOR_ATTACK_TIME		},
	{CFG_MUSIC_NOISE_SUPPRESSOR_RELEASE_TIME,			CFG_MIC_NOISE_SUPPRESSOR_RELEASE_TIME		},
};

const int32_t GAIN_CONTROL_TABLE[][16] =
{
	{CFG_AUDIO_EFFECT_AUX_GAIN_CONTROL_EN, CFG_AUDIO_EFFECT_MIC_BYPASS_GAIN_CONTROL_EN, CFG_AUDIO_EFFECT_MIC_ECHO_GAIN_CONTROL_EN, CFG_AUDIO_EFFECT_MIC_REVERB_GAIN_CONTROL_EN, CFG_AUDIO_EFFECT_MIC_OUT_GAIN_CONTROL_EN, CFG_AUDIO_EFFECT_REC_BYPASS_GAIN_CONTROL_EN,	CFG_AUDIO_EFFECT_REC_EFFECT_GAIN_CONTROL_EN,	CFG_AUDIO_EFFECT_REC_AUX_GAIN_CONTROL_EN,	CFG_AUDIO_EFFECT_REC_REMIND_GAIN_CONTROL_EN,	CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN,	CFG_AUDIO_EFFECT_REMIND_KEY_GAIN_CONTROL_EN,	CFG_AUDIO_EFFECT_REMIND_EFFECT_GAIN_CONTROL_EN,		CFG_AUDIO_EFFECT_I2S_IN_GAIN_CONTROL_EN,	CFG_AUDIO_EFFECT_BT_IN_GAIN_CONTROL_EN,		CFG_AUDIO_EFFECT_USB_IN_GAIN_CONTROL_EN,		CFG_AUDIO_EFFECT_SPDIF_IN_GAIN_CONTROL_EN},
	{CFG_AUDIO_EFFECT_AUX_PREGAIN_MUTE,    CFG_AUDIO_EFFECT_MIC_BYPASS_PREGAIN_MUTE,    CFG_AUDIO_EFFECT_MIC_ECHO_MUTE,    		   CFG_AUDIO_EFFECT_MIC_REVERB_PREGAIN_MUTE,    CFG_AUDIO_EFFECT_MIC_OUT_PREGAIN_MUTE,    CFG_AUDIO_EFFECT_REC_BYPASS_PREGAIN_MUTE, 	CFG_AUDIO_EFFECT_REC_EFFECT_PREGAIN_MUTE,		CFG_AUDIO_EFFECT_REC_AUX_PREGAIN_MUTE,		CFG_AUDIO_EFFECT_REC_REMIND_PREGAIN_MUTE,		CFG_AUDIO_EFFECT_REC_OUT_PREGAIN_MUTE,		CFG_AUDIO_EFFECT_REMIND_KEY_PREGAIN_MUTE,		CFG_AUDIO_EFFECT_REMIND_EFFECT_PREGAIN_MUTE,		CFG_AUDIO_EFFECT_I2S_IN_PREGAIN_MUTE,		CFG_AUDIO_EFFECT_BT_IN_PREGAIN_MUTE,		CFG_AUDIO_EFFECT_USB_IN_PREGAIN_MUTE,			CFG_AUDIO_EFFECT_SPDIF_IN_PREGAIN_MUTE},
	{CFG_AUDIO_EFFECT_AUX_PREGAIN,         CFG_AUDIO_EFFECT_MIC_BYPASS_PREGAIN,         CFG_AUDIO_EFFECT_MIC_ECHO_PREGAIN,         CFG_AUDIO_EFFECT_MIC_REVERB_PREGAIN,         CFG_AUDIO_EFFECT_MIC_OUT_PREGAIN,         CFG_AUDIO_EFFECT_REC_BYPASS_PREGAIN,			CFG_AUDIO_EFFECT_REC_EFFECT_PREGAIN,			CFG_AUDIO_EFFECT_REC_AUX_PREGAIN,			CFG_AUDIO_EFFECT_REC_REMIND_PREGAIN,			CFG_AUDIO_EFFECT_REC_OUT_PREGAIN,			CFG_AUDIO_EFFECT_REMIND_KEY_PREGAIN,			CFG_AUDIO_EFFECT_REMIND_EFFECT_PREGAIN,				CFG_AUDIO_EFFECT_I2S_IN_PREGAIN,			CFG_AUDIO_EFFECT_BT_IN_PREGAIN,				CFG_AUDIO_EFFECT_USB_IN_PREGAIN,				CFG_AUDIO_EFFECT_SPDIF_IN_PREGAIN},
	{CFG_AUDIO_EFFECT_AUX_CHANNEL,         CFG_AUDIO_EFFECT_MIC_BYPASS_CHANNEL,         CFG_AUDIO_EFFECT_MIC_ECHO_CHANNEL,         CFG_AUDIO_EFFECT_MIC_REVERB_CHANNEL,         CFG_AUDIO_EFFECT_MIC_OUT_CHANNEL,         CFG_AUDIO_EFFECT_REC_BYPASS_CHANNEL,			CFG_AUDIO_EFFECT_REC_EFFECT_CHANNEL,			CFG_AUDIO_EFFECT_REC_AUX_CHANNEL,			CFG_AUDIO_EFFECT_REC_REMIND_CHANNEL,			CFG_AUDIO_EFFECT_REC_OUT_CHANNEL,			CFG_AUDIO_EFFECT_REMIND_KEY_CHANNEL,			CFG_AUDIO_EFFECT_REMIND_EFFECT_CHANNEL,				CFG_AUDIO_EFFECT_I2S_IN_CHANNEL,			CFG_AUDIO_EFFECT_BT_IN_CHANNEL,				CFG_AUDIO_EFFECT_USB_IN_CHANNEL,				CFG_AUDIO_EFFECT_SPDIF_IN_CHANNEL},
};

const int16_t BassTrebGainTable[16] =
{
    0xf900, //-7dB
    0xfa00, //-6dB
    0xfb00, //-5dB
    0xfc00, //-4dB
    0xfd00, //-3dB
    0xfe00, //-2dB
    0xff00, //-1dB
    0x0000, //+0dB
    0x0100, //+1dB
    0x0200, //+2dB
    0x0300, //+3dB
    0x0400, //+4dB
    0x0500, //+5dB
    0x0600, //+6dB
    0x0700, //+7dB
    0x0800, //+8dB

#if 0    
    0xf400, //-12dB
    0xf500, //-11dB
    0xf600, //-10dB
    0xf700, //-9dB
    0xf800, //-8dB
    0xf900, //-7dB
    0xfa00, //-6dB
    0xfb00, //-5dB
    0xfc00, //-4dB
    0xfd00, //-3dB
    0xfe00, //-2dB
    0xff00, //-1dB
    0x0000, //+0dB
    0x0100, //+1dB
    0x0200, //+2dB
    0x0300, //+3dB
    0x0400, //+4dB
    0x0500, //+5dB
    0x0600, //+6dB
    0x0700, //+7dB
    0x0800, //+8dB
    0x0900, //+9dB
    0x0a00, //+10dB
    0x0b00, //+11dB
    0x0c00, //+12dB 
#endif
};

EQUnit *eq_unit_aggregate[8] 					=  {&gCtrlVars.music_pre_eq_unit, &gCtrlVars.music_out_eq_unit, &gCtrlVars.mic_pre_eq_unit,  &gCtrlVars.mic_bypass_eq_unit, &gCtrlVars.mic_echo_eq_unit,
									   	   	   		&gCtrlVars.mic_reverb_eq_unit, &gCtrlVars.mic_out_eq_unit, &gCtrlVars.rec_eq_unit};
DRCUnit *drc_unit_aggregate[3]					=  {&gCtrlVars.music_drc_unit, &gCtrlVars.mic_drc_unit,	&gCtrlVars.rec_drc_unit};
ExpanderUnit *expander_unit_aggregate[2]  		=  {&gCtrlVars.music_expander_unit, &gCtrlVars.mic_expander_unit};

EQFilterParams *eq_param_aggregate[8] 			=  {eq1_filter_buf, eq2_filter_buf, eq3_filter_buf,
													eq4_filter_buf, eq5_filter_buf, eq6_filter_buf, eq7_filter_buf, eq8_filter_buf};

GainControlUnit *gain_unit_aggregate[16]		=  {&gCtrlVars.aux_gain_control_unit, &gCtrlVars.mic_bypass_gain_control_unit, &gCtrlVars.mic_echo_control_unit,
												   &gCtrlVars.mic_reverb_gain_control_unit, &gCtrlVars.mic_out_gain_control_unit, &gCtrlVars.rec_bypass_gain_control_unit,
												   &gCtrlVars.rec_effect_gain_control_unit,	&gCtrlVars.rec_aux_gain_control_unit, &gCtrlVars.rec_remind_gain_control_unit,
												   &gCtrlVars.rec_out_gain_control_unit, &gCtrlVars.remind_key_gain_control_unit, &gCtrlVars.remind_effect_gain_control_unit,
												   &gCtrlVars.i2s_gain_control_unit,&gCtrlVars.bt_gain_control_unit,&gCtrlVars.usb_gain_control_unit,&gCtrlVars.spdif_gain_control_unit};
/*
****************************************************************
* 系统变量初始化
*
*
****************************************************************
*/
void CtrlVarsInit(void)
{
    uint32_t i, j;

	APP_DBG("Loading control vars as default\n");
	
    memset(&gCtrlVars,  0, sizeof(gCtrlVars));

	DefaultParamgsInit();
	
#ifdef CFG_FUNC_AUDIO_EFFECT_EN		
    //audio effect
    #if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
	gCtrlVars.music_delay_unit.ct			 	 = NULL;
	gCtrlVars.music_delay_unit.s_buf			 = NULL;
    gCtrlVars.music_delay_unit.enable     		 = CFG_AUDIO_EFFECT_MUSIC_DELAY_EN;
	gCtrlVars.music_delay_unit.delay		     = CFG_MUSIC_PCM_DELAY;
	gCtrlVars.music_delay_unit.delay_samples 	 = gCtrlVars.music_delay_unit.delay*gCtrlVars.sample_rate/1000;
	gCtrlVars.music_delay_unit.max_delay		 = CFG_MUSIC_PCM_DELAY;
	gCtrlVars.music_delay_unit.max_delay_samples = gCtrlVars.music_delay_unit.max_delay*gCtrlVars.sample_rate/1000;
	gCtrlVars.music_delay_unit.high_quality 	 =  0;	
    #endif
	
    #if CFG_AUDIO_EFFECT_MIC_AEC_EN
	gCtrlVars.mic_aec_unit.ct          			 = NULL;
	gCtrlVars.mic_aec_unit.enable      			 = CFG_AUDIO_EFFECT_MIC_AEC_EN;
	gCtrlVars.mic_aec_unit.es_level    			 = CFG_MIC_AEC_ES_LEVEL;
	gCtrlVars.mic_aec_unit.ns_level     		 = CFG_MIC_AEC_NS_LEVEL;
	#endif
	
	gCtrlVars.music_exciter_unit.ct				 = NULL;
	gCtrlVars.music_exciter_unit.enable 		 = CFG_AUDIO_EFFECT_MUSIC_EXCITER_EN;
	gCtrlVars.music_exciter_unit.f_cut  		 = CFG_MUSIC_EXCITER_FCUT;
	gCtrlVars.music_exciter_unit.dry    		 = CFG_MUSIC_EXCITER_DRY;
	gCtrlVars.music_exciter_unit.wet    		 = CFG_MUSIC_EXCITER_WET;
	
	gCtrlVars.auto_tune_unit.ct					 = NULL;
	gCtrlVars.auto_tune_unit.enable				 = CFG_AUDIO_EFFECT_MIC_AUTO_TUNE_EN;
	gCtrlVars.auto_tune_unit.key				 = CFG_MIC_AUTO_TUNE_KEY > 12? AutoTuneKeyTbl[12] : AutoTuneKeyTbl[CFG_MIC_AUTO_TUNE_KEY];
	gCtrlVars.auto_tune_unit.snap_mode  		 = CFG_MIC_AUTO_TUNE_SNAP > 2? AutoTuneSnapTbl[2] : AutoTuneSnapTbl[CFG_MIC_AUTO_TUNE_SNAP];
    gCtrlVars.auto_tune_unit.pitch_accuracy      = 0;

	gCtrlVars.echo_unit.ct						 = NULL;
	gCtrlVars.echo_unit.enable					 = CFG_AUDIO_EFFECT_MIC_ECHO_EN;
	gCtrlVars.echo_unit.attenuation     		 = CFG_MIC_ECHO_ATTENUATION;
	gCtrlVars.echo_unit.delay			         = CFG_MIC_ECHO_DELAY;
	gCtrlVars.echo_unit.max_delay			     = 1000;
	gCtrlVars.echo_unit.delay_samples 		     = gCtrlVars.echo_unit.delay * gCtrlVars.sample_rate / 1000;
	gCtrlVars.echo_unit.max_delay_samples        = gCtrlVars.echo_unit.max_delay*gCtrlVars.sample_rate/1000;
	gCtrlVars.echo_unit.direct_sound 			 = CFG_MIC_ECHO_DIRECT_SND_EN;
	gCtrlVars.echo_unit.fc 						 = CFG_MIC_ECHO_CUTOFF_FREQ;
	gCtrlVars.echo_unit.s_buf					 = NULL;	

	gCtrlVars.pitch_shifter_unit.ct		         = NULL;
	gCtrlVars.pitch_shifter_unit.enable          = CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN;	
	gCtrlVars.pitch_shifter_unit.semitone_steps  = CFG_MIC_PITCH_SHIFTER_SEMITONE_STEP;
	
    #if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
	gCtrlVars.pitch_shifter_pro_unit.ct		     = NULL;
	gCtrlVars.pitch_shifter_pro_unit.enable      = CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN;	
	gCtrlVars.pitch_shifter_pro_unit.semitone_steps = CFG_MUSIC_PITCH_SHIFTER_PRO_SEMITONE_STEP;
	#endif
	
	gCtrlVars.voice_changer_unit.ct		         = NULL;
	gCtrlVars.voice_changer_unit.enable          = CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_EN;
	gCtrlVars.voice_changer_unit.formant_ratio 	 = CFG_MIC_VOICE_CHANGER_FORMANT_RATIO;
	gCtrlVars.voice_changer_unit.pitch_ratio	 = CFG_MIC_VOICE_CHANGER_PITCH_RATIO;

	#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
	gCtrlVars.voice_changer_pro_unit.ct	         = NULL;
	gCtrlVars.voice_changer_pro_unit.enable      = CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN;
	gCtrlVars.voice_changer_pro_unit.formant_ratio = CFG_MIC_VOICE_CHANGER_PRO_FORMANT_RATIO;
	gCtrlVars.voice_changer_pro_unit.pitch_ratio = CFG_MIC_VOICE_CHANGER_PRO_PITCH_RATIO;	
	#endif
	
	gCtrlVars.reverb_unit.ct					 = NULL;
	gCtrlVars.reverb_unit.enable 				 = CFG_AUDIO_EFFECT_MIC_REVERB_EN;
	gCtrlVars.reverb_unit.damping_scale 		 = CFG_MIC_REVERB_DAMPING;
	gCtrlVars.reverb_unit.dry_scale				 = CFG_MIC_REVERB_DRY_SCALE;
	gCtrlVars.reverb_unit.roomsize_scale		 = CFG_MIC_REVERB_ROOM_SIZE;
	gCtrlVars.reverb_unit.wet_scale 			 = CFG_MIC_REVERB_WET_SCALE;
	gCtrlVars.reverb_unit.width_scale 			 = CFG_MIC_REVERB_WIDTH_SCALE;

    gCtrlVars.plate_reverb_unit.ct      		 = NULL;
	gCtrlVars.plate_reverb_unit.enable  		 = CFG_AUDIO_EFFECT_MIC_PLATE_REVERB_EN;
	gCtrlVars.plate_reverb_unit.highcut_freq 	 = CFG_MIC_PLATEREVERB_HIGHCUT_FREQ;
	gCtrlVars.plate_reverb_unit.modulation_en    = CFG_MIC_PLATEREVERB_MODULATION_EN;
	gCtrlVars.plate_reverb_unit.predelay         = CFG_MIC_PLATEREVERB_PREDELAY;
	gCtrlVars.plate_reverb_unit.diffusion        = CFG_MIC_PLATEREVERB_DIFFUSION;
	gCtrlVars.plate_reverb_unit.decay            = CFG_MIC_PLATEREVERB_DECAY;
	gCtrlVars.plate_reverb_unit.damping          = CFG_MIC_PLATEREVERB_DAMPING;
	gCtrlVars.plate_reverb_unit.wetdrymix        = CFG_MIC_PLATEREVERB_WETDRYMIX;
	
    #if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN	
	gCtrlVars.reverb_pro_unit.enable	  		 =  1;
	gCtrlVars.reverb_pro_unit.dry		 		 = -10;
	gCtrlVars.reverb_pro_unit.wet		 		 =  0;
	gCtrlVars.reverb_pro_unit.erwet 	  		 = -9;
	gCtrlVars.reverb_pro_unit.erfactor	   		 =  160;
	gCtrlVars.reverb_pro_unit.erwidth	 	     =  70;
	gCtrlVars.reverb_pro_unit.ertolate	  		 =  40;
	gCtrlVars.reverb_pro_unit.rt60		  		 =  3200;
	gCtrlVars.reverb_pro_unit.delay 	   		 =  20;
	gCtrlVars.reverb_pro_unit.width 	   		 =  100;
	gCtrlVars.reverb_pro_unit.wander	   		 =  35;
	gCtrlVars.reverb_pro_unit.spin		   		 =  70;
	gCtrlVars.reverb_pro_unit.inputlpf	   		 =  17000;
	gCtrlVars.reverb_pro_unit.damplpf	   		 =  7000;
	gCtrlVars.reverb_pro_unit.basslpf	   		 =  500;
	gCtrlVars.reverb_pro_unit.bassb 	   		 =  15;
	gCtrlVars.reverb_pro_unit.outputlpf    		 =  10000;
    #endif

    gCtrlVars.freq_shifter_unit.ct       		 = NULL;
	gCtrlVars.freq_shifter_unit.enable   		 = CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN;
	gCtrlVars.freq_shifter_unit.deltaf   		 = CFG_FREQ_SHIFT_HZ;
	gCtrlVars.freq_shifter_unit.fade_step 		 = 0;
	gCtrlVars.freq_shifter_unit.auto_on_flag 	 = 0;

	gCtrlVars.howling_dector_unit.ct             = NULL;
	gCtrlVars.howling_dector_unit.enable         = CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN;
	gCtrlVars.howling_dector_unit.suppression_mode = CFG_HOWLING_MODE;

	gCtrlVars.vocal_cut_unit.ct          		 = NULL;
	gCtrlVars.vocal_cut_unit.enable      		 = CFG_AUDIO_EFFECT_VOCAL_CUT_EN;
	gCtrlVars.vocal_cut_unit.wetdrymix   		 = CFG_MUSIC_VOCALCUT_WETDRYMIX;
	gCtrlVars.vocal_cut_unit.vocal_cut_en        = 1;

	gCtrlVars.vocal_remove_unit.ct               = NULL;
	gCtrlVars.vocal_remove_unit.enable           = CFG_AUDIO_EFFECT_VOCAL_REMOVE_EN;
	gCtrlVars.vocal_remove_unit.lower_freq       = CFG_MUSIC_VOCALREMOVE_LOW_FREQ;
	gCtrlVars.vocal_remove_unit.higher_freq      = CFG_MUSIC_VOCALREMOVE_HIGH_FREQ;
	gCtrlVars.vocal_remove_unit.step_size        = CFG_MUSIC_VOCALREMOVE_STEP_SIZE;
	gCtrlVars.vocal_remove_unit.vocal_remove_en  = 1;

	#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
    gCtrlVars.music_threed_unit.ct               = NULL;
	gCtrlVars.music_threed_unit.enable           = CFG_AUDIO_EFFECT_MUSIC_3D_EN;
	gCtrlVars.music_threed_unit.intensity        = CFG_MUSIC_PARAM_3D_DEPTH;
	gCtrlVars.music_threed_unit.three_d_en       = 1;
	#endif

	#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
    gCtrlVars.music_threed_plus_unit.ct          = NULL;
	gCtrlVars.music_threed_plus_unit.enable      = CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN;
	gCtrlVars.music_threed_plus_unit.intensity   = CFG_MUSIC_PARAM_3D_PLUS_DEPTH;
	gCtrlVars.music_threed_plus_unit.three_d_en  = 1;
	#endif
	
    #if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
	gCtrlVars.music_vb_unit.enable               = CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN;
	gCtrlVars.music_vb_unit.enhanced             = CFG_MUSIC_PARAM_VB_ENHANCED;
	gCtrlVars.music_vb_unit.intensity            = CFG_MUSIC_PARAM_VB_INTENSITY;
    gCtrlVars.music_vb_unit.f_cut                = CFG_MUSIC_PARAM_VB_CUTOFF_FREQ;
	gCtrlVars.music_vb_unit.vb_en                = 1;
	#endif

	#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
	gCtrlVars.rec_vb_unit.enable               = CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN;
	gCtrlVars.rec_vb_unit.enhanced             = CFG_MUSIC_PARAM_VB_ENHANCED;
	gCtrlVars.rec_vb_unit.intensity            = CFG_MUSIC_PARAM_VB_INTENSITY;
    gCtrlVars.rec_vb_unit.f_cut                = CFG_MUSIC_PARAM_VB_CUTOFF_FREQ;
	gCtrlVars.rec_vb_unit.vb_en                = 1;
	#endif
	
	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
	gCtrlVars.music_vb_classic_unit.enable       = CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN;
	gCtrlVars.music_vb_classic_unit.intensity    = CFG_MUSIC_PARAM_VB_CLASSIC_INTENSITY;
    gCtrlVars.music_vb_classic_unit.f_cut        = CFG_MUSIC_PARAM_VB_CLASSIC_CUTOFF_FREQ;
	gCtrlVars.music_vb_classic_unit.vb_en        = 1;
	#endif
	
	gCtrlVars.chorus_unit.enable                 = CFG_AUDIO_EFFECT_CHORUS_EN;
	gCtrlVars.chorus_unit.delay_length           = CFG_CHORUS_DELAY_LENGTH;
	gCtrlVars.chorus_unit.mod_depth              = CFG_CHORUS_MODULATION_DEPTH;
	gCtrlVars.chorus_unit.mod_rate               = CFG_CHORUS_MODULATION_RATE;
	gCtrlVars.chorus_unit.feedback               = CFG_CHORUS_FEEDBACK;
	gCtrlVars.chorus_unit.dry                    = CFG_CHORUS_DRY;
	gCtrlVars.chorus_unit.wet                    = CFG_CHORUS_WET;

	#if CFG_AUDIO_EFFECT_PINGPONG_EN
	gCtrlVars.ping_pong_unit.enable	  		    =  0;
	gCtrlVars.ping_pong_unit.attenuation        =  8192;
	gCtrlVars.ping_pong_unit.delay		        =  1000;
	gCtrlVars.ping_pong_unit.delay_samples      =  gCtrlVars.ping_pong_unit.delay*gCtrlVars.sample_rate/1000;
	gCtrlVars.ping_pong_unit.high_quality 	    =  0;
	gCtrlVars.ping_pong_unit.wetdrymix	   		=  50;//0-100
	gCtrlVars.ping_pong_unit.max_delay		    =  1000;
	gCtrlVars.ping_pong_unit.max_delay_samples  =  gCtrlVars.ping_pong_unit.max_delay*gCtrlVars.sample_rate/1000;///1000MS = 1000*44.1	
    #endif

    #if CFG_AUDIO_EFFECT_AUTOWAH_EN
    gCtrlVars.auto_wah_unit.ct                  = NULL;
	gCtrlVars.auto_wah_unit.enable	  	        =  0;
	gCtrlVars.auto_wah_unit.modulation_rate     =  1;
	gCtrlVars.auto_wah_unit.min_frequency       =  100;
	gCtrlVars.auto_wah_unit.max_frequency       =  1000;
	gCtrlVars.auto_wah_unit.depth	   		    =  50;//0-100
	gCtrlVars.auto_wah_unit.dry                 =  50;
	gCtrlVars.auto_wah_unit.wet                 =  50;
    #endif

	gCtrlVars.phase_control_unit.enable          = CFG_AUDIO_EFFECT_PHASE_EN;
    gCtrlVars.phase_control_unit.phase_difference = 0;
	gCtrlVars.phase_control_unit.channel         = 1;
	
    #if CFG_AUDIO_EFFECT_GUITAR_GAIN_EN
	gCtrlVars.guitar_gain_control_unit.enable   = 0;
	gCtrlVars.guitar_gain_control_unit.mute     = 0;
	gCtrlVars.guitar_gain_control_unit.channel  = 2;
	gCtrlVars.guitar_gain_control_unit.gain     = 4095;
    #endif

	
	for(i = 0; i < sizeof(expander_unit_aggregate)/sizeof(expander_unit_aggregate[0]); i++)
	{
		expander_unit_aggregate[i]->ct			= NULL;
		expander_unit_aggregate[i]->enable 		= EXPANDER_DEFAULT_TABLE[0][i];
		expander_unit_aggregate[i]->attack_time = EXPANDER_DEFAULT_TABLE[3][i];
		expander_unit_aggregate[i]->ratio		= EXPANDER_DEFAULT_TABLE[2][i];
		expander_unit_aggregate[i]->release_time= EXPANDER_DEFAULT_TABLE[4][i];
		expander_unit_aggregate[i]->threshold   = EXPANDER_DEFAULT_TABLE[1][i];
	}

	for(i = 0; i < sizeof(drc_unit_aggregate)/sizeof(drc_unit_aggregate[0]); i++)
	{
		drc_unit_aggregate[i]->ct			= NULL;

		drc_unit_aggregate[i]->enable		= DRC_DEFAULT_TABLE[0][i];
		drc_unit_aggregate[i]->attack_tc[0]	= DRC_DEFAULT_TABLE[9][i];
		drc_unit_aggregate[i]->attack_tc[1]	= DRC_DEFAULT_TABLE[13][i];
		drc_unit_aggregate[i]->attack_tc[2]	= DRC_DEFAULT_TABLE[17][i];
		drc_unit_aggregate[i]->fc		   	= DRC_DEFAULT_TABLE[2][i];
		drc_unit_aggregate[i]->mode			= DRC_DEFAULT_TABLE[1][i];
		drc_unit_aggregate[i]->pregain1		= DRC_DEFAULT_TABLE[5][i];
		drc_unit_aggregate[i]->pregain2		= DRC_DEFAULT_TABLE[6][i];
		drc_unit_aggregate[i]->q[0]			= DRC_DEFAULT_TABLE[3][i];
		drc_unit_aggregate[i]->q[1]			= DRC_DEFAULT_TABLE[4][i];
		drc_unit_aggregate[i]->ratio[0]		= DRC_DEFAULT_TABLE[8][i];
		drc_unit_aggregate[i]->ratio[1]		= DRC_DEFAULT_TABLE[12][i];
		drc_unit_aggregate[i]->ratio[2]		= DRC_DEFAULT_TABLE[16][i];
		drc_unit_aggregate[i]->release_tc[0]= DRC_DEFAULT_TABLE[10][i];
		drc_unit_aggregate[i]->release_tc[1]= DRC_DEFAULT_TABLE[14][i];
		drc_unit_aggregate[i]->release_tc[2]= DRC_DEFAULT_TABLE[18][i];
		drc_unit_aggregate[i]->threshold[0]	= DRC_DEFAULT_TABLE[7][i];
		drc_unit_aggregate[i]->threshold[1]	= DRC_DEFAULT_TABLE[11][i];
		drc_unit_aggregate[i]->threshold[2]	= DRC_DEFAULT_TABLE[15][i];
	}

	for(i = 0; i < sizeof(eq_unit_aggregate)/sizeof(eq_unit_aggregate[0]); i++)
	{
		eq_unit_aggregate[i]->ct			= NULL;
		eq_unit_aggregate[i]->enable		= EQ_DEFAULT_TABLE[0][i];
		eq_unit_aggregate[i]->filter_count  = EQ_DEFAULT_TABLE[1][i];
		eq_unit_aggregate[i]->pregain		= EQ_DEFAULT_TABLE[2][i];
		eq_unit_aggregate[i]->filter_params = eq_param_aggregate[i];

		for(j = 0; j < eq_unit_aggregate[i]->filter_count; j++)
		{
			eq_unit_aggregate[i]->eq_params[j].enable			= 1;
			eq_unit_aggregate[i]->eq_params[j].f0				= EQ_DEFAULT_TABLE[3 + j *4][i];
			eq_unit_aggregate[i]->eq_params[j].Q				= EQ_DEFAULT_TABLE[4 + j *4][i];
			eq_unit_aggregate[i]->eq_params[j].gain				= EQ_DEFAULT_TABLE[5 + j *4][i];
			eq_unit_aggregate[i]->eq_params[j].type				= EQ_DEFAULT_TABLE[6 + j *4][i];

			(eq_unit_aggregate[i]->filter_params + j)->f0		= EQ_DEFAULT_TABLE[3 + j *4][i];
			(eq_unit_aggregate[i]->filter_params + j)->Q		= EQ_DEFAULT_TABLE[4 + j *4][i];
			(eq_unit_aggregate[i]->filter_params + j)->gain		= EQ_DEFAULT_TABLE[5 + j *4][i];
			(eq_unit_aggregate[i]->filter_params + j)->type		= EQ_DEFAULT_TABLE[6 + j *4][i];
		}
	}


	//gain control
	for(i = 0; i < sizeof(gain_unit_aggregate)/sizeof(gain_unit_aggregate[0]); i++)
	{
		gain_unit_aggregate[i]->enable				= GAIN_CONTROL_TABLE[0][i];
		gain_unit_aggregate[i]->mute				= GAIN_CONTROL_TABLE[1][i];
		gain_unit_aggregate[i]->gain				= GAIN_CONTROL_TABLE[2][i];
		gain_unit_aggregate[i]->channel				= GAIN_CONTROL_TABLE[3][i];
	}	
  
    AudioEffectsAllDisable();

    AudioEffectsDeInit();

	if(GetSystemMode() != AppModeBtHfPlay)
	{
		Communication_Effect_Init();	
	}
	else
	{
		Communication_Effect_Init_BTHFP();
	}
#endif
}

/*
****************************************************************
* 各个模块采样率设置函数
*
*
****************************************************************
*/
void GlobalSampeRateSet(void)
{
	uint16_t i;

	if(gCtrlVars.sys_sample_rate_en)
	{
		for(i = 0; i < sizeof(SupportSampleRateList)/sizeof(SupportSampleRateList[0]); i++)
		{
			if(gCtrlVars.sample_rate == SupportSampleRateList[i])
			{
			    gCtrlVars.sys_sample_rate = i;
			    if(gCtrlVars.adc0_sample_rate < 9 )
		    	{
					gCtrlVars.adc0_sample_rate = i;
		    	}
				if(gCtrlVars.adc1_sample_rate < 9 )
				{
					gCtrlVars.adc1_sample_rate = i;
				}
				if(gCtrlVars.dac0_sample_rate < 9 )
				{
	                gCtrlVars.dac0_sample_rate = i;
				}
				if(gCtrlVars.dac1_sample_rate < 9 )
				{
					gCtrlVars.dac1_sample_rate = i;
				}
				if(gCtrlVars.i2s0_sample_rate < 12 )
				{
					gCtrlVars.i2s0_sample_rate = i;
				}
				if(gCtrlVars.i2s1_sample_rate < 12 )
				{
				    gCtrlVars.i2s1_sample_rate = i;
				}
				if(gCtrlVars.spdif_sample_rate < 12 )
				{
					gCtrlVars.spdif_sample_rate = i;
				}
				break;
			}
		}
	}
}
/*
****************************************************************
* 各个模块采样率设置函数
*
*
****************************************************************
*/
void PrivateSampeRateSet(void)
{
	uint16_t i;

	for(i = 0; i < sizeof(SupportSampleRateList)/sizeof(SupportSampleRateList[0]); i++)
	{
		if(gCtrlVars.sample_rate == SupportSampleRateList[i])
		{
		    gCtrlVars.sys_sample_rate = i;
		    if(gCtrlVars.adc0_sample_rate < 9 )
	    	{
				gCtrlVars.adc0_sample_rate = i;
	    	}
			if(gCtrlVars.adc1_sample_rate < 9 )
			{
				gCtrlVars.adc1_sample_rate = i;
			}
			if(gCtrlVars.dac0_sample_rate < 9 )
			{
                gCtrlVars.dac0_sample_rate = i;
			}
			if(gCtrlVars.dac1_sample_rate < 9 )
			{
				gCtrlVars.dac1_sample_rate = i;
			}
			if(gCtrlVars.i2s0_sample_rate < 12 )
			{
				gCtrlVars.i2s0_sample_rate = i;
			}
			if(gCtrlVars.i2s1_sample_rate < 12 )
			{
			    gCtrlVars.i2s1_sample_rate = i;
			}
			if(gCtrlVars.spdif_sample_rate < 12 )
			{
               #ifdef CFG_FUNC_SPDIF_OUT_EN
               gCtrlVars.spdif_sample_rate = i;
               #endif
			}
			break;
		}
	}
}
/*
****************************************************************
* 各个模块MCLK设置函数
*
*
****************************************************************
*/
void GlobalMclkSet(void)
{
	if(gCtrlVars.sys_mclk_src_en)
	{
		gCtrlVars.adc0_mclk_src      = gCtrlVars.sys_mclk_src;
		gCtrlVars.adc1_mclk_src      = gCtrlVars.sys_mclk_src;
		gCtrlVars.dac0_mclk_src      = gCtrlVars.sys_mclk_src;
		gCtrlVars.dac1_mclk_src      = gCtrlVars.sys_mclk_src;
		gCtrlVars.i2s0_mclk_src      = gCtrlVars.sys_mclk_src;
		gCtrlVars.i2s1_mclk_src      = gCtrlVars.sys_mclk_src;
	}
	if((gCtrlVars.sample_rate == 16000) || (gCtrlVars.sample_rate == 32000) || (gCtrlVars.sample_rate == 48000))
	{
	    //若Clock_AudioPllClockSet中使用了PLL_CLK_USER_DEF，此处需要根据配置的时钟源来选择PLL_CLOCK1或者是PLL_CLOCK2!!

	    if(gCtrlVars.adc0_mclk_src == PLL_CLOCK1)
    	{
			gCtrlVars.adc0_mclk_src = PLL_CLOCK2;
    	}
		if(gCtrlVars.adc1_mclk_src == PLL_CLOCK1)
		{
			gCtrlVars.adc1_mclk_src = PLL_CLOCK2;
		}
		if(gCtrlVars.dac0_mclk_src == PLL_CLOCK1)
		{
			gCtrlVars.dac0_mclk_src = PLL_CLOCK2;
		}
		if(gCtrlVars.dac1_mclk_src == PLL_CLOCK1)
		{
			gCtrlVars.dac1_mclk_src = PLL_CLOCK2;
		}

		if(gCtrlVars.i2s0_mclk_src == PLL_CLOCK1)
		{
			gCtrlVars.i2s0_mclk_src = PLL_CLOCK2;
		}
		if(gCtrlVars.i2s1_mclk_src == PLL_CLOCK1)
		{
			gCtrlVars.i2s1_mclk_src = PLL_CLOCK2;
		}		
	}
	//////如果I2S采用了采样率微调，注释掉以下代码///////////////////////////
	if(gCtrlVars.i2s0_work_mode == 1)/////slave
	{
		//gCtrlVars.i2s0_mclk_src  = GPIO_IN0;
	}
	if(gCtrlVars.i2s1_work_mode == 1)/////slave
	{
		//gCtrlVars.i2s1_mclk_src  = GPIO_IN1;
	}   
}
/*
****************************************************************
* 各个模块MCLK设置函数
*
*
****************************************************************
*/
void PrivateMclkSet(void)
{

	gCtrlVars.adc0_mclk_src      = CFG_ADC0_MCLK_SRC;
	gCtrlVars.adc1_mclk_src      = CFG_ADC1_MCLK_SRC;
	gCtrlVars.dac0_mclk_src      = CFG_DAC0_MCLK_SRC;
	gCtrlVars.dac1_mclk_src      = CFG_DAC1_MCLK_SRC;
	gCtrlVars.i2s0_mclk_src      = CFG_I2S0_MCLK_SRC;
	gCtrlVars.i2s1_mclk_src      = CFG_I2S1_MCLK_SRC;

	//////如果I2S采用了采样率微调，注释掉以下代码///////////////////////////
	if(gCtrlVars.i2s0_work_mode == 1)/////slave
	{
		gCtrlVars.i2s0_mclk_src  = GPIO_IN0;
	}
	if(gCtrlVars.i2s1_work_mode == 1)/////slave
	{
		gCtrlVars.i2s1_mclk_src  = GPIO_IN1;
	}
}
/*
****************************************************************
* 各个模块默认参数设置函数
*
*
****************************************************************
*/
void DefaultParamgsInit(void)
{
	//for system control 0x01
	gCtrlVars.sys_mode            = CFG_SYS_MODE;
	gCtrlVars.sys_reset           = CFG_SYS_RESET;
	gCtrlVars.sys_sample_rate_en  = CFG_SYS_SAMPLE_RATE_EN;
	gCtrlVars.sys_sample_rate     = CFG_SYS_SAMPLE_RATE;
	gCtrlVars.sys_mclk_src_en     = CFG_SYS_MCLK_SRC_EN;
	gCtrlVars.sys_mclk_src        = CFG_SYS_MCLK_SRC;
	gCtrlVars.sys_default_set     = 0;

	//for System status 0x02
	gCtrlVars.cpu_mips            = 0;
	gCtrlVars.UsedRamSize         = 0;
	gCtrlVars.AutoRefresh         = 0;//调音时音效参数发生改变，上位机会自动读取音效数据，1=允许上位读，0=不需要上位机读取
	gCtrlVars.CpuMaxFreq          = 300;
	gCtrlVars.CpuMaxRamSize       = 320;//320k

	//for ADC0 PGA      0x03
    gCtrlVars.pga0_line1_l_en     = 0;
    gCtrlVars.pga0_line1_r_en     = 0;
    gCtrlVars.pga0_line2_l_en     = 0;
    gCtrlVars.pga0_line2_r_en     = 0;
    gCtrlVars.pga0_line4_l_en     = 0;
    gCtrlVars.pga0_line4_r_en     = 0;
    gCtrlVars.pga0_line5_l_en     = 0;
    gCtrlVars.pga0_line5_r_en     = 0;
	
	gCtrlVars.pga0_line1_pin      = CFG_LINE1_PIN_TYPE;
	gCtrlVars.pga0_line2_pin      = CFG_LINE2_PIN_TYPE;
	gCtrlVars.pga0_line4_pin      = CFG_LINE4_PIN_TYPE;
	gCtrlVars.pga0_line5_pin      = CFG_LINE5_PIN_TYPE;

    gCtrlVars.pga0_line1_l_gain   = CFG_LINE1_L_GAIN;
    gCtrlVars.pga0_line1_r_gain   = CFG_LINE1_R_GAIN;
    gCtrlVars.pga0_line2_l_gain   = CFG_LINE2_L_GAIN;
    gCtrlVars.pga0_line2_r_gain   = CFG_LINE2_R_GAIN;
    gCtrlVars.pga0_line4_l_gain   = CFG_LINE4_L_GAIN;
    gCtrlVars.pga0_line4_r_gain   = CFG_LINE4_R_GAIN;
    gCtrlVars.pga0_line5_l_gain     = CFG_LINE5_L_GAIN;
    gCtrlVars.pga0_line5_r_gain     = CFG_LINE5_R_GAIN;
	gCtrlVars.pga0_diff_mode      = CFG_DIFF_MODE;
	gCtrlVars.pga0_diff_l_gain    = CFG_DIFF_L_GAIN;
	gCtrlVars.pga0_diff_r_gain    = CFG_DIFF_R_GAIN;

	gCtrlVars.pga0_zero_cross     = CFG_PGA1_ZERO_CROSS;

    //for ADC0 DIGITAL  0x04
	gCtrlVars.adc0_channel_en     = CFG_ADC0_LR_CHANNEL_EN;
	gCtrlVars.adc0_mute           = CFG_ADC0_DIG_MUTE_EN;
	gCtrlVars.adc0_dig_l_vol      = CFG_ADC0_LEFT_DIG_VOLUME;
	gCtrlVars.adc0_dig_r_vol      = CFG_ADC0_RIGHT_DIG_VOLUME;
	gCtrlVars.adc0_sample_rate    = CFG_ADC0_SAMPLE_RATE;
	gCtrlVars.adc0_lr_swap        = CFG_ADC0_LR_CHANNEL_EN;
	gCtrlVars.adc0_dc_blocker     = CFG_ADC0_HPC;
	gCtrlVars.adc0_fade_time      = CFG_ADC0_FADE_TIME;
	gCtrlVars.adc0_mclk_src       = CFG_ADC0_MCLK_SRC;
	gCtrlVars.adc0_dc_blocker_en  = 0;

	//for AGC0 ADC0     0x05
	gCtrlVars.adc0_agc_mode       = CFG_ADC0_AGC_MODE;
	gCtrlVars.adc0_agc_max_level  = CFG_ADC0_AGC_MAX_LEVEL ;
	gCtrlVars.adc0_agc_target_level = CFG_ADC0_AGC_TARGET_LEVEL;
	gCtrlVars.adc0_agc_max_gain    = CFG_ADC0_AGC_MAX_GAIN;
	gCtrlVars.adc0_agc_min_gain    = CFG_ADC0_AGC_MIN_GAIN ;
	gCtrlVars.adc0_agc_gainoffset  = CFG_ADC0_AGC_OFFSET;
	gCtrlVars.adc0_agc_fram_time   = CFG_ADC0_AGC_FRAM_TIME;
	gCtrlVars.adc0_agc_hold_time   = CFG_ADC0_AGC_HOLD_TIME;
	gCtrlVars.adc0_agc_attack_time = CFG_ADC0_AGC_ATTACK_TIME;
	gCtrlVars.adc0_agc_decay_time  = CFG_ADC0_AGC_DECAY_TIME;
	gCtrlVars.adc0_agc_noise_gate_en = CFG_ADC0_AGC_NOISE_GATE_EN;
	gCtrlVars.adc0_agc_noise_threshold = CFG_ADC0_AGC_NOISE_THRESHOLD;
	gCtrlVars.adc0_agc_noise_gate_mode = CFG_ADC0_AGC_NOISE_GATE_MODE;
	gCtrlVars.adc0_agc_noise_time = CFG_ADC0_AGC_NOISE_GATE_HOLD_TIME;

	//for ADC1 PGA      0x06
	gCtrlVars.line3_l_mic1_en    = CFG_LINE3_L_MIC1_EN;
	gCtrlVars.line3_r_mic2_en    = CFG_LINE3_R_MIC2_EN;
    gCtrlVars.line3_l_mic1_gain  = CFG_LINE3_L_MIC1_GAIN;
	gCtrlVars.line3_l_mic1_boost = CFG_LINE3_L_MIC1_BOOST;
    gCtrlVars.line3_r_mic2_gain  = CFG_LINE3_R_MIC2_GAIN;
    gCtrlVars.line3_r_mic2_boost = CFG_LINE3_R_MIC2_BOOST;
	gCtrlVars.mic_or_line3       = CFG_MIC_LINE_DISP_TYPE;
	gCtrlVars.mic1_line3l_pin_en = CFG_MIC1_LINE3L_PIN_EN;
	gCtrlVars.mic2_line3r_pin_en = CFG_MIC2_LINE3R_PIN_EN;

	//for ADC1 DIGITAL  0x07
    gCtrlVars.adc1_channel_en     = CFG_ADC1_LR_CHANNEL_EN;
    gCtrlVars.adc1_mute           = CFG_ADC1_DIG_MUTE_EN;
    gCtrlVars.adc1_dig_l_vol      = CFG_ADC1_LEFT_DIG_VOLUME;
    gCtrlVars.adc1_dig_r_vol      = CFG_ADC1_RIGHT_DIG_VOLUME;
    gCtrlVars.adc1_sample_rate    = CFG_ADC1_SAMPLE_RATE;
    gCtrlVars.adc1_lr_swap        = CFG_ADC1_LEFT_RIGHT_SWAP;
	gCtrlVars.adc1_dc_blocker     = CFG_ADC1_HPC;
	gCtrlVars.adc1_fade_time      = CFG_ADC1_FADE_TIME;
	gCtrlVars.adc1_mclk_src       = CFG_ADC1_MCLK_SRC;
	gCtrlVars.adc1_dc_blocker_en  = 0;

	//for AGC1  ADC1    0x08
	gCtrlVars.adc1_agc_mode       = CFG_ADC1_AGC_MODE;
	gCtrlVars.adc1_agc_max_level  = CFG_ADC1_AGC_MAX_LEVEL ;
	gCtrlVars.adc1_agc_target_level = CFG_ADC1_AGC_TARGET_LEVEL;
	gCtrlVars.adc1_agc_max_gain    = CFG_ADC1_AGC_MAX_GAIN;
	gCtrlVars.adc1_agc_min_gain    = CFG_ADC1_AGC_MIN_GAIN ;
	gCtrlVars.adc1_agc_gainoffset  = CFG_ADC1_AGC_OFFSET;
	gCtrlVars.adc1_agc_fram_time   = CFG_ADC1_AGC_FRAM_TIME;
	gCtrlVars.adc1_agc_hold_time   = CFG_ADC1_AGC_HOLD_TIME;
	gCtrlVars.adc1_agc_attack_time = CFG_ADC1_AGC_ATTACK_TIME;
	gCtrlVars.adc1_agc_decay_time  = CFG_ADC1_AGC_DECAY_TIME;
	gCtrlVars.adc1_agc_noise_gate_en = CFG_ADC1_AGC_NOISE_GATE_EN;
	gCtrlVars.adc1_agc_noise_threshold = CFG_ADC1_AGC_NOISE_THRESHOLD;
	gCtrlVars.adc1_agc_noise_gate_mode = CFG_ADC1_AGC_NOISE_GATE_MODE;
	gCtrlVars.adc1_agc_noise_time = CFG_ADC1_AGC_NOISE_GATE_HOLD_TIME;

	//for DAC0          0x09
    #ifdef CFG_RES_AUDIO_DAC0_EN
	gCtrlVars.dac0_en             = CFG_DAC0_MODULE_EN;
    #else
	gCtrlVars.dac0_en             = 0;
    #endif
	gCtrlVars.dac0_sample_rate    = CFG_DAC0_SAMPLE_RATE;
	gCtrlVars.dac0_dig_mute       = CFG_DAC0_DIG_MUTE_EN;
	gCtrlVars.dac0_dig_l_vol      = CFG_DAC0_LEFT_DIG_VOLUME;
	gCtrlVars.dac0_dig_r_vol      = CFG_DAC0_RIGHT_DIG_VOLUME;
	gCtrlVars.dac0_dither         = CFG_DAC0_DITHER_EN;
	gCtrlVars.dac0_scramble       = CFG_DAC0_SCRAMBLE_EN;
	gCtrlVars.dac0_out_mode       = CFG_DAC0_OUT_MODE;
	gCtrlVars.dac0_pause_en       = CFG_DAC0_PAUSE_EN;
	gCtrlVars.dac0_sample_mode    = CFG_DAC0_SAMPLE_EDGE_MODE;
	gCtrlVars.dac0_scf_mute       = CFG_DAC0_SCF_MUTE_MODE;
	gCtrlVars.dac0_fade_time      = CFG_DAC0_FADE_TIME;
	gCtrlVars.dac0_zeros_number   = CFG_DAC0_ZERO_NUM;
	gCtrlVars.dac0_mclk_src       = CFG_DAC0_MCLK_SRC;
	gCtrlVars.dac0_out_bit_len    = CFG_DAC0_DOUT_WIDTH;

	//for DAC1          0x0a
    #ifdef CFG_RES_AUDIO_DACX_EN
	gCtrlVars.dac1_en             = CFG_DAC1_MODULE_EN;
    #else
	gCtrlVars.dac1_en             = 0;
    #endif
	gCtrlVars.dac1_sample_rate    = CFG_DAC1_SAMPLE_RATE;
	gCtrlVars.dac1_dig_mute       = CFG_DAC1_DIG_MUTE_EN;
	gCtrlVars.dac1_dig_l_vol      = CFG_DAC1_LEFT_DIG_VOLUME;
	gCtrlVars.dac1_dig_r_vol      = CFG_DAC1_RIGHT_DIG_VOLUME;
	gCtrlVars.dac1_dither         = CFG_DAC1_DITHER_EN;
	gCtrlVars.dac1_scramble       = CFG_DAC1_SCRAMBLE_EN;
	gCtrlVars.dac1_out_mode       = CFG_DAC1_OUT_MODE;
	gCtrlVars.dac1_pause_en       = CFG_DAC1_PAUSE_EN;
	gCtrlVars.dac1_sample_mode    = CFG_DAC1_SAMPLE_EDGE_MODE;
	gCtrlVars.dac1_scf_mute       = CFG_DAC1_SCF_MUTE_MODE;
	gCtrlVars.dac1_fade_time      = CFG_DAC1_FADE_TIME;
	gCtrlVars.dac1_zeros_number   = CFG_DAC1_ZERO_NUM;
	gCtrlVars.dac1_mclk_src       = CFG_DAC1_MCLK_SRC;
	gCtrlVars.dac1_out_bit_len    = CFG_DAC1_DOUT_WIDTH;

	//for i2s0          0x0b
	gCtrlVars.i2s0_tx_en          = CFG_I2S0_TX_EN;
	gCtrlVars.i2s0_rx_en          = CFG_I2S0_RX_EN;
	gCtrlVars.i2s0_sample_rate    = CFG_I2S0_SAMPLE_RATE;
	gCtrlVars.i2s0_mclk_src       = CFG_I2S0_MCLK_SRC;
	gCtrlVars.i2s0_work_mode      = CFG_I2S0_WORK_MODE;
	gCtrlVars.i2s0_word_len       = CFG_I2S0_WORD_LEN;
	gCtrlVars.i2s0_stereo_mono    = CFG_I2S0_MONO_EN;
	gCtrlVars.i2s0_fade_time      = CFG_I2S0_FADE_TIME;
	gCtrlVars.i2s0_format         = CFG_I2S0_FORMAT;
	gCtrlVars.i2s0_bclk_invert_en = CFG_I2S0_BCLK_INVERT_EN;
	gCtrlVars.i2s0_lrclk_invert_en = CFG_I2S0_LRCLK_INVERT_EN;

	//for i2s1          0x0c
	gCtrlVars.i2s1_tx_en          = CFG_I2S1_TX_EN;
	gCtrlVars.i2s1_rx_en          = CFG_I2S1_RX_EN;
	gCtrlVars.i2s1_sample_rate    = CFG_I2S1_SAMPLE_RATE;
	gCtrlVars.i2s1_mclk_src       = CFG_I2S1_MCLK_SRC;
	gCtrlVars.i2s1_work_mode      = CFG_I2S1_WORK_MODE;
	gCtrlVars.i2s1_word_len       = CFG_I2S1_WORD_LEN;
	gCtrlVars.i2s1_stereo_mono    = CFG_I2S1_MONO_EN;
	gCtrlVars.i2s1_fade_time      = CFG_I2S1_FADE_TIME;
	gCtrlVars.i2s1_format         = CFG_I2S1_FORMAT;
	gCtrlVars.i2s1_bclk_invert_en = CFG_I2S1_BCLK_INVERT_EN;
	gCtrlVars.i2s1_lrclk_invert_en = CFG_I2S1_LRCLK_INVERT_EN;

	//for spdif
	gCtrlVars.spdif_en            = CFG_SPDIF_EN;
	gCtrlVars.spdif_sample_rate   = CFG_SPDIF_SAMPLE_RATE;
	gCtrlVars.spdif_channel_mode  = CFG_SPDIF_CHANNEL_MODE;
	gCtrlVars.spdif_in_gpio       = 0;
	gCtrlVars.spdif_lock_status   = 0;

    //system define
	gCtrlVars.sample_rate         = CFG_PARA_SAMPLE_RATE;
    #ifdef	CFG_FUNC_AUDIO_EFFECT_EN	
	gCtrlVars.crypto_en           = CFG_COMMUNICATION_CRYPTO;
	gCtrlVars.password            = CFG_COMMUNICATION_PASSWORD;
    #endif
	gCtrlVars.adc_mic_channel_num = 2;//for mic
	gCtrlVars.adc_line_channel_num = 2;///for line ,fm ,bt ,usb ,i2s
	gCtrlVars.remind_type          = 0;
	gCtrlVars.UsbAudioMute         = 0;
	gCtrlVars.usb_audio_upload_flag =0;
	gCtrlVars.SamplesPerFrame      = CFG_PARA_SAMPLES_PER_FRAME;
	gCtrlVars.audio_effect_init_flag = 0;//1;

	gCtrlVars.UsbMicVolume           = 63;
	gCtrlVars.UsbAudioVolume         = 63;
    #ifdef CFG_FUNC_SHUNNING_EN
    gCtrlVars.ShunningMode          = FALSE;
	#endif
	#ifdef CFG_FUNC_DETECT_MIC_EN
    gCtrlVars.MicOnlin              = TRUE;
    gCtrlVars.Mic2Onlin             = TRUE;
	#endif
    #ifdef CFG_FUNC_DETECT_PHONE_EN
	gCtrlVars.EarPhoneOnlin         = FALSE;
	#endif
	#ifdef CFG_FUNC_DETECT_MIC_SEG_EN
	gCtrlVars.MicSegment            = 0;
	#endif
	PrivateMclkSet();
	GlobalSampeRateSet();
	GlobalMclkSet();
	Line3MicPinSet();
}
/*
****************************************************************
*
*主要用于在上位机上显示MIC或LINE3输入
*MIC或LINE3是一个特别的LINE，用户可在此自行加入修改
*mic_or_line3;
*mic1_line3l_pin_en;
*mic2_line3r_pin_en;

****************************************************************
*/
void Line3MicPinSet(void)
{
	//0=NO MIC, 1= MIC1_LINE3_L, 2= MIC2_LINE3_R, 3 = MCI1_LINE3_L+MIC2_LINE3_R

	#if (CFG_RES_MIC_SELECT == 0)
	gCtrlVars.mic_or_line3       = 1;//0= mic1,mic2;1 = line3; 2 = mic1,line3_r;3 = mic2,line3_l
	gCtrlVars.mic1_line3l_pin_en = 1;//0= no mic1/line3l pin disp, 1= enable mic1/line3l pin disp
	gCtrlVars.mic2_line3r_pin_en = 1;//0= no mic2/line3r pin disp, 1= enable mic2/line3r pin disp
	#endif

	#if (CFG_RES_MIC_SELECT == 1)
	gCtrlVars.mic_or_line3       = 0;//0= mic1,mic2;1 = line3; 2 = mic1,line3_r;3 = mic2,line3_l
	gCtrlVars.mic1_line3l_pin_en = 1;//0= no mic1/line3l pin disp, 1= enable mic1/line3l pin disp
	gCtrlVars.mic2_line3r_pin_en = 0;//0= no mic2/line3r pin disp, 1= enable mic2/line3r pin disp
	#endif

	#if (CFG_RES_MIC_SELECT == 2)
	gCtrlVars.mic_or_line3       = 0;//0= mic1,mic2;1 = line3; 2 = mic1,line3_r;3 = mic2,line3_l
	gCtrlVars.mic1_line3l_pin_en = 0;//0= no mic1/line3l pin disp, 1= enable mic1/line3l pin disp
	gCtrlVars.mic2_line3r_pin_en = 1;//0= no mic2/line3r pin disp, 1= enable mic2/line3r pin disp
	#endif

	#if (CFG_RES_MIC_SELECT == 3)
	gCtrlVars.mic_or_line3       = 0;//0= mic1,mic2;1 = line3; 2 = mic1,line3_r;3 = mic2,line3_l
	gCtrlVars.mic1_line3l_pin_en = 1;//0= no mic1/line3l pin disp, 1= enable mic1/line3l pin disp
	gCtrlVars.mic2_line3r_pin_en = 1;//0= no mic2/line3r pin disp, 1= enable mic2/line3r pin disp
	#endif
}
/*
****************************************************************
*
*
*
****************************************************************
*/
void CpuVerification(void)
{
    uint32_t ClockVal;
	ClockVal = Clock_CoreClockFreqGet()/1000000;
    APP_DBG("core Freq = %ldM\n", ClockVal);
	gCtrlVars.CpuMaxFreq = ClockVal;
}
/*
****************************************************************
*
*
*
****************************************************************
*/
void AudioLineSelSet(void)
{
	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道 

	AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT,  LINEIN_NONE);
	AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN_NONE);

    //AudioADC_AnaInit();
	//AudioADC_VcomConfig(1);//MicBias en
	//AudioADC_MicBias1Enable(1);
	//AudioADC_DynamicElementMatch(ADC0_MODULE, TRUE, TRUE);

	//--------------------line 1-----------------------------------------//
	if(gCtrlVars.pga0_line1_l_en)
	{
		//APP_DBG("LINE 1L En\n");    	 		  
		AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT,LINEIN1_LEFT);
		AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT,  LINEIN1_LEFT,  31 - gCtrlVars.pga0_line1_l_gain, 4);
	}
	if(gCtrlVars.pga0_line1_r_en)
	{
		//APP_DBG("LINE 1R En\n");
		AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT,LINEIN1_RIGHT);
		AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT,  31 - gCtrlVars.pga0_line1_r_gain, 4);
	}
	//---------------line 2-------------------------------------------//
	if(gCtrlVars.pga0_line2_l_en)
	{
		// APP_DBG("LINE 2L En\n");		
		AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT,LINEIN2_LEFT);
		AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT,  LINEIN2_LEFT,  31 - gCtrlVars.pga0_line2_l_gain, 4);
	}
	if(gCtrlVars.pga0_line2_r_en)
	{
		//APP_DBG("LINE 2R En\n");		
		AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT,LINEIN2_RIGHT);
		AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN2_RIGHT,  31 - gCtrlVars.pga0_line2_r_gain, 4);
	}
	//----------------line 4------------------------------------------//
	if(gCtrlVars.pga0_line4_l_en)
	{
		//APP_DBG("LINE4_L En\n");
		#ifndef CFG_FUNC_SW_DEBUG_EN
		GPIO_PortBModeSet(GPIOB0, 6);
		#endif
		AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT,LINEIN4_LEFT);
		AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN4_LEFT,  63 - gCtrlVars.pga0_line4_l_gain, 4);
	}
	if(gCtrlVars.pga0_line4_r_en)
	{
		//APP_DBG("LINE4_R En\n");
		#ifndef CFG_FUNC_SW_DEBUG_EN
		GPIO_PortBModeSet(GPIOB1,6);
		#endif
		AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT,LINEIN4_RIGHT);
		AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN4_RIGHT,  63 - gCtrlVars.pga0_line4_r_gain, 4);
	}
	//-----------------line 5--------------------------------------------//
	if(gCtrlVars.pga0_line5_l_en)
	{
		//APP_DBG("LINE5_L En\n");  
		GPIO_PortBModeSet(GPIOB3, 5);
		AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT,LINEIN5_LEFT);
		AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN5_LEFT,  63 - gCtrlVars.pga0_line5_l_gain, 4);
	}
	if(gCtrlVars.pga0_line5_r_en)
	{
		//APP_DBG("LINE5_R En\n");  
		GPIO_PortBModeSet(GPIOB2, 5);
		AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT,LINEIN5_RIGHT);
		AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN5_RIGHT,  63 - gCtrlVars.pga0_line5_r_gain, 4);
	}
	//APP_DBG("channel: %08lx\n ",*ch_reg);
	//APP_DBG("gain: %08lx\n ",*gain_reg);
}
/*
****************************************************************
*
*
*
****************************************************************
*/
void AudioLine3MicSelect(void)
{
	if(gCtrlVars.line3_l_mic1_en && gCtrlVars.mic1_line3l_pin_en)
	{
		AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT,LINEIN3_LEFT_OR_MIC1);
		AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT,  LINEIN3_LEFT_OR_MIC1, 31 - gCtrlVars.line3_l_mic1_gain, MIC_BOOST_LIST[gCtrlVars.line3_l_mic1_boost]);//0db bypass
	}
	else
	{
		AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT,LINEIN_NONE);
	}

	if(gCtrlVars.line3_r_mic2_en && gCtrlVars.mic2_line3r_pin_en)
	{
		AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT,LINEIN3_RIGHT_OR_MIC2);
		AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2,  31 - gCtrlVars.line3_r_mic2_gain, MIC_BOOST_LIST[gCtrlVars.line3_r_mic2_boost]);
	}
	else
	{
		AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT,LINEIN_NONE);
	}
}
/*
****************************************************************
*
*
*
****************************************************************
*/
void AudioAnaChannelSet(int8_t ana_input_ch)
{
	gCtrlVars.pga0_line1_l_en = 0;
	gCtrlVars.pga0_line1_r_en = 0;
	gCtrlVars.pga0_line2_l_en = 0;
	gCtrlVars.pga0_line2_r_en = 0;
	#if (CFG_RES_MIC_SELECT == 0)//作为linein3应用
	gCtrlVars.line3_l_mic1_en = 0;
	gCtrlVars.line3_r_mic2_en = 0;
	#endif
	gCtrlVars.pga0_line4_l_en = 0;
	gCtrlVars.pga0_line4_r_en = 0;
	gCtrlVars.pga0_line5_l_en = 0;
	gCtrlVars.pga0_line5_r_en = 0;

	if(ANA_INPUT_CH_LINEIN1 == ana_input_ch)
	{
		gCtrlVars.pga0_line1_l_en = 1;
		gCtrlVars.pga0_line1_r_en = 1;
		AudioLineSelSet();
	}

	if(ANA_INPUT_CH_LINEIN2 == ana_input_ch)
	{
		gCtrlVars.pga0_line2_l_en = 1;
		gCtrlVars.pga0_line2_r_en = 1;
		AudioLineSelSet();
	}
	
    #if (CFG_RES_MIC_SELECT == 0)//作为linein3应用
	if(ANA_INPUT_CH_LINEIN3 == ana_input_ch)
	{
		gCtrlVars.line3_l_mic1_en = 1;
		gCtrlVars.line3_r_mic2_en = 1;
		AudioLine3MicSelect();
	}
	#endif
	
	if(ANA_INPUT_CH_LINEIN4 == ana_input_ch)
	{
		gCtrlVars.pga0_line4_l_en = 1;
		gCtrlVars.pga0_line4_r_en = 1;
		AudioLineSelSet();
	}
	
	if(ANA_INPUT_CH_LINEIN5 == ana_input_ch)
	{
		gCtrlVars.pga0_line5_l_en = 1;
		gCtrlVars.pga0_line5_r_en = 1;
		AudioLineSelSet();
	}	
}

/*
****************************************************************
* 音效内存释放函数
*
*
****************************************************************
*/
extern const unsigned char BEEP_44K_TAB[2784];

void BeepEnable(void)
{
    #ifdef CFG_FUNC_BEEP_EN
    if(!gCtrlVars.beep_en)
	{
		gCtrlVars.beep_en = 10;
		gCtrlVars.beepAddr = 0;
	}
    #endif
}

void Beep(int16_t *OutBuf,uint16_t n)
{
#ifdef CFG_FUNC_BEEP_EN
	int32_t Data_L;
	int32_t Data_R;
	static int16_t beep_pcm;
	uint32_t s;

    if(!gCtrlVars.beep_en) return;

    for(s = 0; s < n; s++)
	{
		Data_L = BEEP_44K_TAB[gCtrlVars.beepAddr++];
		Data_R = BEEP_44K_TAB[gCtrlVars.beepAddr++];
		
		beep_pcm = Data_L | (Data_R<<8);
		#if CFG_PARA_MAX_VOLUME_NUM == 32
		Data_L = (int32_t)OutBuf[2*s+0] + ((int32_t)beep_pcm *DigVolTab_32[CFG_PARA_BEEP_DEFAULT_VOLUME] >> 12);
		Data_R = (int32_t)OutBuf[2*s+1] + ((int32_t)beep_pcm *DigVolTab_32[CFG_PARA_BEEP_DEFAULT_VOLUME] >> 12);
		#endif
		
		#if CFG_PARA_MAX_VOLUME_NUM == 16
		Data_L = (int32_t)OutBuf[2*s+0] + ((int32_t)beep_pcm *DigVolTab_16[CFG_PARA_BEEP_DEFAULT_VOLUME] >> 12);
		Data_R = (int32_t)OutBuf[2*s+1] + ((int32_t)beep_pcm *DigVolTab_16[CFG_PARA_BEEP_DEFAULT_VOLUME] >> 12);
		#endif
		
		OutBuf[2 * s + 0] = __nds32__clips(Data_L, (16-1));
		OutBuf[2 * s + 1] = __nds32__clips(Data_R, (16-1));
	}

	if(gCtrlVars.beepAddr > (sizeof(BEEP_44K_TAB)-16*100))
	{
		gCtrlVars.beep_en = 0;
		gCtrlVars.beepAddr = 0;
	}
 #endif
}


