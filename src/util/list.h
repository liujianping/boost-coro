/* 
 * File:   list.h
 * Author: liujianping
 *
 * Created on 2012年5月8日, 下午11:02
 */

#ifndef BOOST_COROUTINE_LIST_H
#define	BOOST_COROUTINE_LIST_H

#include <boost/pool/object_pool.hpp>

namespace boost{
namespace coroutine{
namespace util{
    
    template <class T>
    class list_t
    {
        typedef T data_type;
        typedef struct list_node_t
        {
            list_node_t* next;
            data_type   data;
        }list_node_t;
        typedef boost::object_pool<list_node_t> list_node_objpool_t;
    public:
        list_t():_head(0),_tail(0),_size(0)
        {
        }

        ~list_t()
        {
            if(_head) clear();
        }

        data_type front()
        {
            if(_head) 
                return _head->data;
            return 0;
        }

        void pop_front()
        {
            list_node_t* node;
            if(_head)
            {
                node = _head;
                if(_head == _tail) _tail = 0;
                _head = _head->next; 
                _list_node_objpool.destroy(node);
                _size--;
            }

        }

        void push_front(data_type data)
        {
            list_node_t* node = _list_node_objpool.construct();
            assert(node);
            node->data = data;
            node->next = 0;

            if(!_head)
            {
                _head = node;
                _tail = _head;
            }
            else
            {
                node->next = _head;
                _head = node;
            }
            _size++;
        }

        void  push_back(data_type data)
        {
            list_node_t* node = _list_node_objpool.construct();
            assert(node);
            node->data = data;
            node->next = 0;

            if(!_head)
            {
                _head = node;
                _tail = _head;
            }
            else
            {
                _tail->next = node;
                _tail = _tail->next;        
            }
            _size++;
        }

        bool empty()
        {
            return ( (!_head) ? true : false);
        }

        void clear()
        {
            list_node_t* p;
            while(_head)
            {
                p = _head;
                _head = _head->next;
                _list_node_objpool.destroy(p);
                _size--;
            }
            _tail = _head;
        }        
        
        size_t size()
        {
            return _size;
        }
        
    private:
        list_node_t* _head;
        list_node_t* _tail;
        size_t       _size;
        list_node_objpool_t _list_node_objpool;
    };

}}}
#endif	/* LIST_H */

