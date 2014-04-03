/***************************************************************************
 * 
 * Copyright (c) 2011 Baidu.com, Inc. All Rights Reserved
 * log.h,v 1.0 2012-08-17 11:31 yelu(yelu01@baidu.com)
 * 
 **************************************************************************/
 
 
 
/**
 * @file log.h
 * @author yelu(yelu01@baidu.com)
 * @date 2012-08-17 11:31
 * @version 1.0 
 * @brief wrapper of com_log
 *  
 **/

#pragma once


#include <stdarg.h>
#include <stdio.h>

#define COMLOG_WARNING 2
#define COMLOG_DEBUG 1
#define COMLOG_NOTICE 0
#define COMLOG_FATAL 3

#define LOG(_level_, _fmt_, args...) \
    do\
    {\
        printf("[%s][%d][%s]"_fmt_,  __FILE__, __LINE__, __FUNCTION__, ##args);\
    }while(false)

#define LOG_DEBUG(_fmt_, args...) \
    do\
    {\
        printf("[%s][%d][%s]"_fmt_, __FILE__, __LINE__, __FUNCTION__, ##args);\
    }while(false)

#define LOG_WARNING(_fmt_, args...) \
    do\
    {\
    	printf("[%s][%d][%s]"_fmt_, __FILE__, __LINE__, __FUNCTION__, ##args);\
    }while(false)

#define LOG_NOTICE(_fmt_, args...) \
    do\
    {\
    	printf("[%s][%d][%s]"_fmt_, __FILE__, __LINE__, __FUNCTION__, ##args);\
    }while(false)

#ifdef NOLOG
#undef LOG
#define LOG(_level_, _fmt_, args...) do{}while(false)
#undef DAL_DEBUG
#define DAL_DEBUG(_fmt_, args...) do{}while(false)
#undef DAL_WARNING
#define DAL_WARNING(_fmt_, args...) do{}while(false)
#undef DAL_NOTICE
#define DAL_NOTICE(_fmt_, args...) do{}while(false)
#endif

#ifdef NODEBUG
#undef DAL_DEBUG
#define DAL_DEBUG(_fmt_, args...) do{}while(false)
#endif

