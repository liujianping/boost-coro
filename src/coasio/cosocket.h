/* 
 * File:   cosocket.h
 * Author: liujianping
 *
 * Created on 2012年5月15日, 下午5:59
 */

#ifndef BOOST_COASIO_COSOCKET_H
#define	BOOST_COASIO_COSOCKET_H

#include "coasio/addr.h"
#include "util/var_buf.h"
#include "util/log.h"
#include "coroutine/coreactor.h"
#include "coroutine/coroutine.h"

#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <boost/shared_ptr.hpp> 

namespace boost{
namespace coasio{

class cosocket_t{
public:
    static int set_defer_accept(int fd, int secs)
    {
#ifndef __APPLE__
        return setsockopt(fd, SOL_TCP, TCP_DEFER_ACCEPT, &secs, sizeof(secs)) ;
#endif
        return 0;
    }

    static int set_non_block(int fd, int flag_nonblock)
    {
        int flags;

        flags = fcntl( fd, F_GETFL);
        if( flags < 0 ) return flags;

        if(flag_nonblock)	flags |= O_NONBLOCK;
        else	flags &= ~O_NONBLOCK;

        if( fcntl( fd, F_SETFL, flags ) < 0 ) return -1;

        return 0;	
    }

    static int set_reuse_addr(int fd, int flag_reusable)
    {
        return setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &flag_reusable, sizeof( flag_reusable ) );
    }

    static int set_no_nagel(int fd, int flag_nonagle)
    {
        return setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &flag_nonagle, sizeof( flag_nonagle ) ) ;
    }
    
    static int set_linger(int fd, int linger_enable, int ling_tm)
    {
        if(linger_enable)
        {
            struct linger lgr;
            lgr.l_onoff = linger_enable;
            lgr.l_linger = ling_tm;

            return setsockopt( fd, SOL_SOCKET, SO_LINGER, &lgr, sizeof(lgr));
        }
        return 0;
    }

public:
    cosocket_t(boost::coroutine::coreactor_t* coreactor, int fd = -1):
        _coreactor(coreactor), _sock_fd(fd)
    {
        assert(_coreactor);
    }
    ~cosocket_t()
    {
        if(_sock_fd != -1) close();
    }

    void fd(int fd)
    {
        _sock_fd = fd;
    }

    int  fd()
    {
        return _sock_fd;
    }

