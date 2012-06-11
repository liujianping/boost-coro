/*! @file coreactor.h
 *  @brief coreactor header file
 *  @author liujianping
 *  @date   2012.05.15
 *  @contact rabbit.ljp@gmail.com
 *  In the file,
 *      declared the class boost::coroutine::coreactor_t
 */

#ifndef BOOST_COROUTINE_COREACTOR_H
#define	BOOST_COROUTINE_COREACTOR_H


#include <event.h>    
#include <boost/pool/object_pool.hpp>
#include <boost/utility.hpp>
#include "util/macro.h"
#include "util/linked_allocator.h"
#include <map>
namespace boost{
namespace coroutine{

    typedef struct event_base event_base_t;
    typedef struct event event_t;
    class coroutine_base_t;
    
    class coreactor_t
    {
        BOOST_COROUTINE_NO_COPYABLE(coreactor_t)
        typedef boost::coroutine::memory::linked_allocator_t<sizeof(event_t)> event_allocator_t;
        
        static void _cb_evt(int fd, short events, void* arg);
    public:
        coreactor_t();
        virtual ~coreactor_t();
        
        void run(bool block);
        //! set network event trigger
        void set_io(int fd, short mask, long timeout, 
                           coroutine_base_t* coro = 0);
        void set_signal(int signal,coroutine_base_t* coro = 0);
        void set_timeout(long timeout, coroutine_base_t* coro = 0);
        void cancel(coroutine_base_t* coro = 0);
    private:
        event_base_t*                      _event_base;
        event_allocator_t                  _event_allocator;
        std::map<coroutine_base_t*, event_t*> _triggers;
    };
    
}}

#endif	/* COREACTOR_H */

