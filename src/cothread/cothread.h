/* 
 * File:   cothread.h
 * Author: liujianping
 *
 * Created on 2012年5月7日, 下午3:02
 */

#ifndef BOOST_COROUTINE_COTHREAD_H
#define	BOOST_COROUTINE_COTHREAD_H

#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <set>

#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp> 
#include "util/macro.h"
#include "util/list.h"

namespace boost{
namespace cothread{

    typedef boost::function<void*()> cothread_function_t;
    //! cothread option
    class cothread_option_t;
    //! class inherit level
    class cothread_base_t;
    //! predefine
    class cothread_t;

    typedef boost::coroutine::util::list_t<boost::cothread::cothread_base_t*> cothread_list_t;

}

namespace this_cothread{
    
    
    typedef struct this_cothread_core_data_t
    {
        this_cothread_core_data_t():_current_cothread(0){}
        boost::cothread::cothread_base_t* _current_cothread;
        boost::cothread::cothread_list_t _main_cothread_childs;
        
    }this_cothread_core_data_t;
    
    static boost::thread_specific_ptr<this_cothread_core_data_t> cothread_core_data_key;
    
    boost::cothread::cothread_base_t* get();
    void set(boost::cothread::cothread_base_t* cothread);
    
    void  resume(boost::cothread::cothread_option_t& option);    
    void* join(boost::cothread::cothread_base_t* child = 0);
    
    void child(boost::cothread::cothread_base_t* cothread);
    boost::cothread::cothread_base_t* child();
    int channel_parent();
    int channel_self();
    
}

namespace cothread{
    
    
    class cothread_option_t
    {
        friend class cothread_base_t;
    public:
        cothread_option_t():_detached(false)
        {
            assert(!pthread_attr_init(&_pthread_attr));
        }
        ~cothread_option_t()
        {
            pthread_attr_destroy(&_pthread_attr);
        }

        void stack_size(size_t sz)
        {
            pthread_attr_setstacksize(&_pthread_attr, sz);
        }

        void detachable(bool flag = true)
        {
            _detached = flag;
            if(flag)
            {
                pthread_attr_setdetachstate(&_pthread_attr, PTHREAD_CREATE_DETACHED);
            }
        }

        void guard_size(size_t sz)
        {
            pthread_attr_setguardsize(&_pthread_attr, sz);
        }

    private:
        bool           _detached;
        pthread_attr_t _pthread_attr;
    };

    class cothread_base_t
    {
        BOOST_COROUTINE_NO_COPYABLE(cothread_base_t)
        
        static void* execute(void* ctx);
    public:
        cothread_base_t();
        virtual ~cothread_base_t();
        
        void resume(cothread_option_t& option);
        
        void* join(cothread_base_t* child = 0);
        void  child(cothread_base_t* child);
        cothread_base_t* child();
        
        int  channel_parent();
        int  channel_self();
    protected:
        bool volatile       _runned;
        bool                _joined;
        pthread_t           _pid;
        cothread_function_t _cothread_function;
        int                 _socketpair[2];
        cothread_list_t     _cothread_childs;
    };
    
    class cothread_t: public cothread_base_t
    {
    public:
        template <class F>
        explicit cothread_t(F f):
        cothread_base_t()
        {
            _cothread_function = boost::bind(boost::type<void*>(),f);
        }

        template <class F, class P1>
        explicit cothread_t(F f, P1 p1):
        cothread_base_t()
        {
            _cothread_function = boost::bind(boost::type<void*>(),f, p1);
        }

        template <class F, class P1, class P2>
        explicit cothread_t(F f, P1 p1, P2 p2):
        cothread_base_t()
        {
            _cothread_function = boost::bind(boost::type<void*>(),f, p1, p2);
        }

        template <class F, class P1, class P2, class P3>
        explicit cothread_t(F f, P1 p1, P2 p2, P3 p3):
        cothread_base_t()
        {
            _cothread_function = boost::bind(boost::type<void*>(),f, p1, p2, p3);
        }
        
        template <class F, class P1, class P2, class P3, class P4>
        explicit cothread_t(F f, P1 p1, P2 p2, P3 p3, P4 p4):
        cothread_base_t()
        {
            _cothread_function = boost::bind(boost::type<void*>(),f, p1, p2, p3, p4);
        }
        
        template <class F, class P1, class P2, class P3, class P4, class P5>
        explicit cothread_t(F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5):
        cothread_base_t()
        {
            _cothread_function = boost::bind(boost::type<void*>(),f, p1, p2, p3, p4, p5);
        }

        template <class F, class P1, class P2, class P3, class P4, class P5, class P6>
        explicit cothread_t(F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6):
        cothread_base_t()
        {
            _cothread_function = boost::bind(boost::type<void*>(),f, p1, p2, p3, p4, p5, p6);
        }
        
        template <class F, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
        explicit cothread_t(F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7):
        cothread_base_t()
        {
            _cothread_function = boost::bind(boost::type<void*>(),f, p1, p2, p3, p4, p5, p6, p7);
        }
        
        template <class F, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
        explicit cothread_t(F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8):
        cothread_base_t()
        {
            _cothread_function = boost::bind(boost::type<void*>(),f, p1, p2, p3, p4, p5, p6, p7, p8);
        }
        
        template <class F, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
        explicit cothread_t(F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9):
        cothread_base_t()
        {
            _cothread_function = boost::bind(boost::type<void*>(),f, p1, p2, p3, p4, p5, p6, p7, p8, p9);
        }
    };
    
    #define create_cothread(x,...) new boost::cothread::cothread_t(x, ##__VA_ARGS__)
    #define destroy_cothread(p)    delete p;p=0
}

}

#endif	/* COTHREAD_H */

