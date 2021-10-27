#include <stdint.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "uarts.h"
#include "dma.h"
#include "uarts_interface.h"
#include "timeout.h"
#include "debug.h"
#include "app_config.h"
#include "i2s.h"
#include "i2s_interface.h"
#include "clk.h"
#include "ctrlvars.h"
#include "audio_effect.h"
#include "audio_adc.h"
#include "dac.h"
#include "communication.h"
#include "audio_effect_library.h"
#include "rtos_api.h"
#include "watchdog.h"
#include "mode_switch_api.h"
#include "delay.h"
#include "main_task.h"


#ifdef CFG_FUNC_AUDIO_EFFECT_EN
extern uint8_t  hid_tx_buf[];
extern uint32_t SysemMipsPercent;
int osPortRemainMem(void);
#ifdef FUNC_OS_EN
osMutexId AudioEffectMutex = NULL;
osMutexId LoadAudioParamMutex = NULL;
#endif

#define  CTL_DATA_SIZE   2

uint8_t  tx_buf[256]     = {0xa5, 0x5a, 0x00, 0x00,};
uint8_t  effect_sum		 = 0;
uint16_t effect_list[60] = {0x0};
uint32_t effect_addr[60] = {0x0};
uint32_t eff_addr_compare[60];
uint16_t effect_list_addr[60] = {0x0};
uint8_t  communic_buf[512];
uint32_t communic_buf_w = 0;
uint8_t  cbuf[8];

bool IsEffectSampleLenChange = 0;
bool IsEffectChange = 1;
TIMER EffectChangeTimer;//音效命令延时处理

const uint8_t DrcCommandLen_tab[10]={
      2,//0,//en
      2,//1,//fc
      2,//2,//mode
	  4,//3,//q[0~1]
	  6,//4,//threshold[0~2]
	  6,//5,//ratio[0~2]
	  6,//6,//attack_tc[0~2]
	  6,//7,//release_tc[0~2]
	  2,//8,//pregain1
	  2,//9,//pregain2
};
const char *AudioLibVer =  AUDIO_EFFECT_LIBRARY_VERSION;
const char AudioEffectNameList[][25]=//
{
	#if CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN
	{"2:Music Noise Suppressor "},
	#endif
	#if CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN
	{"1:Mic Noise Suppressor   "},
	#endif
    #if CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN
	{"1:Mic Freq Shifter       "},
	#endif
	#if CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN
	{"1:Mic Howling Dector     "},
	#endif
	#if CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN
	{"1:Mic Pitch Shifter      "},
	#endif
	#if CFG_AUDIO_EFFECT_MIC_AUTO_TUNE_EN
	{"1:Mic Auto Tune          "},
	#endif
	#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_EN
	{"1:Mic Voice Changer      "},
	#endif
    #if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
	{"1:Mic Voice Changer Pro  "},
    #endif
	#if CFG_AUDIO_EFFECT_MIC_ECHO_EN
	{"1:Mic Echo               "},
	#endif
	#if CFG_AUDIO_EFFECT_MIC_REVERB_EN
	{"1:Mic Reverb             "},
	#endif
	#if CFG_AUDIO_EFFECT_MIC_PLATE_REVERB_EN
	{"1:Mic Plate Reverb       "},
	#endif
	#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
	{"1:Mic Pro Reverb         "},
	#endif	
    #if CFG_AUDIO_EFFECT_PINGPONG_EN
    {"3:Guitar PingPong        "},
    #endif
    #if CFG_AUDIO_EFFECT_CHORUS_EN
    {"3:Guitar Chorus          "},
    #endif
    #if CFG_AUDIO_EFFECT_AUTOWAH_EN
    {"3:Guitar Auto Wah        "},
    #endif
    #if CFG_AUDIO_EFFECT_MIC_SILENCE_DECTOR_EN
	{"1:Mic Silence Dector     "},
    #endif
	#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
	{"2:Music Pitch Shifter Pro"},
	#endif
    #if CFG_AUDIO_EFFECT_VOCAL_CUT_EN
	{"2:Music Voice Cut        "},
    #endif
	#if CFG_AUDIO_EFFECT_VOCAL_REMOVE_EN
	{"2:Music Voice Remove     "},
    #endif

    #if CFG_AUDIO_EFFECT_MUSIC_3D_EN
	{"2:Music 3D               "},
    #endif
	#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
	{"2:Music 3D Plus          "},
    #endif
	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
	{"2:Music Virtual Bass     "},
    #endif
	#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
	{"4:Rec Virtual Bass     "},
    #endif
	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
	{"2:Music Virtual Bass Clas"},
    #endif
    #if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
    {"2:Music Stereo Windener"  },
    #endif
	#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
	{"2:Music Delay            "},
    #endif   
	#if CFG_AUDIO_EFFECT_MUSIC_EXCITER_EN
	{"2:Music Exciter          "},
    #endif   
	#if CFG_AUDIO_EFFECT_PHASE_EN
	{"2:Music Phase            "},
    #endif   
	#if CFG_AUDIO_EFFECT_MUSIC_DRC_EN
	{"2:Music Drc              "},
	#endif	
    #if CFG_AUDIO_EFFECT_MIC_DRC_EN
	{"1:Mic Drc                "},
    #endif	
	#if CFG_AUDIO_EFFECT_REC_DRC_EN
	{"4:Rec Drc                "},
    #endif
    #if CFG_AUDIO_EFFECT_MUSIC_PRE_EQ_EN
	{"2:Music Pre Eq           "},
    #endif	
	#if CFG_AUDIO_EFFECT_MUSIC_PRE_EQ_EN
	{"2:Music Out Eq           "},
    #endif	
	#if CFG_AUDIO_EFFECT_MIC_PRE_EQ_EN
	{"1:Mic Pre Eq             "},
    #endif	
	#if CFG_AUDIO_EFFECT_MIC_BYPASS_EQ_EN
	{"1:Mic Bypass Eq          "},
    #endif	
	#if CFG_AUDIO_EFFECT_MIC_ECHO_EQ_EN
	{"1:Mic Echo Eq            "},
    #endif	
	#if CFG_AUDIO_EFFECT_MIC_REVERB_EQ_EN
	{"1:Mic Reverb Eq          "},
    #endif	
	#if CFG_AUDIO_EFFECT_MIC_OUT_EQ_EN
	#ifdef CFG_FUNC_MIC_KARAOKE_EN
	{"1:Mic Out Eq             "},
	#else
	{"4:Rec Pre EQ             "},
	#endif
    #endif	
    #if CFG_AUDIO_EFFECT_REC_EQ_EN
	{"4:Rec Out EQ             "},
    #endif
    #if CFG_AUDIO_EFFECT_AUX_GAIN_CONTROL_EN
	{"2:Music Out Gain         "},
    #endif
	#if CFG_AUDIO_EFFECT_MIC_BYPASS_GAIN_CONTROL_EN
	{"1:Mic Bypass Gain        "},
    #endif
	#if CFG_AUDIO_EFFECT_MIC_ECHO_GAIN_CONTROL_EN
	{"1:Mic Echo Gain          "},
    #endif
	#if CFG_AUDIO_EFFECT_MIC_REVERB_GAIN_CONTROL_EN
	{"1:Mic Reverb Gain        "},
    #endif
	#if CFG_AUDIO_EFFECT_MIC_OUT_GAIN_CONTROL_EN
	{"1:Mic Out Gain           "},
    #endif
	#if CFG_AUDIO_EFFECT_REC_BYPASS_GAIN_CONTROL_EN
	{"4:Rec Bypass Gain        "},
    #endif
	#if CFG_AUDIO_EFFECT_REC_EFFECT_GAIN_CONTROL_EN
	{"4:Rec Effect Gain        "},
    #endif
	#if CFG_AUDIO_EFFECT_REC_AUX_GAIN_CONTROL_EN
	{"4:Rec Music Gain         "},
    #endif
	#if CFG_AUDIO_EFFECT_REC_REMIND_GAIN_CONTROL_EN
	{"4:Rec Effect Remind Gain "},
    #endif 	
	#if CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN
	{"4:Usb Out Gain           "},
    #endif
	#if CFG_AUDIO_EFFECT_REMIND_KEY_GAIN_CONTROL_EN
	{"4:Key Remind Gain        "},
    #endif
	#if CFG_AUDIO_EFFECT_REMIND_EFFECT_GAIN_CONTROL_EN
	{"4:Effect Remind Gain     "},
    #endif 
	#if CFG_AUDIO_EFFECT_I2S_IN_GAIN_CONTROL_EN
	{"2:I2s In Gain            "},
    #endif
	#if CFG_AUDIO_EFFECT_BT_IN_GAIN_CONTROL_EN
	{"2:Bt In Gain             "},
    #endif
	#if CFG_AUDIO_EFFECT_USB_IN_GAIN_CONTROL_EN
	{"2:Usb&Card In Gain       "},
    #endif
	#if CFG_AUDIO_EFFECT_SPDIF_IN_GAIN_CONTROL_EN
	{"2:Spdif In Gain          "},
    #endif
};

const char AudioEffectNameList_HFP[][25]=//
{
    {"Noise Suppressor   	"},
    {"DRC				   	"},
    {"EQ        			"},
	{"Gain      			"},
	{"AEC       			"},
};

const uint16_t HPCList[3]={
	0xffe, //  48k 20Hz  -1.5db 
	0xFFC, //  48k 40Hz  -1.5db 
	0xFFD, //  32k 40Hz  -1.5db 
};

void Communication_Effect_Init(void)
{
	uint32_t i;
	uint8_t eff_addr;

	eff_addr = 0x81;
	communic_buf_w = 0;
	memset(communic_buf, 0, sizeof(communic_buf));
	effect_sum = 0;

	#if CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN
	effect_list[effect_sum] = 5;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.music_expander_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif

	#if CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN
	effect_list[effect_sum] = 5;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.mic_expander_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif

    #if CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN
	effect_list[effect_sum] = 6;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.freq_shifter_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif

	#if CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN
	effect_list[effect_sum] = 7;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.howling_dector_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif
	
	#if CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN
	effect_list[effect_sum] = 9;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.pitch_shifter_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif	

	#if CFG_AUDIO_EFFECT_MIC_AUTO_TUNE_EN
	effect_list[effect_sum] = 0;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.auto_tune_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif

	#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_EN
	effect_list[effect_sum] = 14;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.voice_changer_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif

	#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
	effect_list[effect_sum] = 19;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.voice_changer_pro_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif

	#if CFG_AUDIO_EFFECT_MIC_ECHO_EN
	effect_list[effect_sum] = 3;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.echo_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif

	#if CFG_AUDIO_EFFECT_MIC_REVERB_EN
	effect_list[effect_sum] = 10;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.reverb_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif

	#if CFG_AUDIO_EFFECT_MIC_PLATE_REVERB_EN
	effect_list[effect_sum] = 17;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.plate_reverb_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif

	#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
	effect_list[effect_sum] = 18;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.reverb_pro_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif

	#if CFG_AUDIO_EFFECT_PINGPONG_EN
	effect_list[effect_sum] = 29;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.ping_pong_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
    #endif

	#if CFG_AUDIO_EFFECT_CHORUS_EN
	effect_list[effect_sum] = 26;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.chorus_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
    #endif

	#if CFG_AUDIO_EFFECT_AUTOWAH_EN
	effect_list[effect_sum] = 27;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.auto_wah_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
    #endif
	
    #if CFG_AUDIO_EFFECT_MIC_SILENCE_DECTOR_EN
	effect_list[effect_sum] = 11;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.MicAudioSdct_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
    #endif

	#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
	effect_list[effect_sum] = 22;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.pitch_shifter_pro_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
	#endif
	
    #if CFG_AUDIO_EFFECT_VOCAL_CUT_EN
	effect_list[effect_sum] = 16;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.vocal_cut_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum ++;
    #endif

	#if CFG_AUDIO_EFFECT_VOCAL_REMOVE_EN
	effect_list[effect_sum] = 21;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.vocal_remove_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
    #endif

	#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
	effect_list[effect_sum] = 12;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.music_threed_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
    #endif

	#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
	effect_list[effect_sum] = 30;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.music_threed_plus_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
    #endif
	
    #if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
	effect_list[effect_sum] = 13;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.music_vb_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum ++;
    #endif

	#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
	effect_list[effect_sum] = 13;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.rec_vb_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum ++;
    #endif

	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
	effect_list[effect_sum] = 23;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.music_vb_classic_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
    #endif

    #if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
    effect_list[effect_sum] = 28;
    effect_addr[effect_sum] = (uint32_t)&gCtrlVars.stereo_winden_unit;
    effect_list_addr[effect_sum] = eff_addr++;
    effect_sum++;
    #endif

	#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
	effect_list[effect_sum] = 24;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.music_delay_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum ++;
    #endif    

	#if CFG_AUDIO_EFFECT_MUSIC_EXCITER_EN
	effect_list[effect_sum] = 25;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.music_exciter_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
    #endif    

	#if CFG_AUDIO_EFFECT_PHASE_EN
	effect_list[effect_sum] = 20;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.phase_control_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
    #endif    

	for(i = 0; i < sizeof(drc_unit_aggregate)/sizeof(drc_unit_aggregate[0]); i++)
	{
		if(DRC_DEFAULT_TABLE[0][i] == 1)
		{
			effect_list[effect_sum] = 2;
			effect_addr[effect_sum] = (uint32_t)drc_unit_aggregate[i];
			effect_list_addr[effect_sum] = eff_addr++;
			effect_sum++;
		}
	}

	for(i = 0; i < sizeof(eq_unit_aggregate)/sizeof(eq_unit_aggregate[0]); i++)
	{
		if(EQ_DEFAULT_TABLE[0][i] == 1)
		{
			effect_list[effect_sum] = 4;
			effect_addr[effect_sum] = (uint32_t)eq_unit_aggregate[i];
			effect_list_addr[effect_sum] = eff_addr++;
			effect_sum++;
		}
	}

	for(i = 0; i < sizeof(gain_unit_aggregate)/sizeof(gain_unit_aggregate[0]); i++)
	{
		if(GAIN_CONTROL_TABLE[0][i] == 1)
		{
			effect_list[effect_sum] = 15;
			effect_addr[effect_sum] = (uint32_t)gain_unit_aggregate[i];
			effect_list_addr[effect_sum] = eff_addr++;
			effect_sum++;
		}
	}
}

void Communication_Effect_Init_BTHFP(void)
{
	uint32_t i;
	uint8_t eff_addr;

	eff_addr = 0x81;
	communic_buf_w = 0;
	memset(communic_buf, 0, sizeof(communic_buf));
	effect_sum = 0;

	effect_list[effect_sum] = 5;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.mic_expander_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;

	effect_list[effect_sum] = 2;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.mic_drc_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;	

	effect_list[effect_sum] = 4;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.mic_bypass_eq_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;

	effect_list[effect_sum] = 15;
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.mic_reverb_gain_control_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;

	effect_list[effect_sum] = 100;//AEC,用户自定义
	effect_addr[effect_sum] = (uint32_t)&gCtrlVars.mic_aec_unit;
	effect_list_addr[effect_sum] = eff_addr++;
	effect_sum++;
}

void Communication_Effect_Send(uint8_t *buf, uint32_t len)
{
#ifdef CFG_COMMUNICATION_BY_UART
	//UART1_DMA_BlockSend(buf, len);
	DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, buf, len);
#endif

#ifdef CFG_COMMUNICATION_BY_USB
	memcpy(hid_tx_buf, buf, 256);
#endif
}
void GetAudioLibVer(uint8_t *Buf)
{
	const char *ver;
	uint8_t count;
	ver = AudioLibVer;
	//------------------------------------//
	count = 0;
	while(*ver != '\0')
	{
	  if(*ver == '.')
	  {
		  count++;
		  ver++;
		  continue;
	  }
	  Buf[count] *= 10;
	  Buf[count] += (*ver-0x30);
	  ver++;
	}
}

bool IsEffectBufSampleRateEnalbe = 0;
void Communication_Effect_0x00(void)
{
    memset(tx_buf, 0, sizeof(tx_buf));
	tx_buf[0]  = 0xa5;
	tx_buf[1]  = 0x5a;
	tx_buf[2]  = 0x00;
	tx_buf[3]  = 0x07;
	tx_buf[4]  = 0x30;//20=kmic 21=O26  30=B1X
	tx_buf[5]  = CFG_EFFECT_MAJOR_VERSION;
	tx_buf[6]  = CFG_EFFECT_MINOR_VERSION;
	tx_buf[7]  = CFG_EFFECT_USER_VERSION;
	GetAudioLibVer(&tx_buf[8]);
	//-----------------------------------//
	tx_buf[11] = 0x16;
	Communication_Effect_Send(tx_buf, 12);
}

void Comm_Effect_0x01(uint8_t *buf)
{
	uint16_t TmpData;
	switch(buf[0])
	{
		case 0:////system mode {buf[1] =0 standby mode} {buf[1] =1 sleep mode} value= enum
		    memcpy(&TmpData, &buf[1], 2);
		    TmpData = TmpData > 1? 1 : TmpData;
		    gCtrlVars.sys_mode =  TmpData;
			break;

		case 1:////system reset {buf[1] =0 standby mode} {buf[1] =1 reset mode} value= enum
		    memcpy(&TmpData, &buf[1], 2);
		    TmpData = TmpData > 1? 1 : TmpData;
		    gCtrlVars.sys_reset =  TmpData;
			break;

		case 2:////global system sample rate enable value= bool
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.sys_sample_rate_en =  TmpData;
			break;
		case 3:////global system sample rate set value= enum( 0~8)
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 8? 8 : TmpData;
			gCtrlVars.sys_sample_rate =  TmpData;
			printf("gCtrlVars.sys_sample_rate = %d\n", gCtrlVars.sys_sample_rate);
			//Sam，modify for AEC
			if((gCtrlVars.sys_sample_rate_en == 1) && IsEffectBufSampleRateEnalbe)
			{
				gCtrlVars.sample_rate = SupportSampleRateList[gCtrlVars.sys_sample_rate];
			}
			break;

		case 4:///global System MCLK source enable value= bool
		    memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.sys_mclk_src_en =  TmpData;
			break;

		case 5:///global System MCLK source select value= enum( 0~4)
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4? 4 : TmpData;
			gCtrlVars.sys_mclk_src =  TmpData;
			//AudioClkConfig(gCtrlVars.sys_mclk_src, gCtrlVars.pll_clk_source);
			break;
		case 6:/// System default set (0=standard   1=recove default set)
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.sys_default_set =  TmpData;
			break;				
	}
}

void Communication_Effect_0x01(uint8_t *buf, uint32_t len)
{
	uint16_t i,k;
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x01;
		tx_buf[3]  = 1+7*2;
		tx_buf[4]  = 0xff;
		memcpy(&tx_buf[5],  &gCtrlVars.sys_mode, 2);
		memcpy(&tx_buf[7],  &gCtrlVars.sys_reset, 2);
		memcpy(&tx_buf[9], &gCtrlVars.sys_sample_rate_en, 2);
		memcpy(&tx_buf[11], &gCtrlVars.sys_sample_rate, 2);
		memcpy(&tx_buf[13], &gCtrlVars.sys_mclk_src_en, 2);
		memcpy(&tx_buf[15], &gCtrlVars.sys_mclk_src, 2);
		memcpy(&tx_buf[17], &gCtrlVars.sys_default_set, 2);
		tx_buf[5 + 7*2] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 7*2);
	}
	else
	{
		switch(buf[0])
		{
		    case 0xff:
				buf++;
				for(i = 0; i < 7; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_Effect_0x01(&cbuf[0]);
					buf += 2;
				}
		    	 break;
			default:
				Comm_Effect_0x01(buf);
				break;
		}
	}
}

void Communication_Effect_0x02(void)///systme ram
{
    memset(tx_buf, 0, sizeof(tx_buf));

	tx_buf[0]  = 0xa5;
	tx_buf[1]  = 0x5a;
	tx_buf[2]  = 0x02;
	tx_buf[3]  = 1 +  1 + 2*4 ;
	tx_buf[4]  = 0xff;
	gCtrlVars.UsedRamSize = (320 * 1024 - osPortRemainMem())/1024;
	gCtrlVars.CpuMaxFreq = Clock_CoreClockFreqGet() / 1000000;//240;
	gCtrlVars.cpu_mips = (uint16_t)(((10000 - SysemMipsPercent) * (Clock_CoreClockFreqGet()/1000000)) / 10000);

	memcpy(&tx_buf[5], &gCtrlVars.UsedRamSize, 2);
	memcpy(&tx_buf[7], &gCtrlVars.cpu_mips, 2);
	memcpy(&tx_buf[9], &gCtrlVars.AutoRefresh, 1);

	if(gCtrlVars.AutoRefresh)  gCtrlVars.AutoRefresh--;

	memcpy(&tx_buf[10], &gCtrlVars.CpuMaxFreq, 2);
	memcpy(&tx_buf[12], &gCtrlVars.CpuMaxRamSize, 2);
	tx_buf[14] = 0x16;
	Communication_Effect_Send(tx_buf, 15);
}


void Communication_Effect_0xfc(uint8_t *buf, uint8_t len)//user define tag
{
	memset(tx_buf, 0, sizeof(tx_buf));
	tx_buf[0]  = 0xa5;
	tx_buf[1]  = 0x5a;
	tx_buf[2]  = 0xfc;//control
	tx_buf[3]  = 1+ 4;//len=data_size+index
	tx_buf[4]  = 0xff;///index

	if(len == 0) //ask
	{
		tx_buf[5]  = 1;//data
		tx_buf[6]  = 2;//data
		tx_buf[7]  = 3;//data
		tx_buf[8]  = 4;//data

		tx_buf[9] = 0x16;///end
		Communication_Effect_Send(tx_buf, 10);
	}
	else
	{
		tx_buf[5]  = 1;//data
		tx_buf[6]  = 2;//data
		tx_buf[7]  = 3;//data
		tx_buf[8]  = 4;//data

		tx_buf[9] = 0x16;///end
		Communication_Effect_Send(tx_buf, 10);
	}
}

