/*! @file none.h
 *  @brief boost::coroutine::util::none_mutex_t header file
 *  @author liujianping
 *  @contact rabbit.ljp@gmail.com
 *  In the file,
 *      declared the class boost::coroutine::util::none_mutex_t
 */

#ifndef NONE_H
#define	NONE_H

#include <boost/coroutine/util/macro.h>

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

