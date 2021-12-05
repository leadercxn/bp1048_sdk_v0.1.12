/**
 * @brief 修改日志打印的方式
 */
#ifndef		__TRACE_H__
#define		__TRACE_H__

/**
 * 使用习惯的打印格式
 */
#ifdef TRACE_ENABLE

#define TRACE_LEVEL_DISABLED                    0
#define TRACE_LEVEL_ASSERT                      1
#define TRACE_LEVEL_ERROR                       2
#define TRACE_LEVEL_WARN                        3
#define TRACE_LEVEL_NOTICE                      4
#define TRACE_LEVEL_INFO                        5
#define TRACE_LEVEL_DEBUG                       6
#define TRACE_LEVEL_VERBOSE                     7

#ifndef TRACE_LEVEL
#define TRACE_LEVEL                    			TRACE_LEVEL_DEBUG
#endif

#ifndef TRACE_MODULE
#define TRACE_MODULE __FILE__
#endif

#ifndef TRACE_ASSERT_FORMAT
#define TRACE_ASSERT_FORMAT                     "%-10s\t%4d [A] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_ERROR_FORMAT
#define TRACE_ERROR_FORMAT                      "%-10s\t%4d [E] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_WARN_FORMAT
#define TRACE_WARN_FORMAT                       "%-10s\t%4d [W] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_NOTICE_FORMAT
#define TRACE_NOTICE_FORMAT                     "%-10s\t%4d [N] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_INFO_FORMAT
#define TRACE_INFO_FORMAT                       "%-10s\t%4d [I] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_DEBUG_FORMAT
#define TRACE_DEBUG_FORMAT                      "%-10s\t%4d [D] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_VERBOSE_FORMAT
#define TRACE_VERBOSE_FORMAT                    "%-10s\t%4d [V] ", TRACE_MODULE, __LINE__
#endif

#ifndef TRACE_LINE_ENDING
#define TRACE_LINE_ENDING                       "\n"
#endif

#ifndef TRACE_PRINTF
#define TRACE_PRINTF                            printf
#endif


static void trace_dump(void *p_buffer, uint32_t len)
{
    uint8_t *p = (uint8_t *)p_buffer;
	uint32_t index;

    for (index = 0; index < len; index++)
    {
        TRACE_PRINTF("%02X", p[index]);
    }

    TRACE_PRINTF("\r\n");
}

#define trace_assert(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ASSERT){\
            TRACE_PRINTF(TRACE_ASSERT_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_assertln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ASSERT){\
            TRACE_PRINTF(TRACE_ASSERT_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_a(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ASSERT){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_error(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ERROR){\
            TRACE_PRINTF(TRACE_ERROR_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_errorln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ERROR){\
            TRACE_PRINTF(TRACE_ERROR_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_e(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_ERROR){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_warn(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_WARN){\
            TRACE_PRINTF(TRACE_WARN_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_warnln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_WARN){\
            TRACE_PRINTF(TRACE_WARN_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_w(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_WARN){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_notice(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_NOTICE){\
            TRACE_PRINTF(TRACE_NOTICE_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_noticeln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_NOTICE){\
            TRACE_PRINTF(TRACE_NOTICE_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_n(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_NOTICE){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_info(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_INFO){\
            TRACE_PRINTF(TRACE_INFO_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_infoln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_INFO){\
            TRACE_PRINTF(TRACE_INFO_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_i(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_INFO){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_debug(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_DEBUG){\
            TRACE_PRINTF(TRACE_DEBUG_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_debugln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_DEBUG){\
            TRACE_PRINTF(TRACE_DEBUG_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_d(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_DEBUG){\
            trace_dump(p_buffer,len);}\
    }while(0)

#define trace_verbose(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_VERBOSE){\
            TRACE_PRINTF(TRACE_VERBOSE_FORMAT);TRACE_PRINTF(msg, ##__VA_ARGS__);}\
    }while(0)
#define trace_verboseln(msg, ...)   do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_VERBOSE){\
            TRACE_PRINTF(TRACE_VERBOSE_FORMAT);TRACE_PRINTF(msg TRACE_LINE_ENDING, ##__VA_ARGS__);}\
    }while(0)
#define trace_dump_v(p_buffer,len)    do{\
        if(TRACE_LEVEL >= TRACE_LEVEL_VERBOSE){\
            trace_dump(p_buffer,len);}\
    }while(0)

#else

#define trace_init(...)
#define trace_dump(...)
#define trace_level_set(...)
#define trace_level_get(...)

#define trace_assert(...)
#define trace_assertln(...)
#define trace_dump_a(...)

#define trace_error(...)
#define trace_errorln(...)
#define trace_dump_e(...)

#define trace_warn(...)
#define trace_warnln(...)
#define trace_dump_w(...)

#define trace_notice(...)
#define trace_noticeln(...)
#define trace_dump_n(...)

#define trace_info(...)
#define trace_infoln(...)
#define trace_dump_i(...)

#define trace_debug(...)
#define trace_debugln(...)
#define trace_dump_d(...)

#define trace_verbose(...)
#define trace_verboseln(...)
#define trace_dump_v(...)

#endif




#endif	//__TYPE_H__