void Comm_PGA0_0x03(uint8_t * buf)
{
	uint16_t TmpData;

	switch(buf[0])///ADC0 PGA
	{
		case 0:///line1 Left en?
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
		    gCtrlVars.pga0_line1_l_en = TmpData;
			AudioLineSelSet();
			break;
	
		case 1://line1 Right en?
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
		    gCtrlVars.pga0_line1_r_en = TmpData;		
			AudioLineSelSet();
			break;
			
		case 2:///line2 Left en?
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.pga0_line2_l_en = TmpData;
			AudioLineSelSet();
			break;
			
		case 3://line2 Right en?
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.pga0_line2_r_en = TmpData;	
			AudioLineSelSet();
		    break;
			
		case 4:///line4 Left en?
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.pga0_line4_l_en = TmpData;
			AudioLineSelSet();
			break;
			
		case 5://line4 Right en?
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.pga0_line4_r_en = TmpData;	
			AudioLineSelSet();
		   break;		

		case 6:///line5 Left en?
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.pga0_line5_l_en = TmpData;
			AudioLineSelSet();
			break;
			
		case 7://line5 Right en?
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.pga0_line5_r_en = TmpData;	
			AudioLineSelSet();
		    break;

		case 8:///line1 Left gain
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 31? 31 : TmpData;
			gCtrlVars.pga0_line1_l_gain = TmpData;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT, 31 - gCtrlVars.pga0_line1_l_gain, 4);
			break;
	
		case 9://////line1 Right gain
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 31? 31 : TmpData;
			gCtrlVars.pga0_line1_r_gain = TmpData;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT, 31 - gCtrlVars.pga0_line1_r_gain, 4);
			break;
			
		case 10:///line2 Left gain
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 31? 31 : TmpData;
			gCtrlVars.pga0_line2_l_gain = TmpData;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN2_LEFT, 31 - gCtrlVars.pga0_line2_l_gain, 4);
			break;
	
		case 11://////line2 Right gain
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 31? 31 : TmpData;
			gCtrlVars.pga0_line2_r_gain = TmpData;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN2_RIGHT, 31 - gCtrlVars.pga0_line2_r_gain, 4);
			break;		

		case 12:///line4 Left gain
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 63? 63 : TmpData;
			if(gCtrlVars.pga0_line5_l_en)
			{
				gCtrlVars.pga0_line5_l_gain = TmpData;
				AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN5_LEFT, 63 - gCtrlVars.pga0_line5_l_gain, 4);
			}
			if(gCtrlVars.pga0_line4_l_en)
			{
				gCtrlVars.pga0_line4_l_gain = TmpData;
				AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN4_LEFT, 63 - gCtrlVars.pga0_line4_l_gain, 4);
			}
			break;
	
		case 13://////line4 Right gain
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 63? 63 : TmpData;
			if(gCtrlVars.pga0_line5_r_en)
			{
				gCtrlVars.pga0_line5_r_gain = TmpData;
				AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN5_RIGHT, 63 - gCtrlVars.pga0_line5_r_gain, 4);
			}
			if(gCtrlVars.pga0_line4_r_en)
			{
				gCtrlVars.pga0_line4_r_gain = TmpData;
				AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN4_RIGHT, 63 - gCtrlVars.pga0_line4_r_gain, 4);
			}
			break;	
			
		case 14:///line5 Left gain
		    #if 0
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 63? 63 : TmpData;
			if(gCtrlVars.pga0_line5_l_en)
			{
				gCtrlVars.pga0_line5_l_gain = TmpData;
				AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN5_LEFT, 63 - gCtrlVars.pga0_line5_l_gain, 4);
			}
			if(gCtrlVars.pga0_line4_l_en)
			{
				gCtrlVars.pga0_line4_l_gain = TmpData;
				AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN4_LEFT, 63 - gCtrlVars.pga0_line4_l_gain, 4);
			}
			#endif
			break;
	
		case 15://line5 Right gain
			#if 0
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 63? 63 : TmpData;
			if(gCtrlVars.pga0_line5_r_en)
			{
				gCtrlVars.pga0_line5_r_gain = TmpData;
				AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN5_RIGHT, 63 - gCtrlVars.pga0_line5_r_gain, 4);
			}
			if(gCtrlVars.pga0_line4_r_en)
			{
				gCtrlVars.pga0_line4_r_gain = TmpData;
				AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN4_RIGHT, 63 - gCtrlVars.pga0_line4_r_gain, 4);
			}
			#endif
			break;
	

	   case 16:///PGA0 different mode set 0=L,R, 1=L1+R1,2=L2+R2 3 = xx
		    memcpy(&TmpData, &buf[1], 2);
		    TmpData = TmpData > 3? 3 : TmpData;
		    gCtrlVars.pga0_diff_mode = TmpData;

            if(gCtrlVars.pga0_diff_mode == 1)///l diff input
		    {
		    	AudioADC_PGAMode(0, 1);
		    }
		    else if(gCtrlVars.pga0_diff_mode == 2)///r diff input
		    {
		    	AudioADC_PGAMode(1, 0);
		    }
		    else if(gCtrlVars.pga0_diff_mode == 3)///l+r diff input
		    {
		    	AudioADC_PGAMode(0, 0);
		    }	
			else
			{
			  AudioADC_PGAMode(1, 1);
			}
		    break;
	   case 17:///PGA0 different left gain  0=0db 1=6db 2=10db 3=15db
		    memcpy(&TmpData, &buf[1], 2);
		    TmpData = TmpData > 3? 3 : TmpData;
		    gCtrlVars.pga0_diff_l_gain = TmpData;
		    AudioADC_PGADiffGainSel((uint8_t)gCtrlVars.pga0_diff_l_gain, (uint8_t)gCtrlVars.pga0_diff_r_gain);
		    break;
	   case 18:///PGA0 different right gain    0=0db 1=6db 2=10db 3=15db
		    memcpy(&TmpData, &buf[1], 2);
		    TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.pga0_diff_r_gain = TmpData;
		    AudioADC_PGADiffGainSel((uint8_t)gCtrlVars.pga0_diff_l_gain, (uint8_t)gCtrlVars.pga0_diff_r_gain);
		    break;
		case 19:
			//memcpy(&TmpData, &buf[1], 2);
			//gCtrlVars.pga0_line1_pin = TmpData;
			break;
		case 20:
			//memcpy(&TmpData, &buf[1], 2);
			//gCtrlVars.pga0_line2_pin = TmpData;
			break;
		case 21:
			//memcpy(&TmpData, &buf[1], 2);
			//gCtrlVars.pga0_line4_pin = TmpData;
			break;
		case 22:
			//memcpy(&TmpData, &buf[1], 2);
			//gCtrlVars.pga0_line5_pin = TmpData;
			break;
		default:
			break;
	}
}
//-----------------------------------//
void Communication_Effect_0x03(uint8_t *buf, uint32_t len)////ADC0 PGA
{
	uint16_t i,k;
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]   = 0xa5;
		tx_buf[1]   = 0x5a;
		tx_buf[2]   = 0x03;//cmd
		tx_buf[3]   = 1+23*2;//1 + len * sizeof(int16)
		tx_buf[4]   = 0xff;///all paramgs,,，，，
		memcpy(&tx_buf[5],  &gCtrlVars.pga0_line1_l_en, 2);
		memcpy(&tx_buf[7],  &gCtrlVars.pga0_line1_r_en, 2);
		memcpy(&tx_buf[9],  &gCtrlVars.pga0_line2_l_en, 2);
		memcpy(&tx_buf[11], &gCtrlVars.pga0_line2_r_en, 2);
		memcpy(&tx_buf[13], &gCtrlVars.pga0_line4_l_en, 2);
		memcpy(&tx_buf[15], &gCtrlVars.pga0_line4_r_en, 2);
		memcpy(&tx_buf[17], &gCtrlVars.pga0_line5_l_en, 2);
		memcpy(&tx_buf[19], &gCtrlVars.pga0_line5_r_en, 2);	
		memcpy(&tx_buf[21], &gCtrlVars.pga0_line1_l_gain, 2);
		memcpy(&tx_buf[23], &gCtrlVars.pga0_line1_r_gain, 2);
		memcpy(&tx_buf[25], &gCtrlVars.pga0_line2_l_gain, 2);
		memcpy(&tx_buf[27], &gCtrlVars.pga0_line2_r_gain, 2);
		if(gCtrlVars.pga0_line4_l_en)
		{
			memcpy(&tx_buf[29], &gCtrlVars.pga0_line4_l_gain, 2);
		}
		else if(gCtrlVars.pga0_line5_l_en)
		{
			memcpy(&tx_buf[29], &gCtrlVars.pga0_line5_l_gain, 2);
		}
		else
		{
			memcpy(&tx_buf[29], &gCtrlVars.pga0_line4_l_gain, 2);
		}
		if(gCtrlVars.pga0_line4_r_en)
		{
			memcpy(&tx_buf[31], &gCtrlVars.pga0_line4_r_gain, 2);
		}
		else if(gCtrlVars.pga0_line5_r_en)
		{
			memcpy(&tx_buf[31], &gCtrlVars.pga0_line5_r_gain, 2);
		}
		else
		{
			memcpy(&tx_buf[31], &gCtrlVars.pga0_line4_r_gain, 2);
		}
		memcpy(&tx_buf[33], &gCtrlVars.pga0_line5_l_gain, 2);
		memcpy(&tx_buf[35], &gCtrlVars.pga0_line5_r_gain, 2);
		memcpy(&tx_buf[37], &gCtrlVars.pga0_diff_mode, 2);
		memcpy(&tx_buf[39], &gCtrlVars.pga0_diff_l_gain, 2);
		memcpy(&tx_buf[41], &gCtrlVars.pga0_diff_r_gain, 2);
		memcpy(&tx_buf[43], &gCtrlVars.pga0_line1_pin, 2);
		memcpy(&tx_buf[45], &gCtrlVars.pga0_line2_pin, 2);
		memcpy(&tx_buf[47], &gCtrlVars.pga0_line4_pin, 2);
		memcpy(&tx_buf[49], &gCtrlVars.pga0_line5_pin, 2);
		tx_buf[5 + 23*2]  = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 23*2);
	}
	else
	{
		switch(buf[0])///ADC0 PGA
		{
			case 0xff:
				buf++;
				for(i = 0; i < 19; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_PGA0_0x03(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_PGA0_0x03(buf);
				break;
		}
	}
}

void Comm_ADC0_0x04(uint8_t * buf)
{
	uint16_t TmpData;
	
	switch(buf[0])///adc0 digital channel en
	{
		case 0://ADC0 en?
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.adc0_channel_en = TmpData;
		
			if(gCtrlVars.adc0_channel_en == 0) 
			{
				AudioADC_LREnable(ADC0_MODULE, 0, 0);
			}
			else
			{
				AudioADC_LREnable(ADC0_MODULE, 1, 1);
			}
			break;
		
		case 1:///ADC0 mute select? 
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.adc0_mute = TmpData;
			if(gCtrlVars.adc0_mute == 0) 
			{
				//AudioADC_DigitalMute(ADC0_MODULE, 0, 0);	 
			}
			else if(gCtrlVars.adc0_mute == 1) 
			{
				//AudioADC_DigitalMute(ADC0_MODULE, 1, 0);	
			}
			else if(gCtrlVars.adc0_mute == 2) 
			{
				//AudioADC_DigitalMute(ADC0_MODULE, 0, 1);	
			}
			else if(gCtrlVars.adc0_mute == 3) 
			{
				//AudioADC_DigitalMute(ADC0_MODULE, 1, 1);	
			}
			break;
	
		case 2://adc0 dig vol left
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 0x3fff? 0x3fff : TmpData;
			gCtrlVars.adc0_dig_l_vol = TmpData;
			AudioADC_VolSetChannel(ADC0_MODULE, CHANNEL_LEFT,(uint16_t)gCtrlVars.adc0_dig_l_vol);
			break;
	
		case 3://adc0 dig vol right
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 0x3fff? 0x3fff : TmpData;
			gCtrlVars.adc0_dig_r_vol = TmpData;
			AudioADC_VolSetChannel(ADC0_MODULE, CHANNEL_RIGHT, (uint16_t)gCtrlVars.adc0_dig_r_vol);
			break;
	
		case 4://adc0 sample rate
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 8? 8 : TmpData;
			gCtrlVars.adc0_sample_rate = TmpData;
			GlobalSampeRateSet();
			AudioADC_SampleRateSet(ADC0_MODULE, SupportSampleRateList[gCtrlVars.adc0_sample_rate]);
			break;	
			
		case 5://adc0 LR swap
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.adc0_lr_swap = TmpData;
			if( gCtrlVars.adc0_lr_swap)
			{
				AudioADC_ChannelSwap(ADC0_MODULE, 1);
			}
			else
			{
				AudioADC_ChannelSwap(ADC0_MODULE, 0);
			}
			break;	
	
		 case 6://adc0 hight pass
			memcpy(&gCtrlVars.adc0_dc_blocker, &buf[1], 2);	
			TmpData = TmpData > 2? 2: TmpData;
			gCtrlVars.adc0_dc_blocker = TmpData;  
			AudioADC_HighPassFilterConfig(ADC0_MODULE, HPCList[gCtrlVars.adc0_dc_blocker]);
			break;	
			
		case 7://adc0 fade time
			memcpy(&TmpData, &buf[1], 2);	
			TmpData = TmpData > 255? 255 : TmpData;///default dis  0=dis
			gCtrlVars.adc0_fade_time = TmpData;
			if(gCtrlVars.adc0_fade_time==0)
			{
				AudioADC_FadeDisable(ADC0_MODULE);
			}
			else
			{
				AudioADC_FadeEnable(ADC0_MODULE);
				AudioADC_FadeTimeSet(ADC0_MODULE, (uint8_t)gCtrlVars.adc0_fade_time);
			}
			break;
		case 8://adc0 mclk src
			memcpy(&TmpData, &buf[1], 2);	
			TmpData = TmpData > 4? 4 : TmpData;
			gCtrlVars.adc0_mclk_src = TmpData;
			GlobalMclkSet();
			//-----------------------------------------//
			if((gCtrlVars.adc0_mclk_src == 3) ||(gCtrlVars.adc0_mclk_src == 4)) 
			{
				Clock_AudioMclkSel(AUDIO_ADC0, (MCLK_CLK_SEL)(gCtrlVars.adc0_mclk_src-1));
			}
			else
			{
				Clock_AudioMclkSel(AUDIO_ADC0, (MCLK_CLK_SEL)gCtrlVars.adc0_mclk_src);
			}
		    break;
		case 9://hpc0 en
			memcpy(&TmpData, &buf[1], 2);	
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.adc0_dc_blocker_en = TmpData;
			//-----------------------------------------//
			if(gCtrlVars.adc0_dc_blocker_en)
			{
			   AudioADC_HighPassFilterSet(ADC0_MODULE, 1);
			}
			else
			{
				AudioADC_HighPassFilterSet(ADC0_MODULE, 0);
			}
			break;			
		 default:
		   break;
		}
}
void Communication_Effect_0x04(uint8_t *buf, uint32_t len)////ADC0 DIGITAL
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x04;
		tx_buf[3]  = 1+10*2;//1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
		memcpy(&tx_buf[5], &gCtrlVars.adc0_channel_en, 2);
		memcpy(&tx_buf[7], &gCtrlVars.adc0_mute, 2);
		memcpy(&tx_buf[9], &gCtrlVars.adc0_dig_l_vol, 2);
		memcpy(&tx_buf[11], &gCtrlVars.adc0_dig_r_vol, 2);
		memcpy(&tx_buf[13], &gCtrlVars.adc0_sample_rate, 2);
		memcpy(&tx_buf[15], &gCtrlVars.adc0_lr_swap,2);
		memcpy(&tx_buf[17], &gCtrlVars.adc0_dc_blocker, 2);
		memcpy(&tx_buf[19] ,&gCtrlVars.adc0_fade_time, 2);
		memcpy(&tx_buf[21], &gCtrlVars.adc0_mclk_src, 2);
		memcpy(&tx_buf[23], &gCtrlVars.adc0_dc_blocker_en, 2);////adc0 hpc en
		tx_buf[25] = 0x16;
		Communication_Effect_Send(tx_buf, 26);///25+3*4+1
	}
	else
	{
		switch(buf[0])///ADC0 PGA
		{
			case 0xff:
				buf++;
				for(i = 0; i < 10; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_ADC0_0x04(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_ADC0_0x04(buf);
				break;
		}
	}
}

void Comm_PGA1_0x06(uint8_t * buf)
{
	uint16_t TmpData;

	switch(buf[0])//ADC1 PGA
	{
		case 0:///mic1 en?
		    memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
		    gCtrlVars.line3_l_mic1_en = TmpData;
			AudioLine3MicSelect();
			break;
	
		case 1:////mic2 en ?
		    memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
		    gCtrlVars.line3_r_mic2_en = TmpData;
			AudioLine3MicSelect();
			break;
	
		case 2://mic1 gain
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 31? 31 : TmpData;
			gCtrlVars.line3_l_mic1_gain = TmpData;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 31-gCtrlVars.line3_l_mic1_gain, MIC_BOOST_LIST[gCtrlVars.line3_l_mic1_boost]);
			break;
	
		case 3://mic1 gain boost
		    memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4? 4 : TmpData;		
			gCtrlVars.line3_l_mic1_boost = TmpData;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 31-gCtrlVars.line3_l_mic1_gain, MIC_BOOST_LIST[gCtrlVars.line3_l_mic1_boost]);
			break;		
		case 4://mic2 gain
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 31? 31 : TmpData;
			gCtrlVars.line3_r_mic2_gain = TmpData;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 31-gCtrlVars.line3_r_mic2_gain, MIC_BOOST_LIST[gCtrlVars.line3_r_mic2_boost]);
			break;
	
		case 5://mic2 gain boost
		    memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4? 4 : TmpData;		
			gCtrlVars.line3_r_mic2_boost = TmpData;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 31-gCtrlVars.line3_r_mic2_gain, MIC_BOOST_LIST[gCtrlVars.line3_r_mic2_boost]);
			break;
			
        case 6://mic/line3 display 0=mic1+mic2 1=line3l+line3r  2=mic1+line3r 3=mic2+line3l
            //memcpy(&TmpData, &buf[1], 2);
			//TmpData = TmpData > 3? 3 : TmpData;
			//gCtrlVars.mic_or_line3 = TmpData;
			break;
			
        case 7://0 = no mic1/line3l 1  = have mic1/line3l
            //memcpy(&TmpData, &buf[1], 2);
			//TmpData = TmpData > 1? 1 : TmpData;
			//gCtrlVars.mic1_line3l_pin_en = TmpData;
			break;
			
        case 8://0 = no mic2/line3r 1  = have mic2/line3r
            //memcpy(&TmpData, &buf[1], 2);
			//TmpData = TmpData > 1? 1 : TmpData;
			//gCtrlVars.mic2_line3r_pin_en = TmpData;
			break;
			
		default:
			break;
	}
}

