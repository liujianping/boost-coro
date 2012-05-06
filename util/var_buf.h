/* 
 * File:   var_buf.h
 * Author: liujianping
 *
 * Created on 2012年5月4日, 下午2:54
 */

#ifndef BOOST_COROUTINE_VAR_BUF_H
#define	BOOST_COROUTINE_VAR_BUF_H

#include "util/linked_allocator.h"
#include "util/none.h"
#include "util/singleton.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <string>
#include <vector>

#include <boost/thread/locks.hpp>
#include <boost/pool/object_pool.hpp>

namespace boost{
namespace coroutine{
        
    template<size_t page_size, 
             size_t page_buffize, 
             class  locker>
    class vbuf_page_t
    {
        BOOST_COROUTINE_NO_COPYABLE(vbuf_page_t)
                
        typedef boost::coroutine::memory::linked_allocator_t<page_size, 
                page_buffize, locker> vbuf_allocator_t;
        
        #define vbuf_allocator boost::coroutine::util::singleton<vbuf_allocator_t>::Instance()
    public:
        void*        next;
        size_t       offset_rd;
        size_t       offset_wr;
        char*        data;
    public:
        vbuf_page_t():
        next(0),offset_rd(0),offset_wr(0),data(0)
        {
            data = static_cast<char*>(vbuf_allocator.malloc(page_size));
            assert(data);
        }

        ~vbuf_page_t()
        {
            vbuf_allocator.free(data);
        }

        char* rd_ptr()
        {
            return (data + offset_rd);
        }

        void* wr_ptr()
        {
            return (data + offset_wr);
        }

        size_t wr_size()
        {
            return (page_size - offset_wr);
        }
        size_t rd_size()
        {
            return (offset_wr - offset_rd);
        }
        
        void reset()
        {
            offset_rd = offset_wr = 0;
        }
    };
    
    template<size_t page_size,
             size_t page_buffize,
             class  page_locker = boost::coroutine::util::none_mutex_t>
    class var_buf_t
    {
        BOOST_COROUTINE_NO_COPYABLE(var_buf_t)
        typedef page_locker mutex_type;
        typedef vbuf_page_t<page_size, page_buffize, mutex_type> vbuf_page_type;
        typedef boost::object_pool<vbuf_page_type> vpage_allocator_t;
    private:
        vbuf_page_type* _page_list;
        vbuf_page_type* _page_rd;
        vbuf_page_type* _page_wr;
        mutex_type      _mutex;
        vpage_allocator_t  _page_allocator;
    public:
        //! constructor
        var_buf_t():
        _page_list(0), 
        _page_rd(0),
        _page_wr(0)
        {}

        //! destuctor
      ~var_buf_t()
        {
           clear();
        }

        //! //! get can read length
        size_t length()
        {
            size_t sz = 0;
            vbuf_page_type* page = _page_rd;
            if(0 == _page_rd) return sz;

            while(page != _page_wr)
            {
                sz += page->rd_size();
                page = static_cast<vbuf_page_type*>(page->next);
            }
            sz += page->rd_size();
            return sz;
        }

        //! get the number of can read pages
        size_t rd_pages()
        {
            size_t count = 0;
            if(0 == _page_rd) return count;

            vbuf_page_type* page = _page_rd;
            while(page != _page_wr)
            {
                count++;
                page = static_cast<vbuf_page_type*>(page->next);
            }
            count++;
            return count;
        }

        //! get the number of can write pages
        size_t wr_pages()
        {
            size_t count = 0;
            if(0 == _page_wr) return count;

            vbuf_page_type* page = _page_wr;
            while(page)
            {
                count++;
                page = static_cast<vbuf_page_type*>(page->next);
            }
            return count;
        }

        //! get the number of pages
        size_t pages()
        {
            size_t count = 0;
            if(0 == _page_list) return count;

            vbuf_page_type* page = _page_list;
            while(page)
            {
                count++;
                page = static_cast<vbuf_page_type*>(page->next);
            }
            return count; 
        }

        //! //! get can write length
        size_t wr_size()
        {
            size_t sz = 0;
            vbuf_page_type* page = _page_wr;
            while(page)
            {
                sz += page->wr_size();
                page = static_cast<vbuf_page_type*>(page->next);
            }
            return sz;
        }
        
