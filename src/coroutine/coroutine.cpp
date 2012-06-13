#include "coroutine/coroutine.h"
#include "coreactor.h"
#include <string.h>
#include <stdio.h>
namespace boost{

namespace this_cothread{
    void* malloc(size_t size)
    {
        if(!this_cothread_data_key.get())
            this_cothread_data_key.reset(new this_cothread_data_t);
        return this_cothread_data_key.get()->_context_allocator.malloc(size);
    }

    void free(void* ptr)
    {
        if(!this_cothread_data_key.get())
            this_cothread_data_key.reset(new this_cothread_data_t);
        return this_cothread_data_key.get()->_context_allocator.free(ptr);
    }

    coroutine_objpool_t* coroutine_objpool()
    {
        if(!this_cothread_data_key.get())
            this_cothread_data_key.reset(new this_cothread_data_t);

        return boost::addressof(this_cothread_data_key.get()->_coroutine_objpool);
    }
    boost::coroutine::coro_context_t* main_context()
    {
        if(!this_cothread_data_key.get())
            this_cothread_data_key.reset(new this_cothread_data_t);
        return boost::addressof(this_cothread_data_key.get()->_main_context);
    }
    boost::coroutine::util::trace_t* dtrace()
    {
        if(!this_cothread_data_key.get())
            this_cothread_data_key.reset(new this_cothread_data_t);
        return boost::addressof(this_cothread_data_key.get()->_trace);
    }
}    
namespace this_coroutine{
    
    coroutine_base_t* get()
    {
        if(!this_cothread_data_key.get())
            this_cothread_data_key.reset(new this_cothread_data_t);
        return this_cothread_data_key.get()->_current_coroutine;
    }

    void set(coroutine_base_t* coro)
    {
        if(!this_cothread_data_key.get())
            this_cothread_data_key.reset(new this_cothread_data_t);
        this_cothread_data_key.get()->_current_coroutine = coro;
    }

    void yield()
    {
        if(get()) get()->yield();
    }
    void child(coroutine_base_t* coro)
    {
        if(!this_cothread_data_key.get())
            this_cothread_data_key.reset(new this_cothread_data_t);
        if(this_cothread_data_key.get()->_current_coroutine)
            this_cothread_data_key.get()->_current_coroutine->child(coro);
        else{
            trace("[co:nil] add a child: %s", coro->c_str());
            this_cothread_data_key.get()->_main_childs.push_back(coro);
        }
            
    }
    bool child()
    {
        if(get()) return get()->child();
        else
            return this_cothread_data_key.get()->_main_childs.size() ? true : false;
    }
    coroutine_base_t* parent()
    {
        if(get()) return get()->parent();
        return 0;
    }
    void resume(coroutine_base_t* coro)
    {
        if(get()) get()->resume(coro);
        else
        {
            if(coro)
            {
                if(coro->_stat == CORO_STAT_DONE) return;
                if(coro->_stat & (CORO_STAT_INIT | CORO_STAT_WAIT))
                {
                    if(coro->event_chk(CORO_EVET_CHLD)||
                        coro->msg_chk(CORO_MESG_CHLD))
                    {
                        resume_childs(coro);
                    }
                    if(!coro->event_chk(CORO_EVET_WAIT) &&
                       !coro->msg_chk(CORO_MESG_WAIT))
                    {
                        trace("[co:nil] resume %s", coro->c_str());
                        coro->transfer(0);
                    }
                }
            }
            else
            {
                //! coro is zero
                while(!this_cothread_data_key.get()->_main_childs.empty()) 
                    resume_childs();
            }
        }
    }
    void resume_childs(coroutine_base_t* coro)
    {
        
        if(get()) get()->resume_childs(coro);
        else
        {
            coroutine_list_t& childs = (coro ? coro->_coro_childs :
                                        this_cothread_data_key.get()->_main_childs);
            size_t loop = childs.size();
            while(loop--)
             {
                coroutine_base_t* child = childs.front();
                trace("%s resume child:%s", (coro ? coro->c_str() : "[co:nil]"),
                      child->c_str());
                childs.pop_front();
                if(child->_stat == CORO_STAT_DONE)
                {
                    trace("%s destroy child:%s", 
                           (coro ? coro->c_str() : "[co:nil]"),child->c_str());
                    destroy_coroutine(child);
                    continue;
                }
                
                trace("%s resume child:%s",(coro ? coro->c_str() : "[co:nil]"),
                      child->c_str());
                resume(child);
                
                if(child->_stat == CORO_STAT_DONE)
                {
                    trace("%s destroy child:%s", 
                           (coro ? coro->c_str() : "[co:nil]"),child->c_str());
                    destroy_coroutine(child);
                    continue;
                }

                childs.push_back(child);    
            }
        }
    }
    
