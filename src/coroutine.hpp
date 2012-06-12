/* 

 * Copyright (c) 2012 Liu Jianping <rabbit.ljp@gmail.com>
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:

 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.

 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @2012.06.01 version 0.1 released by liujianping 
 */

#ifndef BOOST_COROUTINE_ANOTHER_V
#define	BOOST_COROUTINE_ANOTHER_V
#include "coroutine/message.h"
#include "coroutine/coroutine.h"
#include "coroutine/coreactor.h"
#include "cothread/cothread.h"
#include "coasio/addr.h"
#include "coasio/cosocket.h"
#include "coasio/acceptor.h"
#include "coasio/connector.h"
#include "util/linked_allocator.h"
#include "util/list.h"
#include "util/log.h"
#include "util/macro.h"
#include "util/none.h"
#include "util/singleton.h"
#include "util/trace.h"
#include "util/var_buf.h"
#include "libcoro/coro.h"

/*! @namespace boost
 *  @brief     namespace of boost
 */
namespace boost{
/*! @namespace boost::coroutine
 *  @brief     coroutine namespace 
 */
namespace coroutine{
    /*! @namespace boost::coroutine::util
    *   @brief     coroutine utility namespace
     *             provide utility class for coroutine 
    */
    namespace util{}
    /*! @namespace boost::coroutine::memory
    *   @brief     coroutine memory utility namespace
    *             provide memory utility class for coroutine 
    */
    namespace memory{}
}
/*! @namespace boost::cothread
 *  @brief     cooperative thread namespace
 */
namespace cothread{}
/*! @namespace boost::this_coroutine
 *  @brief     this coroutine namespace
 *             provide apis to operator the current coroutine
 */
namespace this_coroutine{}
/*! @namespace boost::this_cothread
 *  @brief     this cooperative thread namespace 
 *             provide apis to operator the current cothread
 */
namespace this_cothread{}

/*! @namespace boost::coasio
 *  @brief     cooperative async io namespace 
 *             provide class for the cooperative async io class, specially for network
 */            
namespace coasio{}
}

#endif	/* COROUTINE_H */

