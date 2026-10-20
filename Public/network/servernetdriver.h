#pragma once

#include<vector>
#include<set>
#include<unordered_map>
#include<map>
#include "Public/network/connection.h"
#include "Public/network/conf.h"
#include "Public/network/packet.h"
#include"Public/serializer/FarchiveIn.h"
#include <queue>
#include<tuple>


class ServerNetDriver{
private:
    //建立连接信息
    struct ConnectionEstablishInfo{
        U64 createTime;
        U64 updateTime;
        U8 phase;
        I32 backoff=1;
        std::string cookie;
    };
    //维护的连接数据
    struct ConnectionInfo{
        U64 preAliveTime;
        std::shared_ptr<Connection> connection;
        //
        struct ProcessStateInfo{
            bool status;
            U64 timeStamp;
        };
        std::deque<ProcessStateInfo>ProcessStateQueue;
        U16 ProcessStateErrorCount;
    };
    //关闭连接信息
    struct ConnectionCloseInfo{
        std::shared_ptr<Connection>connection;
        U32 beginTime;
        U8  phase;
        I32 backoff=1;
        I32 closePacketPreSend=0;
    };

private:
std::vector<std::shared_ptr<Connection>>connections;
std::shared_ptr<Sock> sock;
std::unordered_map<std::string,ServerNetDriver::ConnectionInfo>cookieToconnection;
std::map<std::string,ServerNetDriver::ConnectionEstablishInfo>connectionEstablishPhase;
public:
    ServerNetDriver(std::shared_ptr<Sock> sock);
    //接收所有的包
    void receive();
    //根据cookie获取对应的连接
    std::shared_ptr<Connection> getConnection(const std::string&cookie);
    bool keepConnectionAlive(const std::string&cookie);
    //
    virtual ~ServerNetDriver();
private:
    bool connectionPushStatus(const std::string&cookie,ServerNetDriver::ConnectionInfo::ProcessStateInfo info);
    void closeConnection(const std::string&cookie);
    void tick();
    void processConnectionEstablishTimeout();
    void processTimeoutConnection();
    void processConnectionClose();
    void processPacketErrorConnection();
    void parsePacket(void*buf,U32 size,std::shared_ptr<AddrInfo> info);
    bool parseConnectionHello(FArchiveIn&archive,std::shared_ptr<AddrInfo> info);
    void parseConnectionRequest(FArchiveIn&archive,std::shared_ptr<AddrInfo> info);
    void parseRespNewConnection(FArchiveIn&archive,std::shared_ptr<AddrInfo> info);
    bool parseAddNewConnection(FArchiveIn&archive,std::shared_ptr<AddrInfo> info);
    void parseConnectionPacket(FArchiveIn&archive,std::shared_ptr<AddrInfo> info);
    bool sendClientConnectionEstablishInfo(bool ok,U8 phase,U16 data_len,const Byte*data,const AddrInfo&addrinfo);
};