void Communication_Effect_0x06(uint8_t *buf, uint32_t len)////ADC1 PGA
{
	uint16_t i,k;
	
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x06;//cmd
		tx_buf[3]  = 1 + 9*2;//1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;///all paramgs
		memcpy(&tx_buf[5], &gCtrlVars.line3_l_mic1_en, 2);
		memcpy(&tx_buf[7], &gCtrlVars.line3_r_mic2_en, 2);
		memcpy(&tx_buf[9], &gCtrlVars.line3_l_mic1_gain, 2);
		memcpy(&tx_buf[11], &gCtrlVars.line3_l_mic1_boost, 2);
		memcpy(&tx_buf[13], &gCtrlVars.line3_r_mic2_gain, 2);
		memcpy(&tx_buf[15], &gCtrlVars.line3_r_mic2_boost, 2);
		memcpy(&tx_buf[17], &gCtrlVars.mic_or_line3, 2);
		memcpy(&tx_buf[19], &gCtrlVars.mic1_line3l_pin_en, 2);
		memcpy(&tx_buf[21], &gCtrlVars.mic2_line3r_pin_en, 2);
		tx_buf[5 + 9*2]  = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 9*2);
	}
	else
	{
		switch(buf[0])///adc1 digital set
		{
			case 0xff:
				buf++;
				for(i = 0; i < 9; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_PGA1_0x06(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_PGA1_0x06(buf);
				break;
		}
	}
}

void Comm_ADC1_0x07(uint8_t * buf)
{
	uint16_t TmpData;
	
	switch(buf[0])///adc1 digital channel en
	{
		case 0://ADC1 en?
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.adc1_channel_en = TmpData;
		
			if(gCtrlVars.adc1_channel_en == 0) 
			{
				AudioADC_LREnable(ADC1_MODULE, 0, 0);
			}
			else// if(gCtrlVars.adc1_channel_en == 3)
			{
				AudioADC_LREnable(ADC1_MODULE, 1, 1);
			}
			break;
		
		case 1:///ADC1 mute select? 
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.adc1_mute = TmpData;
			if(gCtrlVars.adc1_mute == 0) 
			{
				//AudioADC_DigitalMute(ADC1_MODULE, 0, 0);	 
			}
			else if(gCtrlVars.adc1_mute == 1) 
			{
				//AudioADC_DigitalMute(ADC1_MODULE, 1, 0);	
			}
			else if(gCtrlVars.adc1_mute == 2) 
			{
				//AudioADC_DigitalMute(ADC1_MODULE, 0, 1);	
			}
			else if(gCtrlVars.adc1_mute == 3) 
			{
				//AudioADC_DigitalMute(ADC1_MODULE, 1, 1);	
			}
			break;
	
		case 2://adc1 dig vol left
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 0x3fff? 0x3fff : TmpData;
			gCtrlVars.adc1_dig_l_vol = TmpData;
			AudioADC_VolSetChannel(ADC1_MODULE, CHANNEL_LEFT,(uint16_t)gCtrlVars.adc1_dig_l_vol);
			break;
	
		case 3://adc1 dig vol right
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 0x3fff? 0x3fff : TmpData;
			gCtrlVars.adc1_dig_r_vol = TmpData;
			AudioADC_VolSetChannel(ADC1_MODULE, CHANNEL_RIGHT, (uint16_t)gCtrlVars.adc1_dig_r_vol);
			break;
	
		case 4://adc1 sample rate
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 8? 8 : TmpData;
			gCtrlVars.adc1_sample_rate = TmpData;
			GlobalSampeRateSet();
			AudioADC_SampleRateSet(ADC1_MODULE, SupportSampleRateList[gCtrlVars.adc1_sample_rate]);
			break;	
			
		case 5://adc1 LR swap
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.adc1_lr_swap = TmpData;

			if( gCtrlVars.adc1_lr_swap)
			{
				AudioADC_ChannelSwap(ADC1_MODULE, 1);
			}
			else
			{
				AudioADC_ChannelSwap(ADC1_MODULE, 0);
			}
			break;	
	
		 case 6://adc1 hight pass
			memcpy(&gCtrlVars.adc0_dc_blocker, &buf[1], 2);	
			TmpData = TmpData > 2? 2: TmpData;
			gCtrlVars.adc1_dc_blocker = TmpData;  
			AudioADC_HighPassFilterConfig(ADC1_MODULE, HPCList[gCtrlVars.adc1_dc_blocker]);
			break;	
			
		case 7://adc1 fade time
			memcpy(&TmpData, &buf[1], 2);	
			TmpData = TmpData > 255? 255 : TmpData;///default dis  0=dis
			gCtrlVars.adc1_fade_time = TmpData;
			if(gCtrlVars.adc1_fade_time==0)
			{
				AudioADC_FadeDisable(ADC1_MODULE);
			}
			else
			{
				AudioADC_FadeEnable(ADC1_MODULE);
				AudioADC_FadeTimeSet(ADC1_MODULE, (uint8_t)gCtrlVars.adc1_fade_time);
			}
			break;
		case 8://adc1 mclk src
			memcpy(&TmpData, &buf[1], 2);	
			TmpData = TmpData > 4? 4 : TmpData;
			gCtrlVars.adc1_mclk_src = TmpData;
			GlobalMclkSet();
			if((gCtrlVars.adc1_mclk_src == 3) ||(gCtrlVars.adc1_mclk_src == 4)) 
			{
				Clock_AudioMclkSel(AUDIO_ADC1, (MCLK_CLK_SEL)(gCtrlVars.adc1_mclk_src-1));
			}
			else
			{
				Clock_AudioMclkSel(AUDIO_ADC1, (MCLK_CLK_SEL)gCtrlVars.adc1_mclk_src);
			}
		    break;
		case 9://hpc1 en
			memcpy(&TmpData, &buf[1], 2);	
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.adc1_dc_blocker_en = TmpData;
			if(gCtrlVars.adc1_dc_blocker_en)
			{
			   AudioADC_HighPassFilterSet(ADC1_MODULE, 1);
			}
			else
			{
				AudioADC_HighPassFilterSet(ADC1_MODULE, 0);
			}
			break;			
		 default:
		   break;
		}
}

void Communication_Effect_0x07(uint8_t *buf, uint32_t len)///ADC1 DIGITAL
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x07;
		tx_buf[3]  = 1+10*2;//1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
		memcpy(&tx_buf[5], &gCtrlVars.adc1_channel_en, 2);
		memcpy(&tx_buf[7], &gCtrlVars.adc1_mute, 2);
		memcpy(&tx_buf[9], &gCtrlVars.adc1_dig_l_vol, 2);
		memcpy(&tx_buf[11], &gCtrlVars.adc1_dig_r_vol, 2);
		memcpy(&tx_buf[13], &gCtrlVars.adc1_sample_rate, 2);
		memcpy(&tx_buf[15],  &gCtrlVars.adc1_lr_swap,2);
		memcpy(&tx_buf[17], &gCtrlVars.adc1_dc_blocker, 2);
		memcpy(&tx_buf[19], &gCtrlVars.adc1_fade_time,2);
		memcpy(&tx_buf[21], &gCtrlVars.adc1_mclk_src, 2);
		memcpy(&tx_buf[23], &gCtrlVars.adc1_dc_blocker_en, 2);////adc0 hpc en
		tx_buf[25] = 0x16;
		Communication_Effect_Send(tx_buf, 26);///25+3*4+1
	}
	else
	{
		switch(buf[0])///ADC0 PGA
		{
			case 0xff:
				buf++;
				for(i = 0; i < 10; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_ADC1_0x07(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_ADC1_0x07(buf);
				break;
		}
	}
}

void Comm_AGC1_0x08(uint8_t * buf)
{
	uint16_t TmpData;
	switch(buf[0])//ADC1 AGC
	{
		case 0://AGC {buf[1]=0 dis} {buf[1]=1 left en} {buf[1]=2 right en} {buf[1]=3 left+right en}
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.adc1_agc_mode = TmpData;
			if(gCtrlVars.adc1_agc_mode == 0)
			{
				AudioADC_AGCChannelSel(ADC1_MODULE,0,0);
			}
			if(gCtrlVars.adc1_agc_mode == 1)
			{
				AudioADC_AGCChannelSel(ADC1_MODULE,1,0);
			}
			if(gCtrlVars.adc1_agc_mode == 2)
			{
				AudioADC_AGCChannelSel(ADC1_MODULE,0,1);
			}
			if(gCtrlVars.adc1_agc_mode == 3)
			{
				AudioADC_AGCChannelSel(ADC1_MODULE,1,1);
			}
			break;
			
		case 1://MAX level 
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 31? 31 : TmpData;
			gCtrlVars.adc1_agc_max_level = TmpData;
			AudioADC_AGCMaxLevel(ADC1_MODULE,(uint8_t)gCtrlVars.adc1_agc_max_level);
			break;
		case 2://target level
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 255? 255 : TmpData;
			gCtrlVars.adc1_agc_target_level = TmpData;
			AudioADC_AGCTargetLevel(ADC1_MODULE,(uint8_t)gCtrlVars.adc1_agc_target_level);
			break;
		case 3://max gain
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 63? 63 : TmpData;
			gCtrlVars.adc1_agc_max_gain = TmpData;
			TmpData = 63 - gCtrlVars.adc1_agc_max_gain;
			AudioADC_AGCMaxGain(ADC1_MODULE,(uint8_t)TmpData);
			break;
		case 4://min gain
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 63? 63 : TmpData;
			gCtrlVars.adc1_agc_min_gain = TmpData;
			TmpData = 63 - gCtrlVars.adc1_agc_min_gain;
			AudioADC_AGCMinGain(ADC1_MODULE,(uint8_t)TmpData);
			break;
		case 5://gain offset
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 15? 15 : TmpData;
			gCtrlVars.adc0_agc_gainoffset = TmpData;
			AudioADC_AGCGainOffset(ADC1_MODULE,(uint8_t)gCtrlVars.adc1_agc_gainoffset);
			break;
		case 6://fram time
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4096? 4096 : TmpData;
			gCtrlVars.adc1_agc_fram_time = TmpData;
			AudioADC_AGCFrameTime(ADC1_MODULE,(uint16_t)gCtrlVars.adc1_agc_fram_time);
			break;
		case 7://hold time
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4096? 4096 : TmpData;
			gCtrlVars.adc1_agc_hold_time = TmpData;
			AudioADC_AGCHoldTime(ADC1_MODULE,(uint32_t)gCtrlVars.adc1_agc_hold_time);
			break;
		case 8://attack time
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4096? 4096 : TmpData;
			gCtrlVars.adc1_agc_attack_time = TmpData;
			AudioADC_AGCAttackStepTime(ADC1_MODULE,(uint16_t)gCtrlVars.adc1_agc_attack_time);
			break;
		case 9://dacay time
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4096? 4096 : TmpData;
			gCtrlVars.adc1_agc_decay_time = TmpData;
			AudioADC_AGCDecayStepTime(ADC1_MODULE,(uint16_t)gCtrlVars.adc1_agc_decay_time);
			break;
		case 10://nosie gain en
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.adc1_agc_noise_gate_en = TmpData;
			AudioADC_AGCNoiseGateEnable(ADC1_MODULE,(bool)gCtrlVars.adc1_agc_noise_gate_en);
			break;
		case 11://nosie thershold
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 31? 31 : TmpData;
			gCtrlVars.adc1_agc_noise_threshold = TmpData;
			AudioADC_AGCNoiseThreshold(ADC1_MODULE,(uint8_t)gCtrlVars.adc1_agc_noise_threshold);
			break;
		case 12://nosie gate mode
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.adc1_agc_noise_gate_mode = TmpData;	
			AudioADC_AGCNoiseGateMode(ADC1_MODULE,(uint8_t)gCtrlVars.adc1_agc_noise_gate_mode);
			break;
		case 13://nosie gate hold time
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4096? 4096 : TmpData;
			gCtrlVars.adc1_agc_noise_time = TmpData;
			AudioADC_AGCNoiseHoldTime(ADC1_MODULE,(uint8_t)gCtrlVars.adc1_agc_noise_time);
			break;
		default:
			break;
	}
}

void Communication_Effect_0x08(uint8_t *buf, uint32_t len)////ADC1 AGC
{
	uint16_t i,k;
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x08;
		tx_buf[3]  = 1 + 14*2;//1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
        memcpy(&tx_buf[5], &gCtrlVars.adc1_agc_mode, 2);
		memcpy(&tx_buf[7], &gCtrlVars.adc1_agc_max_level, 2);
		memcpy(&tx_buf[9], &gCtrlVars.adc1_agc_target_level, 2);
		memcpy(&tx_buf[11], &gCtrlVars.adc1_agc_max_gain, 2);
		memcpy(&tx_buf[13], &gCtrlVars.adc1_agc_min_gain, 2);
		memcpy(&tx_buf[15], &gCtrlVars.adc1_agc_gainoffset, 2);
		memcpy(&tx_buf[17], &gCtrlVars.adc1_agc_fram_time, 2);
		memcpy(&tx_buf[19], &gCtrlVars.adc1_agc_hold_time, 2);
		memcpy(&tx_buf[21], &gCtrlVars.adc1_agc_attack_time, 2);
		memcpy(&tx_buf[23], &gCtrlVars.adc1_agc_decay_time, 2);
		memcpy(&tx_buf[25], &gCtrlVars.adc1_agc_noise_gate_en, 2);
		memcpy(&tx_buf[27], &gCtrlVars.adc1_agc_noise_threshold, 2);
		memcpy(&tx_buf[29], &gCtrlVars.adc1_agc_noise_gate_mode, 2);
		memcpy(&tx_buf[31], &gCtrlVars.adc1_agc_noise_time, 2);
		tx_buf[33] = 0x16;
		Communication_Effect_Send(tx_buf, 34);
	}
	else
	{
		switch(buf[0])///ADC1 AGC
		{
			case 0xff:
				buf++;
				for(i = 0; i < 14; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_AGC1_0x08(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_AGC1_0x08(buf);
				break;
		}
	}
}

void Comm_DAC0_0x09(uint8_t * buf)
{
    uint16_t TmpData;
	
	switch(buf[0])////DAC0 set
	{
		case 0://DAC0 en
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.dac0_en = 3;//TmpData;
			if(gCtrlVars.dac0_en)
			{
				//AudioDAC_ChannelEnable(DAC0);
			}
			else
			{
				//AudioDAC_ChannelDisable(DAC0);
			}
			break;	
        case 1://dac0 sample rate 0~8
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 8? 8 : TmpData;
			gCtrlVars.dac0_sample_rate = TmpData;
			GlobalSampeRateSet();
			AudioDAC_SampleRateSet(DAC0,SupportSampleRateList[gCtrlVars.dac0_sample_rate]);
			break;	
        case 2:///dac0 mute
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
		    gCtrlVars.dac0_dig_mute = TmpData;
			if(gCtrlVars.dac0_dig_mute==0)
			{
			  //AudioDAC_DigitalMute(DAC0, 0, 0);
			}
			else if(gCtrlVars.dac0_dig_mute==1)
			{
			  //AudioDAC_DigitalMute(DAC0, 1, 0);
			}
			else if(gCtrlVars.dac0_dig_mute==2)
			{
			  //AudioDAC_DigitalMute(DAC0, 0, 1);
			}
			else if(gCtrlVars.dac0_dig_mute==3)
			{	 
			  //AudioDAC_DigitalMute(DAC0, 1, 1);
			}				
			break;					
		case 3:////dac0 L volume
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 0x3fff? 0x3fff : TmpData;
			gCtrlVars.dac0_dig_l_vol = TmpData;
			AudioDAC_VolSet(DAC0, gCtrlVars.dac0_dig_l_vol, gCtrlVars.dac0_dig_r_vol);
			break;

		case 4:////dac0 R volume
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 0x3fff? 0x3fff : TmpData;
			gCtrlVars.dac0_dig_r_vol = TmpData;
			AudioDAC_VolSet(DAC0, gCtrlVars.dac0_dig_l_vol, gCtrlVars.dac0_dig_r_vol);
			break;
        case 5:///DAC0 dither
            memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
            gCtrlVars.dac0_dither = TmpData;
			if(gCtrlVars.dac0_dither) AudioDAC_DitherEnable(DAC0);
			else                      AudioDAC_DitherDisable(DAC0);              
            break;

		case 6:///dac0 scramble
            memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4? 4 : TmpData;
            gCtrlVars.dac0_scramble = TmpData;
			if(gCtrlVars.dac0_scramble == 0)
			{
				AudioDAC_ScrambleDisable(DAC0);
			}
			else
			{
				AudioDAC_ScrambleEnable(DAC0);
				AudioDAC_ScrambleModeSet(DAC0,(SCRAMBLE_MODULE)gCtrlVars.dac0_scramble);
			}
            break;
			
        case 7:///dac0 stere mode
            memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
            gCtrlVars.dac0_out_mode = TmpData;
			AudioDAC_DoutModeSet(DAC0, (DOUT_MODE)gCtrlVars.dac0_out_mode, (DOUT_WIDTH)gCtrlVars.dac0_out_bit_len);//
            break; 
		case 8:///dac0 pause
		    memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
            gCtrlVars.dac0_pause_en = TmpData;
			if(gCtrlVars.dac0_pause_en) AudioDAC_Pause(DAC0);
			else                        AudioDAC_Run(DAC0);
            break;
		case 9:///dac0 sample mode
		    memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
            gCtrlVars.dac0_sample_mode = TmpData;
			AudioDAC_EdgeSet(DAC0,(EDGE)gCtrlVars.dac0_sample_mode);
            break; 
		case 10:///dac0 scf mute
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.dac0_scf_mute = TmpData;
			if(gCtrlVars.dac0_scf_mute==0)
			{
				AudioDAC_SCFMute(DAC0,0,0);
			}
			else if(gCtrlVars.dac0_scf_mute==1)
			{
				AudioDAC_SCFMute(DAC0,1,0);
			}
			else if(gCtrlVars.dac0_scf_mute==2)
			{
				AudioDAC_SCFMute(DAC0,0,1);
			}
			else if(gCtrlVars.dac0_scf_mute==3)
			{
				AudioDAC_SCFMute(DAC0,1,1);
			}				
            break; 
			
         case 11:///dac0 fade time
            memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 255? 255 : TmpData;
            gCtrlVars.dac0_fade_time = TmpData;
			if(gCtrlVars.dac0_fade_time==0)
			{
				//AudioDAC_FadeDisable(DAC0);
			}
			else
			{
				AudioDAC_FadeEnable(DAC0);
				AudioDAC_FadeTimeSet(DAC0,(uint8_t)gCtrlVars.dac0_fade_time);
			}
            break; 	
			
		 case 12:///dac0 zero num
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 7? 7 : TmpData;
            gCtrlVars.dac0_zeros_number = TmpData;
			AudioDAC_ZeroNumSet(DAC0,(uint8_t)gCtrlVars.dac0_zeros_number);
            break;	
         case 13:///dac0 mclk src
			memcpy(&TmpData, &buf[1], 2);	
		    TmpData = TmpData > 4? 4 : TmpData;
			gCtrlVars.dac0_mclk_src = TmpData;
			GlobalMclkSet();
			if((gCtrlVars.dac0_mclk_src == 3) ||(gCtrlVars.dac0_mclk_src == 4)) 
			{
				Clock_AudioMclkSel(AUDIO_DAC0, (MCLK_CLK_SEL)(gCtrlVars.dac0_mclk_src-1));
			}
			else
			{
				Clock_AudioMclkSel(AUDIO_DAC0, (MCLK_CLK_SEL)gCtrlVars.dac0_mclk_src);
			}
            break;				
		default:
			break;
	}
}

void Communication_Effect_0x09(uint8_t *buf, uint32_t len)///DAC0
{
	uint16_t i,k;
	
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x09;
		tx_buf[3]  = 1 + 14*2;//1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
 	    memcpy(&tx_buf[5], &gCtrlVars.dac0_en, 2);
		memcpy(&tx_buf[7], &gCtrlVars.dac0_sample_rate, 2);
        memcpy(&tx_buf[9], &gCtrlVars.dac0_dig_mute, 2);
		memcpy(&tx_buf[11], &gCtrlVars.dac0_dig_l_vol, 2);
		memcpy(&tx_buf[13], &gCtrlVars.dac0_dig_r_vol, 2);
		memcpy(&tx_buf[15], &gCtrlVars.dac0_dither, 2);
		memcpy(&tx_buf[17], &gCtrlVars.dac0_scramble, 2);
		memcpy(&tx_buf[19], &gCtrlVars.dac0_out_mode, 2);
		memcpy(&tx_buf[21], &gCtrlVars.dac0_pause_en, 2);
		memcpy(&tx_buf[23], &gCtrlVars.dac0_sample_mode, 2);
		memcpy(&tx_buf[25], &gCtrlVars.dac0_scf_mute, 2);
        memcpy(&tx_buf[27], &gCtrlVars.dac0_fade_time, 2);
		memcpy(&tx_buf[29], &gCtrlVars.dac0_zeros_number, 2);
		memcpy(&tx_buf[31], &gCtrlVars.dac0_mclk_src, 2);
		tx_buf[33] = 0x16;
		Communication_Effect_Send(tx_buf,34);
	}
	else
	{
		switch(buf[0])///dac0
		{
			case 0xff:
				buf++;
				for(i = 0; i < 14; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_DAC0_0x09(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_DAC0_0x09(buf);
				break;
		}
	}
}

void Comm_DAC1_0x0A(uint8_t * buf)
{
     uint16_t TmpData;
	
	switch(buf[0])////DAC1 set
	{
		case 0://DAC1 en
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.dac1_en = 1;//TmpData;
			if(gCtrlVars.dac1_en)
			{
				//AudioDAC_ChannelEnable(DAC1);
			}
			else
			{
				//AudioDAC_ChannelDisable(DAC1);
			}
			break;	
        case 1://dac1 sample rate 0~8
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 8? 8 : TmpData;
			gCtrlVars.dac1_sample_rate = TmpData;
			GlobalSampeRateSet();
			AudioDAC_SampleRateSet(DAC1,SupportSampleRateList[gCtrlVars.dac1_sample_rate]);
			break;	
        case 2:///dac1 mute
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
		    gCtrlVars.dac1_dig_mute = TmpData;
			if(gCtrlVars.dac1_dig_mute==0)
			{
				//AudioDAC_DigitalMute(DAC1, 0, 0);
			}
			else if(gCtrlVars.dac1_dig_mute==1)
			{
				//AudioDAC_DigitalMute(DAC1, 1, 0);
			}
			else if(gCtrlVars.dac1_dig_mute==2)
			{
				//AudioDAC_DigitalMute(DAC1, 0, 1);
			}
			else if(gCtrlVars.dac1_dig_mute==3)
			{	 
				//AudioDAC_DigitalMute(DAC1, 1, 1);
			}				
			break;					
		case 3:////dac1 L volume
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 0x3fff? 0x3fff : TmpData;
			gCtrlVars.dac1_dig_l_vol = TmpData;
			AudioDAC_VolSet(DAC1, gCtrlVars.dac1_dig_l_vol, gCtrlVars.dac1_dig_r_vol);
			break;

		case 4:////dac1 R volume
			break;
        case 5:///DAC1 dither
            memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
            gCtrlVars.dac1_dither = TmpData;
			if(gCtrlVars.dac0_dither) AudioDAC_DitherEnable(DAC1);
			else                      AudioDAC_DitherDisable(DAC1);              
            break;

		case 6:///dac1 scramble
            memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4? 4 : TmpData;
            gCtrlVars.dac1_scramble = TmpData;
			if(gCtrlVars.dac1_scramble == 0)
			{
				AudioDAC_ScrambleDisable(DAC1);
			}
			else
			{
				AudioDAC_ScrambleEnable(DAC1);
				AudioDAC_ScrambleModeSet(DAC1,(SCRAMBLE_MODULE)gCtrlVars.dac1_scramble);
			}
            break;
			
        case 7:///dac1 stere mode
            memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
            gCtrlVars.dac1_out_mode = TmpData;
			AudioDAC_DoutModeSet(DAC1, (DOUT_MODE)gCtrlVars.dac1_out_mode, (DOUT_WIDTH)gCtrlVars.dac1_out_bit_len);//
            break; 
		case 8:///dac1 pause
		    memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
            gCtrlVars.dac1_pause_en = TmpData;
			if(gCtrlVars.dac1_pause_en) AudioDAC_Pause(DAC1);
			else                        AudioDAC_Run(DAC1);
            break;
		case 9:///dac1 sample mode
		    memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
            gCtrlVars.dac1_sample_mode = TmpData;
			AudioDAC_EdgeSet(DAC1,(EDGE)gCtrlVars.dac1_sample_mode);
            break; 
		case 10:///dac1 scf mute
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.dac1_scf_mute = TmpData;
			if(gCtrlVars.dac1_scf_mute==0)
			{
				AudioDAC_SCFMute(DAC1,0,0);
			}
			else if(gCtrlVars.dac1_scf_mute==1)
			{
				AudioDAC_SCFMute(DAC1,1,0);
			}
			else if(gCtrlVars.dac1_scf_mute==2)
			{
				AudioDAC_SCFMute(DAC1,0,1);
			}
			else if(gCtrlVars.dac1_scf_mute==3)
			{
				AudioDAC_SCFMute(DAC1,1,1);
			}				
            break; 
			
         case 11:///dac1 fade time
            memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 255? 255 : TmpData;
            gCtrlVars.dac1_fade_time = TmpData;
			if(gCtrlVars.dac1_fade_time==0)
			{
				//AudioDAC_FadeDisable(DAC1);
			}
			else
			{
				AudioDAC_FadeEnable(DAC1);
				AudioDAC_FadeTimeSet(DAC1,(uint8_t)gCtrlVars.dac1_fade_time);
			}
            break; 	
			
		 case 12:///dac1 zero num
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 7? 7 : TmpData;
            gCtrlVars.dac1_zeros_number = TmpData;
			AudioDAC_ZeroNumSet(DAC1,(uint8_t)gCtrlVars.dac1_zeros_number);
            break;	
         case 13:///dac1 mclk src
			memcpy(&TmpData, &buf[1], 2);	
		    TmpData = TmpData > 4? 4 : TmpData;
			gCtrlVars.dac1_mclk_src = TmpData;
			GlobalMclkSet();
			if((gCtrlVars.dac1_mclk_src == 3) ||(gCtrlVars.dac1_mclk_src == 4)) 
			{
				Clock_AudioMclkSel(AUDIO_DAC1, (MCLK_CLK_SEL)(gCtrlVars.dac1_mclk_src-1));
			}
			else
			{
				Clock_AudioMclkSel(AUDIO_DAC1, (MCLK_CLK_SEL)gCtrlVars.dac1_mclk_src);
			}
            break;				
		default:
			break;
	}
}

void Communication_Effect_0x0A(uint8_t *buf, uint32_t len)//DACX
{
	uint16_t i,k;
	
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x0a;
		tx_buf[3]  = 1 + 14*2;//1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
 	    memcpy(&tx_buf[5], &gCtrlVars.dac1_en, 2);
		memcpy(&tx_buf[7], &gCtrlVars.dac1_sample_rate, 2);
        memcpy(&tx_buf[9], &gCtrlVars.dac1_dig_mute, 2);
		memcpy(&tx_buf[11], &gCtrlVars.dac1_dig_l_vol, 2);
		memcpy(&tx_buf[13], &gCtrlVars.dac1_dig_r_vol, 2);
		memcpy(&tx_buf[15], &gCtrlVars.dac1_dither, 2);
		memcpy(&tx_buf[17], &gCtrlVars.dac1_scramble, 2);
		memcpy(&tx_buf[19], &gCtrlVars.dac1_out_mode, 2);
		memcpy(&tx_buf[21], &gCtrlVars.dac1_pause_en, 2);
		memcpy(&tx_buf[23], &gCtrlVars.dac1_sample_mode, 2);
		memcpy(&tx_buf[25], &gCtrlVars.dac1_scf_mute, 2);
        memcpy(&tx_buf[27], &gCtrlVars.dac1_fade_time, 2);
		memcpy(&tx_buf[29], &gCtrlVars.dac1_zeros_number, 2);
		memcpy(&tx_buf[31], &gCtrlVars.dac1_mclk_src, 2);
		tx_buf[33] = 0x16;
		Communication_Effect_Send(tx_buf,34);
	}
	else
	{
		switch(buf[0])///dac0
		{
			case 0xff:
				buf++;
				for(i = 0; i < 14; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_DAC1_0x0A(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_DAC1_0x0A(buf);
				break;
		}
	}
}

void Comm_I2S0_0x0B(uint8_t * buf)
{
    uint16_t TmpData;

	switch(buf[0])////i2s0  set
	{
		case 0://I2S0 TX EN
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.i2s0_tx_en = TmpData;
		    #if CFG_I2S0_OUT_EN
		    if(gCtrlVars.i2s0_tx_en)
		    {
		    	I2S_ModuleTxEnable(I2S0_MODULE);
		    }
			else
			{
				//I2S_ModuleTxDisable(I2S0_MODULE);
			}
			#endif
			break;	
			
        case 1://I2S0 RX EN
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.i2s0_rx_en = TmpData;
		    #if CFG_I2S0_IN_EN
		    if(gCtrlVars.i2s0_rx_en)
		    {
		    	I2S_ModuleRxEnable(I2S0_MODULE);
		    }
			else
			{
				I2S_ModuleRxDisable(I2S0_MODULE);
			}
			#endif
			break;	
			
        case 2:///I2S0 sample rate
            memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 12? 12 : TmpData;
			gCtrlVars.i2s0_sample_rate = TmpData;
			GlobalSampeRateSet();
			#if CFG_I2S0_OUT_EN || CFG_I2S0_IN_EN
			I2S_SampleRateSet(I2S0_MODULE, SupportSampleRateList[gCtrlVars.i2s0_sample_rate]);
			#endif
			break;	
			
		case 3://I2S0 mclk src
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4? 4 : TmpData;
			gCtrlVars.i2s0_mclk_src = TmpData;
			GlobalMclkSet();
			if((gCtrlVars.i2s0_mclk_src == 3) ||(gCtrlVars.i2s0_mclk_src == 4)) 
			{
				#if CFG_I2S0_OUT_EN || CFG_I2S0_IN_EN
				Clock_AudioMclkSel(AUDIO_I2S0, (MCLK_CLK_SEL)(gCtrlVars.i2s0_mclk_src-1));
				#endif
			}
			else
			{
				#if CFG_I2S0_OUT_EN || CFG_I2S0_IN_EN
				Clock_AudioMclkSel(AUDIO_I2S0, (MCLK_CLK_SEL)gCtrlVars.i2s0_mclk_src);
				#endif
			}			
			break;

		case 4:///I2S0 master slave
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
		    gCtrlVars.i2s0_work_mode = TmpData;
		    #if CFG_I2S0_OUT_EN || CFG_I2S0_IN_EN
			if(gCtrlVars.i2s0_work_mode)
			{
				I2S_SlaveModeSet(I2S0_MODULE, (I2S_DATA_FORMAT)gCtrlVars.i2s0_format, (I2S_DATA_LENGTH)gCtrlVars.i2s0_word_len);
			}
			else
			{			
				I2S_MasterModeSet(I2S0_MODULE,(I2S_DATA_FORMAT)gCtrlVars.i2s0_format, (I2S_DATA_LENGTH)gCtrlVars.i2s0_word_len);
			}
			#endif
			break;
			
        case 5:///I2S0 word lenght
            memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
		    gCtrlVars.i2s0_word_len = TmpData;
		    #if CFG_I2S0_OUT_EN || CFG_I2S0_IN_EN
			if(gCtrlVars.i2s0_work_mode)
			{
				I2S_SlaveModeSet(I2S0_MODULE, (I2S_DATA_FORMAT)gCtrlVars.i2s0_format, (I2S_DATA_LENGTH)gCtrlVars.i2s0_word_len);
			}
			else
			{			
				I2S_MasterModeSet(I2S0_MODULE,(I2S_DATA_FORMAT)gCtrlVars.i2s0_format, (I2S_DATA_LENGTH)gCtrlVars.i2s0_word_len);
			}
			#endif
            break;

		case 6:///I2S0 stereo mono
		    memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
		    gCtrlVars.i2s0_stereo_mono = TmpData;
			#if CFG_I2S0_OUT_EN || CFG_I2S0_IN_EN
		    I2S_MonoModeSet(I2S0_MODULE,(bool)gCtrlVars.i2s0_stereo_mono);
			#endif
            break;
			
        case 7:///I2S0 fade time
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 255? 255 : TmpData;
			gCtrlVars.i2s0_fade_time = TmpData;
			#if CFG_I2S0_OUT_EN || CFG_I2S0_IN_EN
			if(gCtrlVars.i2s0_fade_time==0)
			{
				I2S_FadeDisable(I2S0_MODULE);
			}
			else
			{
				I2S_FadeEnable(I2S0_MODULE);
				I2S_FadeTimeSet(I2S0_MODULE,(uint32_t)gCtrlVars.i2s0_fade_time);
			}
			#endif
			break; 
			
		case 8:///i2s0 format
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.i2s0_format = TmpData;
			#if CFG_I2S0_OUT_EN || CFG_I2S0_IN_EN
			if(gCtrlVars.i2s0_work_mode)
			{
				I2S_SlaveModeSet(I2S0_MODULE, (I2S_DATA_FORMAT)gCtrlVars.i2s0_format, (I2S_DATA_LENGTH)gCtrlVars.i2s0_word_len);
			}
			else
			{			
				I2S_MasterModeSet(I2S0_MODULE,(I2S_DATA_FORMAT)gCtrlVars.i2s0_format, (I2S_DATA_LENGTH)gCtrlVars.i2s0_word_len);
			}
			#endif
			break;
			
         case 9:///i2s0 bclk invert
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.i2s0_bclk_invert_en = TmpData;
			#if CFG_I2S0_OUT_EN || CFG_I2S0_IN_EN
			I2S_BclkInvertSet(I2S0_MODULE,(bool)gCtrlVars.i2s0_bclk_invert_en);
			#endif
			break;
			
         case 10:///i2s0 bclk invert
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 1? 1 : TmpData;
			gCtrlVars.i2s0_lrclk_invert_en = TmpData;
			#if CFG_I2S0_OUT_EN || CFG_I2S0_IN_EN
			I2S_LrclkInvertSet(I2S0_MODULE,(bool)gCtrlVars.i2s0_lrclk_invert_en);
			#endif
			break;
			
		default:
			break;
	}
}

void Communication_Effect_0x0B(uint8_t *buf, uint32_t len)////I2S0
{
	uint16_t i,k;
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x0b;
		tx_buf[3]  = 1 + 11*2;//1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
		memcpy(&tx_buf[5], &gCtrlVars.i2s0_tx_en, 2);
		memcpy(&tx_buf[7], &gCtrlVars.i2s0_rx_en, 2);
		memcpy(&tx_buf[9], &gCtrlVars.i2s0_sample_rate, 2);
		memcpy(&tx_buf[11], &gCtrlVars.i2s0_mclk_src, 2);
		memcpy(&tx_buf[13], &gCtrlVars.i2s0_work_mode, 2);
		memcpy(&tx_buf[15], &gCtrlVars.i2s0_word_len, 2);
		memcpy(&tx_buf[17], &gCtrlVars.i2s0_stereo_mono, 2);
		memcpy(&tx_buf[19], &gCtrlVars.i2s0_fade_time, 2);
		memcpy(&tx_buf[21], &gCtrlVars.i2s0_format, 2);
		memcpy(&tx_buf[23], &gCtrlVars.i2s0_bclk_invert_en, 2);
		memcpy(&tx_buf[25], &gCtrlVars.i2s0_lrclk_invert_en, 2);
		tx_buf[27] = 0x16;
		Communication_Effect_Send(tx_buf,28);
	}
	else
	{
		switch(buf[0])///dac1
		{
			case 0xff:
				buf++;
				for(i = 0; i < 11; i++)
				{
					cbuf[0] = i;////id

					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}

					Comm_I2S0_0x0B(&cbuf[0]);

					buf += 2;
				}
				break;
			default:
				Comm_I2S0_0x0B(buf);
				break;
		}
	}
}

void Comm_I2S1_0x0C(uint8_t * buf)
{
	uint16_t TmpData;
	
	 switch(buf[0])////i2s1  set
	 {
		 case 0://I2S1 TX EN
			 memcpy(&TmpData, &buf[1], 2);
			 TmpData = TmpData > 1? 1 : TmpData;
			 gCtrlVars.i2s1_tx_en = TmpData;
		     #if CFG_I2S1_OUT_EN
			 if(gCtrlVars.i2s0_tx_en)
			 {
				 I2S_ModuleTxEnable(I2S1_MODULE);
			 }
			 else
			 {
				 //I2S_ModuleTxDisable(I2S1_MODULE);
			 }
		     #endif
			 break;  
			 
		 case 1://I2S0 RX EN
			 memcpy(&TmpData, &buf[1], 2);
			 TmpData = TmpData > 1? 1 : TmpData;
			 gCtrlVars.i2s1_rx_en = TmpData;
		     #if CFG_I2S1_IN_EN
			 if(gCtrlVars.i2s1_rx_en)
			 {
				 I2S_ModuleRxEnable(I2S1_MODULE);
			 }
			 else
			 {
				 I2S_ModuleRxDisable(I2S1_MODULE);
			 }
		     #endif
			 break;  
			 
		 case 2:///I2S1 sample rate
			 memcpy(&TmpData, &buf[1], 2);
			 TmpData = TmpData > 12? 12 : TmpData;
			 gCtrlVars.i2s1_sample_rate = TmpData;
			 GlobalSampeRateSet();
		     #if CFG_I2S1_OUT_EN || CFG_I2S1_IN_EN
			 I2S_SampleRateSet(I2S1_MODULE, SupportSampleRateList[gCtrlVars.i2s1_sample_rate]);
		     #endif
			 break;  
			 
		 case 3://I2S1 mclk src
			 memcpy(&TmpData, &buf[1], 2);
			 TmpData = TmpData > 4? 4 : TmpData;
			 gCtrlVars.i2s1_mclk_src = TmpData;
			 GlobalMclkSet();
			 if((gCtrlVars.i2s1_mclk_src == 3) ||(gCtrlVars.i2s1_mclk_src == 4)) 
			{
				#if CFG_I2S1_OUT_EN || CFG_I2S1_IN_EN
				Clock_AudioMclkSel(AUDIO_I2S1, (MCLK_CLK_SEL)(gCtrlVars.i2s1_mclk_src-1));
				#endif
			}
			else
			{
				#if CFG_I2S1_OUT_EN || CFG_I2S1_IN_EN
				Clock_AudioMclkSel(AUDIO_I2S1, (MCLK_CLK_SEL)gCtrlVars.i2s1_mclk_src);
				#endif
			}
			 break;
	
		 case 4:///I2S1 master slave
			 memcpy(&TmpData, &buf[1], 2);
			 TmpData = TmpData > 1? 1 : TmpData;
			 gCtrlVars.i2s1_work_mode = TmpData;
		     #if CFG_I2S1_OUT_EN || CFG_I2S1_IN_EN
			 if(gCtrlVars.i2s1_work_mode)
			 {
				 I2S_SlaveModeSet(I2S1_MODULE, (I2S_DATA_FORMAT)gCtrlVars.i2s1_format, (I2S_DATA_LENGTH)gCtrlVars.i2s1_word_len);
			 }
			 else
			 {			 
				 I2S_MasterModeSet(I2S1_MODULE,(I2S_DATA_FORMAT)gCtrlVars.i2s1_format, (I2S_DATA_LENGTH)gCtrlVars.i2s1_word_len);
			 }
		     #endif
			 break;
			 
		 case 5:///I2S1 word lenght
			 memcpy(&TmpData, &buf[1], 2);
			 TmpData = TmpData > 3? 3 : TmpData;
			 gCtrlVars.i2s1_word_len = TmpData;
		     #if CFG_I2S1_OUT_EN || CFG_I2S1_IN_EN
			 if(gCtrlVars.i2s1_work_mode)
			 {
				 I2S_SlaveModeSet(I2S1_MODULE, (I2S_DATA_FORMAT)gCtrlVars.i2s1_format, (I2S_DATA_LENGTH)gCtrlVars.i2s1_word_len);
			 }
			 else
			 {			 
				 I2S_MasterModeSet(I2S1_MODULE,(I2S_DATA_FORMAT)gCtrlVars.i2s1_format, (I2S_DATA_LENGTH)gCtrlVars.i2s1_word_len);
			 }
		     #endif
			 break;
	
		 case 6:///I2S1 stereo mono
			 memcpy(&TmpData, &buf[1], 2);
			 TmpData = TmpData > 1? 1 : TmpData;
			 gCtrlVars.i2s1_stereo_mono = TmpData;
		     #if CFG_I2S1_OUT_EN || CFG_I2S1_IN_EN
			 I2S_MonoModeSet(I2S1_MODULE,(bool)gCtrlVars.i2s1_stereo_mono);
		     #endif
			 break;
			 
		 case 7:///I2S1 fade time
			 memcpy(&TmpData, &buf[1], 2);
			 TmpData = TmpData > 255? 255 : TmpData;
			 gCtrlVars.i2s1_fade_time = TmpData;
		     #if CFG_I2S1_OUT_EN || CFG_I2S1_IN_EN
			 if(gCtrlVars.i2s1_fade_time==0)
			 {
				 I2S_FadeDisable(I2S1_MODULE);
			 }
			 else
			 {
				 I2S_FadeEnable(I2S1_MODULE);
				 I2S_FadeTimeSet(I2S1_MODULE,(uint32_t)gCtrlVars.i2s1_fade_time);
			 }
		     #endif
			 break; 
			 
		 case 8:///i2s1 format
			 memcpy(&TmpData, &buf[1], 2);
			 TmpData = TmpData > 3? 3 : TmpData;
			 gCtrlVars.i2s1_format = TmpData;
		     #if CFG_I2S1_OUT_EN || CFG_I2S1_IN_EN
			 if(gCtrlVars.i2s1_work_mode)
			 {
				 I2S_SlaveModeSet(I2S1_MODULE, (I2S_DATA_FORMAT)gCtrlVars.i2s1_format, (I2S_DATA_LENGTH)gCtrlVars.i2s1_word_len);
			 }
			 else
			 {			 
				 I2S_MasterModeSet(I2S1_MODULE,(I2S_DATA_FORMAT)gCtrlVars.i2s1_format, (I2S_DATA_LENGTH)gCtrlVars.i2s1_word_len);
			 }
	
		     #endif
			 break;
			 
		  case 9:///i2s1 bclk invert
			 memcpy(&TmpData, &buf[1], 2);
			 TmpData = TmpData > 1? 1 : TmpData;
			 gCtrlVars.i2s1_bclk_invert_en = TmpData;
		     #if CFG_I2S1_OUT_EN || CFG_I2S1_IN_EN
			 I2S_BclkInvertSet(I2S1_MODULE,(bool)gCtrlVars.i2s1_bclk_invert_en);
		     #endif
			 break;
			 
		  case 10:///i2s1 bclk invert
			 memcpy(&TmpData, &buf[1], 2);
			 TmpData = TmpData > 1? 1 : TmpData;
			 gCtrlVars.i2s0_lrclk_invert_en = TmpData;
		     #if CFG_I2S1_OUT_EN || CFG_I2S1_IN_EN
			 I2S_LrclkInvertSet(I2S1_MODULE,(bool)gCtrlVars.i2s1_lrclk_invert_en);
		     #endif
			 break;
			 
		 default:
			 break;
	 }
}

void Communication_Effect_0x0C(uint8_t *buf, uint32_t len)////I2S1
{
	uint16_t i,k;
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x0c;
		tx_buf[3]  = 1 + 11*2;//1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
		memcpy(&tx_buf[5], &gCtrlVars.i2s1_tx_en, 2);
		memcpy(&tx_buf[7], &gCtrlVars.i2s1_rx_en, 2);
		memcpy(&tx_buf[9], &gCtrlVars.i2s1_sample_rate, 2);
		memcpy(&tx_buf[11], &gCtrlVars.i2s1_mclk_src, 2);
		memcpy(&tx_buf[13], &gCtrlVars.i2s1_work_mode, 2);
		memcpy(&tx_buf[15], &gCtrlVars.i2s1_word_len, 2);
		memcpy(&tx_buf[17], &gCtrlVars.i2s1_stereo_mono, 2);
		memcpy(&tx_buf[19], &gCtrlVars.i2s1_fade_time, 2);
		memcpy(&tx_buf[21], &gCtrlVars.i2s1_format, 2);
		memcpy(&tx_buf[23], &gCtrlVars.i2s1_bclk_invert_en, 2);
		memcpy(&tx_buf[25], &gCtrlVars.i2s1_lrclk_invert_en, 2);
		tx_buf[27] = 0x16;
		Communication_Effect_Send(tx_buf,28);
	}
	else
	{
		switch(buf[0])///dac1
		{
			case 0xff:
				buf++;
				for(i = 0; i < 11; i++)
				{
					cbuf[0] = i;////id

					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}

					Comm_I2S1_0x0C(&cbuf[0]);

					buf += 2;
				}
				break;
			default:
				Comm_I2S1_0x0C(buf);
				break;
		}
	}
}

