#include "cothread/cothread.h"

namespace boost{

namespace this_cothread{
    boost::cothread::cothread_base_t* get()
    {
        if(!cothread_core_data_key.get())
            cothread_core_data_key.reset(new this_cothread_core_data_t);
        return cothread_core_data_key.get()->_current_cothread;
    }
    void set(boost::cothread::cothread_base_t* cothread)
    {
        if(!cothread_core_data_key.get())
            cothread_core_data_key.reset(new this_cothread_core_data_t);
        cothread_core_data_key.get()->_current_cothread = cothread;
    }
    
    void  resume(boost::cothread::cothread_option_t& option)
    {
    }
    
    void* join(boost::cothread::cothread_base_t* child /*= 0*/)
    {
        if(!child)
        {
            boost::cothread::cothread_base_t* co = 0;
            while((co = boost::this_cothread::child()))
            {
                co->join();
                destroy_cothread(co);
            }
            return 0;
        }
        
        //! child exist
        if(get())
            return get()->join(child);
        else
            return child->join();
    }
    

    
    void child(boost::cothread::cothread_base_t* cothread)
    {
        if(!cothread_core_data_key.get())
                cothread_core_data_key.reset(new this_cothread_core_data_t);
        
        if(get()) get()->child(cothread);
        else
            cothread_core_data_key.get()->_main_cothread_childs.push_back(cothread);
    }

    boost::cothread::cothread_base_t* child()
    {
        if(!cothread_core_data_key.get()) return 0;
        if(get()) return get()->child();
        else{
            boost::cothread::cothread_base_t* child;
            child = cothread_core_data_key.get()->_main_cothread_childs.front();
            cothread_core_data_key.get()->_main_cothread_childs.pop_front();
            return child;
        }
    }
    
    int channel_parent()
    {
        if(get()) return get()->channel_parent();
        return -1;
    }
    
    int channel_self()
    {
        if(get()) return get()->channel_self();
        return -1;
    }    
} 

namespace cothread{
    void* cothread_base_t::execute(void* ctx)
    {
        cothread_base_t* cothread = (cothread_base_t*)ctx;
        boost::this_cothread::set(cothread);
        cothread->_runned = true;

        void* ret = 0;
        if(cothread->_cothread_function)
            ret = cothread->_cothread_function();
        
        //! close read pair
        cothread_base_t* child = 0;
        while((child = cothread->child()))
        {
            cothread->join(child);
            destroy_cothread(child);
        }

        ::close(cothread->channel_self());
        boost::this_cothread::set(0);
        return ret;
    }

    cothread_base_t::cothread_base_t():
    _runned(false),_joined(false),
    _pid(0),
    _cothread_function(0)        
    {
        assert(!socketpair(AF_UNIX, SOCK_STREAM, 0, _socketpair));
    }
    cothread_base_t::~cothread_base_t()
    {
        
    }

    void cothread_base_t::resume(cothread_option_t& option)
    {
        if(_runned) return;
        if(option._detached) _joined = true;
        assert(!pthread_create(&_pid, &(option._pthread_attr), 
                        &cothread_base_t::execute, (void*)this));
    }

    void* cothread_base_t::join(cothread_base_t* child)
    {
        void* retcode = 0;
        if(!child)
        {
            if(!_joined) 
                assert(!pthread_join(_pid, &retcode));
            return retcode;
        }
        
        if(!child->_joined){
            assert(!pthread_join(child->_pid, &retcode));
            child->_joined = true;
        }
        return retcode;
    }

    void cothread_base_t::child(cothread_base_t* child)
    {
        _cothread_childs.push_back(child);
    }

    cothread_base_t* cothread_base_t::child()
    {
        cothread_base_t* front = _cothread_childs.front();
        _cothread_childs.pop_front();
        return front;
    }

    int  cothread_base_t::channel_parent()
    {
        assert(_socketpair[1] != -1);
        return _socketpair[1];
    }
    int  cothread_base_t::channel_self()
    {
        assert(_socketpair[0] != -1);
        return _socketpair[0];
    }
    
}
    
}
