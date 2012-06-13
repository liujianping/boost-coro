/*! @file coroutine.h
 *  @brief boost::coroutine header file
 *  @author liujianping
 *  @contact rabbit.ljp@gmail.com
 *  In the file,
 *      declared the boost::this_cothread namespace for some api.
 *      declared the boost::this_coroutine namespace for some api.
 *      declared the class boost::coroutine::coroutine_base_t
 *      declared the class boost::coroutine::coroutine_t
 *      define the create_coroutine & destroy_coroutine macro
 */

#ifndef BOOST_COROUTINE_COROUTINE_H
#define	BOOST_COROUTINE_COROUTINE_H

#include "libcoro/coro.h"
#include "util/macro.h"
#include "util/list.h"
#include "util/linked_allocator.h"
#include "util/trace.h"
#include "coroutine/message.h"
#include <pthread.h>
#include <assert.h>
#include <list>
#include <boost/thread/tss.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp> 
#include <boost/shared_ptr.hpp> 
#include <boost/pool/object_pool.hpp>
namespace boost{
namespace coroutine{

    /*! \typedef stat_t*/
    typedef unsigned short stat_t;
    
    /*! \def CORO_STAT_INIT 0x01*/
    #define CORO_STAT_INIT      0x01   
    /*! \def CORO_STAT_WAIT 0x02*/
    #define CORO_STAT_WAIT      0x02   
    /*! \def CORO_STAT_BUSY 0x04*/
    #define CORO_STAT_BUSY      0x04
    /*! \def CORO_STAT_DONE 0x08*/
    #define CORO_STAT_DONE      0x08
    
    /*! \def CORO_EVET_WAIT 0x10*/
    #define CORO_EVET_WAIT      0x01
    /*! \def CORO_EVET_TIME 0x20*/
    #define CORO_EVET_COME      0x02
    /*! \def CORO_EVET_CHLD 0x40*/
    #define CORO_EVET_TIME      0x04
    /*! \def CORO_EVET_CANCEL 0x80*/
    #define CORO_EVET_SIGNAL    0x08
    /*! \def CORO_EVET_CANCEL 0x80*/
    #define CORO_EVET_CANCEL    0x10
    /*! \def CORO_EVET_CHLD 0x40*/
    #define CORO_EVET_CHLD      0x80

    /*! \def CORO_MESG_WAIT 0x10*/
    #define CORO_MESG_WAIT      0x01
    /*! \def CORO_MESG_CHLD 0x40*/    
    #define CORO_MESG_CHLD      0x80

    #define CORO_LOW_MASK       0x0f
    #define CORO_HIGH_MASK      0xf0
    /*! \class coreactor_t
     *  \brief coreactor class is a functor, used for reactor coroutine 
     */
    class coreactor_t;
    /*! \typedef coro_context_t*/
    typedef struct coro_context coro_context_t;
    /*! \typedef coro_function_t */
    typedef boost::function<void()> coro_function_t;
    /*! \class coroutine_base_t */
    class coroutine_base_t;
    /*! \class coroutine_t */
    class coroutine_t;
    /*! \class cothread_t */
    class cothread_t;
    /*! \typedef coroutine_list_t */
    typedef boost::coroutine::util::list_t<coroutine_base_t*> coroutine_list_t; 
    
    /*! \typedef message_ptr_t */
    typedef boost::shared_ptr<message_t>  message_ptr_t;
    /*! \typedef comessage_list_t */
    typedef std::list<message_ptr_t> comessage_list_t;
        
 }

namespace this_cothread{
    typedef  boost::coroutine::memory::linked_allocator_t<NORMAL_CORO_STACK_SIZE, 32> 
             context_allocator_t;
    /*
    typedef  boost::coroutine::memory::linked_allocator_t<NORMAL_CORO_STACK_SIZE*2, 32> 
             context_allocator_t;
    typedef  boost::coroutine::memory::linked_allocator_t<NORMAL_CORO_STACK_SIZE*4, 32> 
             context_allocator_t;
    typedef  boost::coroutine::memory::linked_allocator_t<NORMAL_CORO_STACK_SIZE*8, 32> 
             context_allocator_t;
    typedef  boost::coroutine::memory::linked_allocator_t<NORMAL_CORO_STACK_SIZE*16, 32> 
             context_allocator_t;
    typedef  boost::coroutine::memory::linked_allocator_t<NORMAL_CORO_STACK_SIZE*32, 32> 
             context_allocator_t;
     */
    typedef  boost::object_pool<boost::coroutine::coroutine_t> coroutine_objpool_t;
    