void Comm_SPDIF_0x0D(uint8_t * buf)
{
    uint16_t TmpData;
	switch(buf[0])////i2s1  set
	{
		case 0:///Enable TX,RX 0=disable,1=rx,2=tx
		    #ifdef CFG_FUNC_SPDIF_EN
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 2? 2 : TmpData;
		    gCtrlVars.spdif_en = TmpData;
            #else
			gCtrlVars.spdif_en = 0;
			#endif
			break;
		case 1:///sample rate, only TX mode valid
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 12? 12 : TmpData;
            #if CFG_SPDIF_OUT_EN
			gCtrlVars.spdif_sample_rate = TmpData;
            #endif
			break;
		case 2:///channel mode
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 2? 2 : TmpData;
			gCtrlVars.spdif_channel_mode = TmpData;
			break;
		case 3:///gpio select 0=A25(in),1=A26(in),2=A27(in/out),3=A28(in),
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 3? 3 : TmpData;
			gCtrlVars.spdif_in_gpio = TmpData;
			break;
		case 4:///SPDIF status 0=unlock, 1=lock
			//gCtrlVars.spdif_lock_status;
			break;			
	}
}

void Communication_Effect_0x0D(uint8_t *buf, uint32_t len)////SPDIF
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x0d;
		tx_buf[3]  = 1 + 5*2;//1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
		memcpy(&tx_buf[5], &gCtrlVars.spdif_en, 2);
		memcpy(&tx_buf[7], &gCtrlVars.spdif_sample_rate, 2);
   		memcpy(&tx_buf[9], &gCtrlVars.spdif_channel_mode, 2);
   		memcpy(&tx_buf[11], &gCtrlVars.spdif_in_gpio, 2);
		memcpy(&tx_buf[13], &gCtrlVars.spdif_lock_status, 2);
		tx_buf[15] = 0x16;
		Communication_Effect_Send(tx_buf,16);
	}
	else
	{
		switch(buf[0])///spdif
		{
			case 0xff:
				buf++;
				for(i = 0; i < 4; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_SPDIF_0x0D(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_SPDIF_0x0D(buf);
				break;
		}
	}
}

uint8_t user_rx_data[32];
uint32_t user_offset;
uint8_t user_rLen;
uint8_t *pUserData;
uint8_t MV_UserWriteData(void)//OK = TURE, FAIL= FALSE
{
	uint8_t i;
	pUserData = (uint8_t*)osPortMallocFromEnd(4096);// fifo
	if(pUserData != NULL)
	{
		memset(pUserData, 0, 4096);
	}
	else
	{
		return FALSE;
	}
    if(user_offset%4096)
    {
    	osPortFree(pUserData);
    	return FALSE;
    }
	SpiFlashRead(user_offset,  pUserData, 4096, 1);
	vTaskDelay(5);
	SpiFlashErase(SECTOR_ERASE, user_offset/4096, 1);
	vTaskDelay(50);
	DBG("[Write ] ");
	for(i = 0; i < user_rLen; i++)
	{
		pUserData[i] = user_rx_data[i];
		DBG("%02x ",user_rx_data[i]);
	}
	DBG("\n");
	SpiFlashWrite(user_offset, pUserData, 4096, 1);
	vTaskDelay(50);
	WDG_Feed();
	SpiFlashRead(user_offset,  pUserData, 128, 1);
	DBG("[Read ] ");
	for(i = 0; i < user_rLen; i++)
	{
		DBG("%02x ",pUserData[i]);
	}
	DBG("\n");

	if(memcmp(pUserData,user_rx_data,user_rLen) != 0)
	{
		osPortFree(pUserData);
		return FALSE;
	}
	else
	{
		osPortFree(pUserData);
		return TRUE;
	}
}

void Communication_Effect_0x11(uint8_t *buf, uint32_t len)
{

#define CFG_USER_MAJOR_VERSION  CFG_EFFECT_MAJOR_VERSION//Major
#define CFG_USER_MINOR_VERSION  CFG_EFFECT_MINOR_VERSION//minor//
#define CFG_USER_PATCH_VERSION  CFG_EFFECT_USER_VERSION//revision

	uint8_t sLen,ret;

	if(buf[0]==0x01)//write data 37
	{
		//a5 5a 11 25 01 1f e0 00 12 11 22 33 44 55 66 77 88 99 00 00 00 00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 16
		//7 Bytes：1 Byte（Index）+1 Byte（DataLength）+2 Bytes（WriteResult）+ 3 Bytes(3个0xFF，reserved）
		//----------------------------------------------------------------
		//DataLength ---> 写入数据长度
		//WriteResult ---> 写数据的结果
		//WriteResult Case 1:烧录OK，回OK(0x4F,0x4B)
		//WriteResult Case 2:烧录NG，回NG(0x4E,0x47)
		//Index ---> 0x01
		user_offset =   (uint32_t) buf[1]<<16;
		user_offset |=  (uint32_t) buf[2]<<8;
		user_offset |=  (uint32_t) buf[3];


		user_rLen  = buf[4];

		for(sLen= 0; sLen < 32; sLen++)
		{
		   user_rx_data[sLen]= buf[sLen+5];
		}

        //DBG("offse:%08x   data_len:%02d\n",user_offset,user_rLen);
        ret = MV_UserWriteData();

		sLen = 0;

		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[sLen++]  = 0xa5;
		tx_buf[sLen++]  = 0x5a;
		tx_buf[sLen++]  = 0x11;
		tx_buf[sLen++]  = 0x08;//

		tx_buf[sLen++]  = 0x01;//index

		tx_buf[sLen++]  = user_rLen;//Len

		if(ret)
		{
		    tx_buf[sLen++]  = 0x4f;//result OK    = 4F 4B
		    tx_buf[sLen++]  = 0x4b;//result Fail  = 4E 47
		}
		else
		{
		    tx_buf[sLen++]  = 0x4e;//result OK    = 4F 4B
		    tx_buf[sLen++]  = 0x47;//result Fail  = 4E 47
		}

		tx_buf[sLen++]  = 0xff;//reserv
		tx_buf[sLen++]  = 0xff;//reserv
		tx_buf[sLen++] =  0xff;// reserv
		tx_buf[sLen++] = 0x16;
		Communication_Effect_Send(tx_buf, sLen);
	}
	else//if(buf[0]==0x00)//get ver
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x11;
		tx_buf[3]  = 0x07;
		tx_buf[4]  = 0x30;//20=kmic 21=O26  30=B1X
		tx_buf[5]  = CFG_USER_MAJOR_VERSION;//Major
		tx_buf[6]  = CFG_USER_MINOR_VERSION;//minor//
		tx_buf[7]  = CFG_USER_PATCH_VERSION;//revision

		tx_buf[8]  = 0xaa;//reserv Major
		tx_buf[9]  = 0xbb;//reserv minor
		tx_buf[10]  = 0xcc;//reserv revision
		tx_buf[11] = 0x16;
		Communication_Effect_Send(tx_buf, 12);
	}
}

//参数列表
void Communication_Effect_0x80(uint8_t *buf, uint32_t len)
{
	uint16_t i;

	memset(tx_buf, 0, sizeof(tx_buf));
    if(buf[0] == 0)
	{
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = 0x80;
		tx_buf[3] = effect_sum * 2 + 2;
		tx_buf[4]  = 0x00;///index
		tx_buf[5] = effect_sum;
		memcpy(&tx_buf[6], effect_list, effect_sum * 2);
		tx_buf[6 + effect_sum * 2] = 0x16;
		Communication_Effect_Send(tx_buf, 7 + effect_sum * 2);
	}
	else
	{	
		if( (buf[0] <= effect_sum ) && (buf[0] > 0 ) )///////(A5 5A 80 01 00 16=EFF_0)(A5 5A 80 01 01 16=EFF_1)
		{
			//APP_DBG("Effect Name:%s\n",&AudioEffectNameList[buf[0]-1][0]);
			tx_buf[0]  = 0xa5;
			tx_buf[1]  = 0x5a;
			tx_buf[2]  = 0x80;
			tx_buf[3]  = 25 * 1 + 1;//len
			tx_buf[4]  = buf[0];///audio effect number
			tx_buf[30]  = 0x16;
			if(GetSystemMode() != AppModeBtHfPlay)
			{
				for(i = 0; i < 25; i++)
				{
					tx_buf[i + 5] = AudioEffectNameList[buf[0]-1][i];
				}
				Communication_Effect_Send(tx_buf, 31);
			}
			else
			{
				for(i = 0; i < 25; i++)
				{
					tx_buf[i + 5] = AudioEffectNameList_HFP[buf[0]-1][i];
				}
				Communication_Effect_Send(tx_buf, 31);
			}
	  
		}///end of if(buf[0] < effect_sum )	 
	}
}

void Communication_Effect_0xff(uint8_t *buf, uint32_t len)
{
    uint32_t TmpData;
	memset(tx_buf, 0, sizeof(tx_buf));
    if(len == 0)
	{
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = 0xff;
		tx_buf[3] = 3;
		tx_buf[4] = 0;//index0
		tx_buf[5] = (uint8_t)gCtrlVars.crypto_en;
		tx_buf[6] = 0;
		tx_buf[7]  = 0x16;///
		Communication_Effect_Send(tx_buf, 8);
	}
	else
	{
		memcpy(&TmpData, &buf[0], 4);
	    tx_buf[0] = 0xa5;
	    tx_buf[1] = 0x5a;
		tx_buf[2] = 0xFF;
		tx_buf[3] = 3;
		tx_buf[4] = 1;//index1	
		if(gCtrlVars.password != TmpData)
		{	   
			tx_buf[5] = 0;///passwrod err
		}
		else
		{
		    tx_buf[5] = 1;///passwrod ok		
		}
		tx_buf[7] = 0x16;
		Communication_Effect_Send(tx_buf, 8);
	}	
}