    int create(int family, int type, int flags)
    {
        int fd = ::socket(family, type, flags);
        if(fd != -1)
        {
            _sock_fd = fd;
            int i = 1;
            setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&i, sizeof(int));
        }
        return fd;
    }

    int bind(const sock_addr_t& sock_addr)
    {
        return ::bind(_sock_fd, sock_addr.sock_addr(), sock_addr.sock_addr_size());
    }

    int listen(int backlog)
    {
        return ::listen(_sock_fd, backlog);
    }

    int accept(sock_addr_t& sock_addr, long timeout  = -1)
    {
        set_event(EV_READ|EV_PERSIST, timeout);

        sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        int client = ::accept(_sock_fd, (sockaddr*)&addr, &addrlen);
        if(client == -1)
            return -1;
        cosocket_t::set_non_block(client, 1);
        sock_addr.assign((const sockaddr*)&addr);
        return client;    
    }

    int connect(const sock_addr_t& sock_addr, long timeout  /*= -1*/)
    {
        cosocket_t::set_non_block(_sock_fd, 1);

        if (::connect(_sock_fd, sock_addr.sock_addr(), sock_addr.sock_addr_size()) == -1)
        {
            if(errno != EINPROGRESS) return -1;
            set_event(EV_WRITE , timeout);

            int error = -1;
            int len = sizeof(error);
            ::getsockopt(_sock_fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
            if(error != 0) return -1;
        }

        cosocket_t::set_non_block(_sock_fd, 0);
        return 0;
    }

    int close(int linger_enable = 0, int ling_tm = 0)
    {
        if(linger_enable) cosocket_t::set_linger(_sock_fd, linger_enable, ling_tm);
        ::close(_sock_fd); _sock_fd = -1;
        return 0;
    }

    int remote_addr(sock_addr_t& sock_addr)
    {
        union {
            sockaddr generic;
            sockaddr_in  in_4;
            sockaddr_in6 in_6;
            sockaddr_un unix_domain;
        } uaddr;

        memset(&uaddr, 0, sizeof(uaddr));

        socklen_t addrlen = sizeof(uaddr);

        if ( getpeername(_sock_fd, &uaddr.generic, &addrlen) != 0 ) {
            return -1;
        }

        sock_addr.assign((const sockaddr*) &uaddr);
        return 0;
    }

    int local_addr(sock_addr_t& sock_addr)
    {
        union {
            sockaddr generic;
            sockaddr_in  in_4;
            sockaddr_in6 in_6;
            sockaddr_un unix_domain;
        } uaddr;

        memset(&uaddr, 0, sizeof(uaddr));

        socklen_t addrlen = sizeof(uaddr);

        if ( getsockname(_sock_fd, &uaddr.generic, &addrlen) != 0 ) {
            return -1;
        }

        sock_addr.assign((const sockaddr*) &uaddr);
        return 0;
    }
    
#define SILLY_NONBLOCK_SOCK_ERR(e)       \
            if(e >= 0) return e;             \
            if(errno == EINTR) continue;     \
            if(errno != EAGAIN) return e;

    ssize_t read(char* buf, size_t len, long timeout = -1)
    {
        set_event(EV_READ , timeout);
        while(true)
        {
            ssize_t retcode = ::read(_sock_fd, buf, len);
            SILLY_NONBLOCK_SOCK_ERR(retcode)
            return 0;
        }
    }
    ssize_t write(char* buf, size_t len, long timeout = -1)
    {
        set_event(EV_WRITE , timeout);
        while(true)
        {
            ssize_t retcode = ::write(_sock_fd, buf, len);
            SILLY_NONBLOCK_SOCK_ERR(retcode)
            return 0;
        }
        return 0;
    }

    ssize_t send(void* buf, size_t len, long timeout = -1)
    {
        while(true)
        {
            ssize_t retcode = ::send(_sock_fd, buf, len, MSG_DONTWAIT);
            SILLY_NONBLOCK_SOCK_ERR(retcode)
            break;
        }

        set_event(EV_WRITE , timeout);

        while(true)
        {
            ssize_t retcode = ::send(_sock_fd, buf, len, MSG_DONTWAIT);
            SILLY_NONBLOCK_SOCK_ERR(retcode)
            return 0;
        }
    }

    ssize_t sendv(const iovec* vec, size_t count, long timeout = -1)
    {
        cosocket_t::set_non_block(_sock_fd, 1);
        while (true) {
            // Optimize event register
            ssize_t retcode = ::writev(_sock_fd, vec, count);
            SILLY_NONBLOCK_SOCK_ERR(retcode)
            break;
        }

        set_event(EV_WRITE , timeout);

        while (true) {
            // Optimize event register
            ssize_t retcode = ::writev(_sock_fd, vec, count);
            SILLY_NONBLOCK_SOCK_ERR(retcode)
            return 0;
        }
    }

    ssize_t recv(void* buf, size_t len, long timeout = -1)
    {
        //cosocket_t::set_non_block(_sock_fd, 1);
        while(true)
        {
            ssize_t retcode = ::recv(_sock_fd, buf, len, MSG_DONTWAIT);
            SILLY_NONBLOCK_SOCK_ERR(retcode)
            break;
        }

        set_event(EV_READ , timeout);

        while(true)
        {
            ssize_t retcode = ::recv(_sock_fd, buf, len, MSG_DONTWAIT);
            SILLY_NONBLOCK_SOCK_ERR(retcode)
            return 0;
        }
    }

    ssize_t recvv(const iovec* vec, size_t count, long timeout = -1)
    {
        cosocket_t::set_non_block(_sock_fd, 1);
        while (true) {
            // Optimize event register
            ssize_t retcode = ::readv(_sock_fd, vec, count);
            SILLY_NONBLOCK_SOCK_ERR(retcode)
            break;
        }

        set_event(EV_READ , timeout);

        while (true) {
            // Optimize event register
            ssize_t retcode = ::readv(_sock_fd, vec, count);
            SILLY_NONBLOCK_SOCK_ERR(retcode)
            return 0;
        }
    }
    
    template <typename vbuf_type>
    ssize_t send_vbuf(vbuf_type& vbuf, long timeout = -1)
    {
        std::vector<struct iovec> vec;
        vbuf.rd_iovec(vec);
        size_t retcode = sendv(&vec[0], vec.size(), timeout);
        return retcode;
    }

    template <typename vbuf_type>
    ssize_t recv_vbuf(vbuf_type& vbuf, size_t len, long timeout = -1)
    {
        std::vector<struct iovec> vec;
        vbuf.reserve(len);
        vbuf.wr_iovec(vec);
        size_t retcode = recvv(&vec[0], vec.size(), timeout);
        if(retcode > 0) vbuf.wr_position(retcode);

        return retcode;    
    }
    
    ssize_t recv_from(sock_addr_t& sock_addr, void* buf, size_t len, long timeout = -1)
    {
        set_event(EV_READ , timeout);

        sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        ssize_t rlen = ::recvfrom(_sock_fd, buf, len, 0,(sockaddr*)&addr, &addrlen);

        sock_addr.assign((const sockaddr*)&addr);
        return rlen;
    }

    ssize_t send_to(sock_addr_t& sock_addr, void* buf, size_t len, long timeout = -1)
    {
        set_event(EV_WRITE , timeout);
        ssize_t wlen = ::sendto(_sock_fd, buf, len, 0, sock_addr.sock_addr(), sock_addr.sock_addr_size());
        return wlen;
    }    
    
    void set_event(short events, long timeout, boost::coroutine::coroutine_base_t* coro = 0)
    {
        assert(_sock_fd != -1);
        _coreactor->set_io(_sock_fd, events, timeout, (coro ? coro : boost::this_coroutine::get()));
    }
private:
    boost::coroutine::coreactor_t* _coreactor;
    int _sock_fd;
};   


typedef boost::shared_ptr<cosocket_t> cosocket_ptr_t;

}}

#endif	/* COSOCKET_H */