    typedef struct this_cothread_data_t
    {
        this_cothread_data_t():
        _main_context(),
        _current_coroutine(0),
        _context_allocator(),
        _coroutine_objpool()
        {
        }
        boost::coroutine::coro_context_t          _main_context;
        boost::coroutine::coroutine_list_t        _main_childs;
        boost::coroutine::coroutine_base_t*       volatile _current_coroutine;
        boost::coroutine::util::trace_t           _trace;
        context_allocator_t    _context_allocator;
        coroutine_objpool_t    _coroutine_objpool;
    }this_cothread_data_t;
   
    static boost::thread_specific_ptr<this_cothread_data_t>  this_cothread_data_key;
    
    void* malloc(size_t size);
    void free(void* ptr);

    coroutine_objpool_t* coroutine_objpool();
    
    boost::coroutine::coro_context_t* main_context();
    boost::coroutine::util::trace_t* dtrace();
}

namespace this_coroutine{
    using namespace boost::this_cothread;
    using namespace boost::coroutine;

    coroutine_base_t* get();
    void set(coroutine_base_t* coro);
    
    void resume(coroutine_base_t* coro = 0);
    void resume_childs(coroutine_base_t* coro = 0);
    
    void yield();
    void child(coroutine_base_t* coro);
    bool child();
    coroutine_base_t* parent();
    
    //! coro event
    void event_set(stat_t event);
    void event_add(stat_t event);
    void event_del(stat_t event);
    bool event_chk(stat_t event);
    bool event_equal(stat_t event);
    
    void msg_set(stat_t msg);
    void msg_add(stat_t msg);
    void msg_del(stat_t msg);
    bool msg_chk(stat_t msg);
    
    //! coro message
    void  mput(message_ptr_t& msg);
    void  mbroadcast(message_ptr_t& msg);
    int   mget(message_ptr_t& msg, bool yield = false);
    
    void  usleep(coreactor_t* coreactor, long usec);
    void  c_str(const char* name);
    const char* c_str();
}

namespace coroutine{
    
    /*! @class coroutine_base_t 
     *  @brief coroutine base class
     *         provide the coroutine api
     */
    class coroutine_base_t
    {
        BOOST_COROUTINE_NO_COPYABLE(coroutine_base_t)
        
        friend class coreactor_t;
        friend void  boost::this_coroutine::resume(coroutine_base_t* coro);
        friend void  boost::this_coroutine::resume_childs(coroutine_base_t* coro);
        friend void  boost::this_coroutine::yield();
        friend void  boost::this_coroutine::child(coroutine_base_t* coro);
        friend bool  boost::this_coroutine::child();
        friend coroutine_base_t*  boost::this_coroutine::parent();
        friend void  boost::this_coroutine::event_set(stat_t event);
        friend void  boost::this_coroutine::event_add(stat_t event);
        friend void  boost::this_coroutine::event_del(stat_t event);
        friend bool  boost::this_coroutine::event_chk(stat_t event);
        friend bool  boost::this_coroutine::event_equal(stat_t event);
        friend void  boost::this_coroutine::msg_set(stat_t msg);
        friend void  boost::this_coroutine::msg_add(stat_t msg);
        friend void  boost::this_coroutine::msg_del(stat_t msg);
        friend bool  boost::this_coroutine::msg_chk(stat_t msg);
        friend void  boost::this_coroutine::mput(message_ptr_t& msg);
        friend void  boost::this_coroutine::mbroadcast(message_ptr_t& msg);
        friend int   boost::this_coroutine::mget(message_ptr_t& msg, bool yield);
        friend void  boost::this_coroutine::usleep(coreactor_t* coreactor, long usec);
        //! wrapper for my function execute
        static void execute(void* ctx);
    public:
        //!
        coroutine_base_t(size_t stack_sz = NORMAL_CORO_STACK_SIZE);
        ~coroutine_base_t();
        //! self yield
        void yield();
        
        //! resume a nested coro, current should be running
        void resume(coroutine_base_t* coro);
        void resume_childs(coroutine_base_t* coro = 0);
        
        //! coro add child coro, then coro will be freed by the parent
        void child(coroutine_base_t* coro);
        bool child();
        
        coroutine_base_t* parent();
        //! coro message
        void  mput(message_ptr_t& msg);
        void  mbroadcast(message_ptr_t& msg);
        int   mget(message_ptr_t& msg, bool yield = false);
        
        //! coro sleep usec
        void  usleep(coreactor_t* coreactor, long usec);
        //! coro event
        void event_set(stat_t event);
        void event_add(stat_t event);
        void event_del(stat_t event);
        bool event_chk(stat_t event);
        bool event_equal(stat_t event);
        
        //! coro msg
        void msg_set(stat_t msg);
        void msg_add(stat_t msg);
        void msg_del(stat_t msg);
        bool msg_chk(stat_t msg);
        
