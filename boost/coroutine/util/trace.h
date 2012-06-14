/* 
 * File:   trace.h
 * Author: james
 *
 * Created on 2012年5月10日, 下午4:50
 */

#ifndef BOOST_COROUTINE_TRACE_H
#define	BOOST_COROUTINE_TRACE_H

#include <boost/thread/tss.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace boost{
namespace coroutine{
    namespace util{
    
    class trace_t
    {
    public:
        trace_t(){}
        ~trace_t(){}
        
        void out(const char* file, size_t line, const char* func,
                 const char* format, ...)
        {
            boost::lock_guard<boost::mutex> guard(_mutex);
            va_list vl;
            va_start(vl, format);
            print(file, line, func, format, vl);
            va_end(vl);
        }
        void print(const char* file, size_t line, const char* func,
                   const char* fmt, va_list ap)
        {
            // format log message
            char logmsg[1024] = {0};
            //sprintf(logmsg, "[%zx]%s(%s:%zu)", (ssize_t)pthread_self(), func, file, line);
            sprintf(logmsg, "[%zx]%s:", (ssize_t)pthread_self(), func);
            int len = strlen(logmsg);
            vsnprintf(logmsg + len, 1024 - len - 2, fmt, ap);
            std::cout<<logmsg<<std::endl;
        }
    private:
        boost::mutex _mutex;
    };
}}}

#endif	/* TRACE_H */

