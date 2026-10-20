#include "Public/network/connection.h"
#include<iostream>

Connection::Connection(std::shared_ptr<AddrInfo> addr_info,const std::string&cookie)
{
    TargetInfo=addr_info;
    bclose=false;
}

void Connection::close()
{
    bclose=true;
}

bool Connection::closed()
{
    return false;
}

bool Connection::parsePacket(FArchiveIn &archive)
{
    archive.SetPos(0);
    //判断是否是连接
    bool connect=false;
    archive.Serialize(&connect,1);
    if(connect || archive.IsError())return false;
    //
    return true;
}

std::shared_ptr<AddrInfo> Connection::getAddrInfo() const
{
    return TargetInfo;
}

std::string Connection::getCookie() const
{
    return cookie;
}

Connection::~Connection()
{
}
