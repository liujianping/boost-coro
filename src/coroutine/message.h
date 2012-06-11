/*! @file message.h
 *  @brief boost::coroutine::message header file
 *  @author liujianping
 *  @date   2012.05.25
 *  @contact rabbit.ljp@gmail.com
 *  In the file,
 *      declared the class boost::coroutine::message_t
 *  
 */

#ifndef BOOST_COROUTINE_MESSAGE_H
#define	BOOST_COROUTINE_MESSAGE_H

#include "util/var_buf.h"
#include <string>

namespace boost{
namespace coroutine{

    #define MSG_INT   0x01
    #define MSG_LONG  0x02
    #define MSG_CHAR  0x04
    #define MSG_VBUF  0x08
    #define MSG_TYPE  0x10
    
    #ifndef MSG_PAGE_SZ
    #define MSG_PAGE_SZ 128
    #endif

    #ifndef MSG_PAGE_BUFFIZE
    #define MSG_PAGE_BUFFIZE 4
    #endif

    typedef var_buf_t<MSG_PAGE_SZ, MSG_PAGE_BUFFIZE> vbuf_t;
    //! @class message_t
    //! @brief class of message_t
    class message_t
    {
        BOOST_COROUTINE_NO_COPYABLE(message_t)
    public:
        message_t():flag(0){}
        explicit message_t(int v):flag(MSG_INT){
            value.i = v;
        }
        explicit message_t(long v):flag(MSG_LONG){
            value.l = v;
        }
        explicit message_t(char v):flag(MSG_CHAR){
            value.c = v;
        }
        
        explicit message_t(const char* s):flag(MSG_VBUF){
            vbuf.assign(s, strlen(s));
        }

        explicit message_t(const std::string& str):flag(MSG_VBUF){
            vbuf.assign(str.c_str(), str.length());
        }
        virtual ~message_t()
        {
            vbuf.clear();
        }
        
        void append(int v){
            assert(!(flag & MSG_INT));
            flag |= MSG_INT;
            value.i = v;
        }
        void append(long v){
            assert(!(flag & MSG_LONG));
            flag |= MSG_LONG;
            value.l = v;
        }
        void append(char v){
            assert(!(flag & MSG_CHAR));
            flag |= MSG_CHAR;
            value.c = v;
        }
        void append(const char* s, size_t sz){
            flag |= MSG_VBUF;
            vbuf.append(s, sz);
        }
        void append(const std::string& s){
            flag |= MSG_VBUF;
            vbuf.append(s.c_str(), s.length());
        }

    public:
        unsigned char flag;///< flag of the message
        vbuf_t        vbuf;///< vbuf of the message, store variable msgs
        //! @union message_t.u
        //! @brief store fixed variable msg
        union u{
            int       i;
            long      l;
            char      c;
        }            value;///< value of the message.u

    };
    
}}

#endif	/* COMESSAGE_H */

