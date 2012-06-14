#include <boost/coroutine/coroutine/coreactor.h>
#include <boost/coroutine/coroutine/coroutine.h>
#include <assert.h>

namespace boost{
namespace coroutine{
    void coreactor_t::_cb_evt(int fd, short events, void* arg)
    {
        coroutine_base_t* c = (coroutine_base_t*)arg;
        assert(c); 
        c->event_del(CORO_EVET_WAIT);
        if(events & EV_TIMEOUT) c->event_add(CORO_EVET_TIME);
    }
    
    coreactor_t::coreactor_t():_event_base(0)
    {
        assert(_event_base = static_cast<event_base_t*>(event_init()));
        trace("event_init:%p", _event_base);
    }
    coreactor_t::~coreactor_t()
    {
        event_base_free(_event_base);
    }

    void coreactor_t::run(bool block)
    {
        do{
            if(block)
                event_base_loop(_event_base, EVLOOP_ONESHOT);
            else
            event_base_loop(_event_base, EVLOOP_NONBLOCK);
            boost::this_coroutine::yield();
        }while(!_triggers.empty());
        
    }
    //! set network event trigger
    void coreactor_t::set_io(int fd, short mask, long timeout, 
                        boost::coroutine::coroutine_base_t* coro /*= 0*/)
    {
        coroutine_base_t* c = (coro ? coro : boost::this_coroutine::get());
        c->event_set(CORO_EVET_WAIT);
        
        event_t* event= static_cast<event_t*>(_event_allocator.malloc(sizeof(event_t)));
        assert(event);
        if(_triggers.find(c) != _triggers.end())
        {
            trace("set io coro exist, must be persist event");
            _event_allocator.free(event);
        }
        else
        {
            event_base_set(_event_base, event);
            event_set(event, fd, mask, &coreactor_t::_cb_evt, c);
            timeval tv = {timeout / 1000, (timeout % 1000) * 1000};
            event_add(event, timeout > 0 ? &tv : 0);
            _triggers.insert(std::make_pair(c, event));
        }

        boost::this_coroutine::yield();
        
        if(mask & EV_PERSIST) return;

        _triggers.erase(c);
        event_del(event);
        _event_allocator.free(event);
    }
    
    void coreactor_t::set_timeout(long timeout,
                           boost::coroutine::coroutine_base_t* coro /*= 0*/)
    {
        if(timeout <= 0) return;
        coroutine_base_t* c = (coro ? coro : boost::this_coroutine::get());
        event_t* event = static_cast<event_t*>(_event_allocator.malloc(sizeof(event_t)));
        assert(event);
        c->event_set(CORO_EVET_WAIT);

        event_base_set(_event_base, event);
        timeout_set(event, &coreactor_t::_cb_evt, c);
        timeval tv = {timeout / 1000, (timeout % 1000) * 1000};
        timeout_add(event,&tv);
        
        _triggers.insert(std::make_pair(c, event));
        boost::this_coroutine::yield();
        _triggers.erase(c);
        event_del(event);
        _event_allocator.free(event);
    }
    
    void coreactor_t::set_signal(int signal,
                           boost::coroutine::coroutine_base_t* coro /*= 0*/)
    {
        coroutine_base_t* c = (coro ? coro : boost::this_coroutine::get());
        event_t* event = static_cast<event_t*>(_event_allocator.malloc(sizeof(event_t)));
        assert(event);
        c->event_set(CORO_EVET_WAIT);

        event_base_set(_event_base, event);
        signal_set(event, signal, &coreactor_t::_cb_evt, c);
        signal_add(event, 0);
        _triggers.insert(std::make_pair(c, event));
        boost::this_coroutine::yield();
        _triggers.erase(c);
        event_del(event);
        _event_allocator.free(event);
    }

    void coreactor_t::cancel(coroutine_base_t* coro /*= 0*/)
    {
        coroutine_base_t* c = coro ? coro : boost::this_coroutine::get();
        if(_triggers.find(c) == _triggers.end()) return;
        
        c->event_del(CORO_EVET_WAIT);
        c->event_add(CORO_EVET_CANCEL);
        event_del(_triggers[c]);
        _triggers.erase(c);
        _event_allocator.free(_triggers[c]);
    }
}}


