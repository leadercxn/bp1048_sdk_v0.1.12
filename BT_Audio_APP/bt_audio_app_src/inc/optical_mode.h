#ifndef __OPTICAL_MODE_H__
#define __OPTICAL_MODE_H__


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus



MessageHandle GetOpticalMessageHandle(void);

int32_t OpticalPlayCreate(MessageHandle parentMsgHandle);

int32_t OpticalPlayKill(void);


int32_t OpticalPlayStart(void);


int32_t OpticalPlayPause(void);


int32_t OpticalPlayResume(void);


int32_t OpticalPlayStop(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __OPTICAL_MODE_H__
