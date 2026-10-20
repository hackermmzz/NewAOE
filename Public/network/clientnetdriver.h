#pragma once

#include<vector>
#include "Public/network/connection.h"
#include "Public/network/conf.h"
#include "Public/network/packet.h"

class ClientNetDriver{
private:
std::shared_ptr<AddrInfo> server_addr;
std::unique_ptr<Connection> server;
std::shared_ptr<Sock> sock;
public:
    ClientNetDriver(std::shared_ptr<Sock> sock,const AddrInfo&addrInfo);
    void receive();
    void connect();
    void close();
    virtual ~ClientNetDriver();
private:
    U8 connectImpl(FArchiveIn&archive);
    void tick();
    void parsePacket(void*buf,U32 siz);
    bool parseConnectionHello();
    bool parseRespNewConnection();
    bool parseAddNewConnection(const std::vector<Byte>&data);
};
