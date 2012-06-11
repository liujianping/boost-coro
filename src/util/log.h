/* 
 * File:   log.h
 * Author: liujianping
 *
 * Created on 2012年5月21日, 下午8:54
 */

#ifndef LOG_H
#define	LOG_H

#include <errno.h>
#include <string.h>
#include <assert.h>

#define DLOG
#define ENABLE_LOG_ERROR
#define ENABLE_LOG_WARN
#define ENABLE_LOG_INFO
#define ENABLE_LOG_DEBUG



#ifdef ENABLE_LOG_INFO
#define loginfo(x,arg...) msg_info(2,x,##arg)
#else
#define loginfo(x,arg...) {}
#endif

#ifdef ENABLE_LOG_WARN
#define logwarn(x,arg...) msg_info(3,x,##arg)
#else
#define logwarn(x,arg...) {}
#endif

#ifdef ENABLE_LOG_ERROR
#define logerror(x,arg...) msg_info(4,x,##arg)
#else
#define logerror(x,arg...) {}
#endif

#ifdef ENABLE_LOG_DEBUG
#define logdebug(x,arg...) msg_info(0,x,##arg)
#else
#define logdebug(x,arg...) {}
#endif

#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

static const char* priority_array[5] = 
{
    "DEBUG",
    "TRACE",
    "INFO ",
    "WARN ",
    "ERROR"
};

static void msg_info(short pri, const char* fmt, ...)
{
    char msg[1024];
    va_list va;
    va_start(va, fmt);
    vsnprintf(msg, 1023, fmt, va);
    va_end(va);
    msg[1023] = '\0';
    volatile pthread_t pid= pthread_self();
    cout <<"["<<priority_array[pri]<<"]"<<"["<<time(0)<<"]:["<<pid<<"]"<< msg <<endl;
}


#endif	/* LOG_H */

