 /*	$inc		*/
/*	$each		*/
/*	$<	
	;	**************************************************************************
	;	*
	;	* EOSDEF.HD
	;	*
	;	**************************************************************************
	;	
	$>	*/
/*	$<	
	;	**************************************************************************
	;	* 
	;	**************************************************************************
	$>	*/
/*	$h		*/
	#define	NULL	((void *)0)
/*	$inc		*/
/*	$each		*/
	#undef FALSE	//for warning by pi
	#define	FALSE	0
	#undef TRUE		//for warning by pi
	#define	TRUE	1
	#define	OFF	0
	#define	ON	1
	#define	ERROR	(-1)
	#define	EOF	(-1)
/*	$<	
	;	**************************************************************************
	;	* 
	;	**************************************************************************
	$>	*/
/*	$h		*/
	typedef	int BOOL;
	typedef	unsigned char BYTE;
	typedef	char TCHAR;
	typedef	unsigned short WORD;
	typedef	unsigned long DWORD;
	typedef	unsigned long _LONG;
	typedef	unsigned long _UINT;
	typedef	void VOID;
	typedef	BOOL* PBOOL;
	typedef	const BOOL* PCBOOL;
	typedef	BYTE* PBYTE;
	typedef	const BYTE* PCBYTE;
	typedef	TCHAR* PSTR;
	typedef	const TCHAR* PCSTR;
	typedef	WORD* PWORD;
	typedef	const WORD* PCWORD;
	typedef	DWORD* PDWORD;
	typedef	const DWORD* PCDWORD;
	typedef	_LONG* PLONG;
	typedef	const _LONG* PCLONG;
	typedef	_UINT* PUINT;
	typedef	const _UINT* PCUINT;
	typedef	VOID* PVOID;
	typedef	const VOID* PCVOID;
/*	$each		*/
/*	$<	
	;	**************************************************************************
	;	*
	;	**************************************************************************
	$>	*/
/*	$h		*/
	#define	MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
	#define	MAKEDWORD(a, b) ((DWORD)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
	#define	MAKELONG(a, b) ((LONG)(((WORD)(a)) | ((LONG)((WORD)(b))) << 16))
	#define	LOWORD(l) ((WORD)(l))
	#define	HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
	#define	LOBYTE(w) ((BYTE)(w))
	#define	HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))
/*	$each		*/
/*	$<	
	;	**************************************************************************
	;	* 
	;	**************************************************************************
	$>	*/
	#define	BIT0	0x00000001
	#define	BIT1	0x00000002
	#define	BIT2	0x00000004
	#define	BIT3	0x00000008
	#define	BIT4	0x00000010
	#define	BIT5	0x00000020
	#define	BIT6	0x00000040
	#define	BIT7	0x00000080
	#define	BIT8	0x00000100
	#define	BIT9	0x00000200
	#define	BIT10	0x00000400
	#define	BIT11	0x00000800
	#define	BIT12	0x00001000
	#define	BIT13	0x00002000
	#define	BIT14	0x00004000
	#define	BIT15	0x00008000
	#define	BIT16	0x00010000
	#define	BIT17	0x00020000
	#define	BIT18	0x00040000
	#define	BIT19	0x00080000
	#define	BIT20	0x00100000
	#define	BIT21	0x00200000
	#define	BIT22	0x00400000
	#define	BIT23	0x00800000
	#define	BIT24	0x01000000
	#define	BIT25	0x02000000
	#define	BIT26	0x04000000
	#define	BIT27	0x08000000
	#define	BIT28	0x10000000
	#define	BIT29	0x20000000
	#define	BIT30	0x40000000
	#define	BIT31	0x80000000
/*	$<	
	;	**************************************************************************
	;	* 
	;	**************************************************************************
	$>	*/
	#define	SIZE_1K	1024
	#define	SIZE_2K	(SIZE_1K * 2)
	#define	SIZE_4K	(SIZE_1K * 4)
	#define	SIZE_8K	(SIZE_1K * 8)
	#define	SIZE_16K	(SIZE_1K * 16)
	#define	SIZE_32K	(SIZE_1K * 32)
	#define	SIZE_64K	(SIZE_1K * 64)
	#define	SIZE_128K	(SIZE_1K * 128)
	#define	SIZE_256K	(SIZE_1K * 256)
	#define	SIZE_512K	(SIZE_1K * 512)
	#define	SIZE_1024K	(SIZE_1K * 1024)
	#define	SIZE_1M	(SIZE_1024K)
	#define	SIZE_2M	(SIZE_1M * 2)
	#define	SIZE_4M	(SIZE_1M * 4)
	#define	SIZE_8M	(SIZE_1M * 8)
	#define	SIZE_16M	(SIZE_1M * 16)
	#define	SIZE_32M	(SIZE_1M * 32)
	#define	SIZE_64M	(SIZE_1M * 64)
	#define	SIZE_128M	(SIZE_1M * 128)
/*	$<	
	;	**************************************************************************
	;	* 
	;	**************************************************************************
	$>	*/
/*	$h		*/
	#ifdef	_DEBUG /* _DEBUG { */
	#define	DEBUGGER()	_debugger()
	#define	DBG_MSG(msg)	printf(msg)
	#define	DBG_MSG1(msg,	p1) printf(msg, p1)
	#define	DBG_MSG2(msg,	p1, p2) printf(msg, p1, p2)
	#define	DBG_MSG3(msg,	p1, p2, p3) printf(msg, p1, p2, p3)
	#define	ERR_MSG(msg)	printf(msg)
	#define	ERR_MSG1(msg,	p1) printf(msg, p1)
	#define	ERR_MSG2(msg,	p1, p2) printf(msg, p1, p2)
	#define	ERR_MSG3(msg,	p1, p2, p3) printf(msg, p1, p2, p3)
	#define	ASSERT(b)	\
	b	? (void)0 : \
	(printf("Assertion	failed: "#b" FILE %s, LINE %d\n", __FILE__, __LINE__), DEBUGGER(), (void)0)
	#else	/* } ELSE(_DEBUG) { */
	#define	DEBUGGER()	_debugger()
	#define	DBG_MSG(msg)	printf(msg)
	#define	DBG_MSG1(msg,	p1) printf(msg, p1)
	#define	DBG_MSG2(msg,	p1, p2) printf(msg, p1, p2)
	#define	DBG_MSG3(msg,	p1, p2, p3) printf(msg, p1, p2, p3)
	#define	ERR_MSG(msg)	printf(msg)
	#define	ERR_MSG1(msg,	p1) printf(msg, p1)
	#define	ERR_MSG2(msg,	p1, p2) printf(msg, p1, p2)
	#define	ERR_MSG3(msg,	p1, p2, p3) printf(msg, p1, p2, p3)
	#define	ASSERT(b)	
	#endif	/* } !_DEBUG */
/*	$each		*/
/*	$inc		*/
