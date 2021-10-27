/**
*************************************************************************************
* @file	    reverb_pro.h
* @brief	Professional stereo reverberation effect
*
* @author	ZHAO Ying (Alfred)
* @version	v1.2.3
*
* &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
*************************************************************************************
*/

#ifndef __REVERB_PRO_H__
#define __REVERB_PRO_H__

#include <stdint.h>

#define MAX_REVERB_PRO_CONTEXT_SIZE_32000 152836 // maximum context size required in bytes for 32000Hz
#define MAX_REVERB_PRO_CONTEXT_SIZE_44100 169712 // maximum context size required in bytes for 44100Hz
#define MAX_REVERB_PRO_CONTEXT_SIZE_48000 175468 // maximum context size required in bytes for 48000Hz

/** error code for reverb */
typedef enum _REVERB_PRO_ERROR_CODE
{
    REVERB_PRO_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,    
    REVERB_PRO_ERROR_ILLEGAL_PARAMETER_LEVEL,
    REVERB_PRO_ERROR_ILLEGAL_PARAMETER_EARLY_REFLECTION,
    REVERB_PRO_ERROR_ILLEGAL_PARAMETER_LATE_REVERB,
    REVERB_PRO_ERROR_PERMISSION_NOT_GRANTED,

    // No Error
    REVERB_PRO_ERROR_OK = 0,					/**< no error              */
} REVERB_PRO_ERROR_CODE;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

   /**
    * @brief Initialize stereo reverberation effect module.
    * @param ct             Pointer to the reverb context object. 
    *        The context object can be allocated to a global buffer or a buffer created from heap
    *        or any free space in memory.
    *        The maximum context size used is no more than MAX_REVERB_PRO_CONTEXT_SIZE_32000, 
    *        MAX_REVERB_PRO_CONTEXT_SIZE_44100 or MAX_REVERB_PRO_CONTEXT_SIZE_48000 for sample rate 
    *        at 32kHz, 44.1kHz or 48kHz respectively.
    *        The actual context size used is related to the values of reverb parameters "erfactor" & "delay" 
    *        and can be checked by calling reverb_pro_context_size() after successful initialization.
    *
    * @param sample_rate    Sample rate. Only 3 sample rates are supported, i.e. 32000, 44100 & 48000 Hz.
    *
    * ---------- Reverb Parameters ----------
    *
    *           [Level]
    * @param dry            final dry mix [-70 to 10] dB
    * @param wet            final wet(late reverb) mix [-70 to 10] dB
    * @param erwet          final wet(early reflection) mix [-70 to 10] dB
    *
    *           [Early Reflection]
    * @param erfactor       early reflection factor [50 to 250] %
    * @param erwidth        early reflection width [-100 to 100] %
    * @param ertolate       early reflection amount [0 to 100] %
    *
    *           [Late Reverb]
    * @param rt60           reverb time decay [100 to 15000] millisecond
    * @param delay          amount of delay [0 to 100] millisecond
    * @param width          width of reverb L/R mix [0 to 100] %
    *
    * @param wander         LFO(low-frequency oscillator) wander amount [10 to 60] %
    * @param spin           LFO(low-frequency oscillator) spin amount [0 to 1000] %
    *
    * @param inputlpf       lowpass cutoff for input [200 to 18000] Hz
    * @param damplpf        lowpass cutoff for dampening [200 to 18000] Hz
    * @param basslpf        lowpass cutoff for bass [50 to 1050] Hz
    * @param bassb          bass boost [0 to 50] %
    * @param outputlpf      lowpass cutoff for output [200 to 18000] Hz
    */
    int32_t reverb_pro_init(
        uint8_t *ct, 
        int32_t sample_rate,
        int32_t dry,
        int32_t wet,
        int32_t erwet,
        int32_t erfactor,
        int32_t erwidth,
        int32_t ertolate,
        int32_t rt60,
        int32_t delay,
        int32_t width,
        int32_t wander,
        int32_t spin,
        int32_t inputlpf,
        int32_t damplpf,
        int32_t basslpf,
        int32_t bassb,
        int32_t outputlpf);

   /**
    * @brief Apply stereo reverberation effect to a frame of PCM data (must be stereo, i.e. 2 channels)
    * @param ct Pointer to the reverb context object.
    * @param pcm_in PCM input buffer. The PCM layout is like "L,R,L,R,..."
    * @param pcm_out PCM output buffer. The PCM layout is like "L,R,L,R,..."
    *        pcm_out can be the same as pcm_in. In this case, the PCM data is changed in-place.
    * @param n Number of PCM samples to process. Note that one (L,R) pair is counted as 1.
    * @return error code. REVERB_PRO_ERROR_OK means successful, other codes indicate error.
    */
    int32_t reverb_pro_apply(uint8_t *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n);


   /**
    * @brief Obtain the size of reverb context.
    * @param ct Pointer to the reverb context object.
    * @return the size of reverb context in bytes.
    * @note The returned size is only valid after reverb_pro_init() is called.
    */
    int32_t reverb_pro_context_size(uint8_t *ct);


   /**
	* @brief Configure stereo reverberation effect's level parameters.
	* @param dry            final dry mix [-70 to 10] dB
	* @param wet            final wet(late reverb) mix [-70 to 10] dB
	* @param erwet          final wet(early reflection) mix [-70 to 10] dB
	* @return error code. REVERB_PRO_ERROR_OK means successful, other codes indicate error.
	* @note Do not call this function before reverb_pro_init() is called.
	*/
	int32_t reverb_pro_configure_level(uint8_t *ct, int32_t dry, int32_t wet, int32_t erwet);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif // __REVERB_PRO_H__