        size_t rd_size()
        {
            size_t sz = 0;
            vbuf_page_type* page = _page_rd;
            while(page)
            {
                sz += page->rd_size();
                page = static_cast<vbuf_page_type*>(page->next);
            }
            return sz;
        }
        //! prealloc the size
        void reserve(size_t sz)
        {
            //! shrink
            shrink();

            if(!_page_wr && !_page_rd && !_page_list)
            {
                {
                    boost::lock_guard<mutex_type> guard(_mutex);
                    _page_list = _page_allocator.construct();
                }
                
                _page_wr = _page_list;
                _page_rd = _page_list;
            }
            assert(_page_wr);
            size_t cur_sz = wr_size();

            vbuf_page_type* tail = page_tail();

            vbuf_page_type* npage;
            while(cur_sz < sz)
            {
                {
                    boost::lock_guard<mutex_type> guard(_mutex);
                    npage = _page_allocator.construct();
                }
                
                assert(npage);
                if(0 == tail)
                    tail = npage;
                else
                {
                    tail->next = npage;
                    tail = static_cast<vbuf_page_type*>(tail->next);
                }
                cur_sz += page_size;
            }
        }
        //! dealloc the readed buffer
        void shrink()
        {
            vbuf_page_type* head = _page_list;
            while(head != _page_rd)
            {
                _page_list = static_cast<vbuf_page_type*>(head->next);
                
                {
                   boost::lock_guard<mutex_type> guard(_mutex);
                   _page_allocator.destroy(head);
                }
                
                head = _page_list;
            }
        }
        //! clear all memory
        void clear()
        {
            vbuf_page_type* head = _page_list;
            while(head)
            {
                _page_list = static_cast<vbuf_page_type*>(head->next);
                {
                   boost::lock_guard<mutex_type> guard(_mutex);
                   _page_allocator.destroy(head);
                }
                head = _page_list;
            }
            _page_rd = 0;
            _page_wr = 0;
        }
        //! check readable
        bool empty()
        {
            return length() ? true : false;
        }

        void reset()
        {
            vbuf_page_type* page = _page_list;
            while(page)
            {
                page->reset();
                page = static_cast<vbuf_page_type*>(page->next);   
            }
            _page_rd = _page_list;
            _page_wr = _page_list;
        }

        void rd_iovec(std::vector<struct iovec>& vc)
        {
            size_t pages = rd_pages();
            if(0 == pages)
            {
                vc.clear();
                return;
            }

            vc.reserve(pages);
            vbuf_page_type* page = _page_rd;
            int i = 0;
            while(pages)
            {
                struct iovec iv;
                iv.iov_base = (void*)page->rd_ptr();
                iv.iov_len = page->rd_size();
                vc.push_back(iv);
                page = static_cast<vbuf_page_type*>(page->next);
                pages--;
            }
        }

        void wr_iovec(std::vector<struct iovec>& vc)
        {
            size_t pages = wr_pages();
            if(0 == pages)
            {
                vc.clear();
                return;
            }
            vc.reserve(pages);
            vbuf_page_type* page = _page_wr;
            while(pages)
            {
                struct iovec iv;
                iv.iov_base = page->wr_ptr();
                iv.iov_len = page->wr_size();
                vc.push_back(iv);
                page = static_cast<vbuf_page_type*>(page->next);
                pages--;
            }
        }

        //! write buffer
        size_t assign(const char* ptr, size_t len)
        {
            reset();
            return write(ptr, len);
        }
        size_t append(const char* ptr, size_t len)
        {
            return write(ptr, len);
        }
        size_t append(const var_buf_t& vbuf)
        {
            size_t appended = 0;
            vbuf_page_type* page = vbuf._page_rd;
            while(page)
            {
                appended += this->append(page->rd_ptr(), page->rd_size());
                if(page == vbuf._page_wr) break;
                page = static_cast<vbuf_page_type*>(page->next);
            }
            return appended;
        }

        //! equal to append
        size_t write(const char* ptr, size_t len)
        {
            size_t cur_sz = wr_size();
            if(len > cur_sz) reserve(len);

            size_t want_write = len;
            size_t pos = 0;
            while(want_write)
            {
                assert(_page_wr);
                size_t wr = (want_write > _page_wr->wr_size() ? _page_wr->wr_size() : want_write);
                memcpy(_page_wr->wr_ptr(), (ptr + pos), wr);
                pos += wr;
                _page_wr->offset_wr += wr;
                want_write -= wr;
                if(want_write) _page_wr = static_cast<vbuf_page_type*>(_page_wr->next);
                assert(_page_wr);
            }
            return (len -  want_write);
        }

