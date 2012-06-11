/* 
 * File:   acceptor.h
 * Author: liujianping
 *
 * Created on 2012年5月21日, 下午9:28
 */

#ifndef BOOST_COASIO_ACCEPTOR_H
#define	BOOST_COASIO_ACCEPTOR_H

#include "coroutine/coreactor.h"
#include "coasio/cosocket.h"
#include "coasio/addr.h"

#include <sys/socket.h>

namespace boost{
namespace coasio{

class acceptor_t {
public:
    explicit acceptor_t(boost::coroutine::coreactor_t* coreactor):
    _coreactor(coreactor), _cosocket_ptr(new cosocket_t(coreactor))
    {}
    
    virtual ~acceptor_t()
    {}
    
    void stop()
    {
        _coreactor->cancel(0);
        _stopped = true;
    }
    
    void operator()(sock_addr_t& sock_addr, int type = SOCK_STREAM, int flag = 0)
    {
        if(_cosocket_ptr->create(sock_addr.family(), type, flag) < 0)
        {
            logerror("acceptor_t create socket failed:%s",
                    strerror(errno));
            return;
        }

        if(_cosocket_ptr->bind(sock_addr))
        {
            logerror("acceptor_t bind socket failed:%s",
                    strerror(errno));
            return;
        }

        if(_cosocket_ptr->listen(5))
        {
            logerror("acceptor_t listen socket failed:%s",
                    strerror(errno));
            return;
        }
        cosocket_t::set_non_block(_cosocket_ptr->fd(), 1);
        while(!_stopped)
        {
            sock_addr_t sock_addr_c;
            int client = _cosocket_ptr->accept(sock_addr_c);
            if(client < 0) continue;

            //! client be audit
            if(audit(client, sock_addr_c))
            {
                ::close(client);
                continue;
            }

            //! accept user function
            accept(client, sock_addr_c);
        }
    }
    
    virtual void accept(int client, sock_addr_t& remote_addr){
        
    }
    virtual bool audit(int client, sock_addr_t& remote_addr){
        return false;
    }
protected:
    boost::coroutine::coreactor_t*     _coreactor;
    cosocket_ptr_t   _cosocket_ptr;
    bool             _stopped;    
};
    

}}


#endif	/* ACCEPTOR_H */
