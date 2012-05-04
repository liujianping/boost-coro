/* 
 * File:   allocator.h
 * Author: james
 *
 * Created on 2011年8月1日, 下午4:03
 */

#ifndef ALLOCATOR_H
#define	ALLOCATOR_H

#include <stddef.h>
#include <assert.h>
#include <boost/thread/locks.hpp>
#include "util/none.h"

namespace boost{
namespace coroutine{
namespace memory{
    
    //! linked allocator used for big memory buffering
    template<size_t block_size = 512, 
             size_t chunk_size = 8 ,
             class locker = boost::coroutine::util::none_mutex_t>
    class linked_allocator_t
    {    
        typedef locker mutex_type;
        typedef struct free_node_t
        {
            free_node_t* next;
        }free_node_t;
        
        BOOST_COROUTINE_NO_COPYABLE(linked_allocator_t)
    public:
        linked_allocator_t():
              _size(block_size), 
              _expand_blocks(chunk_size),
              _free_blocks(0),      
              _free_list(0)
        {}
        
        ~linked_allocator_t()
        {
            boost::lock_guard<mutex_type> guard(_mutex);
            free_node_t* head = _free_list;
            free_node_t* p;
            while(head)
            {
                p = head;
                head = head->next;
                ::free(p);
            }
        }

        void* malloc(size_t size)
        {
            assert(size == _size);
            boost::lock_guard<mutex_type> guard(_mutex);

            if(0 == _free_list) 
            {
                expand_free_list();
            }
            free_node_t* head = _free_list;
            _free_list = head->next;
            _free_blocks--;

            return (void*)head;
        }

        void  free(void* ptr)
        {
            if(!ptr) return;
            boost::lock_guard<mutex_type> guard(_mutex);
            if(_free_blocks >= _expand_blocks * 2) ::free(ptr);
            else
            {
                free_node_t* head = static_cast<free_node_t*>(ptr);
                head->next = _free_list;
                _free_list = head;
                _free_blocks++;
            }
        }
    private:
        void expand_free_list()
        {
            
            size_t new_sz = _size >= sizeof(free_node_t*) ? _size : sizeof(free_node_t*);
            free_node_t* runner = static_cast<free_node_t*>(::malloc(new_sz));
            assert(runner);
            _free_list = runner;
            _free_blocks++;
            for(size_t i = 0; i < _expand_blocks; i++)
            {
                runner->next = static_cast<free_node_t*>(::malloc(new_sz));
                if(runner->next)
                {
                    _free_blocks++;
                   runner = runner->next;
                } 
            }
            runner->next = 0;
        } 

    private:
        size_t  _size;
        size_t  _expand_blocks;
        size_t  _free_blocks;
        free_node_t*  _free_list;
        mutex_type    _mutex;
    };    
}}}
        

#endif	/* ALLOCATOR_H */

