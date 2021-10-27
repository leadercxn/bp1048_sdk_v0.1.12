#ifndef __DECODER_TYPE_H__
#define __DECODER_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

#ifndef CFG_APP_CONFIG
#define USE_MP3_DECODER
//#define USE_WMA_DECODER
#define USE_SBC_DECODER
//#define USE_WAV_DECODER
//#define USE_FLAC_DECODER
//#define USE_AAC_DECODER
//#define USE_AIF_DECODER
//#define USE_AMR_DECODER
//#define USE_APE_DECODER
#endif


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//_
