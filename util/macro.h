/* 
 * File:   macro.h
 * Author: liujianping
 *
 * Created on 2012年5月4日, 下午2:47
 */

#ifndef MACRO_H
#define	MACRO_H

#define BOOST_COROUTINE_DELETE_COPY_CTOR(CLASS) \
    private: \
      CLASS(CLASS&); \
    public:

#define BOOST_COROUTINE_DELETE_COPY_ASSIGN(CLASS) \
    private: \
      CLASS& operator=(CLASS&); \
    public:


#define BOOST_COROUTINE_NO_COPYABLE(CLASS) \
    BOOST_COROUTINE_DELETE_COPY_CTOR(CLASS) \
    BOOST_COROUTINE_DELETE_COPY_ASSIGN(CLASS)


#endif	/* MACRO_H */

