#pragma once
#include <memory>

#include<vector>
#include "Public/type/type.h"
#include "Public/network/foundation/addr.h"
#include "Public/network/conf.h"
//基础sock类型
class Sock{
public:
    virtual ~Sock()=default;
    virtual U32 read(void*buf,U32 siz,AddrInfo&addr)const=0;
    virtual U32 write(const void*buf,U32 siz,const AddrInfo&addr)const=0;
    virtual std::shared_ptr<AddrInfo> newInfo()const=0;
};


//udp的sock类型
class NetSock:public Sock{
public:
    NetSock();//客户端创建
    NetSock(const char*ip,U16 port);//服务器创建，绑定端口
    NetSock(const NetSock&)=delete;
    NetSock&operator=(const NetSock&)=delete;
    virtual U32 read(void*buf,U32 siz,AddrInfo&addr)const override;
    virtual U32 write(const void*buf,U32 siz,const AddrInfo&addr)const override;
    virtual ~NetSock();
    virtual std::shared_ptr<AddrInfo> newInfo()const;
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};


//共享内存的sock类型
class ProcSock:public Sock{
public:
    explicit ProcSock(U16 queue_size,U16 msgSize);
    ProcSock(const ProcSock&)=delete;
    ProcSock&operator=(const ProcSock&)=delete;
    ProcAddrInfo addr()const;
    virtual U32 read(void*buf,U32 siz,AddrInfo&addr)const override;
    virtual U32 write(const void*buf,U32 siz,const AddrInfo&addr)const override;
    virtual ~ProcSock();
    virtual std::shared_ptr<AddrInfo> newInfo()const;
private:
    struct Impl;
    mutable std::unique_ptr<Impl> m_impl;
};