        //! read buffer
        size_t read(void* ptr, size_t len)
        {
            size_t cur_length = length();
            size_t want_read = (cur_length < len ? cur_length : len);
            size_t pos = 0;
            while(want_read)
            {
                assert(_page_rd);
                size_t rd = (want_read > _page_rd->rd_size() ? _page_rd->rd_size() : want_read);
                memcpy(ptr + pos, _page_rd->rd_ptr(), rd);
                pos += rd;
                _page_rd->offset_rd += rd;
                want_read -= rd;
                if(want_read) _page_rd = static_cast<vbuf_page_type*>(_page_rd->next);
            }
            return (len - want_read);
        }

        size_t read_by_line(std::string& line)
        {
            return read_by_endof_delimer("\n", line);
        }
        
        vbuf_page_type* find(const char* s)
        {
            vbuf_page_type* page = _page_rd;
            bool finded = false;
            while(page)
            {
                if(strnstr(page->rd_ptr(), s, page->rd_size())) 
                {
                    finded = true;
                    break;
                }    
                page = static_cast<vbuf_page_type*>(page->next);
            }
            return (finded ? page: 0);        
        }
        
        size_t read_by_endof_delimer(const char* d, std::string& s)
        {
            vbuf_page_type* page = find(d);
            if(!page) return 0;
            
            size_t readed = 0;
            while(_page_rd != page)
            {
                readed += _page_rd->rd_size();
                s.append(static_cast<const char*>(_page_rd->rd_ptr()), _page_rd->rd_size());
                _page_rd = static_cast<vbuf_page_type*>(_page_rd->next);
            }
            
            char* finded = strnstr(page->rd_ptr(), d, page->rd_size());
            if(!finded) return 0;
            size_t last = (finded - page->rd_ptr());
            readed += last;
            s.append(static_cast<const char*>(_page_rd->rd_ptr()), last);
            _page_rd->offset_rd += last + strlen(d);

            return readed;
        }

        size_t read_by_size(void* ptr, size_t sz)
        {
            return read(ptr, sz);
        }

        void readed()
        {
            _page_rd = _page_wr;
            _page_rd->offset_rd = _page_wr->offset_wr;
        }

        size_t rd_position(size_t inc)
        {
            if(!_page_rd) return 0;
            size_t len = length();
            if(inc >= len)
            {
                _page_rd->offset_rd = _page_rd->offset_wr;
                _page_rd = _page_wr;
                _page_rd->offset_rd = _page_rd->offset_wr;
                return len;
            }

            size_t wt_pos = inc;
            size_t mv_pos = 0;
            while(wt_pos)
            {
                mv_pos = (wt_pos > _page_rd->rd_size() ? _page_rd->rd_size() : wt_pos);

                _page_rd->offset_rd += mv_pos;
                _page_rd = static_cast<vbuf_page_type*>(_page_rd->next);

                wt_pos -= mv_pos;
            }

            return inc;
        }

        vbuf_page_type* page_tail()
        {
            vbuf_page_type* start = _page_wr;
            vbuf_page_type* tail = 0;
            while(start)
            {
                if(0 == start->next)
                {
                    tail = start;
                    break;
                }
                start = static_cast<vbuf_page_type*>(start->next);
            }
            return tail;
        }

        size_t wr_position(size_t inc)
        {
            if(!_page_wr) return 0;
            size_t len = wr_size();
            if(inc >= len)
            {
                _page_wr->offset_wr = page_size;
                _page_wr = page_tail();
                _page_wr->offset_wr = page_size;
                return len;
            }

            size_t wt_pos = inc;
            size_t mv_pos = 0;
            while(wt_pos)
            {
                mv_pos = (wt_pos > _page_wr->wr_size() ? _page_wr->wr_size() : wt_pos);

                _page_wr->offset_wr += mv_pos;
                _page_wr = static_cast<vbuf_page_type*>(_page_wr->next);

                wt_pos -= mv_pos;
            }
            return inc;
        }

        //! copy buffer to string, will not change read offset
        std::string to_string()
        {
            std::string s;
            if(!_page_rd) return s;
            vbuf_page_type* page = _page_rd;
            while(page != _page_wr)
            {
                s.append(page->rd_ptr(), page->rd_size());
                page = static_cast<vbuf_page_type*>(page->next);
            }
            s.append((char*)page->rd_ptr(), page->rd_size());
            return s;
        }    
    };
}}

#endif	/* VAR_BUF_H */

