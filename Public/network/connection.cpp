#include "Public/network/connection.h"
#include<iostream>
#include"Public/vec/bitset.hpp"
#include "Public/profile/profile.h"
#include <limits>
#include "Public/serializer/FarchiveOut.h"

Connection::Connection(std::shared_ptr<AddrInfo> addr_info,const std::string&cookie)
{
    TargetInfo=addr_info;
    this->cookie=cookie;
    reliableManager=std::make_unique<BunchReliableManager>();
    unReliableManager=std::make_unique<BunchUnReliableManager>();
    bclose=false;
    bclosed=false;
    bNeedclose=false;
    needCloseBeginTime=0;
    globalPacketID=1;
    sendHistorySeq=1;
    recvHistorySeq=1;
    sendHistoryLatestSeq=0;
    recvLatestHistorySeq=0;
    recvLatestHistorySeqTick=0;
    recvHistotySeqEnhance=0;
    keepAliveLastSendTime=GetTimeMS();
}

void Connection::close()
{
    //不可以重复调用
    if(bclose)return;
    bclose=true;
    needCloseBeginTime=GetTimeMS();
    reliableManager->close([=](){
        this->bclosed=true;
    });
}

bool Connection::closed()const
{
    return bclosed;
}

bool Connection::receive(FArchiveIn &archive)
{
    PROFILE_TRACE();
    //如果连接关闭了就不收包了
    if(closed())return false;
    //跳过cookie和connect位
    archive.SetPos(1+(PACKET_COOKIE_SIZE<<3));
    //拿到关闭连接位和包的编号
    bool close=false;
    U64 packID;
    archive.Serialize(&close,1);
    archive.Serialize(packID);
    //判断包的编号是否合法
    if(archive.IsError() || 
        packID<=recvLatestHistorySeqTick || 
        packID>=recvHistorySeq+PACKET_ACKED_HISTORY_MAX_BITS 
    ){
        /*
        1、包的编号<=历史已发送ack的记录
        2、包的编号>=接收ack位图最大记录
        则包非法，直接丢弃，可能由于网速原因导致
        */
        return true;
    }
    //判断是否是断开连接的包
    if(close)
    {
        if(!bNeedclose){
            bclose=true;
            bNeedclose=true;
            needCloseBeginTime=GetTimeMS();
        }
    }
    //解析普通包
    if(!parseHeader(packID,archive))return false;
    //
    return true;
}

void Connection::reliableSend(const Byte *buf, U32 bits)
{
    reliableManager->send(buf,bits);
}

