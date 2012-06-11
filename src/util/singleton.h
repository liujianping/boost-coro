/* 
 * File:   singleton.h
 * Author: james
 *
 * Created on 2011年5月20日, 下午3:27
 */

#ifndef SINGLETON_H
#define	SINGLETON_H

#include "util/none.h"
//#include <boost/thread/locks.hpp>

namespace boost{
namespace coroutine{
namespace util{
           
    template<class T>
    class singleton
    {
        BOOST_COROUTINE_NO_COPYABLE(singleton)
                
        typedef T value_type;
        //typedef Lock mutex_type;
    public:
        template<typename T1>
        static void constructor(T1 t1)
        {
            if(0 == m_instance)
            {
                //boost::lock_guard<mutex_type> guard(m_lock);
                m_instance = new value_type(t1);
                atexit(Destroy);
            }
        }

        template<typename T1, typename T2>
        static void constructor(T1 t1, T2 t2)
        {
            if(0 == m_instance)
            {       
                //boost::lock_guard<mutex_type> guard(m_lock);
                m_instance = new value_type(t1, t2);
                atexit(Destroy);
            }
        }

        template<typename T1, typename T2, typename T3>
        static void constructor(T1 t1, T2 t2, T3 t3)
        {
            if(0 == m_instance)
            {
                //boost::lock_guard<mutex_type> guard(m_lock);
                m_instance = new value_type(t1, t2, t3);
                atexit(Destroy);
            }
        }

        static value_type& Instance()
        {
            if(0 == m_instance)
            {
                //boost::lock_guard<mutex_type> guard(m_lock);
                m_instance = new value_type();
                atexit(Destroy);
            }
            return *m_instance;
        }

    private:
        static void Destroy()
        {
            if(m_instance)
            {
                //boost::lock_guard<mutex_type> guard(m_lock);
                delete m_instance;
                m_instance = 0;
            }
        }
        static value_type* volatile m_instance;
        //static mutex_type m_lock;
    };

    //template<class T, class mutex_type>
    template<class T>
    T* volatile singleton<T>::m_instance = 0;  
        
}}}

#endif	/* SINGLETON_H */