    void event_set(stat_t event)
    {
        if(get()) get()->event_set(event);
    }
    void event_add(stat_t event)
    {
        if(get()) get()->event_add(event);
    }
    void event_del(stat_t event)
    {
        if(get()) get()->event_del(event);
    }
    bool event_chk(stat_t event)
    {
        if(get()) 
            return get()->event_chk(event);
        return false;
    }
    bool event_equal(stat_t event)
    {
        if(get()) 
            return get()->event_equal(event);
        return false;
    }
    void msg_set(stat_t msg)
    {
        if(get()) get()->msg_set(msg);
    }
    void msg_add(stat_t msg)
    {
        if(get()) get()->msg_add(msg);
    }
    void msg_del(stat_t msg)
    {
        if(get()) get()->msg_del(msg);
    }
    bool msg_chk(stat_t msg)
    {
        if(get()) 
            return get()->msg_chk(msg);
        return false;
    }
    void  mput(message_ptr_t& msg)
    {
        if(get()) get()->mput(msg);
    }
    void  mbroadcast(message_ptr_t& msg)
    {
        if(get()) get()->mbroadcast(msg);
    }
    int   mget(message_ptr_t& msg, bool yield_enable)
    {
        if(get()) return get()->mget(msg, yield_enable);
        return 0;
    }
    void  usleep(coreactor_t* coreactor, long usec)
    {
        if(get()) get()->usleep(coreactor, usec);
    }

    void  c_str(const char* name)
    {
        if(get()) get()->c_str(name);
    }
    
    const char* c_str()
    {
        if(get()) return get()->c_str();
        return "[co:nil]";
    }
}

namespace coroutine{
    
    void coroutine_base_t::execute(void* ctx)
    {
        assert(ctx);
        coroutine_base_t* coroutine = static_cast<coroutine_base_t*>(ctx);
        //! set current runing coro
        boost::this_coroutine::set(coroutine);
        
        //! coro self custom function
        if(coroutine->_coro_function) coroutine->_coro_function();
        
        //! after execute the custom function, coro stat (BUSY)
        assert(CORO_STAT_BUSY == coroutine->_stat );
        //! executeing childs coroutines
        while(!coroutine->_coro_childs.empty())
        {
            coroutine->resume_childs();
        }
        coroutine->_stat = CORO_STAT_DONE;
        //! tranfer back to the pre context
        coroutine->transfer_pre();
    }

    coroutine_base_t::coroutine_base_t(size_t stack_sz):
    _stack_sz(stack_sz),
    _stat(CORO_STAT_INIT),_evet(0),
    _coro_context(0),_coro_pre(0),_coro_parent(0),
    _coro_function(0)
    {
        memset(_coro_name, 0, 32);
    }
    
    coroutine_base_t::~coroutine_base_t()
    {
        if(_coro_context)
            boost::this_cothread::free(_coro_context);
        _coro_context = 0;
    }
    //! self yield
    void coroutine_base_t::yield()
    {
        trace("%s yield", c_str());
        resume_childs();
        transfer_pre();
    }

    void coroutine_base_t::resume(coroutine_base_t* coro)
    {
        if(coro)
        {
            if(coro->_stat == CORO_STAT_DONE) return;
            if(coro->_stat & (CORO_STAT_INIT | CORO_STAT_WAIT))
            {
                if(coro->event_chk(CORO_EVET_CHLD)||
                   coro->msg_chk(CORO_MESG_CHLD))
                {
                    resume_childs(coro);
                }

                if(!coro->event_chk(CORO_EVET_WAIT) &&
                   !coro->msg_chk(CORO_MESG_WAIT))
                {
                    trace("%s resume %s", c_str(), coro->c_str());
                    coro->transfer(this);
                }
                
            }
        }
    } 
    //! coro add child coro, then coro will be freed by the parent
    void coroutine_base_t::child(coroutine_base_t* coro)
    {
        assert(coro);
        trace("%s add a child: %s", c_str(), coro->c_str());
        coro->_coro_parent = this;
        _coro_childs.push_back(coro);
    }
    
