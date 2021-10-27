#ifndef __COAXIAL_MODE_H__
#define __COAXIAL_MODE_H__


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus



MessageHandle GetCoaxialMessageHandle(void);

int32_t CoaxialPlayCreate(MessageHandle parentMsgHandle);

int32_t CoaxialPlayKill(void);


int32_t CoaxialPlayStart(void);


int32_t CoaxialPlayPause(void);


int32_t CoaxialPlayResume(void);


int32_t CoaxialPlayStop(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __COAXIAL_MODE_H__
