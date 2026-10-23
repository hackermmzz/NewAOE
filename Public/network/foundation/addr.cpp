#include "Public/network/foundation/addr.h"
#include<memory.h>
#include<string>

NetAddrInfo::NetAddrInfo():m_port(0){
    m_ip[0]=0;
}

NetAddrInfo::NetAddrInfo(const char*ip,U16 port):NetAddrInfo(){
    set(ip,port);
}

void NetAddrInfo::set(const char*ip,U16 port){
    m_port=port;
    if(!ip){
        m_ip[0]=0;
        return;
    }
    U32 i=0;
    for(;i+1<IP_CAP&&ip[i];++i){
        m_ip[i]=ip[i];
    }
    m_ip[i]=0;
}

const char*NetAddrInfo::ip()const{
    return m_ip;
}

U16 NetAddrInfo::port()const{
    return m_port;
}

const char *NetAddrInfo::Info() const
{
    static thread_local char buf[IP_CAP+10];
    std::string info(ip());
    info+="_"+std::to_string(port());
    memcpy(buf,info.data(),info.size());
    buf[info.size()]=0;
    return buf;
}

void NetAddrInfo::copy(const AddrInfo &info)
{
    const NetAddrInfo*info_=static_cast<const NetAddrInfo*>(&info);
    this->m_port=info_->m_port;
    memcpy(this->m_ip,info_->m_ip,this->IP_CAP);
}

ProcAddrInfo::ProcAddrInfo(){
    m_name[0]=0;
}

ProcAddrInfo::ProcAddrInfo(const char*name):ProcAddrInfo(){
    set(name);
}

void ProcAddrInfo::set(const char*name){
    if(!name){
        m_name[0]=0;
        return;
    }
    U32 i=0;
    for(;i+1<NAME_CAP&&name[i];++i){
        m_name[i]=name[i];
    }
    m_name[i]=0;
}

const char*ProcAddrInfo::name()const{
    return m_name;
}

const char *ProcAddrInfo::Info() const
{
    return name();
}

void ProcAddrInfo::copy(const AddrInfo &info)
{
    const ProcAddrInfo*info_=static_cast<const ProcAddrInfo*>(&info);
    memcpy(this->m_name,info_->m_name,this->NAME_CAP);
}
