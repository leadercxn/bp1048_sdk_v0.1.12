#ifndef __REVERB_P_H__
#define __REVERB_P_H__

//注意:该库基于reverb pro库和src库
//SRC库版本要更新到v2.7.2版本

int32_t reverb_pro_init_p(
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


int32_t reverb_pro_apply_p(uint8_t *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n);


#endif