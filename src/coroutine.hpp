/* 
 * File:   coroutine.h
 * Author: liujianping
 *
 * Created on 2012年6月8日, 下午5:56
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