void Communication_Effect_GainControl(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	GainControlUnit *p = (GainControlUnit *)addr;
	uint16_t TmpData;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 1 + 3*2;//1 + len * sizeof(int16);
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->mute, 2);
		memcpy(&tx_buf[9], &p->gain, 2);
		tx_buf[11] = 0x16;
		Communication_Effect_Send(tx_buf, 12);
	}
	else//set
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&p->enable, &buf[1], 2);
				break;

			case 1:
				memcpy(&p->mute, &buf[1], 2);
				break;

			case 2:
				memcpy(&TmpData, &buf[1], 2);
				TmpData = TmpData > 0x3fff? 0x3fff : TmpData;
				p->gain = TmpData;
				break;

			case 0xff:
			    memcpy(&TmpData,&buf[1], 2);
			    p->enable = TmpData;
		        memcpy(&TmpData, &buf[3],2);
				p->mute = TmpData;
		        memcpy(&TmpData, &buf[5],2);
				p->gain = TmpData;
				break;
			default:
				break;
		}
	  }
}

void Comm_Effect_AutoTune(AutoTuneUnit *p, uint8_t *buf)
{
	uint16_t TmpData;
	uint16_t i;

	switch(buf[0])//
	{
		case 0:
			memcpy(&TmpData, &buf[1], 2);
			if(p->enable != TmpData)
			{
				p->enable = TmpData;
				if(p->enable)
				{
					AudioEffectAutoTuneInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					if(p->enable == 1)
					{
						IsEffectChange = 1;
					}					
				}
				else
				{
					IsEffectChange = 1;
				}
			}
			break;
	
		case 1:
			memcpy(&TmpData, &buf[1], 2);
			for(i = 0; i < 13; i++)
			{
				if(buf[1] == AutoTuneKeyTbl[i]) 
				{
					if(TmpData != p->key)
					{
						p->key = TmpData;
						AudioEffectAutoTuneConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					}
					break;
				}
			}
			break;
	
		case 2:
			memcpy(&TmpData, &buf[1], 2);
			for(i = 0; i < 3; i++)
			{
				if(buf[1] == AutoTuneSnapTbl[i]) 
				{
					p->snap_mode = TmpData;
					break;
				}
			}
			break;
			
		case 3:
			memcpy(&TmpData, &buf[1], 2);
			if(TmpData>2) TmpData = 2;
			p->pitch_accuracy = TmpData;
			break;
			
		default:
			break;
	}
}

void Communication_Effect_AutoTune(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	AutoTuneUnit *p = (AutoTuneUnit *)addr;
	uint32_t i,k;

    memset(tx_buf, 0, sizeof(tx_buf));
	if(len == 0)//ask
	{
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2*4 + 1;
		tx_buf[4] = 0xff;
		
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->key, 2);
		memcpy(&tx_buf[9], &p->snap_mode, 2);
		memcpy(&tx_buf[11], &p->pitch_accuracy, 2);

		tx_buf[13] = 0x16;
		Communication_Effect_Send(tx_buf, 14);
	}
	else//set
	{
		switch(buf[0])//
		{
			case 0xff:
				buf++;
				for(i = 0; i < 4; i++)
				{
					cbuf[0] = i;////id

					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}

					Comm_Effect_AutoTune(p,&cbuf[0]);

					buf += 2;
				}
				break;
			default:
				Comm_Effect_AutoTune(p,buf);
				break;
		}
	}
}

void Communication_Effect_PitchShifter(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	PitchShifterUnit *p = (PitchShifterUnit *)addr;
	uint16_t TmpData16;
	int16_t TmpDataS16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 1 + 2*2;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->semitone_steps, 2);

		tx_buf[9] = 0x16;
		Communication_Effect_Send(tx_buf, 10);
	}
	else//set
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);

				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectPitchShifterInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}				
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;

			case 1:
				memcpy(&TmpDataS16, &buf[1], 2);
				TmpDataS16 = TmpDataS16 > 120? 120:TmpDataS16;
				TmpDataS16 = TmpDataS16 < -120? -120:TmpDataS16;
				if(p->semitone_steps != TmpDataS16)
				{
					p->semitone_steps = TmpDataS16;
					AudioEffectPitchShifterConfig(p);
				}
				break;

			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				memcpy(&TmpDataS16, &buf[3], 2);
				TmpDataS16 = TmpDataS16 > 120? 120:TmpDataS16;
				TmpDataS16 = TmpDataS16 < -120? -120:TmpDataS16;
				
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectPitchShifterInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}
					else
					{
						IsEffectChange = 1;						
					}
				}
				if(p->semitone_steps != TmpDataS16)
				{
    				if(p->enable)
    				{
    					p->semitone_steps = TmpDataS16;
    					AudioEffectPitchShifterConfig(p);
    				}
				}
				break;
			default:
				break;
		}
	}
}

void Communication_Effect_PitchShifterPro(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
	PitchShifterProUnit *p = (PitchShifterProUnit *)addr;
	uint16_t TmpData16;
	int16_t TmpDataS16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 1 + 2*2;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->semitone_steps, 2);
		tx_buf[9] = 0x16;
		Communication_Effect_Send(tx_buf, 10);
	}
	else//set
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);

				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectPitchShifterProInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}		
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;

			case 1:
				memcpy(&TmpDataS16, &buf[1], 2);
				TmpDataS16 = TmpDataS16 > 120? 120:TmpDataS16;
				TmpDataS16 = TmpDataS16 < -120? -120:TmpDataS16;
				if(p->semitone_steps != TmpDataS16)
				{
					p->semitone_steps = TmpDataS16;
					AudioEffectPitchShifterProConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
				}
				break;

			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				memcpy(&TmpDataS16, &buf[3], 2);
				TmpDataS16 = TmpDataS16 > 120? 120:TmpDataS16;
				TmpDataS16 = TmpDataS16 < -120? -120:TmpDataS16;
				
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectPitchShifterProInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				if(p->semitone_steps != TmpDataS16)
				{
    				if(p->enable)
    				{
    					p->semitone_steps = TmpDataS16;
						AudioEffectPitchShifterProConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
    				}
				}
				break;
			default:
				break;
		}
	}
#endif
}

void Comm_Effect_Reverb(ReverbUnit *p, uint8_t * buf)
{
	int16_t TmpData16;

	switch(buf[0])//
	{
		case 0:
			memcpy(&TmpData16, &buf[1], 2);
			if(p->enable != TmpData16)
			{
				p->enable = TmpData16;
				if(p->enable)
				{
					AudioEffectReverbInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					if(p->enable)
					{
						IsEffectChange = 1;				
					}
				}
				else
				{
					IsEffectChange = 1;
				}
			}
			break;
	
		case 1:
			memcpy(&TmpData16, &buf[1], 2);
			if(TmpData16 > 200)
			{
				TmpData16 = 200;
			}
			if(p->dry_scale != TmpData16 )
			{
				p->dry_scale = TmpData16;
				AudioEffectReverbConfig(p);
			}
			break;
	
		case 2:
			memcpy(&TmpData16, &buf[1], 2);
			if(TmpData16 > 300)
			{
				TmpData16 = 300;
			}
			if(p->wet_scale != TmpData16)
			{
				p->wet_scale = TmpData16;
				AudioEffectReverbConfig(p);
			}
			break;
	
		case 3:
			memcpy(&TmpData16, &buf[1], 2);
			if(TmpData16 > 100)
			{
				TmpData16 = 100;
			}
			if(p->width_scale != TmpData16)
			{
				p->width_scale = TmpData16;
				AudioEffectReverbConfig(p);
			}
			break;
	
		case 4:
			memcpy(&TmpData16, &buf[1], 2);
			if(TmpData16 > 100)
			{
				TmpData16 = 100;
			}
			if(p->roomsize_scale != TmpData16)
			{
				p->roomsize_scale = TmpData16;
				AudioEffectReverbConfig(p);
			}
			break;
	
		case 5:
			memcpy(&TmpData16, &buf[1], 2);
			if(TmpData16 > 100)
			{
				TmpData16 = 100;
			}
			if( p->damping_scale != TmpData16)
			{
				p->damping_scale = TmpData16;
				AudioEffectReverbConfig(p);
			}
			break;
	
		case 6:
			memcpy(&TmpData16, &buf[1], 2);
			if(TmpData16 > 1)
			{
				TmpData16 = 1;
			}
			if(p->mono != TmpData16)
			{
				p->mono = TmpData16;
				AudioEffectReverbConfig(p);
			}
			break;
		}
}

void Communication_Effect_Reverb(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	ReverbUnit *p = (ReverbUnit *)addr;
	uint16_t i,k;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 7 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->dry_scale, 2);
		memcpy(&tx_buf[9], &p->wet_scale,2);
		memcpy(&tx_buf[11], &p->width_scale, 2);
		memcpy(&tx_buf[13], &p->roomsize_scale, 2);
		memcpy(&tx_buf[15], &p->damping_scale, 2);
		memcpy(&tx_buf[17], &p->mono, 2);
		tx_buf[5+7*2] = 0x16;
		Communication_Effect_Send(tx_buf, 7*2 + 6);
	}
	else
	{
		switch(buf[0])//
		{
			case 0xff:
				buf++;
				for(i = 0; i < 7; i++)
				{
					cbuf[0] = i;////id

					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_Effect_Reverb(p,&cbuf[0]);

					buf += 2;
				}
				break;				
			default:
				Comm_Effect_Reverb(p,buf);
				break;
		}
	}
}

void Communication_Effect_SilenceDector(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	SilenceDetectorUnit *p = (SilenceDetectorUnit *)addr;
	uint16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] =2 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->level, 2);
		tx_buf[5 + 2 * 2] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 2 * 2);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectSilenceDectorInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			default:
				break;
		}
	}
}

void Communication_Effect_ThreeD(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
	ThreeDUnit *p = (ThreeDUnit *)addr;
	int16_t TmpData16;
	int16_t TmpDataS16;
	
	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->intensity, 2);
		tx_buf[5 + 2 * 2] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 2 * 2);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectThreeDInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;				
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			case 1:				
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 100;
				}
				if(p->intensity != TmpData16)
				{
					p->intensity = TmpData16;
					AudioEffectThreeDInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
				}
				break;	
			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
			    memcpy(&TmpDataS16, &buf[3], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectThreeDInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;				
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				if(p->intensity != TmpDataS16)
				{
					p->intensity = TmpDataS16;
    				if(p->enable)
    				{
						AudioEffectThreeDInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
    				}
				}
				break;
			default:
				break;
		}
	}
#endif
}

void Communication_Effect_ThreeDPlus(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
	ThreeDPlusUnit *p = (ThreeDPlusUnit *)addr;
	int16_t TmpData16;
	int16_t TmpDataS16;
	
	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->intensity, 2);
		tx_buf[5 + 2 * 2] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 2 * 2);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectThreeDPlusInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;				
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			case 1:				
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 100;
				}
				if(p->intensity != TmpData16)
				{
					p->intensity = TmpData16;
					AudioEffectThreeDPlusInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
				}
				break;	
			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
			    memcpy(&TmpDataS16, &buf[3], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectThreeDPlusInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;				
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				if(p->intensity != TmpDataS16)
				{
					p->intensity = TmpDataS16;
    				if(p->enable)
    				{
						AudioEffectThreeDPlusInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
    				}
				}
				break;
			default:
				break;
		}
	}
#endif
}

void Communication_Effect_VB(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
	VBUnit *p = (VBUnit *)addr;
	int16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 4 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->f_cut, 2);
		memcpy(&tx_buf[9], &p->intensity, 2);
		memcpy(&tx_buf[11], &p->enhanced, 2);
		tx_buf[5 + 4 * 2] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 4 * 2);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
						if(p == &gCtrlVars.rec_vb_unit)
						{
							AudioEffectVBInit(p,1, gCtrlVars.sample_rate);
						}
						else
						#endif
						{
							AudioEffectVBInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
						}
						if(p->enable)
						{
							IsEffectChange = 1;				
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 300)
				{
					TmpData16 = 300;
				}
				else if(TmpData16 < 30)
				{
					TmpData16 = 30;
				}
				if(p->f_cut != TmpData16)
				{
					p->f_cut = TmpData16;
					#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
					if(p == &gCtrlVars.rec_vb_unit)
					{
						AudioEffectVBConfig(p, 1, gCtrlVars.sample_rate);
					}
					else
					#endif
					{
						AudioEffectVBConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					}
				}
				break;	
		    case 2:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 100;
				}
				if(p->intensity != TmpData16)
				{
					p->intensity = TmpData16;
					#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
					if(p == &gCtrlVars.rec_vb_unit)
					{
						AudioEffectVBConfig(p, 1, gCtrlVars.sample_rate);
					}
					else
					#endif
					{
						AudioEffectVBConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					}
				}
				break;	
		    case 3:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 1)
				{
					TmpData16 = 1;
				}
				if( p->enhanced != TmpData16)
				{
					p->enhanced = TmpData16;
					#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
					if(p == &gCtrlVars.rec_vb_unit)
					{
						AudioEffectVBConfig(p, 1, gCtrlVars.sample_rate);
					}
					else
					#endif
					{
						AudioEffectVBConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					}
				}
				break;	
			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				p->enable = TmpData16;
				memcpy(&TmpData16, &buf[3], 2);
				p->f_cut = TmpData16;
			    memcpy(&TmpData16, &buf[5], 2);
				p->intensity = TmpData16;
			    memcpy(&TmpData16, &buf[7], 2);
				p->enhanced = TmpData16;
				if(p->enable)
				{
					#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
					if(p == &gCtrlVars.rec_vb_unit)
					{
						AudioEffectVBInit(p, 1, gCtrlVars.sample_rate);
					}
					else
					#endif
					{
						AudioEffectVBInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					}
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}
#endif
}

void Communication_Effect_VBClassic(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
	VBClassicUnit *p = (VBClassicUnit *)addr;
	int16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 3 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->f_cut, 2);
		memcpy(&tx_buf[9], &p->intensity, 2);
		tx_buf[5 + 3 * 2] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 3 * 2);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectVBClassicInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;				
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 300)
				{
					TmpData16 = 300;
				}
				else if(TmpData16 < 30)
				{
					TmpData16 = 30;
				}
				if(p->f_cut != TmpData16)
				{
					p->f_cut = TmpData16;
					AudioEffectVBClassicConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
				}
				break;	
		    case 2:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 100;
				}
				if(p->intensity != TmpData16)
				{
					p->intensity = TmpData16;
					AudioEffectVBClassicConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
				}
				break;	
			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				p->enable = TmpData16;
				memcpy(&TmpData16, &buf[3], 2);
				p->f_cut = TmpData16;
			    memcpy(&TmpData16, &buf[5], 2);
				p->intensity = TmpData16;
				if(p->enable)
				{
					AudioEffectVBClassicInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}
#endif
}
void Communication_Effect_StereoWindener(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
	StereoWindenUnit *p = (StereoWindenUnit *)addr;
	int16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->shaping, 2);

		tx_buf[9] = 0x16;
		Communication_Effect_Send(tx_buf, 10);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:			
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;

					if(p->enable)
					{
						AudioEffectStereoWidenerInit(p,gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;				
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 1)
				{
					TmpData16 = 1;
				}
				p->shaping  = TmpData16;
				if(p->enable)
				{
					AudioEffectStereoWidenerInit(p,gCtrlVars.sample_rate);
				}
				break;

			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				p->enable = TmpData16;

				memcpy(&TmpData16, &buf[3], 2);
				p->shaping = TmpData16;

				if(p->enable)
				{
					AudioEffectStereoWidenerInit(p,gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}
#endif
}
void Communication_Effect_Delay(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
	PcmDelayUnit *p = (PcmDelayUnit *)addr;
	int16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2 * 4 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->delay, 2);
		memcpy(&tx_buf[9], &p->max_delay, 2);
		memcpy(&tx_buf[11], &p->high_quality, 2);

		tx_buf[5 + 2 * 4] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 2 * 4);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						if(p->high_quality)
						{
							if(p->max_delay > 1000)
							{
								p->max_delay = 1000;
							}
						}
						else
						{
							if(p->max_delay > 3000)
							{
								p->max_delay = 3000;
							}
						}

						p->max_delay_samples = (p->max_delay*gCtrlVars.sample_rate)/1000;
						p->delay = p->delay > p->max_delay? p->max_delay : p->delay;
						p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;	
					
						AudioEffectPcmDelayInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}				
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
				
			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->delay != TmpData16)
				{
					p->delay = TmpData16;
					
					if(p->delay > p->max_delay)
					{
						p->delay = p->max_delay;
					}

					p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;

				}
				break;	
				
			case 2:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->max_delay != TmpData16)
				{
					p->max_delay = TmpData16;
					if(p->high_quality)
					{
						if(p->max_delay > 1000)
						{
							p->max_delay = 1000;
						}
					}
					else
					{
						if(p->max_delay > 3000)
						{
							p->max_delay = 3000;
						}
					}

					p->max_delay_samples = (p->max_delay*gCtrlVars.sample_rate)/1000;
					p->delay = p->delay > p->max_delay? p->max_delay : p->delay;
					p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;	
					
					if(p->enable)
					{
			    		IsEffectChange = 1;
					}
				}
				break;
				
			case 3:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->high_quality != TmpData16)
				{					
					p->high_quality = TmpData16;

					if(p->high_quality)
					{
						if(p->max_delay > 1000)
						{
							p->max_delay = 1000;
						}
					}
					else
					{
						if(p->max_delay > 3000)
						{
							p->max_delay = 3000;
						}
					}
					p->max_delay_samples = (p->max_delay*gCtrlVars.sample_rate)/1000;
					p->delay = p->delay > p->max_delay? p->max_delay : p->delay;
					p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;	
					
					if(p->enable)
					{
			    		IsEffectChange = 1;
					}
				}
				break;	
				
			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				p->enable = TmpData16;
			
				memcpy(&TmpData16, &buf[3], 2);
				p->delay = TmpData16;					

				memcpy(&TmpData16, &buf[5], 2);
				p->max_delay = TmpData16;
				
				memcpy(&TmpData16, &buf[7], 2);
				p->high_quality = TmpData16;								

				if(p->high_quality)
				{
					if(p->max_delay > 1000)
					{
						p->max_delay = 1000;
					}
				}
				else
				{
					if(p->max_delay > 3000)
					{
						p->max_delay = 3000;
					}
				}
				p->max_delay_samples = (p->max_delay*gCtrlVars.sample_rate)/1000;
				p->delay = p->delay > p->max_delay? p->max_delay : p->delay;
				p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;				
				
				if(p->enable)
				{
					AudioEffectPcmDelayInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				break;
				
			default:
				break;
		}
	}
#endif
}

void Communication_Effect_HarmonicExciter(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	ExciterUnit *p = (ExciterUnit *)addr;
	int16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2 * 4 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->f_cut, 2);
		memcpy(&tx_buf[9], &p->dry, 2);
		memcpy(&tx_buf[11], &p->wet, 2);
		tx_buf[5 + 2 * 4] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 2 * 4);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{	
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectExciterInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}				
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				if((TmpData16 > 10000) || (TmpData16 < 1000))
				{
					TmpData16 = 1000;
				}
				if(p->f_cut != TmpData16)
				{
					p->f_cut = TmpData16;
					AudioEffectExciterConfig(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
				}
				break;	
			case 2:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 80;
				}
				if(p->dry != TmpData16)
				{
					p->dry = TmpData16;
				}
				break;	
			case 3:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 80;
				}
				if(p->wet != TmpData16)
				{
					p->wet = TmpData16;
				}
				break;	
			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				p->enable = TmpData16;
				memcpy(&TmpData16, &buf[3], 2);
				p->f_cut = TmpData16;
				memcpy(&TmpData16, &buf[5], 2);
				p->dry = TmpData16;
				memcpy(&TmpData16, &buf[7], 2);
				p->wet = TmpData16;	
				if(p->enable)
				{
					AudioEffectExciterInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}
}

void Communication_Effect_Vocal_Cut(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	VocalCutUnit *p = (VocalCutUnit *)addr;
	uint16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->wetdrymix, 2);
		tx_buf[5 + 2 * 2] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 2 * 2);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectVocalCutInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}				
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 100;
				}
				p->wetdrymix = TmpData16;
				break;	
			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				p->enable = TmpData16;
			
				memcpy(&TmpData16, &buf[3], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 100;
				}
				p->wetdrymix = TmpData16;				
				
				if(p->enable)
				{
					AudioEffectVocalCutInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}
}

void Communication_Effect_Vocal_Remove(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	VocalRemoveUnit *p = (VocalRemoveUnit *)addr;
	uint16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2 * 3 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->lower_freq, 2);
		memcpy(&tx_buf[9], &p->higher_freq, 2);
		tx_buf[5 + 2 * 3] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 2 * 3);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectVocalRemoveInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 20000)
				{
					TmpData16 = 200;
				}
				p->lower_freq = TmpData16;
				if(p->enable)
				{
					AudioEffectVocalRemoveConfig(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
				}
				break;			
			case 2:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 20000)
				{
					TmpData16 = 15000;
				}
				p->higher_freq = TmpData16;
				if(p->enable)
				{
					AudioEffectVocalRemoveConfig(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
				}
				break;		
			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				p->enable = TmpData16;
			
				memcpy(&TmpData16, &buf[3], 2);
				if(TmpData16 > 20000)
				{
					TmpData16 = 200;
				}
				p->lower_freq = TmpData16;

				memcpy(&TmpData16, &buf[5], 2);
				if(TmpData16 > 20000)
				{
					TmpData16 = 15000;
				}
				p->higher_freq = TmpData16;
				
				if(p->enable)
				{
					AudioEffectVocalRemoveInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}

}

void Communication_Effect_Chorus(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_CHORUS_EN
	ChorusUnit *p = (ChorusUnit *)addr;
	uint16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2 * 7 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->delay_length, 2);
		memcpy(&tx_buf[9], &p->mod_depth, 2);		
		memcpy(&tx_buf[11], &p->mod_rate, 2);
		memcpy(&tx_buf[13], &p->feedback, 2);
		memcpy(&tx_buf[15], &p->dry, 2);
        memcpy(&tx_buf[17], &p->wet, 2);
		tx_buf[5 + 2 * 7] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 2 * 7);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectChorusInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 25)
				{
					TmpData16 = 13;
				}
				p->delay_length = TmpData16;
				break;			
			case 2:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 24)
				{
					TmpData16 = 3;
				}
				p->mod_depth = TmpData16;
				break;		
			case 3:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 10;
				}
				p->mod_rate = TmpData16;
				break;			
			case 4:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 50)
				{
					TmpData16 = 30;
				}
				p->feedback = TmpData16;
				break;			
			case 5:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 90;
				}
				p->dry = TmpData16;
				break;		
			case 6:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 60;
				}
				p->wet = TmpData16;
				break;		
			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				p->enable = TmpData16;
			
				memcpy(&TmpData16, &buf[3], 2);
				if(TmpData16 > 25)
				{
					TmpData16 = 13;
				}
				p->delay_length = TmpData16;

				memcpy(&TmpData16, &buf[5], 2);
				if(TmpData16 > 24)
				{
					TmpData16 = 3;
				}
				p->mod_depth = TmpData16;

				memcpy(&TmpData16, &buf[7], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 10;
				}
				p->mod_rate = TmpData16;

				memcpy(&TmpData16, &buf[9], 2);
				if(TmpData16 > 50)
				{
					TmpData16 = 30;
				}
				p->feedback = TmpData16;

				memcpy(&TmpData16, &buf[11], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 90;
				}
				p->dry = TmpData16;

				memcpy(&TmpData16, &buf[13], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 60;
				}
				p->wet = TmpData16;
				
				if(p->enable)
				{
					AudioEffectChorusInit(p,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}
#endif //CFG_AUDIO_EFFECT_CHORUS_EN
}

void Communication_Effect_Phase(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	PhaseControlUnit *p = (PhaseControlUnit *)addr;
	uint16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->phase_difference, 2);
		tx_buf[5 + 2 * 2] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 2 * 2);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&p->enable, &buf[1], 2);
				break;
			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 1)
				{
					TmpData16 = 0;
				}
				p->phase_difference = TmpData16;
				break;	
			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				p->enable = TmpData16;
				memcpy(&TmpData16, &buf[3], 2);
				if(TmpData16 > 1)
				{
					TmpData16 = 0;
				}
				p->phase_difference = TmpData16;				
				break;
			default:
				break;
		}
	}
}

