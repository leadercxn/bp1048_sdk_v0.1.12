/*
*****************************************************************************
*					Mountain View Silicon Tech. Inc.
*	Copyright 2012, Mountain View Silicon Tech. Inc., ShangHai, China
*					All rights reserved.
*
* Filename:			timeout.h
* Description:		declare	varable and function here
*
******************************************************************************
*/

/**
 * @addtogroup TIMER
 * @{
 * @defgroup timeout timeout.h
 * @{
 */
#ifndef __TIMEROUT_H__
#define __TIMEROUT_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

#define TIME_DIFF(a, b)		\
	 ((uint32_t)(((int32_t)(a) - (int32_t)(b)) > 0 ? \
				((int32_t)(a) - (int32_t)(b)) : ((int32_t)(b) - (int32_t)(a))))

#define TIME_AFTER(a, b)		\
	 ((int32_t)(b) - (int32_t)(a) < 0)

#define TIME_BEFORE(a, b)	TIME_AFTER(b, a)

#define TIME_AFTER_EQ(a, b)	\
	 ((int32_t)(a) - (int32_t)(b) >= 0)

#define TIME_BEFORE_EQ(a, b)	TIME_AFTER_EQ(b, a)
/*
#define	TIMEOUT_ON_SYSTICK(j)	\
	(TIME_BEFORE_EQ(j, OSSysTickGet()))
 */

#define	TIMEOUT_ON_AUXTMR(j)	\
	(TIME_BEFORE_EQ(j, GetSysTick1MsCnt()))
	
/*
 * When we convert to jiffies then we interpret incoming values the following way:
 *
 * - negative values mean 'infinite timeout' (MAX_JIFFY_OFFSET)
 *
 * - 'too large' values [that would result in larger than
 *   MAX_JIFFY_OFFSET values] mean 'infinite timeout' too.
 *
 * - all other values are converted to jiffies by either multiplying
 *   the input value by a factor or dividing it with a factor
 *
 * We must also be careful about 32-bit overflows.
 */

#define		MSECS_TO_TICKS(ms) (((ms) + (1000UL / HZ) - 1) / (1000UL / HZ))
#define		MSECS_DOWNTO_TICKS(ms) ((ms) / (1000UL / HZ))

/*
 * convert HZ value to usecond(s)
 */
#define		TICKS_TO_USECS(ts)	((1000000UL / HZ) * (ts))

/*
 * convert HZ value to msecond(s)
 */
#define		TICKS_TO_MSECS(ts)	((1000UL / HZ) * (ts))

/*
 * convert usecond(s) to HZ value
 */
#define		USECS_TO_TICKS(us)\
	((us) > TICKS_TO_USECS(((~0UL >> 1) >> 1) - 1) ? \
		((~0UL >> 2) - 1) : (((us) + (1000000UL / HZ) - 1) / (1000000UL / HZ)))

//#define	UPTO_TICKS(ts)	((ts) + OSSysTickGet())
/*
 * convert msecond(s) to HZ value and add it upto current jiffies
 */
//#define	MSECS_UPTO_TICKS(ms)	(MSECS_TO_TICKS(ms) + OSSysTickGet())
extern unsigned int GetSysTick1MsCnt(void);
#define	MSECS_UPTO_AUXTMR(ms)	((ms) + GetSysTick1MsCnt())

/*
 * define timer check for init task period event
 */
typedef struct _TMR_EVT_LIST
{
	uint8_t	Reload;
	uint32_t	InterTmr;
	int32_t (*TmrFunc)(uint16_t Msg, uint8_t* Buf, int32_t BufLen);
} TMR_EVT_LIST;

// define softtimer structure.
typedef struct	_TIMER
{
	uint32_t	TimeOutVal; 								//time out value
	uint32_t	TickValCache;			  					//soft timer setting value
	bool	IsTimeOut;									//time out flag
} TIMER;

typedef void (*DelayMsFunction)(unsigned int Ms);
//default:DelayMs
extern DelayMsFunction DelayMsFunc;
// Get now time from system start.
// extern uint32_t GetCurrentTime(void);

// Get time of some softtimer from setting to now.
extern uint32_t PastTimeGet(TIMER* timer);

// Set time out value of softtimer.
extern void TimeOutSet(TIMER* timer, uint32_t timeout);

// Check whether time out.
extern bool IsTimeOut(TIMER* timer);

void DelayMs(unsigned int Ms);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__TIMEROUT_H__
/**
 * @}
 * @}
 */

