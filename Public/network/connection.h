#pragma once

#include"Public/network/foundation/sock.h"
#include <memory>
#include<string>
#include "Public/serializer/FarchiveIn.h"

class Connection{
private:
    //目标地址信息
    std::shared_ptr<AddrInfo> TargetInfo;
    std::string cookie;
    bool bclose;
public:
    Connection(std::shared_ptr<AddrInfo> addr_info,const std::string&cookie);
    virtual void close();
    virtual bool closed();
    virtual bool parsePacket(FArchiveIn&archive);
    virtual std::shared_ptr<AddrInfo> getAddrInfo()const;
    virtual std::string getCookie()const;
    virtual ~Connection();
};