void Communication_Effect_PlateReverb(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	PlateReverbUnit *p = (PlateReverbUnit *)addr;
	uint16_t TmpData16;
	int16_t TmpDataS16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 8 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->highcut_freq, 2);
		memcpy(&tx_buf[9], &p->modulation_en, 2);
		memcpy(&tx_buf[11], &p->predelay, 2);
		memcpy(&tx_buf[13], &p->diffusion, 2);
		memcpy(&tx_buf[15], &p->decay, 2);
		memcpy(&tx_buf[17], &p->damping, 2);
		memcpy(&tx_buf[19], &p->wetdrymix, 2);
		tx_buf[5+8*2] = 0x16;
		Communication_Effect_Send(tx_buf, 8*2 + 6);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectPlateReverbInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;

			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > gCtrlVars.sample_rate/2)
				{
					TmpData16 = gCtrlVars.sample_rate/2;
				}
				if(p->highcut_freq != TmpData16)
				{
					p->highcut_freq = TmpData16;
					AudioEffectPlateReverbHighcutModulaConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					AudioEffectPlateReverbConfig(p);
				}
				break;

			case 2:
				memcpy(&TmpDataS16, &buf[1], 2);
				if(TmpDataS16 > 1)
				{
					TmpDataS16 = 1;
				}
				if(TmpDataS16 < 0)
				{
					TmpData16 = 0;
				}				
				if(p->modulation_en != TmpDataS16)
				{
					p->modulation_en = TmpDataS16;
					AudioEffectPlateReverbHighcutModulaConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					AudioEffectPlateReverbConfig(p);
				}
				break;

			case 3:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 4410)
				{
					TmpData16 = 4410;
				}
				if(p->predelay != TmpData16)
				{
					p->predelay = TmpData16;
					AudioEffectPlateReverbConfig(p);
				}
				break;

			case 4:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 100;
				}
				if(p->diffusion != TmpData16)
				{
					p->diffusion = TmpData16;
					AudioEffectPlateReverbConfig(p);
				}
				break;

			case 5:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 100;
				}
				if(p->decay != TmpData16)
				{
					p->decay = TmpData16;
					AudioEffectPlateReverbConfig(p);
				}
				break;

			case 6:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 10000)
				{
					TmpData16 = 10000;
				}
				if(p->damping != TmpData16)
				{
					p->damping = TmpData16;
					AudioEffectPlateReverbConfig(p);
				}
				break;

			case 7:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 100)
				{
					TmpData16 = 100;
				}
				if(p->wetdrymix != TmpData16)
				{
					p->wetdrymix = TmpData16;
					AudioEffectPlateReverbConfig(p);
				}
				break;
				
			case 0xff:
				memcpy(&TmpDataS16, &buf[1], 2);
				p->enable = TmpDataS16;
				memcpy(&TmpDataS16, &buf[3], 2);
				p->highcut_freq = TmpDataS16;
				memcpy(&TmpDataS16, &buf[5], 2);
				p->modulation_en = TmpDataS16;
				memcpy(&TmpDataS16, &buf[7], 2);
				p->predelay = TmpDataS16;
				memcpy(&TmpDataS16, &buf[9], 2);
				p->diffusion = TmpDataS16;
				memcpy(&TmpDataS16, &buf[11], 2);
				p->decay = TmpDataS16;
				memcpy(&TmpDataS16, &buf[13], 2);
				p->damping = TmpDataS16;
				memcpy(&TmpDataS16, &buf[15], 2);
				p->wetdrymix = TmpDataS16;
				if(p->enable)
				{
					AudioEffectPlateReverbInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					AudioEffectPlateReverbConfig(p);
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}
}
#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
void Comm_Effect_ReverbPro(ReverbProUnit *p, uint8_t *buf)
{
	int16_t TmpDataS16;

	switch(buf[0])
	{
		case 0:
			memcpy(&TmpDataS16, &buf[1], 2);
			if(p->enable != TmpDataS16)
			{
				p->enable = TmpDataS16;
				if(p->enable)
				{
					AudioEffectReverbProInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					if(p->enable)
					{
						IsEffectChange = 1;
					}
				}
				else
				{
					IsEffectChange = 1;
				}
			}
			break;
		case 1://dry -70 to +10db
			memcpy(&TmpDataS16, &buf[1], 2);
			p->dry = TmpDataS16;
			break;
		case 2://wet -70 to +10dbmemcpy
			memcpy(&TmpDataS16, &buf[1], 2);
			p->wet = TmpDataS16;
			break;
		case 3://erwet -70 to +10dbmemcpy
			memcpy(&TmpDataS16, &buf[1], 2);
			p->erwet = TmpDataS16;
			break;
		case 4://erfactor 50 to 250%memcpy
			memcpy(&TmpDataS16, &buf[1], 2);
			p->erfactor = TmpDataS16;
			break;
		case 5://erwitdh -100 to 100%memcpy
			memcpy(&TmpDataS16, &buf[1], 2);
			p->erwidth = TmpDataS16;
			break;
		case 6://ertolate 0 to 100%memcpy
			memcpy(&TmpDataS16, &buf[1], 2);
			p->ertolate = TmpDataS16;
			break;
		case 7://rt60 100 to 15000msmemcpy
			memcpy(&TmpDataS16, &buf[1], 2);
			p->rt60 = TmpDataS16;
			break;
		case 8://delay 0 to 100msmemcpy
			memcpy(&TmpDataS16, &buf[1], 2);
			p->delay = TmpDataS16;
			break;
		case 9://width 0 to 100%memcpy
			memcpy(&TmpDataS16, &buf[1], 2);
			p->width = TmpDataS16;
			break;
		case 10:////wander 10 to 60%
			memcpy(&TmpDataS16, &buf[1], 2);
			p->wander = TmpDataS16;
			break;
		case 11://spin 0 to 1000%
			memcpy(&TmpDataS16, &buf[1], 2);
			p->spin = TmpDataS16;
			break;
		case 12://inputlpf 200hz to 18000hz
			memcpy(&TmpDataS16, &buf[1], 2);
			p->inputlpf = TmpDataS16;
			break;
		case 13:////damplpf 200hz to 18000hz
			memcpy(&TmpDataS16, &buf[1], 2);
			p->damplpf = TmpDataS16;
			break;
		case 14:////basslpf 50hz to 1050hz
			memcpy(&TmpDataS16, &buf[1], 2);
			p->basslpf = TmpDataS16;
			break;
		case 15:////bassb 0 to 50%
			memcpy(&TmpDataS16, &buf[1], 2);
			p->bassb = TmpDataS16;
			break;
		case 16:////outputlpf 200hz to 18000hz
			memcpy(&TmpDataS16, &buf[1], 2);
			p->outputlpf = TmpDataS16;
			break;
	}

}
#endif

void Communication_Effect_ReverbPro(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
	ReverbProUnit *p = (ReverbProUnit *)addr;
	uint16_t i,k;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 17 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5],  &p->enable, 2);
		memcpy(&tx_buf[7],  &p->dry, 2);
		memcpy(&tx_buf[9],  &p->wet, 2);
		memcpy(&tx_buf[11], &p->erwet, 2);
		memcpy(&tx_buf[13], &p->erfactor, 2);
		memcpy(&tx_buf[15], &p->erwidth, 2);
		memcpy(&tx_buf[17], &p->ertolate, 2);
		memcpy(&tx_buf[19], &p->rt60, 2);
		memcpy(&tx_buf[21], &p->delay, 2);
		memcpy(&tx_buf[23], &p->width, 2);
		memcpy(&tx_buf[25], &p->wander, 2);
		memcpy(&tx_buf[27], &p->spin, 2);
		memcpy(&tx_buf[29], &p->inputlpf, 2);
		memcpy(&tx_buf[31], &p->damplpf, 2);
		memcpy(&tx_buf[33], &p->basslpf, 2);
		memcpy(&tx_buf[35], &p->bassb, 2);
		memcpy(&tx_buf[37], &p->outputlpf, 2);
		tx_buf[5 + 17 * 2] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 17 * 2);
	}
	else
	{
		switch(buf[0])//
		{

			case 0xff:
					buf++;
					for(i = 0; i < 17; i++)
					{
						cbuf[0] = i;////id
						for(k = 0; k < 2; k++)
						{
							cbuf[ k + 1] = (uint8_t)buf[k];
						}
						Comm_Effect_ReverbPro(p,&cbuf[0]);
						buf += 2;
					}
					break;
				default:
					Comm_Effect_ReverbPro(p,buf);
					break;

		}

		AudioEffectReverProbConfig(p,gCtrlVars.sample_rate);
	}
#endif
}

void Communication_Effect_PingPong(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_PINGPONG_EN
	PingPongUnit *p = (PingPongUnit *)addr;
	int16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2 * 6 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->attenuation, 2);
		memcpy(&tx_buf[9], &p->delay, 2);
		memcpy(&tx_buf[11], &p->high_quality, 2);
		memcpy(&tx_buf[13], &p->wetdrymix, 2);
        memcpy(&tx_buf[15], &p->max_delay, 2);
		tx_buf[5+6*2] = 0x16;
		Communication_Effect_Send(tx_buf, 6*2 + 6);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;

					if(p->high_quality)
					{
						if(p->max_delay > 1000)
						{
							p->max_delay = 1000;
						}
					}
					else
					{
						if(p->max_delay > 3000)
						{
							p->max_delay = 3000;
						}
					}

					p->max_delay_samples = (p->max_delay*gCtrlVars.sample_rate)/1000;
					p->delay = p->delay > p->max_delay? p->max_delay : p->delay;
					p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;	
					
					if(p->enable)
					{
						//AudioEffectPingPongInit(p,gCtrlVars.sample_rate);
						IsEffectChange = 1;
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				TmpData16 = TmpData16 == 0? 1 : TmpData16;

				if(p->attenuation != TmpData16)
				{
					p->attenuation = TmpData16;
				}
				break;
			case 2:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->delay != TmpData16)
				{
					p->delay = TmpData16;
					if(p->delay > p->max_delay)
					{
						p->delay = p->max_delay;
					}

					p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;
				}
				break;
			case 3:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->high_quality != TmpData16)
				{
					if(p->enable)
					{
			    		IsEffectChange = 1;
					}
					p->high_quality = TmpData16;

					if(p->high_quality)
					{
						if(p->max_delay > 1000)
						{
							p->max_delay = 1000;
						}
					}
					else
					{
						if(p->max_delay > 3000)
						{
							p->max_delay = 3000;
						}
					}
					p->max_delay_samples = (p->max_delay*gCtrlVars.sample_rate)/1000;
					p->delay = p->delay > p->max_delay? p->max_delay : p->delay;
					p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;	
				}
				break;

			case 4:
				memcpy(&TmpData16, &buf[1], 2);
				p->wetdrymix = TmpData16;
				break;
			case 5:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->max_delay != TmpData16)
				{
					p->max_delay = TmpData16;
					if(p->high_quality)
					{
						if(p->max_delay > 1000)
						{
							p->max_delay = 1000;
						}
					}
					else
					{
						if(p->max_delay > 3000)
						{
							p->max_delay = 3000;
						}
					}

					p->max_delay_samples = (p->max_delay*gCtrlVars.sample_rate)/1000;
					p->delay = p->delay > p->max_delay? p->max_delay : p->delay;
					p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;	
					
					if(p->enable)
					{
			    		IsEffectChange = 1;
					}
				}
				break;
			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				p->enable = TmpData16;

				memcpy(&TmpData16, &buf[3], 2);
				p->attenuation = TmpData16;

				memcpy(&TmpData16, &buf[5], 2);
				p->delay = TmpData16;

				memcpy(&TmpData16, &buf[7], 2);
				p->high_quality = TmpData16;

				memcpy(&TmpData16, &buf[9], 2);
				p->wetdrymix = TmpData16;

				memcpy(&TmpData16, &buf[5], 2);
				p->max_delay = TmpData16;

				if(p->high_quality)
				{
					if(p->max_delay > 1000)
					{
						p->max_delay = 1000;
					}
				}
				else
				{
					if(p->max_delay > 3000)
					{
						p->max_delay = 3000;
					}
				}
				p->max_delay_samples = (p->max_delay*gCtrlVars.sample_rate)/1000;
				p->delay = p->delay > p->max_delay? p->max_delay : p->delay;
				p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;
				
				if(p->enable)
				{
					AudioEffectPingPongInit(p,gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}
#endif

}
void Communication_Effect_AutoWah(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_AUTOWAH_EN
	AutoWahUnit *p = (AutoWahUnit *)addr;
	int16_t TmpData16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 2 * 7 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->modulation_rate, 2);
		memcpy(&tx_buf[9], &p->min_frequency, 2);
		memcpy(&tx_buf[11], &p->max_frequency, 2);
		memcpy(&tx_buf[13], &p->depth, 2);
		memcpy(&tx_buf[15], &p->dry, 2);
		memcpy(&tx_buf[17], &p->wet, 2);

		tx_buf[19] = 0x16;
		Communication_Effect_Send(tx_buf, 20);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;

					if(p->enable)
					{
						AudioEffectAutoWahInit(p,gCtrlVars.sample_rate);
						IsEffectChange = 1;
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;
			case 1:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->modulation_rate != TmpData16)
				{
					p->modulation_rate = TmpData16;
					if(p->enable)
					{
						AudioEffectAutoWahInit(p,gCtrlVars.sample_rate);
						IsEffectChange = 1;
					}
				}
				break;
			case 2:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->min_frequency != TmpData16)
				{
					p->min_frequency = TmpData16;
					if(p->enable)
					{
						AudioEffectAutoWahInit(p,gCtrlVars.sample_rate);
						IsEffectChange = 1;
					}
				}
				break;
			case 3:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->max_frequency != TmpData16)
				{
					p->max_frequency = TmpData16;
					if(p->enable)
					{
						AudioEffectAutoWahInit(p,gCtrlVars.sample_rate);
						IsEffectChange = 1;
					}
				}
				break;

			case 4:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->depth != TmpData16)
				{
					p->depth = TmpData16;
					if(p->enable)
					{
						AudioEffectAutoWahInit(p,gCtrlVars.sample_rate);
						IsEffectChange = 1;
					}
				}
				break;
			case 5:
				memcpy(&TmpData16, &buf[1], 2);
				p->dry = TmpData16;
				break;
			case 6:
				memcpy(&TmpData16, &buf[1], 2);
				p->wet = TmpData16;
				break;

			case 0xff:
				memcpy(&TmpData16, &buf[1], 2);
				p->enable = TmpData16;

				memcpy(&TmpData16, &buf[3], 2);
				p->modulation_rate = TmpData16;

				memcpy(&TmpData16, &buf[5], 2);
				p->min_frequency = TmpData16;

				memcpy(&TmpData16, &buf[7], 2);
				p->max_frequency = TmpData16;

				memcpy(&TmpData16, &buf[9], 2);
				p->depth = TmpData16;

				memcpy(&TmpData16, &buf[11], 2);
				p->dry = TmpData16;

				memcpy(&TmpData16, &buf[13], 2);
				p->wet = TmpData16;

				if(p->enable)
				{
					AudioEffectAutoWahInit(p,gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}
#endif

}

void Communication_Effect_Echo(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	EchoUnit *p = (EchoUnit *)addr;
	uint16_t TmpData16;
    int16_t TmpDataS16;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 6 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->fc, 2);
		memcpy(&tx_buf[9], &p->attenuation, 2);
		memcpy(&tx_buf[11], &p->delay, 2);
		memcpy(&tx_buf[13], &p->direct_sound, 2);
		memcpy(&tx_buf[15], &p->max_delay, 2);

		tx_buf[5+6*2] = 0x16;
		Communication_Effect_Send(tx_buf, 6*2 + 6);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						p->max_delay = p->max_delay > 3000? 3000 : p->max_delay;
						p->max_delay_samples = (p->max_delay*gCtrlVars.sample_rate)/1000;
						p->delay = p->delay > p->max_delay? p->max_delay : p->delay;
						p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;	
						
						AudioEffectEchoInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;

			case 1:
				memcpy(&TmpDataS16, &buf[1], 2);
				if(TmpDataS16 > 24000)
				{
					TmpDataS16 = 24000;
				}
				else if(TmpDataS16 < 0)
				{
					TmpDataS16 = 0;
				}
				if(p->fc != TmpDataS16)
				{
					p->fc = TmpDataS16;
					AudioEffectEchoConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
				}
				break;

			case 2:
				memcpy(&TmpData16, &buf[1], 2);
				TmpData16 = TmpData16 == 0? 1 : TmpData16;
				if(p->attenuation != TmpData16)
				{
					p->attenuation = TmpData16;
				}
				break;

			case 3:
				memcpy(&TmpData16, &buf[1], 2);

				if(TmpData16 > p->max_delay)
				{
					TmpData16 = p->max_delay;
				}
				if(p->delay != TmpData16)
				{
					p->delay = TmpData16;
					p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;
					
				}
				break;

			case 4:
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 1)
				{
					TmpData16 = 1;
				}
				if(p->direct_sound != TmpData16)
				{
					p->direct_sound = TmpData16;
				}
				break;
				
			case 5:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->max_delay != TmpData16)
				{
					p->max_delay = TmpData16;
					
					p->max_delay = p->max_delay > 3000? 3000 : p->max_delay;
					p->max_delay_samples = (p->max_delay*gCtrlVars.sample_rate)/1000;
					p->delay = p->delay > p->max_delay? p->max_delay : p->delay;
					p->delay_samples = (p->delay*gCtrlVars.sample_rate)/1000;	
					
					if(p->enable)
					{
						IsEffectChange = 1;
					}
				}
				break;
				
			case 0xff:
				memcpy(&TmpDataS16, &buf[1], 2);
				p->enable = TmpDataS16;
			
		        memcpy(&TmpDataS16, &buf[3], 2);
				p->fc = TmpDataS16;
				
		        memcpy(&TmpDataS16, &buf[5], 2);
				p->attenuation = TmpDataS16;
				
		        memcpy(&TmpDataS16, &buf[7], 2);
				p->delay = TmpDataS16;
				
		        memcpy(&TmpDataS16, &buf[9], 2);
				p->direct_sound = TmpDataS16;

				memcpy(&TmpDataS16, &buf[11], 2);
				p->max_delay = TmpDataS16;
				
				p->max_delay = p->max_delay > 3000? 3000 : p->max_delay;
				
				p->delay = p->delay > p->max_delay? p->max_delay : p->delay;
				p->delay_samples = p->delay * gCtrlVars.sample_rate / 1000;
				p->max_delay_samples = (p->max_delay*gCtrlVars.sample_rate)/1000;
				
				if(p->enable)					
				{
					AudioEffectEchoInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}
}

void Communication_Effect_VoiceChanger(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	VoiceChangerUnit *p = (VoiceChangerUnit *)addr;
	int16_t TmpData1;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 3 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->pitch_ratio, 2);
		memcpy(&tx_buf[9], &p->formant_ratio, 2);
		tx_buf[5+3*2] = 0x16;
		Communication_Effect_Send(tx_buf, 3*2 + 6);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData1, &buf[1], 2);
				if(p->enable != TmpData1)
				{				
					p->enable = TmpData1;
					if(p->enable)
					{
						AudioEffectVoiceChangerInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;

			case 1:
				memcpy(&TmpData1, &buf[1], 2);
				TmpData1 = TmpData1 > 300?300:TmpData1;
				TmpData1 = TmpData1 < 50?50:TmpData1;
				if(p->pitch_ratio != TmpData1)
				{
					p->pitch_ratio = TmpData1;
					AudioEffectVoiceChangerConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
				}
				break;

			case 2:
				memcpy(&TmpData1, &buf[1], 2);
				TmpData1 = TmpData1 > 200?200:TmpData1;
				TmpData1 = TmpData1 < 66? 66:TmpData1;
				if(p->formant_ratio != TmpData1)
				{
					p->formant_ratio = TmpData1;
					AudioEffectVoiceChangerConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
				}
				break;


			case 0xff:
				memcpy(&TmpData1, &buf[1], 2);
				p->enable = TmpData1;
			    memcpy(&TmpData1, &buf[3], 2);
				p->pitch_ratio = TmpData1;
				memcpy(&TmpData1, &buf[5], 2);
				p->formant_ratio = TmpData1;
				if(p->enable)
				{
					AudioEffectVoiceChangerInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				
				break;
			default:
				break;
		}
	}
}

void Communication_Effect_VoiceChangerPro(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
	VoiceChangerProUnit *p = (VoiceChangerProUnit *)addr;
	int16_t TmpData1;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 3 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->pitch_ratio, 2);
		memcpy(&tx_buf[9], &p->formant_ratio, 2);

		tx_buf[5+3*2] = 0x16;
		Communication_Effect_Send(tx_buf, 3*2 + 6);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData1, &buf[1], 2);
				if(p->enable != TmpData1)
				{
					p->enable = TmpData1;
					if(p->enable)
					{
						AudioEffectVoiceChangerProInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
						else
						{
							IsEffectChange = 1;
						}
					}					
				}
				break;

			case 1:
				memcpy(&TmpData1, &buf[1], 2);

				TmpData1 = TmpData1 > 300?300:TmpData1;
				TmpData1 = TmpData1 < 50?50:TmpData1;

				if( p->pitch_ratio  != TmpData1)
				{
					p->pitch_ratio = TmpData1;
					AudioEffectVoiceChangerProConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
				}
				break;

			case 2:
				memcpy(&TmpData1, &buf[1], 2);

				TmpData1 = TmpData1 > 200?200:TmpData1;
				TmpData1 = TmpData1 < 66? 66:TmpData1;

				if(p->formant_ratio != TmpData1)
				{
					p->formant_ratio = TmpData1;
					AudioEffectVoiceChangerProConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
				}
				break;


			case 0xff:
				memcpy(&TmpData1, &buf[1], 2);
				p->enable = TmpData1;
			
			    memcpy(&TmpData1, &buf[3], 2);
				p->pitch_ratio = TmpData1;				
	
				memcpy(&TmpData1, &buf[5], 2);
				p->formant_ratio = TmpData1;

				if(p->enable)
				{
					AudioEffectVoiceChangerProInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					IsEffectChange = 1;
				}
				break;
			default:
				break;
		}
	}
#endif
}

void Comm_Effect_Expander(ExpanderUnit *p, uint8_t * buf)
{
    uint16_t TmpData16;
	int16_t TmpDataS16;
	
	switch(buf[0])//
	{
		case 0:
			memcpy(&TmpData16, &buf[1], 2);
			if(p->enable != TmpData16)
			{
				p->enable = TmpData16;
				if(p->enable)
				{
					AudioEffectExpanderInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
					if(p->enable)
					{
						IsEffectChange = 1;
					}
				}
				else
				{
					IsEffectChange = 1;
				}
			}
			break;
	
		case 1:
			memcpy(&TmpDataS16, &buf[1], 2);
			if(TmpDataS16 < -9000)
			{
				TmpDataS16 = -9000;
			}
			else if(TmpDataS16 > 0)
			{
				TmpDataS16 = 0;
			}
			if(p->threshold != TmpDataS16)
			{
				p->threshold = TmpDataS16;
				//AudioEffectExpanderInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
				AudioEffectExpanderThresholdConfig(p);
			}
			break;
	
		case 2:
			memcpy(&TmpDataS16, &buf[1], 2);
			if(TmpDataS16 > 1000)
			{
				TmpDataS16 = 1000;
			}
			else if(TmpDataS16 < 1)
			{
				TmpDataS16 = 1;
			}
			if(p->ratio != TmpDataS16)
			{
				p->ratio = TmpDataS16;
				AudioEffectExpanderConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
			}
			break;
	
		case 3:
			memcpy(&TmpDataS16, &buf[1], 2);
			if(TmpDataS16 > 7500)
			{
				TmpDataS16 = 7500;
			}
			else if(TmpDataS16 < 0)
			{
				TmpData16 = 0;
			}
			if(p->attack_time != TmpDataS16)
			{
				p->attack_time = TmpDataS16;
				AudioEffectExpanderConfig(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
			}
			break;
	
		case 4:
			memcpy(&TmpDataS16, &buf[1], 2);
			if(TmpDataS16 > 7500)
			{
				TmpDataS16 = 7500;
			}
			else if(TmpDataS16 < 0)
			{
				TmpDataS16 = 0;
			}
			if(p->release_time != TmpDataS16)
			{
				p->release_time = TmpDataS16;
				AudioEffectExpanderInit(p, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
			}
			break;
		 default:
		 	break;
		}

}
void Commuinication_Effect_Expander(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	ExpanderUnit *p = (ExpanderUnit *)addr;
	//int32_t TmpData;
	uint16_t i,k;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 5 * 2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->threshold, 2);
		memcpy(&tx_buf[9], &p->ratio, 2);
		memcpy(&tx_buf[11], &p->attack_time, 2);
		memcpy(&tx_buf[13], &p->release_time, 2);

		tx_buf[5+5*2] = 0x16;
		Communication_Effect_Send(tx_buf, 5*2 + 6);
	}
	else
	{
		switch(buf[0])//
		{
			case 0xff:
				buf++;
				for(i = 0; i < 5; i++)
				{
				   cbuf[0] = i;////id
				   for(k = 0; k < CTL_DATA_SIZE; k++)
				   {
					   cbuf[ k + 1] = (uint8_t)buf[k];
				   }
			       Comm_Effect_Expander(p,&cbuf[0]);
				   buf += 2;
				}
				break;		
			default:
			    Comm_Effect_Expander(p,buf);
				break;
		}
	}
}

