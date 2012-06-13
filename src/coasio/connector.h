/* 
 * File:   connector.h
 * Author: liujianping
 *
 * Created on 2012年5月21日, 下午9:28
 */


#ifndef BOOST_COASIO_ACCEPTOR_H
#define	BOOST_COASIO_ACCEPTOR_H

#include "coroutine/coreactor.h"
#include "coasio/cosocket.h"
#include "coasio/addr.h"

namespace boost{
namespace coasio{

class connector_t {
public:
    connector_t(boost::coroutine::coreactor_t* coreactor):
    _coreactor(coreactor), _cosocket_ptr(new cosocket_t(coreactor))
    {}
    
    virtual ~connector_t()
    {}
    
    void operator()(sock_addr_t& sock_addr, long timeout = 0)
    {
        if(_cosocket_ptr->create(sock_addr.family(), SOCK_STREAM, 0) < 0)
        {
            connect_failed();
            return;
        }

        if(_cosocket_ptr->connect(sock_addr, timeout))
        {
            connect_failed();
            return;
        }

        //! connected
        connect_succeed();
    }
    
    virtual void connect_failed() = 0;
    virtual void connect_succeed() = 0;
protected:
    boost::coroutine::coreactor_t*     _coreactor;
    cosocket_ptr_t   _cosocket_ptr;
};
    

}}


#endif	/* ACCEPTOR_H */



