/* 
 * File:   none.h
 * Author: liujianping
 *
 * Created on 2012年5月4日, 下午2:04
 */

#ifndef NONE_H
#define	NONE_H

#include "util/macro.h"

namespace boost{
namespace coroutine{
namespace util{
    
   class none_mutex_t
   {
       BOOST_COROUTINE_NO_COPYABLE(none_mutex_t)
   public:
       none_mutex_t(){}
       ~none_mutex_t(){}
       void lock()
       {}
       
       void unlock()
       {}
       
       bool try_lock()
       {
           return true;
       }
   };
}}}

#endif	/* NONE_H */

