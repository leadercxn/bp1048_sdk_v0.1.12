
#ifdef __IAR_SYSTEMS_ICC__     /* IAR Compiler */
#define inline inline
#endif

#ifdef  __CC_ARM               /* ARM Compiler */
#define inline __inline
#endif

#define EXPORT 

#define FIXED_POINT
//#define DISABLE_FLOAT_API
//#define DISABLE_VBR
////#define DISABLE_WIDEBAND
//#define DISABLE_NOTIFICATIONS
//#define DISABLE_WARNINGS
//#define RELEASE

#define NB_ENC_STACK                4096
#define NB_DEC_STACK                2048
#define MAX_CHARS_PER_FRAME         1024
