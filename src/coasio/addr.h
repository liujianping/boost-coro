/* 
 * File:   addr.h
 * Author: liujianping
 *
 * Created on 2012年5月21日, 下午8:23
 */

#ifndef BOOST_COASIO_ADDR_H
#define	BOOST_COASIO_ADDR_H

#include <assert.h>
#include <ostream>
#include <string>
#include <sstream> 
#include <string.h>
#include <algorithm>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <assert.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#define ADDR_PREFIX_INET        "inet://"
#define ADDR_PREFIX_TCP         "tcp://"
#define ADDR_PREFIX_UDP         "udp://"
#define ADDR_PREFIX_LOCAL       "local://"
#define ADDR_PREFIX_FILE        "file://"
#define ADDR_LOCALHOST          "127.0.0.1"

namespace boost{
namespace coasio{

class inet_addr_t
{
    static std::string get_ip_by_host(const std::string & host)
    {
            std::string sIP = host;

            if ( sIP != "0.0.0.0" ) {

                    struct hostent * result;
                    result = gethostbyname(host.c_str());
                    if(!result) sIP = "0.0.0.0";
                    else
                    {
                        if (result->h_addr_list[0] == NULL ) {
                                sIP = "0.0.0.0";
                        }
                        else {
                                char tmp[16];
                                sIP = inet_ntop(AF_INET, result->h_addr_list[0], tmp, sizeof(tmp));
                        }
                    }
            }

            return sIP;
    }
public:
    inet_addr_t(unsigned short port):
    _host("0.0.0.0"), _host_ip("0.0.0.0"), _port(port)
    {
    }

    inet_addr_t(const char* host, unsigned short port):
    _host(host), _port(port)
    {
    }

    inet_addr_t(const struct sockaddr_in * sockaddr)
    {
        char szIP[16];
        _host_ip = inet_ntop(AF_INET, &sockaddr->sin_addr, szIP, sizeof(szIP))==NULL?"0.0.0.0":szIP; 
        _port = ntohs(sockaddr->sin_port);
        _host = _host_ip;
    }

    std::string to_string() const
    {
        std::ostringstream out;
        out << host() << ":" << port();
        return out.str();
    }

    bool operator==(const inet_addr_t& addr)
    {
        return _host == addr._host && _host_ip == addr._host_ip && _port == addr._port;
    }

    bool operator!=(const inet_addr_t& addr)
    {
        return !(*this == addr);
    }

    inline const std::string& host() const
    {
        return _host;
    }

    inline const std::string& host_ip() const
    {
        return _host_ip;
    }

    inline const unsigned short port() const
    {
        return _port;
    }

private:
    std::string _host;
    std::string _host_ip;
    unsigned short _port;
};

class unix_addr_t
{
public:
    unix_addr_t(const std::string& path)
    :_path(path)
    {
    }

    unix_addr_t(const struct sockaddr_un* unix_addr)
    :_path(unix_addr->sun_path)
    {
    }

    std::string to_string() const
    {
        return path();
    }
    const std::string& path() const 
    {return _path;}
    
private:
    std::string _path;
};

class sock_addr_t
{
public:
    sock_addr_t():
    _str(), _len(0), _sockaddr()    
    {
        memset(&_sockaddr, 0, sizeof(_sockaddr));
    }
    sock_addr_t(const unix_addr_t& unix_addr):
        _str(), _len(0), _sockaddr()
    {
        assign(unix_addr);
    }

    sock_addr_t(const inet_addr_t& inet_addr):
        _str(), _len(0), _sockaddr()
    {
        assign(inet_addr);
    }

    sock_addr_t(const sockaddr* sock_addr):
        _str(), _len(0), _sockaddr()
    {
        assign(sock_addr);
    }

    void assign(const unix_addr_t& unix_addr)
    {
            memset(&_sockaddr, 0, sizeof(_sockaddr));	

            _sockaddr.unix_domain.sun_family = AF_LOCAL;
            strncpy(_sockaddr.unix_domain.sun_path, unix_addr.path().c_str(), sizeof(_sockaddr.unix_domain.sun_path) - 1);
            _len = sizeof(_sockaddr.unix_domain);
            _str = unix_addr.to_string();
    }

