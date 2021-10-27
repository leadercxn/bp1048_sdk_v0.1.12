#ifndef __SPDIF_MODE_H__
#define __SPDIF_MODE_H__


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus


bool SpdifPlayCreate(MessageHandle parentMsgHandle);

bool SpdifPlayStart(void);

bool SpdifPlayPause(void);

bool SpdifPlayResume(void);

bool SpdifPlayStop(void);

bool SpdifPlayKill(void);

MessageHandle GetSpdifPlayMessageHandle(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SPDIF_MODE_H__