void Communication_Effect_FreqShifter(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{    
    uint8_t  deltaf_idx;
	uint16_t TmpData;
	FreqShifterUnit *p = (FreqShifterUnit *)addr;	

	if(len == 0)//ask
	{
		 memset(tx_buf, 0, sizeof(tx_buf));
	    for(deltaf_idx = 0;deltaf_idx < 8;deltaf_idx++)
		{
			if(p->deltaf == DeltafTable[deltaf_idx])
			{
				break;
			}
		}
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 5;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		tx_buf[7]  = deltaf_idx;

		tx_buf[9] = 0x16;
		Communication_Effect_Send(tx_buf, 10);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData, &buf[1], 2);
				if(p->enable != TmpData)
				{
					p->enable = TmpData;
					if(p->enable)
					{
						AudioEffectFreqShifterInit(p);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;

			case 1:
				memcpy(&TmpData, &buf[1], 2);
				TmpData = TmpData > 8? 8:TmpData;
				if(DeltafTable[TmpData] != p->deltaf)
				{
					p->deltaf = TmpData > 8? DeltafTable[8] : DeltafTable[TmpData];
					AudioEffectFreqShifterConfig(p);
				}
				break;
				
			case 0xff:
				memcpy(&TmpData, &buf[1], 2);
				p->enable = TmpData;
				
				memcpy(&TmpData, &buf[3], 2);
				TmpData = TmpData > 8? 8:TmpData;
				p->deltaf = DeltafTable[TmpData];
			    if(p->enable)
				{
					AudioEffectFreqShifterInit(p);
					IsEffectChange = 1;
				}
				break;
		}
	}
}

void Communication_Effect_HowlingDector(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{    
	uint16_t TmpData;
	HowlingDectorUnit *p = (HowlingDectorUnit *)addr;	

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 5;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		tx_buf[7]  = p->suppression_mode;

		tx_buf[9] = 0x16;
		Communication_Effect_Send(tx_buf, 10);
	}
	else
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData, &buf[1], 2);
				if(p->enable != TmpData)
				{
					p->enable = TmpData;
					if(p->enable)
					{
						AudioEffectHowlingSuppressorInit(p);
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;

			case 1:
				memcpy(&TmpData, &buf[1], 2);
				TmpData = TmpData > 2? 2:TmpData;
				if(p->suppression_mode != TmpData)
				{
					p->suppression_mode = TmpData;
					AudioEffectHowlingSuppressorConfig(p);
				}
				break;
				
			case 0xff:
				memcpy(&TmpData, &buf[1], 2);
				p->enable = TmpData;
				
				memcpy(&TmpData, &buf[3], 2);
				p->suppression_mode = TmpData;
					
				if(p->enable)
				{
					AudioEffectHowlingSuppressorInit(p);
					IsEffectChange = 1;
				}
				break;
		}
	}
}

void Comm_Effect_DRC(DRCUnit * p, uint8_t * buf)
{
    uint16_t TmpData16;
	int16_t TmpDataS16;
	int16_t TmpData1S16;
	int16_t TmpData2S16;

	switch(buf[0])//
	{
		case 0:
			memcpy(&TmpData16, &buf[1], 2);
			if(p->enable != TmpData16)
			{
				p->enable = TmpData16;
				if(p->enable)
				{
					if(p == &gCtrlVars.rec_drc_unit)
					{
						AudioEffectDRCInit(p, 1, gCtrlVars.sample_rate);
					}
					else
					{
						AudioEffectDRCInit(p, 2, gCtrlVars.sample_rate);
					}
					if(p->enable)
					{
						IsEffectChange = 1;
					}
				}
				else
				{
					IsEffectChange = 1;
				}
			}
			break;
	
		case 1:
			memcpy(&TmpData16, &buf[1], 2);
           if(TmpData16 > 20000)
			{
				TmpData16 = 20000;
			}
			if(p->fc != TmpData16)
			{
				p->fc = TmpData16;
				if(p == &gCtrlVars.rec_drc_unit)
				{
					AudioEffectDRCConfig(p, 1, gCtrlVars.sample_rate);
				}
				else
				{
					AudioEffectDRCConfig(p, 2, gCtrlVars.sample_rate);
				}
			}
			break;
	
		case 2:
			memcpy(&TmpData16, &buf[1], 2);
			if(TmpData16 > 6)
			{
				TmpData16 = 6;
			}
	
			if(p->mode != TmpData16)
			{
				p->mode = TmpData16;
				if(p == &gCtrlVars.rec_drc_unit)
				{
					AudioEffectDRCConfig(p, 1, gCtrlVars.sample_rate);
				}
				else
				{
					AudioEffectDRCConfig(p, 2, gCtrlVars.sample_rate);
				}
			}
			break;
	
		case 3:
			memcpy(&TmpDataS16, &buf[1], 2);
			memcpy(&TmpData1S16, &buf[1], 2);
	
			if((TmpDataS16 != p->q[0]) || (TmpData1S16 != p->q[1]))
			{
				p->q[0] = TmpDataS16;
				p->q[1] = TmpData1S16;
				if(p == &gCtrlVars.rec_drc_unit)
				{
					AudioEffectDRCConfig(p, 1, gCtrlVars.sample_rate);
				}
				else
				{
					AudioEffectDRCConfig(p, 2, gCtrlVars.sample_rate);
				}
			}
			break;
	
		case 4:
			memcpy(&TmpDataS16, &buf[1], 2);
			memcpy(&TmpData1S16, &buf[3], 2);
			memcpy(&TmpData2S16, &buf[5], 2);
	
			TmpDataS16 = TmpDataS16 < -9000? -9000 : TmpDataS16;
			TmpDataS16 = TmpDataS16 > 0? 0 : TmpDataS16;
			TmpData1S16 = TmpData1S16 < -9000? -9000 : TmpData1S16;
			TmpData1S16 = TmpData1S16 > 0? 0 : TmpData1S16;
			
			TmpData2S16 = TmpData2S16 < -9000? -9000 : TmpData2S16;
			TmpData2S16 = TmpData2S16 > 0? 0 : TmpData2S16;
	
			if((TmpDataS16 != p->threshold[0]) || (TmpData1S16 != p->threshold[1]) || (TmpData2S16 != p->threshold[2]))
			{
				p->threshold[0] = TmpDataS16;
				p->threshold[1] = TmpData1S16;
				p->threshold[2] = TmpData2S16;
				if(p == &gCtrlVars.rec_drc_unit)
				{
					AudioEffectDRCConfig(p, 1, gCtrlVars.sample_rate);
				}
				else
				{
					AudioEffectDRCConfig(p, 2, gCtrlVars.sample_rate);
				}
			}
			break;
	
		case 5:
			memcpy(&TmpDataS16, &buf[1], 2);
			memcpy(&TmpData1S16, &buf[3], 2);
			memcpy(&TmpData2S16, &buf[5], 2);
	
			TmpDataS16  = TmpDataS16 < 1? 	   1 : TmpDataS16;
			TmpDataS16  = TmpDataS16 > 1000?	1000 : TmpDataS16;
			
			TmpData1S16 = TmpData1S16 < 1?	   1 : TmpData1S16;
			TmpData1S16 = TmpData1S16 > 1000? 1000 : TmpData1S16;
			
			TmpData2S16 = TmpData2S16 < 1?	   1 : TmpData2S16;
			TmpData2S16 = TmpData2S16 > 1000? 1000 : TmpData2S16;
	
			if((TmpDataS16 != p->ratio[0]) || (TmpData1S16 != p->ratio[1]) || (TmpData2S16 != p->ratio[2]))
			{
				p->ratio[0] = TmpDataS16;
				p->ratio[1] = TmpData1S16;
				p->ratio[2] = TmpData2S16;
				if(p == &gCtrlVars.rec_drc_unit)
				{
					AudioEffectDRCConfig(p, 1, gCtrlVars.sample_rate);
				}
				else
				{
					AudioEffectDRCConfig(p, 2, gCtrlVars.sample_rate);
				}
			}
			break;
		case 6:
			memcpy(&TmpDataS16, &buf[1], 2);
			memcpy(&TmpData1S16, &buf[3], 2);
			memcpy(&TmpData2S16, &buf[5], 2);
	
			TmpDataS16  = TmpDataS16 < 0? 	   0 : TmpDataS16;
			TmpDataS16  = TmpDataS16 > 7500?	7500 : TmpDataS16;
			
			TmpData1S16 = TmpData1S16 < 0?	   0 : TmpData1S16;
			TmpData1S16 = TmpData1S16 > 7500? 7500 : TmpData1S16;
			
			TmpData2S16 = TmpData2S16 < 0?	   0 : TmpData2S16;
			TmpData2S16 = TmpData2S16 > 7500? 7500 : TmpData2S16;
	
			if((TmpDataS16 != p->attack_tc[0]) || (TmpData1S16 != p->attack_tc[1]) || (TmpData2S16 != p->attack_tc[2]))
			{
				p->attack_tc[0] = TmpDataS16;
				p->attack_tc[1] = TmpData1S16;
				p->attack_tc[2] = TmpData2S16;
				if(p == &gCtrlVars.rec_drc_unit)
				{
					AudioEffectDRCConfig(p, 1, gCtrlVars.sample_rate);
				}
				else
				{
					AudioEffectDRCConfig(p, 2, gCtrlVars.sample_rate);
				}
			}
			break;
		case 7:
			memcpy(&TmpDataS16, &buf[1], 2);
			memcpy(&TmpData1S16, &buf[3], 2);
			memcpy(&TmpData2S16, &buf[5], 2);
	
			TmpDataS16  = TmpDataS16 < 0? 	   0 : TmpDataS16;
			TmpDataS16  = TmpDataS16 > 7500?	7500 : TmpDataS16;
			
			TmpData1S16 = TmpData1S16 < 0?	   0 : TmpData1S16;
			TmpData1S16 = TmpData1S16 > 7500? 7500 : TmpData1S16;
			
			TmpData2S16 = TmpData2S16 < 0?	   0 : TmpData2S16;
			TmpData2S16 = TmpData2S16 > 7500? 7500 : TmpData2S16;
	
			if((TmpDataS16 != p->release_tc[0]) || (TmpData1S16 != p->release_tc[1]) || (TmpData2S16 != p->release_tc[2]))
			{
				p->release_tc[0] = TmpDataS16;
				p->release_tc[1] = TmpData1S16;
				p->release_tc[2] = TmpData2S16;
				if(p == &gCtrlVars.rec_drc_unit)
				{
					AudioEffectDRCConfig(p, 1, gCtrlVars.sample_rate);
				}
				else
				{
					AudioEffectDRCConfig(p, 2, gCtrlVars.sample_rate);
				}
			}
			break;
	
		case 8:
			memcpy(&TmpData16, &buf[1], 2);
			p->pregain1 = TmpData16;
			break;
	
		case 9:
			memcpy(&TmpData16, &buf[1], 2);
			p->pregain2 = TmpData16;
			break;
		default:
			 break;
	}

}

void Commuinication_Effect_DRC(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	DRCUnit *p = (DRCUnit *)addr;
	uint16_t i,k,s;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 19 * 2 + 1;
		tx_buf[4] = 0xff;

		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->fc, 2);
		memcpy(&tx_buf[9], &p->mode, 2);
		memcpy(&tx_buf[11], &p->q[0], 2);
		memcpy(&tx_buf[13], &p->q[1], 2);
		memcpy(&tx_buf[15], &p->threshold[0], 2);
		memcpy(&tx_buf[17], &p->threshold[1], 2);
		memcpy(&tx_buf[19], &p->threshold[2], 2);
		memcpy(&tx_buf[21], &p->ratio[0], 2);
		memcpy(&tx_buf[23], &p->ratio[1], 2);
		memcpy(&tx_buf[25], &p->ratio[2], 2);
		memcpy(&tx_buf[27], &p->attack_tc[0], 2);
        memcpy(&tx_buf[29], &p->attack_tc[1], 2);
		memcpy(&tx_buf[31], &p->attack_tc[2], 2);
		memcpy(&tx_buf[33], &p->release_tc[0], 2);
		memcpy(&tx_buf[35], &p->release_tc[1], 2);
		memcpy(&tx_buf[37], &p->release_tc[2], 2);
		memcpy(&tx_buf[39], &p->pregain1, 2);
		memcpy(&tx_buf[41], &p->pregain2, 2);

		tx_buf[5+19*2] = 0x16;
		Communication_Effect_Send(tx_buf, 19*2 + 6);
	}
	else
	{
       switch(buf[0])
       {
			case 0xff:
				buf++;
				for(i = 0; i < 10; i++)
				{
					cbuf[0] = i;////id
					s = DrcCommandLen_tab[i];
					for(k = 0; k < s; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_Effect_DRC(p,&cbuf[0]);
					buf += s;
				}
				break;		
			default:
				Comm_Effect_DRC(p,buf);
				break;

		}
	}
}

void Communication_Effect_EQ(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	EQUnit *p = (EQUnit *)addr;
	int16_t TmpData,TmpData1,TmpData2,TmpData3,TmpData4,pos;
	int16_t TmpDataS16;
	uint32_t i;

	if(len == 0)//ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 3 * 2 + 10*5*2 + 1;
		tx_buf[4] = 0xff;
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->pregain, 2);
		memcpy(&tx_buf[9], &p->filter_count, 2);
		
		pos = 11;
		for(i = 0; i < 10; i++)
		{			
			TmpData  = (uint16_t)p->eq_params[i].enable;
			TmpData1 = (uint16_t)p->eq_params[i].type;
			TmpData2 = (uint16_t)p->eq_params[i].f0;
			TmpData3 = (uint16_t)p->eq_params[i].Q;
			TmpData4 = (uint16_t)p->eq_params[i].gain;
			
			memcpy(&tx_buf[pos], &TmpData,  2);
			pos += 2;
			
			memcpy(&tx_buf[pos], &TmpData1, 2);
			pos += 2;
			
			memcpy(&tx_buf[pos], &TmpData2, 2);
			pos += 2;
			
			memcpy(&tx_buf[pos], &TmpData3, 2);
			pos += 2;
			
			memcpy(&tx_buf[pos], &TmpData4, 2);
			pos += 2;		
		 }

		tx_buf[5+3 * 2 + 10*5*2] = 0x16;
		Communication_Effect_Send(tx_buf, 3 * 2 + 10*5*2 + 6);
	}
	else//set
	{
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData, &buf[1], 2);
				if(p->enable != TmpData)
				{
					p->enable = TmpData;
					if(p->enable)
					{
						if(p == &gCtrlVars.rec_eq_unit)
						{
							AudioEffectEQInit(p, 1, gCtrlVars.sample_rate);
						}
						#ifndef CFG_FUNC_MIC_KARAOKE_EN
						else if(p == &gCtrlVars.mic_out_eq_unit)
						{
							AudioEffectEQInit(p, 1, gCtrlVars.sample_rate);
						}
						#endif
						else
						{
							AudioEffectEQInit(p, 2, gCtrlVars.sample_rate);
						}
						if(p->enable)
						{
							IsEffectChange = 1;
						}
					}
					else
					{
						IsEffectChange = 1;
					}
				}
				break;

			case 1:
				memcpy(&TmpDataS16, &buf[1], 2);
				if(p->pregain != TmpDataS16)
				{
					p->pregain = (int32_t)TmpDataS16;
					AudioEffectEQPregainConfig(p);
				}
				break;

			case 2:
				memcpy(&TmpData, &buf[1], 2);
				if(p->filter_count != TmpData)
				{
					p->filter_count = TmpData;
					AudioEffectEQFilterConfig(p, gCtrlVars.sample_rate);
				}
				break;

			case 0xff:
				memcpy(&TmpData, &buf[1], 2);
				if(!p->enable)
				{					
					p->enable = TmpData;
					if(p->enable)
					{
						if(p == &gCtrlVars.rec_eq_unit)
						{
							AudioEffectEQInit(p, 1, gCtrlVars.sample_rate);
						}
						#ifndef CFG_FUNC_MIC_KARAOKE_EN
						else if(p == &gCtrlVars.mic_out_eq_unit)
						{
							AudioEffectEQInit(p, 1, gCtrlVars.sample_rate);
						}
						#endif
						else
						{
							AudioEffectEQInit(p, 2, gCtrlVars.sample_rate);
						}
						IsEffectChange = 1;
					}			
				}				
				p->enable = TmpData;

				memcpy(&TmpData, &buf[3], 2);
				p->pregain = TmpData;

				memcpy(&TmpData, &buf[5], 2);
				p->filter_count = TmpData;

                pos = 0;
                for(i = 0; i < 10; i++)
                {
	                memcpy(&TmpData, &buf[7 + pos], 2);
					memcpy(&TmpData1, &buf[9 + pos], 2);
					memcpy(&TmpData2, &buf[11 + pos], 2);
					memcpy(&TmpData3, &buf[13 + pos], 2);
					memcpy(&TmpData4, &buf[15 + pos], 2);
					pos += 10;
					
					p->eq_params[i].enable   = TmpData;
					p->eq_params[i].type     = TmpData1;
					p->eq_params[i].f0	     = TmpData2;
					p->eq_params[i].Q	     = TmpData3;
					p->eq_params[i].gain     = TmpData4;
                }

				p->filter_count = 0;
				for(i = 0; i < 10; i++)
				{
					if(p->eq_params[i].enable)
					{
						p->filter_params[p->filter_count].Q    = p->eq_params[i].Q;
						p->filter_params[p->filter_count].f0   = p->eq_params[i].f0;
						p->filter_params[p->filter_count].gain = p->eq_params[i].gain;
						p->filter_params[p->filter_count].type = p->eq_params[i].type;
						p->filter_count ++;		 
					}
				}
				AudioEffectEQFilterConfig(p, gCtrlVars.sample_rate);
				break;

			default:
				if(buf[0] < 53)
				{
					uint8_t flg = 0;
					memcpy(&TmpDataS16, &buf[1], 2);
					if(!((buf[0]- 3) % 5))
					{
						if(TmpDataS16 != p->eq_params[(buf[0]-3)/5].enable)
						{
							p->eq_params[(buf[0]-3)/5].enable = TmpDataS16;
							flg = 1;
						}
					}
					else if(!((buf[0]- 4) % 5))
					{
						if(TmpDataS16 != p->eq_params[(buf[0]-4)/5].type)
						{
							p->eq_params[(buf[0]-4)/5].type = TmpDataS16;
							flg = 1;
						}
					}
					else if(!((buf[0]- 5) % 5))
					{
						if(TmpDataS16 != p->eq_params[(buf[0]-5)/5].f0)
						{
							p->eq_params[(buf[0]-5)/5].f0 = TmpDataS16;
							flg = 1;
						}
					}
					else if(!((buf[0]- 6) % 5))
					{
						if(TmpDataS16 != p->eq_params[(buf[0]-6)/5].Q)
						{
							p->eq_params[(buf[0]-6)/5].Q = TmpDataS16;
							flg = 1;
						}
					}
					else
					{
						if(TmpDataS16 != p->eq_params[(buf[0]-7)/5].gain)
						{
							p->eq_params[(buf[0]-7)/5].gain = TmpDataS16;
							flg = 1;
						}
					}
					if(flg)
					{
						p->filter_count = 0;
						for(i = 0; i < 10; i++)
						{
							if(p->eq_params[i].enable)
							{
								p->filter_params[p->filter_count].Q    = p->eq_params[i].Q;
								p->filter_params[p->filter_count].f0   = p->eq_params[i].f0;
								p->filter_params[p->filter_count].gain = p->eq_params[i].gain;
								p->filter_params[p->filter_count].type = p->eq_params[i].type;
								p->filter_count ++;
							}
						}
						AudioEffectEQFilterConfig(p, gCtrlVars.sample_rate);
					}
				}
				break;
		}
	}
}


void Communication_Effect_AEC(uint8_t Control, uint32_t addr, uint8_t *buf, uint32_t len)
{
	AecUnit *p = (AecUnit *)addr;
	int16_t TmpData16;
	//int16_t TmpDataS16;
	int8_t 	TmpData8;

	APP_DBG("addr = %x\n", addr);
	if(len == 0)//ask
	{
		APP_DBG("Communication_Effect_AEC ask\n");
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = Control;
		tx_buf[3] = 0x27;
		tx_buf[4] = 0xf0;

		APP_DBG("p->param_cnt = %d\n", p->param_cnt);
		memcpy(&tx_buf[5], &p->enable, 2);
		memcpy(&tx_buf[7], &p->param_cnt, 1);
		memcpy(&tx_buf[8], &p->param1_name_len, 1);
		//memcpy(&tx_buf[9], &p->param1_name, p->param1_name_len);
		memcpy(&tx_buf[9], &p->param1_name, 9);
		memcpy(&tx_buf[18], &p->param1_state, 1);
		memcpy(&tx_buf[19], &p->param1_value_min, 2);
		memcpy(&tx_buf[21], &p->param1_value_max, 2);
		memcpy(&tx_buf[23], &p->es_level, 2);
		memcpy(&tx_buf[25], &p->param2_name_len, 1);
		memcpy(&tx_buf[26], &p->param2_name, 10);
		memcpy(&tx_buf[36], &p->param2_state, 1);
		memcpy(&tx_buf[37], &p->param2_value_min, 2);
		memcpy(&tx_buf[39], &p->param2_value_max, 2);
		memcpy(&tx_buf[41], &p->ns_level, 2);
		
		tx_buf[0x27 + 4] = 0x16;
		Communication_Effect_Send(tx_buf, 44);
	}
	else
	{
		APP_DBG("Communication_Effect_AEC write\n");
		APP_DBG("buf[0] = %d\n", buf[0]);
		switch(buf[0])//
		{
			case 0:
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						AudioEffectAecInit(p, 1, 16000);//固定为16K采样率
						if(p->enable)
						{
							//IsEffectChange = 1;				
						}
					}
					else
					{
						//IsEffectChange = 1;
					}
				}
				break;
			case 1:				
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 5)
				{
					TmpData16 = 5;
				}
				if(p->es_level != TmpData16)
				{
					p->es_level = TmpData16;
					AudioEffectAecInit(p, 1, 16000);//固定为16K采样率
				}
				break;	
			case 2:				
				memcpy(&TmpData16, &buf[1], 2);
				if(TmpData16 > 5)
				{
					TmpData16 = 5;
				}
				if(p->ns_level != TmpData16)
				{
					p->ns_level = TmpData16;
					AudioEffectAecInit(p, 1, 16000);//固定为16K采样率
				}
				break;	
			case 0xf0://自定义音效，全部参数为0xF0	
				memcpy(&TmpData16, &buf[1], 2);
				if(p->enable != TmpData16)
				{
					p->enable = TmpData16;
					if(p->enable)
					{
						//AudioEffectAecInit(p, 1, 16000);//固定为16K采样率
						if(p->enable)
						{
							//IsEffectChange = 1;				
						}
					}
					else
					{
						//IsEffectChange = 1;
					}
				}
				memcpy(&TmpData8, &buf[3], 1);
				p->param_cnt = TmpData8;
				memcpy(&TmpData8, &buf[4], 1);
				p->param1_name_len = TmpData8;
				memcpy(p->param1_name, &buf[5], p->param1_name_len);
				memcpy(&TmpData8, &buf[14], 1);
				p->param1_state = TmpData8;
				memcpy(&TmpData16, &buf[15], 2);
				p->param1_value_min = TmpData16;
				memcpy(&TmpData16, &buf[17], 2);
				p->param1_value_max = TmpData16;
				memcpy(&TmpData16, &buf[19], 2);
				p->es_level = TmpData16;
