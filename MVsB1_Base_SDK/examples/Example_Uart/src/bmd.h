/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  none
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *    bmd.h
 *
 * Project:
 * --------
 *   None
 *
 * Description:
 * ------------
 *   This file is intends for ring-buffer API.
 *
 * Author:
 * -------
 * -------
 *
*/
#ifndef BMD_H
#define BMD_H

//#include "drv_comm.h"

typedef struct __BUFFER_INFO
{
	uint16_t	Read;		/* @field Current Read index. */
	uint16_t	Write;		/* @field Current Write index. */
	uint16_t	Length;		/* @field Length of buffer */
	uint8_t	*CharBuffer;		/* @field Start of buffer */
}BUFFER_INFO;

#define ResetFifo(Buffer)   		(Buffer.Write = Buffer.Read = 0)
#define BWrite(Buffer)	   		(Buffer->Write)
#define BRead(Buffer)		   	(Buffer->Read)
#define BLength(Buffer)	   		(Buffer->Length)
#define BStartAddr(Buffer)     (Buffer->CharBuffer)
#define BuffWrite(Buffer)   		(Buffer->CharBuffer+Buffer->Write)
#define BuffRead(Buffer)	   	(Buffer->CharBuffer+Buffer->Read)

#define BWrite_addr(Buffer)	   (Buffer.Write)
#define BRead_addr(Buffer)		   (Buffer.Read)
#define BLength_addr(Buffer)	   (Buffer.Length)
#define BuffWrite_addr(Buffer)   (Buffer.CharBuffer+Buffer.Write)
#define BuffRead_addr(Buffer)	   (Buffer.CharBuffer+Buffer.Read)
#define Buff_EndAddr(Buffer)     (Buffer.CharBuffer+Buffer.Length-1)
#define Buff_StartAddr(Buffer)     (Buffer.CharBuffer)

#define Buff_isEmpty	1
#define Buff_notEmpty	0
#define Buff_isFull	1
#define Buff_notFull	0
#define Buff_PushOK	0
#define Buff_PushErr	1
#define Buff_PopOK	0
#define Buff_PopErr	1

#define Buf_init(_Buffer,_Buffaddr,_uTotalSize) \
{\
   BUFFER_INFO *_Buf=_Buffer;\
   _Buf->Read = 0;\
	_Buf->Write = 0;\
	_Buf->Length = _uTotalSize;\
	_Buf->CharBuffer = _Buffaddr;\
}\

#define Buf_IsFull(_Buffer,_result)   \
{\
   BUFFER_INFO *_Buf=_Buffer;\
	uint16_t _tmp = BRead(_Buf);\
	if (_tmp == 0)\
		_tmp = BLength(_Buf);\
	if ( (_tmp-BWrite(_Buf)) == 1)\
	{\
		_result = Buff_isFull;\
	}\
	else\
	{\
		_result = Buff_notFull;\
	}\
}\

#define Buf_GetRoomLeft(_Buffer,_RoomLeft)   \
{\
   BUFFER_INFO *_Buf=_Buffer;\
   if ( BRead(_Buf) <= BWrite(_Buf) ) \
	{\
      _RoomLeft = BLength(_Buf) - BWrite(_Buf) + BRead(_Buf) - 1;\
	}\
	else\
	{\
		_RoomLeft = BRead(_Buf) - BWrite(_Buf) - 1;\
	}\
}\

#define Buf_Push_Multi(_Buffer,_pushDataPointer,_multi,offset) \
{\
   BUFFER_INFO *_Buf=_Buffer;\
   if( (BWrite(_Buf) + _multi) >= BLength(_Buf) ){\
      memcpy(BuffWrite(_Buf), _pushDataPointer + offset, (BLength(_Buf) - BWrite(_Buf))*sizeof(uint8_t));\
      memcpy(BStartAddr(_Buf) , _pushDataPointer + offset + (BLength(_Buf) - BWrite(_Buf)), (_multi- (BLength(_Buf) - BWrite(_Buf)))*sizeof(uint8_t));\
      BWrite(_Buf) = (BWrite(_Buf) + _multi) - BLength(_Buf);\
   }\
   else{\
   memcpy(BuffWrite(_Buf), _pushDataPointer + offset, _multi*sizeof(uint8_t));\
         BWrite(_Buf) += _multi;\
}\
}\

