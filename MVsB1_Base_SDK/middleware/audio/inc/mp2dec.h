#ifndef __MP2_DEC_H__
#define __MP2_DEC_H__
#include "type.h"

//MP2_decode_frame(PcmDataBuf, &Mp2CurFrameBuf[0], MP2_FRAME_LEN/2);
//PcmPlay(PcmDataBuf, 1152*4);
#define MP2_FRAME_LEN			626
void MP2_decode_init(void* MemAddr);
bool MP2_decode_frame(void* PcmData, uint8_t* Mp2Data);
				 	
#endif