//				if(p->enable)
//				{
//					AudioEffectAecInit(p, 1, 16000);//固定为16K采样率
//				}

				memcpy(&TmpData8, &buf[21], 1);
				p->param2_name_len = TmpData8;
				memcpy(p->param2_name , &buf[22], p->param2_name_len);
				memcpy(&TmpData8 , &buf[32], 1);
				p->param2_state = TmpData8;
				memcpy(&TmpData16, &buf[33], 2);
				p->param2_value_min = TmpData16;
				memcpy(&TmpData16, &buf[35], 2);
				p->param2_value_max = TmpData16;
				memcpy(&TmpData16, &buf[37], 2);
				p->ns_level = TmpData16;

				if(p->enable)
				{
					AudioEffectAecInit(p, 1, 16000);//固定为16K采样率
				}
				break;
			default:
				break;
		}
	}
}


void Communication_Effect_After_0x80(uint8_t Control, uint8_t *buf, uint32_t len)
{
	switch(effect_list[Control - 0x81])
	{
		case 0:
			Communication_Effect_AutoTune(Control, effect_addr[Control - 0x81], buf, len);
			break;

		case 2:
			Commuinication_Effect_DRC(Control, effect_addr[Control - 0x81], buf, len);
			break;

		case 3:
			Communication_Effect_Echo(Control, effect_addr[Control - 0x81], buf, len);
			break;

		case 4:
			Communication_Effect_EQ(Control, effect_addr[Control - 0x81], buf, len);
			break;

		case 5:
			Commuinication_Effect_Expander(Control, effect_addr[Control - 0x81], buf, len);
			break;
			
        case 6:
			Communication_Effect_FreqShifter(Control, effect_addr[Control - 0x81], buf, len);	
			break;

		case 7:
			Communication_Effect_HowlingDector(Control, effect_addr[Control - 0x81], buf, len);
			break;
		
		case 9:
			Communication_Effect_PitchShifter(Control, effect_addr[Control - 0x81], buf, len);
			break;

		case 10:
			Communication_Effect_Reverb(Control, effect_addr[Control - 0x81], buf, len);
			break;
		
		case 11:
			Communication_Effect_SilenceDector(Control, effect_addr[Control - 0x81], buf, len);
			break;
		
        case 12:
			Communication_Effect_ThreeD(Control, effect_addr[Control - 0x81], buf, len);
			break;
		
		case 13:
			Communication_Effect_VB(Control, effect_addr[Control - 0x81], buf, len);
			break;
		
		case 14:
			Communication_Effect_VoiceChanger(Control, effect_addr[Control - 0x81], buf, len);
			break;

		case 15:
			Communication_Effect_GainControl(Control, effect_addr[Control - 0x81], buf, len);
			break;
		
		case 16:
			Communication_Effect_Vocal_Cut(Control, effect_addr[Control - 0x81], buf, len);
			break;

        case 17:
			Communication_Effect_PlateReverb(Control, effect_addr[Control - 0x81], buf, len);
			break;
		
        case 18:
			Communication_Effect_ReverbPro(Control, effect_addr[Control - 0x81], buf, len);	
			break;
			
		case 19:
			Communication_Effect_VoiceChangerPro(Control, effect_addr[Control - 0x81], buf, len);
			break;	
			
		case 20:
			Communication_Effect_Phase(Control, effect_addr[Control - 0x81], buf, len);
			break;	
			
		case 21:
			Communication_Effect_Vocal_Remove(Control, effect_addr[Control - 0x81], buf, len);
			break;	
			
		case 22:
			Communication_Effect_PitchShifterPro(Control, effect_addr[Control - 0x81], buf, len);
			break;
			
		case 23:
			Communication_Effect_VBClassic(Control, effect_addr[Control - 0x81], buf, len);
			break;	

		case 24:
			Communication_Effect_Delay(Control, effect_addr[Control - 0x81], buf, len);
			break;
			
		case 25:
			Communication_Effect_HarmonicExciter(Control, effect_addr[Control - 0x81], buf, len);
			break;
			
		case 26:
			Communication_Effect_Chorus(Control, effect_addr[Control - 0x81], buf, len);
			break;
			
		case 27:
			Communication_Effect_AutoWah(Control, effect_addr[Control - 0x81], buf, len);
			break;

		case 28:
			Communication_Effect_StereoWindener(Control, effect_addr[Control - 0x81], buf, len);
			break;

		case 29:
			Communication_Effect_PingPong(Control, effect_addr[Control - 0x81], buf, len);
			break;
			
		case 30:
			Communication_Effect_ThreeDPlus(Control, effect_addr[Control - 0x81], buf, len);
			break;

		case 100:
			Communication_Effect_AEC(Control, effect_addr[Control - 0x81], buf, len);
			break;

		default:
			break;
	}
}

void Communication_Effect_Config(uint8_t Control, uint8_t *buf, uint32_t len)
{
	switch(Control)
	{
		case 0x00:
			Communication_Effect_0x00();
			break;
		
		case 0x01:
			Communication_Effect_0x01(buf, len);
			break;	
		
	    case 0x02:
			Communication_Effect_0x02();
			break;
		
		case 0x03:
			Communication_Effect_0x03(buf, len);
			break;

		case 0x04:
			Communication_Effect_0x04(buf, len);
			break;

		case 0x06:
			Communication_Effect_0x06(buf, len);
			break;

		case 0x07:
			Communication_Effect_0x07(buf, len);
			break;
		
		case 0x08:
			Communication_Effect_0x08(buf, len);
			break;

		case 0x09:
			Communication_Effect_0x09(buf, len);
			break;

		case 0x0A:
			Communication_Effect_0x0A(buf, len);
			break;
		
		case 0x0B:
			Communication_Effect_0x0B(buf, len);
			break;
		
		case 0x0C:
			Communication_Effect_0x0C(buf, len);
			break;

		case 0x0D:
			Communication_Effect_0x0D(buf, len);
			break;
			
  		case 0x11://用户自定义烧录数据处理
	        Communication_Effect_0x11(buf, len);
	        break;
			
		case 0x80:
			Communication_Effect_0x80(buf, len);
			break;
			
		case 0xfc://user define tag
			 Communication_Effect_0xfc(buf, len);
			break;

		case 0xff:
			Communication_Effect_0xff(buf, len);
			break;		

		default:		
			if((Control >= 0x81) && (Control < 0xfe))
			{
			    #ifdef FUNC_OS_EN
				osMutexLock(AudioEffectMutex);
			    #endif
				Communication_Effect_After_0x80(Control, buf, len);
				#ifdef FUNC_OS_EN
				osMutexUnlock(AudioEffectMutex);
				#endif

				if(GetSystemMode() == AppModeBtHfPlay)//BT HFP帧长固定
				{
					break;
				}
				
			    if(gCtrlVars.howling_dector_unit.enable)
				{
					gCtrlVars.SamplesPerFrame = 256;
				}
				#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
				else if(gCtrlVars.voice_changer_pro_unit.enable)
				{
					gCtrlVars.SamplesPerFrame = 256;
				}
				#endif
				else if(gCtrlVars.vocal_remove_unit.enable)
				{
					gCtrlVars.SamplesPerFrame = 256;
				}
				#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
				else if(gCtrlVars.reverb_pro_unit.enable)
				{
					gCtrlVars.SamplesPerFrame = 256;
				}
				#endif
			    else if(gCtrlVars.pitch_shifter_unit.enable)
				{
					gCtrlVars.SamplesPerFrame = CFG_MIC_PITCH_SHIFTER_FRAME_SIZE / 2;
				}
			    else if(gCtrlVars.auto_tune_unit.enable)
				{
					gCtrlVars.SamplesPerFrame = 256;
				}
			    else
				{
					gCtrlVars.SamplesPerFrame = CFG_PARA_MIN_SAMPLES_PER_FRAME;
				}
				#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
				if(gCtrlVars.pitch_shifter_pro_unit.enable)
				{
					gCtrlVars.SamplesPerFrame = CFG_MUSIC_PITCH_SHIFTER_PRO_FRAME_SIZE;
				}
				#endif
				if(gCtrlVars.voice_changer_unit.enable)
				{
					gCtrlVars.SamplesPerFrame = VC_FRAME_SIZE;
				}

				//音效整体处理流程完善之后才能这样处理
				////AEC 帧长固定为256，后续有调音的情况下再做调整，sam，20190725
				//if(gCtrlVars.mic_aec_unit.enable)
				//{
				//	gCtrlVars.SamplesPerFrame = 256;
				//	APP_DBG("AEC samples = 256\n");
				//}
				
				if(gCtrlVars.SamplesPerFrame != mainAppCt.SamplesPreFrame)
				{
					if(len > 0)//上位机正在设置
					{
						IsEffectSampleLenChange = 1;
					}
				}
				//if(IsEffectChange)
				//{
				//	if(len == 0)//上位机正在查询
				//	{
				//		IsEffectChange = 0;
				//	}
				//}
				//else if(len > 0)
				//{
				//	IsEffectChange = 1;
				//}
				TimeOutSet(&EffectChangeTimer, 500);
			}	
			break;
	}
	//-----Send ACK ----------------------//
	if(Control > 0xf0)///有效控制寄存器范围   这句解决加调音参数加密问题
	{
		return;
	}
	
	if((Control > 2)&&(Control != 0x80))
	{
		if(len > 0)// if(len = 0) {polling all parameter}
		{
			if(Control == 0x11) //用户自定义烧录数据处理 
			{
				Communication_Effect_Send(tx_buf, 1);
				return;
			}
			memset(tx_buf, 0, sizeof(tx_buf));
			tx_buf[0] = Control;
			Communication_Effect_Send(tx_buf, 1);
		}
	}
}

void EffectChange(void)
{
	if(!IsTimeOut(&EffectChangeTimer))
	{
		return;
	}
	TimeOutSet(&EffectChangeTimer, 500);
	if(!SoftFlagGet(SoftFlagFrameSizeChange))
	{
		if(GetSystemMode() == AppModeBtHfPlay)//BT HFP帧长固定
		{
			return;
		}
		if(IsEffectSampleLenChange)
		{
			APP_DBG("IsEffectSampleLenChange\n");
			if(gCtrlVars.SamplesPerFrame != mainAppCt.SamplesPreFrame)
			{
				SamplesFrameUpdataMsg();//发现帧变化
				IsEffectSampleLenChange = 0;
				IsEffectChange = 0;
			}
		}

		if(IsEffectChange)
		{
			APP_DBG("IsEffectChange\n");
			EffectUpdataMsg();
			IsEffectChange = 0;
			IsEffectSampleLenChange = 0;
		}
	}
}

void Communication_Effect_Process(uint32_t sum_len)
{
	uint32_t i;
	uint32_t packet_len;
	uint8_t  save_flg =0;

	if(sum_len >= 5)
	{
		for(i = 0; i < sum_len; i++)
		{
			if(communic_buf[i] == 0xA5)
			{
				if(i + 4 > sum_len)
				{
					save_flg = 1;
				}
				else
				{
					if(communic_buf[i+1] == 0x5A)
					{
						packet_len = communic_buf[i+3];
						if(i + 5 + packet_len > sum_len)
						{
							save_flg = 1;
						}
						else
						{
							if(communic_buf[i+4+packet_len] == 0x16)
							{
							   Communication_Effect_Config(communic_buf[i+2], &communic_buf[i+4], communic_buf[i+3]);
								//GetAudioEffectMaxValue();
								i += (packet_len + 4);
							}
						}
					}
				}

				if(save_flg)
				{
					memcpy(communic_buf, &communic_buf[i], sum_len - i);
					communic_buf_w = sum_len - i;
					break;
				}
			}
		}
		if(i >= sum_len)
		{
			communic_buf_w = 0;
		}
	}
	else
	{
		communic_buf_w = sum_len;
	}
}

#ifdef CFG_COMMUNICATION_BY_UART
void UART1_Communication_Init(uint8_t *s_tx_buf, uint32_t s_tx_len, uint8_t *s_rx_buf, uint32_t s_rx_len)
{
	GPIO_PortAModeSet(CFG_UART_COMMUNICATION_TX_PIN, CFG_UART_COMMUNICATION_TX_PIN_MUX_SEL); //A10 UART TX
	GPIO_PortAModeSet(CFG_UART_COMMUNICATION_RX_PIN, CFG_UART_COMMUNICATION_RX_PIN_MUX_SEL); //A9 UART RX
	UARTS_Init(1, 115200, 8, 0, 1);

	if(DMA_CircularConfig(PERIPHERAL_ID_UART1_RX, 0, s_tx_buf, s_tx_len) != DMA_OK)
	{
		return;
	}
	DMA_ChannelEnable(PERIPHERAL_ID_UART1_RX);
	UART1_IOCtl(UART_IOCTL_DMA_RX_EN, 1);


	if(DMA_CircularConfig(PERIPHERAL_ID_UART1_TX, 0, s_rx_buf, s_rx_len) != DMA_OK)
	{
		return;
	}
	DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);
    UART1_IOCtl(UART_IOCTL_DMA_TX_EN, 1);
}

void UART1_Communication_Receive_Process(void)
{
	uint32_t len;
	len = DMA_CircularDataLenGet(PERIPHERAL_ID_UART1_RX);
	if(len > 0)
	{
		len = (len + communic_buf_w) > sizeof(communic_buf)? (sizeof(communic_buf) - communic_buf_w) : len;
		DMA_CircularDataGet(PERIPHERAL_ID_UART1_RX, &communic_buf[communic_buf_w], len);

//		APP_DBG("Message: ");
//		for(i = 0; i < len; i++)
//		{
//			APP_DBG("0x%02x, ", communic_buf[communic_buf_w+i]);
//		}
//		APP_DBG("\n");
		Communication_Effect_Process(communic_buf_w + len);
		gCtrlVars.IsEffectChangedByPcTool = 1;
	}
}

void UART_Communication_Heart_Message(void)
{
	Communication_Effect_0x00();
}
#endif


void HIDUsb_Rx(uint8_t *buf,uint16_t len)
{
#ifdef CFG_COMMUNICATION_BY_USB
	 UsbLoadAudioMode(len,buf);
#endif
}
/*
****************************************************************
*  对导入的调音参数做合法性判断处理
*
*
****************************************************************
*/
bool  AudioEffectListJudge(uint16_t len, const uint8_t *pbuff)
{	 
	uint16_t cmd_len,packet_len,k,i;
	uint8_t end_code,Control,eff_total;
	uint8_t *buff;
	uint16_t position;
	buff = (uint8_t *)pbuff;

	if(buff ==NULL)
	{
		return FALSE;
	}

    for(i = 0; i < sizeof(eff_addr_compare)/sizeof(eff_addr_compare[0]);i++)
	{
		eff_addr_compare[i] = 0x00;
	}

	position = 0;
	eff_total = 0;
	
	while(position < len)
	{
		if ((buff[position] ==0xa5) && (buff[position+1] ==0x5a))
		{
			Control   = buff[position + 2];//
			packet_len = buff[position + 3];
			end_code	= buff[position + 4 + packet_len];//0x16

			if(end_code == 0x16)////end code ok
			{
				cmd_len = packet_len + 5;

				for(k = 0; k < cmd_len; k++)//get signal COMMAND
				{
					communic_buf[k] = buff[position++];
				}

				Control = communic_buf[2];
				//APP_DBG("%02X\n",Control);
				if((Control >= 0x81) && (Control < 0xfe))
				{
					if(Control != 0xfc)///user define tag
					{
						eff_addr_compare[eff_total++] = Control;
					}
				}
			}
			else  // end code error
			{
				position++;
			}
		}
		else // serch start code....
		{
			position++;
		}
	}
    
	if(eff_total != effect_sum)
	{
		APP_DBG("audio effect list illega,read list= %d, local_list = %d\n",eff_total,effect_sum);
		return FALSE;
	}
	
	for(i = 0; i < effect_sum;i++)
	{
		//APP_DBG("list read_list= %d, local_list = %ld\n",eff_addr[i],effect_list_addr[i]);
		if( eff_addr_compare[i]!= effect_list_addr[i])
		{
			APP_DBG("audio effect list not equal\n");
			return FALSE;
		}
	}
    
  	APP_DBG("audio effect OK \n");
	
	return TRUE;
}

/*
****************************************************************
* 导入调音参数，并解析处理
*
*
****************************************************************
*/
void LoadAudioMode(uint16_t len,const uint8_t *buff, uint8_t init_flag)
{
	uint16_t cmd_len,packet_len,k;
	uint8_t end_code,Control;
	uint16_t position;

	//StartWriteCmd= LOAD_BUSY;//disable init command pares,curr is local command,not PC command,
	if(buff == NULL)
	{
		return;
	}
	//加锁，防止频繁切换模式并连调音工具时出现音效参数解析异常问题出现
	if(LoadAudioParamMutex != NULL)
	{
		osMutexLock(LoadAudioParamMutex);
	}
	
	memset(communic_buf ,0,sizeof(communic_buf));
	communic_buf_w = 0;
	position = 0;
	while(position < len)
	{
		if ((buff[position] ==0xa5) && (buff[position+1] ==0x5a))
		{
			Control   = buff[position + 2];//
			packet_len = buff[position + 3];
			end_code   = buff[position + 4 + packet_len];//0x16

			if(end_code == 0x16)////end code ok
			{
				cmd_len = packet_len + 5;

				for(k = 0; k < cmd_len; k++)//get signal COMMAND
				{
					communic_buf[k]=buff[position++];
					//APP_DBG("%02X ",communic_buf[k]);
				}
				if(init_flag == 0)// only hardware init
				{
					if(Control < 0x80)
				    {
				       Communication_Effect_Config(communic_buf[2], &communic_buf[4], communic_buf[3]);
				    }
				}
				else if(init_flag == 1)///only effect init
				{
					if(Control > 0x80)
					{
					  Communication_Effect_Config(communic_buf[2], &communic_buf[4], communic_buf[3]);
					  GetAudioEffectMaxValue();
					}
				}
				else/// hardware + effect init
				{
					Communication_Effect_Config(communic_buf[2], &communic_buf[4], communic_buf[3]);
					GetAudioEffectMaxValue();
					WDG_Feed();
				}

				for(k = 0; k < cmd_len; k++)//
				{
					communic_buf[k]=0;
				}
				communic_buf_w = 0;
			}
			else  // end code error
			{
				position++;
			}
	 	}
		else // serch start code....
		{
			position++;
		}
	}
	//解锁，防止频繁切换模式并连调音工具时出现音效参数解析异常问题出现
	if(LoadAudioParamMutex != NULL)
	{
		osMutexUnlock(LoadAudioParamMutex);
	}
	
	gCtrlVars.AutoRefresh = 1;//////调音时模式发生改变，上位机会自动读取音效数据，1=允许上位读，0=不需要上位机读取
	TimeOutSet(&EffectChangeTimer, 5);//临时修改方案，保证功能模式切换时，能快速解析调音参数，优化声音突变问题。
	vTaskDelay(10);
}
/*
****************************************************************
* USB导入调音参数，并解析处理
*
*
****************************************************************
*/
void UsbLoadAudioMode(uint16_t len,uint8_t *buff)
{
#ifdef CFG_COMMUNICATION_BY_USB

	uint16_t cmd_len,packet_len,k;
	uint8_t end_code;
	uint16_t position;

	//加锁，防止频繁切换模式并连调音工具时出现音效参数解析异常问题出现
	if(LoadAudioParamMutex != NULL)
	{
		osMutexLock(LoadAudioParamMutex);
	}
	
	for(k = 0; k < sizeof(communic_buf); k++)//
	{
		communic_buf[k]=0;
	}
	
	communic_buf_w = 0;
	position = 0;
	while(position < len)
	{
		if ((buff[position] ==0xa5) && (buff[position+1] ==0x5a))
		{
			packet_len = buff[position + 3];

			end_code   = buff[position + 4 + packet_len];//0x16

			if(end_code == 0x16)////end code ok
			{
				cmd_len = packet_len + 5;
				communic_buf_w = 0;

				for(k = 0; k < cmd_len; k++)//get signal COMMAND
				{
					communic_buf[k]=buff[position++];
				}

				Communication_Effect_Config(communic_buf[2], &communic_buf[4], communic_buf[3]);
				gCtrlVars.IsEffectChangedByPcTool = 1;
                //SaveAudioParamasToRam(&communic_buf[0],communic_buf[3] + 5);
				for(k = 0; k < cmd_len; k++)//
				{
					communic_buf[k]=0;
				}

				communic_buf_w = 0;
			}
			else  // end code error
			{
				position++;
			}
	 	}
		else // serch start code....
		{
			position++;
		}
	}
	//解锁，防止频繁切换模式并连调音工具时出现音效参数解析异常问题出现
	if(LoadAudioParamMutex != NULL)
	{
		osMutexUnlock(LoadAudioParamMutex);
	}
#endif	
}

#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
#define EQ_PARAM_LEN   112
/*
****************************************************************
* EQ 参数加载函数
*
*
****************************************************************
*/
void LoadEqMode(const uint8_t *buff)
{
	uint16_t cmd_len,packet_len,k;
	uint8_t end_code,Control;
	uint16_t position;
	uint32_t ctl_addr;

	memset(communic_buf ,0,sizeof(communic_buf));

	communic_buf_w = 0;
	position = 0;
//-----------get eq control word---------------------------//
	ctl_addr = (uint32_t)&gCtrlVars.music_out_eq_unit;
	for(k = 0; k < effect_sum; k++)
	{
		if(effect_addr[k] == ctl_addr)
         {
			Control = effect_list_addr[k];
			break;
         }
	}

    if(k == effect_sum)
    {
    	APP_DBG("No EQ Control word\n");
    	return;
    }

	if(effect_list[k] != 4)
    {
    	APP_DBG("EQ Control word Err\n");
    	return;
    }
//--------------------------------------------------------------------------------//

	while(position < EQ_PARAM_LEN)
	{
		if ((buff[position] ==0xa5) && (buff[position+1] ==0x5a))
		{
			//Control   = buff[position + 2];//
			packet_len = buff[position + 3];
			end_code   = buff[position + 4 + packet_len];//0x16

			if(end_code == 0x16)////end code ok
			{
				cmd_len = packet_len + 5;

				for(k = 0; k < cmd_len; k++)//get signal COMMAND
				{
					communic_buf[k]=buff[position++];
					//APP_DBG("%02X ",communic_buf[k]);
				}
				communic_buf[2] = Control;
				Communication_Effect_Config(communic_buf[2], &communic_buf[4], communic_buf[3]);

				for(k = 0; k < cmd_len; k++)//
				{
					communic_buf[k]=0;
				}

				communic_buf_w = 0;
			}
			else  // end code error
			{
				position++;
			}
	 	}
		else // serch start code....
		{
			position++;
		}
	}		
	gCtrlVars.AutoRefresh = 1;//////调音时模式发生改变，上位机会自动读取音效数据，1=允许上位读，0=不需要上位机读取
	TimeOutSet(&EffectChangeTimer, 5);//临时修改方案，保证功能模式切换时，能快速解析调音参数，优化声音突变问题。
	vTaskDelay(10);
}
#endif
#endif