#define Buf_Push(_Buffer,_pushData) \
{\
   BUFFER_INFO *_Buf=_Buffer;\
   *BuffWrite(_Buf) = _pushData;\
   if(BWrite(_Buf) >= (BLength(_Buf) - 1))\
   {\
   	BWrite(_Buf) = 0;\
   }\
   else\
   {\
	BWrite(_Buf)++;\
   }\
}\
//获取最新收到的数据
#define Buf_GetBytesAvail(_Buffer,_BytesAvail) \
{\
   BUFFER_INFO *_Buf = _Buffer;\
	_BytesAvail = 0;\
	if (BWrite(_Buf) >= BRead(_Buf))\
		_BytesAvail = BWrite(_Buf) - BRead(_Buf);\
	else\
		_BytesAvail = BLength(_Buf) - BRead(_Buf) + BWrite(_Buf);	\
}\
//输出多个数据
#define Buf_Pop_Multi(_Buffer,_popDataPointer,_multi,offset) \
{\
   BUFFER_INFO *_Buf=_Buffer;\
   if( (BRead(_Buf) + _multi) >= BLength(_Buf) ){\
   	memcpy(_popDataPointer + offset, BuffRead(_Buf), (BLength(_Buf) - BRead(_Buf))*sizeof(uint8_t));\
   	memcpy(_popDataPointer + offset + (BLength(_Buf) - BRead(_Buf)), BStartAddr(_Buf), (_multi- (BLength(_Buf) - BRead(_Buf)))*sizeof(uint8_t));\
         BRead(_Buf) = BRead(_Buf) + _multi - BLength(_Buf) ;}\
   else{\
   	 memcpy(_popDataPointer + offset, BuffRead(_Buf), _multi);\
         BRead(_Buf) += _multi;}\
}\

#define Buf_Pop(_Buffer,_popData)   \
{\
   BUFFER_INFO *_Buf = _Buffer;\
	_popData= *BuffRead(_Buf);\
	BRead(_Buf)++;\
	if (BRead(_Buf) >= BLength(_Buf))\
	{\
		BRead(_Buf) -= BLength(_Buf);\
	}\
}\

/* should be deleted */
#define Buf_IsEmpty(_Buffer,_result)   \
{\
   BUFFER_INFO *_Buf = _Buffer;\
	if ( BRead(_Buf) == BWrite(_Buf) ) \
	{\
		_result = Buff_isEmpty;\
	}\
	else\
	{\
		_result = Buff_notEmpty;\
	}\
}\

/* void Get32FromBuff(BUFFER_INFO *Buf,uint32_t DATA) */
#define Get32FromBuf(_Buffer,_DATA)    \
{\
   BUFFER_INFO *_Buf = _Buffer;\
	uint8_t	_tmp,_index;\
	uint32_t 	_tmp32;\
	_DATA =0;\
	for (_index =0;_index < 4;_index++)\
	{\
		Buff_Pop(_Buf,&_tmp);\
		_tmp32 = (uint32_t)_tmp;\
		(_DATA) |= (_tmp32 << (8*_index));\
	}\
}\

/*void Put32toBuff(BUFFER_INFO *Buf,uint32_t *DATA)*/
#define Put32toBuf(_Buffer,_DATA)    \
{\
   BUFFER_INFO *_Buf = _Buffer;\
	uint8_t	_tmp,_index;\
	uint32_t 	_tmp32;\
	for (_index =0;_index < 4;_index++)\
	{\
		_tmp32 = ((*_DATA) >> (8*_index));\
		_tmp = (uint8_t)_tmp32;\
		Buff_Push(_Buf,&_tmp);\
	}\
}\

#define Buf_Flush(_Buffer) \
{\
   BUFFER_INFO *_Buf = _Buffer;\
	_Buf->Write = _Buf->Read = 0;\
}

#define Buf_look(_Buffer,_popData,_num)   \
{\
   BUFFER_INFO *_Buf = _Buffer;\
   uint8_t _index;\
   uint16_t _tmp;\
   _tmp = BRead(Buf);\
   for(_index=0;_index<_num;_index++)\
   {\
	   *_popData= *(Buf->CharBuffer+_tmp);\
	   _tmp++;\
	   if (_tmp >= BLength(Buf))\
	   {\
		   _tmp -= BLength(Buf);\
	   }\
	}\
}

// MoDIS parser skip start
#if 1
extern void Buff_init(BUFFER_INFO *Buf,uint8_t *Buffaddr, uint16_t uTotalSize);
extern void Buff_Push(BUFFER_INFO *Buf,uint8_t *pushData);
extern void Buff_Pop(BUFFER_INFO *Buf,uint8_t *popData);
extern uint8_t Buff_IsEmpty(BUFFER_INFO *Buf);
extern uint8_t Buff_IsFull (BUFFER_INFO *Buf);
extern uint16_t Buff_GetRoomLeft (BUFFER_INFO *Buf);
extern uint16_t Buff_GetBytesAvail (BUFFER_INFO *Buf);
extern uint16_t Buff_GetLength(BUFFER_INFO *Buf);
extern void Buff_Flush (BUFFER_INFO *Buf);
extern void Buff_look(BUFFER_INFO *Buf,uint8_t *popData,uint8_t num);
extern void Get32FromBuff(BUFFER_INFO *Buf,uint32_t *DATA);
extern void Put32toBuff(BUFFER_INFO *Buf,uint32_t *DATA);
extern void MemCPY(uint8_t *dst,uint8_t *src,uint32_t len);
extern void MemSET(uint8_t *dst,uint8_t data,uint32_t len);
#endif
// MoDIS parser skip end

#endif

