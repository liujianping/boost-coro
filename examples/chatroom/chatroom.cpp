#include <boost/coroutine/coroutine.hpp>
#include <map>
using namespace boost::coasio;
using namespace boost::coroutine;
static void room(coreactor_t* coreactor);
static void chat(int fd, coreactor_t* coreactor);
static void chat_rd(cosocket_ptr_t socket);
static void chat_wr(cosocket_ptr_t socket);


void room(coreactor_t* coreactor)
{
        boost::this_coroutine::c_str("room");

    message_ptr_t msg;
    while(boost::this_coroutine::mget(msg, true))
    {
        logdebug("room mget msg[%p] flag:%X", msg.get(), msg->flag);
        //! acceptor msg
        if(msg->flag == MSG_INT)
        {
            logdebug("room mget p msg[%p]: sock %d", msg.get(), msg->value.i);
            coroutine_base_t* c = create_coroutine(chat, msg->value.i, coreactor);
            boost::this_coroutine::child(c);
            continue;
        }

        //! chater msg
        if((msg->flag & MSG_VBUF) && (msg->flag & MSG_CHAR))
        {
            if(msg->value.c == 'c')
            {
                logdebug("room [co:%p] get c msg[%p]:%s", boost::this_coroutine::get(), msg.get(), msg->vbuf.to_string().c_str());
                message_ptr_t bcast;
                bcast = msg;
                bcast->value.c = 'p';
                boost::this_coroutine::mbroadcast(bcast);
            }
        }
        //! none child, room still exist
        //! if(boost::this_coroutine::child() == 0) break;
    }
}
void chat(int fd, coreactor_t* coreactor)
{
        boost::this_coroutine::c_str("chat");
    cosocket_ptr_t socket(new cosocket_t(coreactor, fd));
    
    
    var_buf_t<32,1> vbuf;
    char* welcome = "please input a name:";
    socket->send((void*)welcome, strlen(welcome));
    if(socket->recv_vbuf(vbuf, 32) <= 0)
    {
        std::cout<<"client disconnected."<<std::endl;
        return;
    }
    std::string line;
    if(vbuf.read_by_endof_delimer("\r\n", line) >= 0)
    {
        message_ptr_t msg(new message_t('c'));
        msg->flag |= MSG_VBUF;
        msg->vbuf.append(line);
        msg->vbuf.append(" join the room\r\n", 17);
        boost::this_coroutine::parent()->mput(msg);
    }

    //! create rd/wr coro
    boost::coroutine::coroutine_base_t* coro_rd, *coro_wr;
    coro_rd = create_coroutine(chat_rd, socket);
    coro_wr = create_coroutine(chat_wr, socket);
    boost::this_coroutine::child(coro_rd);
    boost::this_coroutine::child(coro_wr);
    
    //! handle parent bcast msg & rd child msg
    message_ptr_t msg;
    while(boost::this_coroutine::mget(msg,true))
    {
        //! parent bcast msg
        if(msg->flag == (MSG_VBUF|MSG_CHAR))
        {
            //msg from parent
            if(msg->value.c == 'p')
            {
                logdebug("chat [co:%p] get p msg[%p]:%s", boost::this_coroutine::get(), msg.get(), msg->vbuf.to_string().c_str());
                coro_wr->mput(msg);
                continue;
            }
            
            //msg from child
            if(msg->value.c == 'c')
            {
                logdebug("chat [co:%p] get c msg[%p]:%s", boost::this_coroutine::get(), msg.get(), msg->vbuf.to_string().c_str());
                
                message_ptr_t c(new message_t('c'));
                c->vbuf.append(line);
                c->vbuf.append(" :", 2);
                std::string s = msg->vbuf.to_string(); 
                c->vbuf.append(s);
                c->flag |= MSG_VBUF;
                trace("chat [co:%p] mput p [co:%p] msg[%p]:%s", boost::this_coroutine::get(), 
                      boost::this_coroutine::parent(), msg.get(), c->vbuf.to_string().c_str());

                boost::this_coroutine::parent()->mput(c);
                continue;
            }
            
        }
        
        //! rd quit
        if(msg->flag == MSG_INT)
        {
            message_ptr_t c(new message_t('c'));
            c->vbuf.append(line);
            c->vbuf.append(" quit the room.\r\n", 17);
            c->flag |= MSG_VBUF;
            trace("chat [co:%p] mput p [co:%p] msg[%p]:%s", boost::this_coroutine::get(), 
                    boost::this_coroutine::parent(), msg.get(), c->vbuf.to_string().c_str());

            boost::this_coroutine::parent()->mput(c);
            break;
        }
    }
    message_ptr_t c(new message_t(1));
    coro_wr->mput(c);        
}

void chat_rd(cosocket_ptr_t socket)
{
        boost::this_coroutine::c_str("chat_rd");
    var_buf_t<128, 2> vbuf;
    while(socket->recv_vbuf(vbuf, 128) > 0)
    {
        std::string line;
        if(vbuf.read_by_endof_delimer("\r\n", line) > 0)
        {
            if(line == "quit") break;
            
            message_ptr_t msg(new message_t('c'));
            msg->flag |= MSG_VBUF;
            msg->vbuf.append(line);
            msg->vbuf.append("\r\n", 2);
            boost::this_coroutine::parent()->mput(msg);
        }
    }
    message_ptr_t msg(new message_t(1));
    boost::this_coroutine::parent()->mput(msg);

}
void chat_wr(cosocket_ptr_t socket)
{
    boost::this_coroutine::c_str("chat_wr");
    message_ptr_t msg;
    while(boost::this_coroutine::mget(msg,true))
    {
        if(msg->flag == MSG_INT) break;
        logdebug("chat wr [co:%p] get p msg[%p]:%s", boost::this_coroutine::get(), msg.get(), msg->vbuf.to_string().c_str());
        if(socket->send_vbuf(msg->vbuf) <= 0) break;
    }
}


class chatroom_acceptor_t: public boost::coasio::acceptor_t
{
public:
    chatroom_acceptor_t(coreactor_t* coreactor):
    boost::coasio::acceptor_t(coreactor),
    _coro_chatroom(0)
    {}
    
    virtual void accept(int client, sock_addr_t& remote_addr){
        boost::this_coroutine::c_str("accept");
        message_ptr_t msg(new message_t(client));
        chatroom()->mput(msg);
    }
    
    coroutine_base_t* chatroom()
    {
        if(!_coro_chatroom)
        {
            _coro_chatroom = create_coroutine(room, _coreactor);
            assert(_coro_chatroom);
            boost::this_coroutine::child(_coro_chatroom);
        }
        return _coro_chatroom;
    }
private:
    coroutine_base_t* _coro_chatroom;
};

int main()
{
    coreactor_t* coreactor = new coreactor_t();
    chatroom_acceptor_t chatroom_acceptor(coreactor);
    sock_addr_t sock_addr = boost::coasio::to_sock_addr("inet://127.0.0.1:9999");
    
    boost::this_coroutine::child(create_coroutine(chatroom_acceptor, sock_addr));
    boost::this_coroutine::child(create_coroutine(&coreactor_t::run, coreactor, true));
    
    boost::this_coroutine::resume();
}