    bool coroutine_base_t::child()
    {
        return _coro_childs.size() ? true : false;
    }
        
    coroutine_base_t* coroutine_base_t::parent()
    {
        return _coro_parent;
    }
    //! coro event
    void coroutine_base_t::event_set(stat_t event)
    {
        _evet &= ~CORO_EVET_WAIT;
        _evet |= event;
        if(_coro_parent && event != CORO_EVET_WAIT)
        {
           _coro_parent->event_add(CORO_EVET_CHLD);
        }
        trace("%s event =:[%x]", c_str(), _evet);
    }
    void coroutine_base_t::event_add(stat_t event)
    {
        _evet |= event;
        if(_coro_parent && event != CORO_EVET_WAIT)
        {
           _coro_parent->event_add(CORO_EVET_CHLD);
        }
        trace("%s event =:[%x]", c_str(), _evet);
    }
    void coroutine_base_t::event_del(stat_t event)
    {
        if((_evet & event) && _coro_parent && event == CORO_EVET_WAIT)
        {
           _coro_parent->event_add(CORO_EVET_CHLD);
        }
        _evet &= ~event;
        trace("%s event =:[%x]", c_str(), _evet);
    }
    bool coroutine_base_t::event_chk(stat_t event)
    {
        trace("%s event check:[%x] & [%x]", c_str(), _evet, event);
        if(_evet & event) return true;
        return false;
    }
    bool coroutine_base_t::event_equal(stat_t event)
    {
        trace("%s event equal:[%x] == [%x]", c_str(), _evet, event);
        if(_evet ==  event) return true;
        return false;
    }
    
    void coroutine_base_t::msg_set(stat_t msg)
    {
        _msg &= ~CORO_MESG_WAIT;
        _msg |= msg;
        if(_coro_parent && msg != CORO_MESG_WAIT)
        {
           _coro_parent->msg_add(CORO_MESG_CHLD);
        }
        trace("%s msg =:[%x]", c_str(), _msg);
    }
    void coroutine_base_t::msg_add(stat_t msg)
    {
        _msg |= msg;
        if(_coro_parent && msg != CORO_MESG_WAIT)
        {
           _coro_parent->msg_add(CORO_MESG_CHLD);
        }
        trace("%s msg =:[%x]", c_str(), _msg);
    }
    void coroutine_base_t::msg_del(stat_t msg)
    {
        if((_msg & CORO_MESG_WAIT) && _coro_parent && msg == CORO_MESG_WAIT)
        {
           _coro_parent->msg_add(CORO_MESG_CHLD);
        }
        _msg &= ~msg;
        trace("%s msg =:[%x]", c_str(), _msg);
    }
    bool coroutine_base_t::msg_chk(stat_t msg)
    {
        trace("%s msg check:[%x] & [%x]", c_str(), _msg, msg);
        if(_msg & msg) return true;
        return false;
    }

    //! cstr
    void  coroutine_base_t::c_str(const char* name)
    {
        memset(_coro_name, 0, 32);
        sprintf(_coro_name, "[co:%p:%s]", this, name);
    }
    
    const char* coroutine_base_t::c_str()
    {
        if(strlen(_coro_name)) return _coro_name;
        
        sprintf(_coro_name, "[co:%p]", this);
        return _coro_name;
    }

        //! coro message
    void  coroutine_base_t::mput(message_ptr_t& msg)
    {
        msg_del(CORO_MESG_WAIT);
        _coro_messages.push_back(msg);
    }
    void  coroutine_base_t::mbroadcast(message_ptr_t& msg)
    {
        size_t loop = this->_coro_childs.size();
        while(loop--)
        {
            coroutine_base_t* child = _coro_childs.front();
            _coro_childs.pop_front();
            child->mput(msg);
            _coro_childs.push_back(child);
        }
    }
    
