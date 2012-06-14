/* 
 * File:   var_buf.h
 * Author: liujianping
 *
 * Created on 2012年5月4日, 下午2:54
 */

#ifndef BOOST_COROUTINE_VAR_BUF_H
#define	BOOST_COROUTINE_VAR_BUF_H

#include <boost/coroutine/util/linked_allocator.h>
#include <boost/coroutine/util/none.h>
#include <boost/coroutine/util/macro.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <string>
#include <cstring>
#include <vector>
#include <string.h>

#include <boost/thread/locks.hpp>
#include <boost/pool/object_pool.hpp>

#ifndef __APPLE__
static char* strnstr(const char * str1, const char * str2, size_t n)
{
    char *cp =(char *)str1;
    char *s1,*s2;
    size_t left = n;
    if(!str2||!*str2)
        return (char*)str1;
    
    while(*cp && left)
    {
          s1 = cp;
          s2 = (char *) str2;
          while ( *s1 && *s2 && !(*s1-*s2))
              s1++, s2++;
          if (!*s2)
              return(cp);
          cp++;
          left--;
    }
     return 0;
}
#endif

namespace boost{
namespace coroutine{
        
    template<size_t page_size, 
             size_t page_buffize>
    class vbuf_page_t
    {
        BOOST_COROUTINE_NO_COPYABLE(vbuf_page_t)
                
        typedef boost::coroutine::memory::linked_allocator_t<page_size, 
                page_buffize> vbuf_allocator_t;
        
    public:
        void*        next;
        size_t       offset_rd;
        size_t       offset_wr;
        char*        data;
        vbuf_allocator_t& vbuf_allocator;
    public:
        vbuf_page_t(vbuf_allocator_t& allocator):
        next(0),offset_rd(0),offset_wr(0),data(0),vbuf_allocator(allocator)
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
    
    template<size_t page_size = 512,
             size_t page_buffize = 4>
    class var_buf_t
    {
        BOOST_COROUTINE_NO_COPYABLE(var_buf_t)
        typedef var_buf_t<page_size, page_buffize> var_buf_type;
        typedef vbuf_page_t<page_size, page_buffize> vbuf_page_type;
        typedef boost::object_pool<vbuf_page_type> vpage_allocator_t;
        typedef boost::coroutine::memory::linked_allocator_t<page_size, 
                page_buffize> vbuf_allocator_t;
    private:
        vbuf_page_type* _page_list;
        vbuf_page_type* _page_rd;
        vbuf_page_type* _page_wr;
        vpage_allocator_t  _page_allocator;
        vbuf_allocator_t   _vbuf_allocator;
    public:
        //! constructor
        var_buf_t():
        _page_list(0), 
        _page_rd(0),
        _page_wr(0)
        {}

        //! destuctor
        virtual ~var_buf_t()
        {
           clear();
        }
        //! readable length
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
            if(page) count++;
            
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

            if(!_page_wr || (!_page_rd && !_page_list))
            {
                _page_list = _page_allocator.construct(_vbuf_allocator);
                _page_wr = _page_list;
                _page_rd = _page_list;
            }
            assert(_page_wr);
            size_t cur_sz = wr_size();

            vbuf_page_type* tail = page_tail();

            vbuf_page_type* npage;
            while(cur_sz < sz)
            {
                npage = _page_allocator.construct(_vbuf_allocator);
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
            while(head && head != _page_rd)
            {
                _page_list = static_cast<vbuf_page_type*>(head->next);
                _page_allocator.destroy(head);
                
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
                _page_allocator.destroy(head);
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
            vbuf_page_type* page = _page_rd;
            while(page && page != _page_wr)
            {
                struct iovec iv;
                iv.iov_base = (void*)page->rd_ptr();
                iv.iov_len = page->rd_size();
                vc.push_back(iv);
                page = static_cast<vbuf_page_type*>(page->next);
            }
            if(page) 
            {
                struct iovec iv;
                iv.iov_base = (void*)page->rd_ptr();
                iv.iov_len = page->rd_size();
                vc.push_back(iv);
            }
        }

        void wr_iovec(std::vector<struct iovec>& vc)
        {
            vbuf_page_type* page = _page_wr;
            while(page)
            {
                struct iovec iv;
                iv.iov_base = page->wr_ptr();
                iv.iov_len = page->wr_size();
                vc.push_back(iv);
                page = static_cast<vbuf_page_type*>(page->next);
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
        size_t append(std::string& s)
        {
            return write(s.c_str(), s.length());
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
            size_t rd_pos = inc;
            size_t mv_pos = 0;
            while(rd_pos && _page_rd)
            {
                mv_pos = (rd_pos > _page_rd->rd_size() ? _page_rd->rd_size() : rd_pos);

                _page_rd->offset_rd += mv_pos;
                _page_rd = static_cast<vbuf_page_type*>(_page_rd->next);

                rd_pos -= mv_pos;
            }

            return (inc - rd_pos);
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
            size_t wt_pos = inc;
            size_t mv_pos = 0;
            while(wt_pos && _page_wr)
            {
                mv_pos = (wt_pos > _page_wr->wr_size() ? _page_wr->wr_size() : wt_pos);

                _page_wr->offset_wr += mv_pos;
                _page_wr = static_cast<vbuf_page_type*>(_page_wr->next);

                wt_pos -= mv_pos;
            }
            return (inc - wt_pos);
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
            if(page) s.append(page->rd_ptr(), page->rd_size());
            return s;
        }    
    };
}}

#endif	/* VAR_BUF_H */