    void assign(const inet_addr_t& inet_addr)
    {
            memset(&_sockaddr, 0, sizeof(_sockaddr));

            _sockaddr.in_4.sin_family = AF_INET;
            _sockaddr.in_4.sin_port = htons(inet_addr.port());
            inet_pton(AF_INET, inet_addr.host_ip().c_str(), &_sockaddr.in_4.sin_addr);	

            _len = sizeof(_sockaddr.in_4);
            _str = inet_addr.to_string();
    }

    void assign(const sockaddr* sock_addr)
    {
        assert(sock_addr);
        if(sock_addr->sa_family == AF_INET) 
            assign(inet_addr_t((const sockaddr_in *)sock_addr));
        else if(sock_addr->sa_family == AF_LOCAL) 
            assign(unix_addr_t((const sockaddr_un *)sock_addr));
        else
            assert(0);
    }

    int to_inet_addr(inet_addr_t & inet_addr) const
    {
            if ( family() != AF_INET ) {
                    return -1;
            }

            inet_addr = inet_addr_t((const sockaddr_in*)sock_addr());
            return 0;    
    }

    int to_unix_addr(unix_addr_t & unix_addr) const
    {
            if ( family() != AF_LOCAL ) {
                    return -1;
            }

            unix_addr = unix_addr_t((const sockaddr_un*)sock_addr());
            return 0;  
    }

    // for stl container
    bool operator<(const sock_addr_t & sock_addr) const
    {
        return memcmp(&_sockaddr, &sock_addr._sockaddr, sizeof(_sockaddr)) < 0;
    }

    // for stl container
    bool operator==(const sock_addr_t & sock_addr) const
    {
        return memcmp(&_sockaddr, &sock_addr._sockaddr, sizeof(_sockaddr)) == 0;
    }

    const struct sockaddr * sock_addr() const
        { return &_sockaddr.generic; }

    socklen_t sock_addr_size() const
            { return _len; }

    int family() const
    { return _sockaddr.generic.sa_family; }

    const std::string & to_string() const
    { return _str; }
    bool operator!=(const sock_addr_t & sock_addr) const 
    { return !(*this==sock_addr); }

private:
    std::string _str;
    socklen_t   _len;
    union{
        sockaddr generic;
        sockaddr_in  in_4;
        sockaddr_in6 in_6;
        sockaddr_un unix_domain;
    } _sockaddr;
};


//! common functions
/**
    * Parse socket address
    * "inet://9999" => "inet://127.0.0.1:9999"
    * "inet://localhost:9999"
    * "inet://127.0.0.1:9999"
    * "local:///tmp/test.sock"
    * "local://foo"
    *
    * When inet's host not griven, bindhost be used.
    */
sock_addr_t to_sock_addr(const char* addr, const char* bindhost=0)
{
	assert(addr);
	if (strncmp(addr, ADDR_PREFIX_INET, strlen(ADDR_PREFIX_INET)) == 0) {
		addr += strlen(ADDR_PREFIX_INET);

		const char* host = addr;
		unsigned short port = 0;

		const char* pos = NULL;
		if ((pos = (const char*)strstr(addr, ":")) != NULL) {
			port = atoi(pos + 1);
                        std::string shost(host, pos);
			return sock_addr_t(sock_addr_t(inet_addr_t(shost.c_str(), port)));
		}
		else {
			if (bindhost) {
				host = bindhost;
			}
			else {
				host = ADDR_LOCALHOST;
			}

			port = atoi(addr);
			return sock_addr_t(sock_addr_t(inet_addr_t(host, port)));
		}
	}
	else if (strncmp(addr, ADDR_PREFIX_LOCAL, strlen(ADDR_PREFIX_LOCAL)) == 0) {
		return sock_addr_t(sock_addr_t(unix_addr_t(addr + strlen(ADDR_PREFIX_LOCAL))));
        }
	else if (strncmp(addr, ADDR_PREFIX_FILE, strlen(ADDR_PREFIX_FILE)) == 0) {
		return sock_addr_t(sock_addr_t(unix_addr_t(addr + strlen(ADDR_PREFIX_FILE))));
	}
	else {
                return sock_addr_t(sock_addr_t(unix_addr_t(addr)));
	}
}

}}
#endif	/* ADDR_H */