    int   coroutine_base_t::mget(message_ptr_t& msg, bool yield_enable)
    {
        if(yield_enable)
        {
            while(_coro_messages.empty())
            {
                msg_set(CORO_MESG_WAIT);
                yield();
            }
            msg = _coro_messages.front();
            _coro_messages.pop_front();
            return 1;
        }
        
        if(_coro_messages.empty()) return 0;
        
        msg = _coro_messages.front();
        _coro_messages.pop_front();
        return 1;
    }

    //! coro sleep usec
    void coroutine_base_t::usleep(coreactor_t* coreactor, long usec)
    {
        assert(coreactor);
        coreactor->set_timeout(usec);
    }

    coro_context_t* coroutine_base_t::coro_context()
    {
        if(!_coro_context)
        {
            _coro_context = static_cast<coro_context_t*>(
            boost::this_cothread::malloc(_stack_sz));
            assert(_coro_context);

            coro_create(_coro_context, 
                        &coroutine_base_t::execute, 
                        (void*)this,
                        (char*)_coro_context + sizeof(coro_context_t),
                        _stack_sz - sizeof(coro_context_t));
        }
        return _coro_context;
    }

        
    //! switch to current coroutinue execute
    void coroutine_base_t::transfer(coroutine_base_t* coro_pre)
    {
        //! INIT => WAIT
        if(_stat == CORO_STAT_INIT) _stat = (_stat << 1);
        
        //! make sure stat be WAIT
        assert(_stat == CORO_STAT_WAIT);
        
        //! stat (WAIT => BUSY)
        _stat = (_stat << 1);
        boost::this_coroutine::set(this);
        _coro_pre = coro_pre;
        if(!coro_pre)
        {
            trace("[co:nil] transfer to %s.", c_str());
            coro_transfer(boost::this_cothread::main_context(), coro_context());
        }
        else{
            //! prep BUSY => WAIT
            _coro_pre->_stat = (_coro_pre->_stat >> 1);
            trace("%s transfer to %s.", coro_pre->c_str(), c_str());
            coro_transfer(coro_pre->coro_context(), coro_context());
        }
    }
        
    //! switch back to the pre coroutine
    void coroutine_base_t::transfer_pre()
    {
        //! current coro stat (BUSY => WAIT)
        if(_stat == CORO_STAT_BUSY)
             _stat = (_stat >> 1);
        
        trace("%s transfer to pre %s .", c_str(), 
                (_coro_pre ? _coro_pre->c_str() : "[co:nil]"));
        if(!_coro_pre)
        {
            boost::this_coroutine::set(0);
            coro_transfer(coro_context(), boost::this_cothread::main_context());
        }
        else
        {
            boost::this_coroutine::set(_coro_pre);
            assert(_coro_pre->_stat == CORO_STAT_WAIT);
            _coro_pre->_stat = (_coro_pre->_stat << 1);
            coro_transfer(coro_context(), _coro_pre->coro_context());
        }
    }
    void coroutine_base_t::resume_childs(coroutine_base_t* coro /*= 0*/)
    {
        coroutine_base_t* cur = (coro ? coro : this);
        if((coro == this) && 
           !cur->event_chk(CORO_EVET_CHLD) &&
           !cur->msg_chk(CORO_EVET_CHLD)) 
            return;

        cur->event_del(CORO_EVET_CHLD);
        cur->msg_del(CORO_MESG_CHLD);
        size_t loop = cur->_coro_childs.size();
        while(loop--)
        {
            coroutine_base_t* child = cur->_coro_childs.front();
            cur->_coro_childs.pop_front();
            if(child->_stat == CORO_STAT_DONE)
            {
                trace("%s destroy child:%s", c_str(), child->c_str());
                destroy_coroutine(child);
                continue;
            }

            trace("%s resume child:%s", c_str(), child->c_str());
            resume(child);

            if(child->_stat == CORO_STAT_DONE)
            {
                trace("%s destroy child:%s", c_str(), child->c_str());
                destroy_coroutine(child);
                continue;
            }
            cur->_coro_childs.push_back(child);    
        }

    }
}

}
