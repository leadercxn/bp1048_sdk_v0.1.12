/**
 **************************************************************************************
 * @file    line_play.h
 * @brief   linein 
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-1-5 11:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __LINE_MODE_H__
#define __LINE_MODE_H__


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 



MessageHandle GetLineInMessageHandle(void);

bool LineInPlayCreate(MessageHandle parentMsgHandle);

bool LineInPlayKill(void);

bool LineInPlayStart(void);

bool LineInPlayPause(void);

bool LineInPlayResume(void);

bool LineInPlayStop(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __LINE_MODE_H__
