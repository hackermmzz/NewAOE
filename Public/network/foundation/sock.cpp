#include "Public/network/foundation/sock.h"

#include "Public/network/conf.h"

#include <boost/asio.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include"Public/util/util.h"

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace asio=boost::asio;
using asio::ip::udp;

struct NetSock::Impl{
    asio::io_context io;
    udp::socket sock;
    Impl():sock(io){}
};

namespace{

void openNonBlocking(udp::socket&sock,const udp::endpoint&local,bool bindPort){
    boost::system::error_code ec;
    sock.open(local.protocol(),ec);
    if(ec){
        return;
    }
    if(bindPort){
        sock.bind(local,ec);
        if(ec){
            sock.close();
            return;
        }
    }
    sock.non_blocking(true,ec);
    sock.set_option(asio::socket_base::receive_buffer_size(262144));    //设置为缓冲区最大值
    if(ec){
        sock.close();
    }
}

}

NetSock::NetSock():m_impl(std::make_unique<Impl>()){
    openNonBlocking(m_impl->sock,udp::endpoint(udp::v4(),0),false);
}

NetSock::NetSock(const char*ip,U16 port):m_impl(std::make_unique<Impl>()){
    if(!ip){
        return;
    }
    boost::system::error_code ec;
    const asio::ip::address address=asio::ip::make_address(ip,ec);
    if(ec){
        return;
    }
    openNonBlocking(m_impl->sock,udp::endpoint(address,port),true);
}

NetSock::~NetSock(){
    //Impl 在头文件里不完整，析构必须放在 cpp。reset 会关掉套接字再销毁 io_context。
    m_impl.reset();
}

std::shared_ptr<AddrInfo> NetSock::newInfo() const
{
    return std::make_shared<NetAddrInfo>();
}

U32 NetSock::read(void*buf,U32 siz,AddrInfo&addr)const{
    NetAddrInfo*info=dynamic_cast<NetAddrInfo*>(&addr);
    if(!info||!buf||!m_impl->sock.is_open()){
        return 0;
    }
    boost::system::error_code ec;
    udp::endpoint sender;
    const std::size_t n=m_impl->sock.receive_from(asio::buffer(buf,siz),sender,0,ec);
    if(ec){
        return 0;
    }
    info->set(sender.address().to_string().c_str(),static_cast<U16>(sender.port()));
    return static_cast<U32>(n);
}

U32 NetSock::write(const void*buf,U32 siz,const AddrInfo&addr)const{
    const NetAddrInfo*info=dynamic_cast<const NetAddrInfo*>(&addr);
    if(!info||!buf||!m_impl->sock.is_open()){
        return 0;
    }
    boost::system::error_code ec;
    const asio::ip::address address=asio::ip::make_address(info->ip(),ec);
    if(ec){
        return 0;
    }
    const udp::endpoint dest(address,info->port());
    const std::size_t n=m_impl->sock.send_to(asio::buffer(buf,siz),dest,0,ec);
    if(ec){
        return 0;
    }
    return static_cast<U32>(n);
}

namespace ipc=boost::interprocess;

struct ProcSock::Impl{
    ProcAddrInfo addr;
    std::unique_ptr<ipc::message_queue> queue;
    std::vector<Byte> scratch;
};

ProcSock::ProcSock(U16 queue_size,U16 msgSize):m_impl(std::make_unique<Impl>()){
    if(msgSize==0){
        return;
    }
    m_impl->scratch.resize(msgSize);
    U32 retries=0;
    for(;;){
        std::string name;
        try{
            name=UUID();
            m_impl->queue=std::make_unique<ipc::message_queue>(ipc::create_only,name.c_str(),queue_size,msgSize);
            m_impl->addr.set(name.c_str());
            return;
        }catch(const ipc::interprocess_exception&ex){
            if(!name.empty()&&ex.get_error_code()!=ipc::already_exists_error){
                ipc::message_queue::remove(name.c_str());
            }
        }catch(...){
            if(!name.empty()){
                ipc::message_queue::remove(name.c_str());
            }
        }
        if(retries==3){
            m_impl->queue.reset();
            m_impl->addr.set(nullptr);
            return;
        }
        ++retries;
    }
}

ProcSock::~ProcSock(){
    if(!m_impl){
        return;
    }
    const std::string name=m_impl->addr.name();
    m_impl->queue.reset();
    m_impl.reset();
    if(!name.empty()){
        ipc::message_queue::remove(name.c_str());
    }
}

std::shared_ptr<AddrInfo> ProcSock::newInfo() const
{
    return std::make_shared<ProcAddrInfo>();
}

ProcAddrInfo ProcSock::addr()const{
    return m_impl->addr;
}

U32 ProcSock::read(void*buf,U32 siz,AddrInfo&addr)const{
    ProcAddrInfo*addrInfo=dynamic_cast<ProcAddrInfo*>(&addr);
    if(!addrInfo||!buf||!m_impl->queue){
        return 0;
    }
    try{
        unsigned int priority=0;
        ipc::message_queue::size_type recvd=0;
        if(!m_impl->queue->try_receive(m_impl->scratch.data(),m_impl->scratch.size(),recvd,priority)){
            return 0;
        }
        if(recvd>siz || recvd<ProcAddrInfo::NAME_CAP){
            return 0;
        }
        //先拷贝名称
        constexpr auto nameLen=ProcAddrInfo::NAME_CAP;
        char name[nameLen];
        memcpy(name,m_impl->scratch.data(),nameLen);
        memcpy(buf,static_cast<Byte*>(m_impl->scratch.data())+nameLen,recvd-nameLen);
        addrInfo->set(name);
        return static_cast<U32>(recvd-nameLen);
    }catch(...){
        return 0;
    }
}

U32 ProcSock::write(const void*buf,U32 siz,const AddrInfo&addr)const{
    const ProcAddrInfo*info=dynamic_cast<const ProcAddrInfo*>(&addr);
    if(!info||!buf||!info->name()[0]){
        return 0;
    }
    try{
        //拷贝数据
        const char*name=m_impl->addr.name();
        constexpr U32 nameLen=ProcAddrInfo::NAME_CAP;
        U32 dataLen=siz+nameLen;
        std::unique_ptr<Byte[]> buffer(new Byte[dataLen]);
        memcpy(static_cast<void*>(buffer.get()),name,nameLen);
        memcpy(static_cast<void*>(&(buffer.get()[nameLen])),buf,siz);
        //
        ipc::message_queue queue(ipc::open_only,info->name());
        if(!queue.try_send(buffer.get(),dataLen,0)){
            return 0;
        }
        return siz;
    }catch(...){
        return 0;
    }
}
