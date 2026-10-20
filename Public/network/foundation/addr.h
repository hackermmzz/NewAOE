#pragma once
#include "Public/type/type.h"

//基础的地址信息
class AddrInfo{
public:
    virtual ~AddrInfo()=default;
    virtual const char* Info()const =0;
    virtual void copy(const AddrInfo&info)=0;
};


//网络连接的地址信息
class NetAddrInfo:public AddrInfo{
public:
    NetAddrInfo();
    NetAddrInfo(const char*ip,U16 port);
    void set(const char*ip,U16 port);
    const char*ip()const;
    U16 port()const;
    virtual const char* Info()const;
    virtual void copy(const AddrInfo&info);
private:
    static constexpr U32 IP_CAP=64;
    char m_ip[IP_CAP];
    U16 m_port;
};


//共享内存的地址信息，保存消息队列名
class ProcAddrInfo:public AddrInfo{
public:
    static constexpr U32 NAME_CAP=64;

public:
    ProcAddrInfo();
    explicit ProcAddrInfo(const char*name);
    void set(const char*name);
    const char*name()const;
    virtual const char* Info()const;
    virtual void copy(const AddrInfo&info);
private:
    char m_name[NAME_CAP];
};
