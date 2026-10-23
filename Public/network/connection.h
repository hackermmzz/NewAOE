#pragma once

#include"Public/network/foundation/sock.h"
#include <memory>
#include<string>
#include"Public/network/packet.h"
#include "Public/serializer/FarchiveIn.h"
#include "Public/vec/bitset.hpp"
#include<map>
#include "Public/network/bunch.h"
#include<list>

class Connection{
private: 
    struct PacketAckedHistoryInfo{
        U64 ackedHistorySeq;
        //对应的Bunch头信息
        std::vector<BunchHeader> bunchHeader;
    };
private:
    //目标地址信息
    std::shared_ptr<AddrInfo> TargetInfo;
    //连接的cookie
    std::string cookie;
    //是否关闭连接（发送方）
    bool bclose;
    //是否关闭了连接
    bool bclosed;
    //是否需要关闭连接 （接收方）
    bool bNeedclose;
    //被迫关闭连接起始时间
    U64 needCloseBeginTime;
    //全局packetID
    U64 globalPacketID;
    //发送的packet里面ack位图信息
    std::map<U64,Connection::PacketAckedHistoryInfo>packetAckedHistoryInfo;
    //本地接收的历史记录
    U64 recvHistorySeq; 
    U64 recvHistotySeqEnhance;
    U64 recvLatestHistorySeq;
    U64 recvLatestHistorySeqTick;
    BitSet<PACKET_ACKED_HISTORY_MAX_BITS>recvHistoryBits;
    //本地发送的历史记录
    U64 sendHistorySeq; //history第一位表示的起始点
    U64 sendHistoryLatestSeq; //发送的包里面接收方已经确认丢失/接收的最大位
    BitSet<PACKET_ACKED_HISTORY_MAX_BITS>sendHistoryBits; //历史记录确认位图
    // 可靠bunch管理器
    std::unique_ptr<BunchReliableManager> reliableManager;
    //不可靠bunch管理器
    std::unique_ptr<BunchUnReliableManager>unReliableManager;
    //keepalive包上次发送时间
    U64 keepAliveLastSendTime;
    //packet临时内存
    struct PacketWrapInfo{
        PacketAckedHistoryInfo historyInfo;
        U32 bytes;
        U64 packetID;
        Byte buf[PACKET_MAX_SIZE];
    }packetWrapInfo;
    //bunch待发送队列
    std::list<BunchSendInfo>bunchWaitForSend;
public:
    Connection(std::shared_ptr<AddrInfo> addr_info,const std::string&cookie);
    void close();
    bool closed()const;
    bool receive(FArchiveIn&archive);
    void reliableSend(const Byte*buf,U32 bits);
    void send(const Sock& sock);
    std::shared_ptr<AddrInfo> getAddrInfo()const;
    std::string getCookie()const;
    ~Connection();
    void postRecevive();
private:
    bool wrapBunchToPacket(std::vector<const BunchSendInfo*>&bunch,bool needAckedInfo);
    bool sendWrapPacket(const Sock& sock);
    void Acked(U64 packetID,bool acked);
    void notifyBunchAcked(std::vector<BunchHeader>&bunchVec,bool acked);
    U64 canSend(bool keepAlive);
    bool parseHeader(U64 packID,FArchiveIn&archive);
    bool parseBunch(FArchiveIn&archive);
};