void Connection::send(const Sock &sock)
{
    //如果连接已经关闭了，就不发包了
    if(closed())return;
    //
    bool reliableNull=false;
    while(bunchWaitForSend.size()<CONNECTION_MAX_SENDING_QUEUE_SIZE){
        auto&&ret=reliableNull?unReliableManager->getBunch():reliableManager->getBunch();
        if(!ret.first){
            //如果出于关闭状态，那么只能发送reliable的bunch
            if(!(reliableNull||bclose))reliableNull=true;
            else break;
            continue;
        }
        bunchWaitForSend.push_back(ret.second);
    }
    //如果没有包或者本连接接收到远程关闭信号了，那么就当keepalive了去发送
    auto curTime=GetTimeMS();
    bool networkNoBlock=false;
    std::vector<const BunchSendInfo*>bunchMerge;
    if(bunchWaitForSend.empty()){
        if((curTime-keepAliveLastSendTime)>=KEEP_ALIVE_SEND_INTERVAL){
            keepAliveLastSendTime=curTime;
            networkNoBlock=wrapBunchToPacket(bunchMerge,true)&&sendWrapPacket(sock);
        }
    }else{
        keepAliveLastSendTime=curTime;
        bool first=true;
        while(bunchWaitForSend.empty()==false){
            //合并bunch
            U64 curBits=0;
            bunchMerge.clear();
            for(auto itr=bunchWaitForSend.begin();itr!=bunchWaitForSend.end();++itr){
                auto&info=(*itr);
                curBits+=info.bits;
                if(((curBits+7)>>3)+
                    sizeof(PacketNotificationHeader)+
                    sizeof(Packet::packID)+
                    sizeof(Packet::cookie)+1
                    >PACKET_MAX_SIZE){
                        break;
                    }
                bunchMerge.push_back(&info);
            }
            //
            networkNoBlock=wrapBunchToPacket(bunchMerge,first)&&sendWrapPacket(sock);
            if(!networkNoBlock)break;
            for(auto i=bunchMerge.size();i;--i){
                bunchWaitForSend.pop_front();
            }
            //
            first=false;
        }
    }
    //通知管理器发送完毕
    reliableManager->postsend();
    unReliableManager->postsend();
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

void Connection::postRecevive()
{
    PROFILE_TRACE();

    //判断是否处于Darining阶段且超时了
    if(bclose){
        if(GetTimeMS()>=needCloseBeginTime+CONNECTION_DRAINING_TIME){
            bclosed=true;
        }
    }
    //推进recv历史
    recvLatestHistorySeqTick=recvLatestHistorySeq;
    recvHistorySeq+=recvHistotySeqEnhance;
    recvHistoryBits>>=recvHistotySeqEnhance;
    recvHistotySeqEnhance=0;
    //推进send历史
    U64 sendHistoryEnhance=sendHistoryLatestSeq+1-sendHistorySeq;
    for(U64 i=0,j=sendHistorySeq;i<sendHistoryEnhance;++i,++j){
        packetAckedHistoryInfo.erase(j);
    }
    sendHistoryBits>>=sendHistoryEnhance;
    sendHistorySeq=sendHistoryLatestSeq+1;
}

bool Connection::wrapBunchToPacket(std::vector<const BunchSendInfo*>&bunch, bool needAckedInfo)
{
    //
    U8 recvAckedBits[PACKET_ACKED_HISTORY_MAX_BITS>>3];
    //预分配一个ID
    U64 packetID=canSend(needAckedInfo);
    U16 historyCount=0;//位图有效位个数
    U64 historySeq=0;
    if(0==packetID)return false;
    //
    if(needAckedInfo){
        //生成ack信息
        memcpy(&recvAckedBits,&recvHistoryBits,PACKET_ACKED_HISTORY_MAX_BITS>>3);
        historyCount=recvLatestHistorySeq+1-recvHistorySeq;
        historySeq=recvLatestHistorySeq;
    }
    FArchiveOut out;
    out.SetBuffer(packetWrapInfo.buf,sizeof(packetWrapInfo.buf)<<3);
    //判断是否是close包
    bool needclose=false;
    //如果包里面有一次close标志，最终都是close
    if(bclose){
        for(auto&bh:bunch){
            if(bh->close){
                needclose=true;
                break;
            }
        }
    }
    //
    bool connect=false;
    out.Serialize(&connect,1);
    out.Serialize(cookie.data(),PACKET_COOKIE_SIZE<<3);
    out.Serialize(&needclose,1);
    out.Serialize(packetID);
    //包装header
    out.Serialize(historySeq);
    out.Serialize(historySeq);
    out.Serialize(historyCount);
    out.Serialize(static_cast<void*>(recvAckedBits),PACKET_ACKED_HISTORY_MAX_BITS);
    //包装bunch
    for(auto&bc:bunch){
        out.Serialize(bc->bunch.get(),bc->bits);
    }
    Assert(out.IsError(),"按道理这里不应该出问题的!上层应该保证大小总和不超过单包大小的!");
    //额外信息
    packetWrapInfo.packetID=packetID;
    packetWrapInfo.bytes=out.ByteSize();
    packetWrapInfo.historyInfo.ackedHistorySeq=recvLatestHistorySeq;
    packetWrapInfo.historyInfo.bunchHeader.clear();
    for(auto&bh:bunch)packetWrapInfo.historyInfo.bunchHeader.push_back(bh->header);
    //
    return true;
}

bool Connection::sendWrapPacket(const Sock &sock)
{
    //
    bool success=packetWrapInfo.bytes==sock.write(packetWrapInfo.buf,packetWrapInfo.bytes,*TargetInfo);
    if(success){
        //寄存起来
        packetAckedHistoryInfo[packetWrapInfo.packetID]=packetWrapInfo.historyInfo;
    }
    return success;
}

void Connection::Acked(U64 packetID, bool acked)
{
    auto itr=packetAckedHistoryInfo.find(packetID);
    if(itr==packetAckedHistoryInfo.end())return;
    if(packetID<sendHistorySeq)return;
    auto pos=packetID-sendHistorySeq;
    if(pos>=PACKET_ACKED_HISTORY_MAX_BITS)return;
    acked?sendHistoryBits.set(pos):sendHistoryBits.reset(pos);//标志为1/0表示这个包接收端收到了/未收到
    //
    auto&packetInfo=itr->second;
    auto&bunchHeader=packetInfo.bunchHeader;
    //接收端知道了我已经收到了哪些包，要推进本地接收位图
    U64 more=packetInfo.ackedHistorySeq+1-recvHistorySeq;
    if(more>PACKET_ACKED_HISTORY_MAX_BITS)return;
    recvHistotySeqEnhance=std::max(recvHistotySeqEnhance,more);
    //通知相应的bunch已经发送成功了
    notifyBunchAcked(bunchHeader,acked);
}

void Connection::notifyBunchAcked(std::vector<BunchHeader> &bunchVec,bool acked)
{
    for(auto&bh:bunchVec){
        if(bh.bReliable){
            reliableManager->notify(acked,bh);
        }else{
            unReliableManager->notify(acked,bh);
        }
    }
}

U64 Connection::canSend(bool keepAlive)
{
    if(globalPacketID<(
        keepAlive?
        PACKET_ACKED_HISTORY_MAX_BITS+sendHistorySeq:
        PACKET_MAX_SEND_INSIGHT+sendHistorySeq
    )){
        return globalPacketID++;
    }
    return 0;
}


bool Connection::parseHeader(U64 packID,FArchiveIn&archive)
{
    PROFILE_TRACE();

    //解析packet
    U64 ackedPacketID,ackedHistorySeq;
    U16 historyCount;
    archive.Serialize(ackedPacketID);
    archive.Serialize(ackedHistorySeq);
    archive.Serialize(historyCount);
    if(archive.IsError()||historyCount>PACKET_ACKED_HISTORY_MAX_BITS)return false;
    BitSet<PACKET_ACKED_HISTORY_MAX_BITS> bs;
    archive.Serialize(&bs,PACKET_ACKED_HISTORY_MAX_BITS);
    if(archive.IsError())return false;
    //解析Bunch数据,如果解析失败就当是丢包处理就行了
    if(!parseBunch(archive))return false;
    //处理ack信息
    for(I64 i=historyCount-1,j=0;i>=0;--i,++j){
        I64 targetSeq=ackedHistorySeq-i;
        bool acked=bs.test(j);
        Acked(targetSeq,acked);
    }
    //更新接收的最大视野标记包收到
    recvLatestHistorySeq=std::max(recvLatestHistorySeq,packID);
    recvHistoryBits.set(packID-recvHistorySeq);
    //发送包已知最新历史情况
    sendHistoryLatestSeq=std::max(sendHistoryLatestSeq,ackedHistorySeq);
    //
    return true;
}

bool Connection::parseBunch(FArchiveIn &archive)
{
    U64 originPos;
    //因为packet里面没有信息告诉接收方包有多大，可以在解析到bunch的时候判断剩余bit是否小于8如果是说明这些bit是发包的补位
    while(archive.BitsLeft()>=8){ 
        bool reliable=false;
        originPos=archive.GetPos();
        archive.Serialize(&reliable,1);
        archive.SetPos(originPos);
        if(!(reliable?
            reliableManager->receive(archive):
            unReliableManager->receive(archive))
        )return false;
    }
    return true;
}
