
#ifndef __AI_RESAMPLE_H__
#define __AI_RESAMPLE_H__

#include <string.h>
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

//ת������ʼ��
void ai_resample_init(void);

//ת����
uint16_t ai_resample_applay(int16_t* pcm_in, int16_t* pcm_out, uint16_t n)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