        //! cstr
        void  c_str(const char* name);
        const char* c_str();
    private:     
        //! coro raw context
        coro_context_t* coro_context();
        //! switch to current coroutinue execute
        void transfer(coroutine_base_t* coro_pre = 0);
        //! switch back to the pre coroutine
        void transfer_pre();
    protected:
        size_t                  _stack_sz;
        stat_t                  _stat;
        //! store coro event results, get/set/clear
        stat_t                  _evet;
        stat_t                  _msg;
        coro_context_t*         _coro_context;
        coroutine_base_t*       _coro_pre;
        coroutine_base_t*       _coro_parent;
        coro_function_t         _coro_function;
        coroutine_list_t        _coro_childs;
        comessage_list_t        _coro_messages;
        char                    _coro_name[32];
    };
    
    class coroutine_t: public coroutine_base_t
    {
    public:
        template <class F>
        explicit coroutine_t(F f, 
                 size_t stack_sz = NORMAL_CORO_STACK_SIZE):
        coroutine_base_t(stack_sz)
        {
            _coro_function = boost::bind(boost::type<void>(),f);
            
        }

        template <class F, class P1>
        explicit coroutine_t(F f, P1 p1, 
                             size_t stack_sz = NORMAL_CORO_STACK_SIZE):
        coroutine_base_t(stack_sz)
        {
            _coro_function = boost::bind(boost::type<void>(),f, p1);
        }

        template <class F, class P1, class P2>
        explicit coroutine_t(F f, P1 p1, P2 p2, 
                             size_t stack_sz = NORMAL_CORO_STACK_SIZE):
        coroutine_base_t(stack_sz)
        {
            _coro_function = boost::bind(boost::type<void>(),f, p1, p2);
        }

        template <class F, class P1, class P2, class P3>
        explicit coroutine_t(F f, P1 p1, P2 p2, P3 p3, 
        size_t stack_sz = NORMAL_CORO_STACK_SIZE):
        coroutine_base_t(stack_sz)
        {
            _coro_function = boost::bind(boost::type<void>(),f, p1, p2, p3);
        }
        
        template <class F, class P1, class P2, class P3, class P4>
        explicit coroutine_t(F f, P1 p1, P2 p2, P3 p3, P4 p4, 
        size_t stack_sz = NORMAL_CORO_STACK_SIZE):
        coroutine_base_t(stack_sz)
        {
            _coro_function = boost::bind(boost::type<void>(),f, p1, p2, p3, p4);
        }
        
        template <class F, class P1, class P2, class P3, class P4, class P5>
        explicit coroutine_t(F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, 
        size_t stack_sz = NORMAL_CORO_STACK_SIZE):
        coroutine_base_t(stack_sz)
        {
            _coro_function = boost::bind(boost::type<void>(),f, p1, p2, p3, p4, p5);
        }

        template <class F, class P1, class P2, class P3, class P4, class P5, class P6>
        explicit coroutine_t(F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, 
        size_t stack_sz = NORMAL_CORO_STACK_SIZE):
        coroutine_base_t(stack_sz)
        {
            _coro_function = boost::bind(boost::type<void>(),f, p1, p2, p3, p4, p5, p6);
        }
        
        template <class F, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
        explicit coroutine_t(F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, 
        size_t stack_sz = NORMAL_CORO_STACK_SIZE):
        coroutine_base_t(stack_sz)
        {
            _coro_function = boost::bind(boost::type<void>(),f, p1, p2, p3, p4, p5, p6, p7);
        }
        
        template <class F, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
        explicit coroutine_t(F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, 
        size_t stack_sz = NORMAL_CORO_STACK_SIZE):
        coroutine_base_t(stack_sz)
        {
            _coro_function = boost::bind(boost::type<void>(),f, p1, p2, p3, p4, p5, p6, p7, p8);
        }
        
        template <class F, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
        explicit coroutine_t(F f, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, 
        size_t stack_sz = NORMAL_CORO_STACK_SIZE):
        coroutine_base_t(stack_sz)
        {
            _coro_function = boost::bind(boost::type<void>(),f, p1, p2, p3, p4, p5, p6, p7, p8, p9);
        }
    };
    
    #define create_coroutine(x, ...) \
            boost::this_cothread::coroutine_objpool()->construct(x, ##__VA_ARGS__)
    #define destroy_coroutine(c) \
            boost::this_cothread::coroutine_objpool()->destroy(static_cast<boost::coroutine::coroutine_t*>(c))    

#ifdef ENABLE_TRACE
    #define trace(fmt, ...) \
                boost::this_cothread::dtrace()->out(__FILE__,__LINE__,__FUNCTION__,\
                fmt, ##__VA_ARGS__)
#else
    #define trace(fmt, ...)     
#endif 

}





}

#endif	/* COROUTINE_H */

