#include "Public/network/bunch.h"
#include <algorithm>
#include <iterator>
#include "Public/util/util.h"
#include "Public/network/conf.h"
#include "Public/serializer/bitcopy.hpp"
#include <iostream>
#include "Public/network/packet.h"
#include "Public/serializer/FarchiveOut.h"

BunchReliableManager::BunchReliableManager()
{
    bclose=false;
    globalBunchInd=0;
    sendAckedCount=0;
    recvTotalBits=0;
    recvLatestBunchInd=0;
    sendNotifyCallback=nullptr;
}


void BunchReliableManager::notify(bool ack, BunchHeader &header)
{
    //接收端对接收包的响应
    bool legal=(
        !sendQueue.empty()&&
        header.bunchInd>=sendQueue.front().info.header.bunchInd&&
        header.bunchInd<=sendQueue.back().info.header.bunchInd);
    if(!legal)return;
    //
    for(auto itr=sendQueue.begin();itr!=sendQueue.end();++itr){
        auto ind=itr->info.header.bunchInd;
        if(ind==header.bunchInd){
            auto&info=*itr;
            if(ack){
                if(!info.acked){
                    info.acked=true;
                    ++sendAckedCount;
                }
            }else if(!info.acked){
                info.acked=false;
                BunchSend(info.info);
            }
            break;
        }
        Assert(ind>header.bunchInd,"怎么可能大于呢?");
    }
    //判断是否完毕
    if(sendAckedCount==sendQueue.size()){
        NotifySendOK();
    }
}

void BunchReliableManager::send(const Byte *data, U32 bit_size)
{
    sendWithNotify(data,bit_size,nullptr);
}

void BunchReliableManager::sendWithNotify(const Byte *data, U32 bit_size, BunchReliableManager::SendNotifyCallback callback)
{
    if(bclose)return;//如果发送了关闭连接得可靠bunch，那么就不允许后续得可靠数据进入
    if(bit_size==0||bit_size>(BUNCH_MAX_SIZE<<3)||data==nullptr)return;
    std::shared_ptr<Byte[]>dataCP(new Byte[(bit_size+7)>>3]);
    memcpy(dataCP.get(),data,(bit_size+7)>>3);  //因为肯定是整数字节
    waitQueue.push(std::remove_reference_t<decltype(waitQueue.front())>{dataCP,bit_size,callback,false});
}

void BunchReliableManager::close(BunchReliableManager::SendNotifyCallback callback)
{
    if(bclose)return;
    bclose=true;
    std::shared_ptr<Byte[]>dataCP(new Byte[1]());
    waitQueue.push(std::remove_reference_t<decltype(waitQueue.front())>{dataCP,1,callback,true});
}

void BunchReliableManager::postsend()
{
    
}

bool BunchReliableManager::receive(FArchiveIn &archive)
{
    bool breliable=0;
    bool bPartial=0;
    bool bFirstPartial=0;
    bool bFinalPartial=0;
    U16 bitSize=0;
    U64 bunchInd=0;
    archive.Serialize(&breliable,1);
    archive.Serialize(&bPartial,1);
    archive.Serialize(&bFirstPartial,1);
    archive.Serialize(&bFinalPartial,1);
    archive.Serialize(bitSize,bunchInd);
    std::shared_ptr<Byte[]>data(new Byte[(bitSize+7)>>3]);
    archive.Serialize(data.get(),bitSize);
    if(archive.IsError())return false;
    //如果没有数据，那么直接返回
    if(bitSize==0)return true; 
    //如果BunchInd小于等于处理正确得最新历史，那么直接返回
    if(bunchInd<=recvLatestBunchInd)return true;
    //
    //如果不是分片直接提交给上层
    if(!bPartial){
        //判断id是否匹配
        if(recvQueue.size())return false;//不可能接收队列不为空的情况下还能收到整包
        FArchiveIn archive;
        archive.SetBuffer(data.get(),bitSize);
        bool ret=processReceivedBunch(archive);
        if(ret)recvLatestBunchInd=std::max(recvLatestBunchInd,bunchInd);
        return ret;
    }
    //判断改bunch头是否正确
    if(recvQueue.empty()==false){
        if(bFirstPartial){
            if(recvQueue.front().ind<bunchInd)return false;
        }else if(bFinalPartial){
            if(recvQueue.back().ind>bunchInd)return false;
        }
    }
    //寻找插入位置
    auto fit_itr=recvQueue.end();
    for(auto itr=recvQueue.begin();itr!=recvQueue.end();++itr){
        if(itr->ind==bunchInd)return true;
        if(itr->ind>bunchInd){
            fit_itr=itr;
            break;
        }
    }
    RecvInfo info;
    info.bit_size=bitSize;
    info.data=data;
    info.ind=bunchInd;
    info.head=bFirstPartial;
    info.tail=bFinalPartial;
    recvTotalBits+=info.bit_size;
    recvQueue.insert(fit_itr,info);
    if(recvTotalBits>(BUNCH_MAX_SIZE<<3) || 
        recvQueue.size()>((BUNCH_MAX_SIZE+PACKET_MAX_SIZE-1)/PACKET_MAX_SIZE*2)){
        /*
        如果接收大小超过了最大限制或者接收的包数量太多了
        （这里*2是因为很明显一个packet纯chunk数据除去最后一个分片肯定至少占一半），
        那么直接丢弃，
        返回false,不理他(因为这分明是故意发的恶心人的包)
        */
        recvTotalBits=0;
        recvQueue.clear();
        return false;
    }
    //如果全部接收完毕，返回给上层
    if(recvQueue.front().head&&
    recvQueue.back().tail&&
    recvQueue.size()==recvQueue.back().ind-recvQueue.front().ind+1
    ){
        //合并数据提交上层
        FArchiveIn archive;
        Byte buf[BUNCH_MAX_SIZE];
        archive.SetBuffer(buf,recvTotalBits);
        //拷贝数据
        U32 bitoff=0;
        for(auto&ele:recvQueue){
            BitCopy(buf,bitoff,ele.data.get(),0,ele.bit_size);
            bitoff+=ele.bit_size;
        }
        //交给上层处理
        bool ret=processReceivedBunch(archive);
        if(ret)recvLatestBunchInd=std::max(recvLatestBunchInd,recvQueue.back().ind);
        //清空队列
        recvTotalBits=0;
        recvQueue.clear();
        return ret;
    }
    //
    return true;
}

std::pair<bool,BunchSendInfo> BunchReliableManager::getBunch()
{
    //如果队列空了，先看看等待队列是否还有数据
    if(needSendQueue.empty()){
        //如果还是在发上一个bunch，那么直接返回NULL
        if(!sendQueue.empty())return {false,BunchSendInfo{}};
        //如果等待队列也为空，直接返回
        if(waitQueue.empty())return  {false,BunchSendInfo{}};
        //反之，sendQueue可以发下一个bunch了
        pushFromWaitQueue();
    }
    //
    auto ret=needSendQueue.front();
    //
    needSendQueue.pop();
    //
    return {true,ret};
}

void BunchReliableManager::postreceive()
{
}

void BunchReliableManager::BunchSend(const BunchSendInfo&bunch)
{
    needSendQueue.push(bunch);
}

void BunchReliableManager::NotifySendOK()
{
    sendAckedCount=0;
    sendQueue.clear();
    if(sendNotifyCallback)sendNotifyCallback();
    sendNotifyCallback=nullptr;
}

bool BunchReliableManager::processReceivedBunch(FArchiveIn&archive)
{
    //目前先直接打印数据
    std::unique_ptr<Byte[]>data(new Byte[archive.ByteSize()+1]());
    archive.Serialize(data.get(),archive.Bits());
    std::cout<<((const char*)(data.get()))<<std::endl;
    return true;
}

void BunchReliableManager::pushFromWaitQueue()
{
    auto info=waitQueue.front();
    waitQueue.pop();
    //单个可发Bunch最大byte
    constexpr U32 limit=PACKET_MAX_SIZE-(sizeof(PacketNotificationHeader)+sizeof(Packet::packID)+sizeof(Packet::cookie)+sizeof(BunchHeader)+1);
    constexpr U32 limitBits=limit<<3;
    U32 offset=0;
    U32 bitsize=std::get<1>(info);
    Byte*buf=std::get<0>(info).get();
    sendNotifyCallback=std::get<2>(info);
    bool partial=bitsize>limitBits;
    bool close=std::get<3>(info);
    while(bitsize){
        auto bit_size=std::min(limitBits,bitsize);
        if(bit_size!=bitsize)bit_size=bit_size>>3<<3;
        bitsize-=bit_size;
        //
        {
            //拿到索引
            BunchHeader header;
            header.bPartial=partial;
            header.bFinalPartial=partial&&bitsize==0;
            header.bFirstPartial=partial&&offset==0;
            header.bitSize=bit_size;
            header.bReliable=true;
            header.bunchInd=++globalBunchInd;
            //
            U32 bytes=sizeof(BunchHeader)+((header.bitSize+7)>>3);
            std::shared_ptr<Byte[]>bunch(new Byte[bytes]);
            //
            FArchiveOut out;
            bool reliable=header.bReliable,partial=header.bPartial,firstP=header.bFirstPartial,finalP=header.bFinalPartial;
            U16 bitsize=header.bitSize;
            U64 bunchInd=header.bunchInd;
            out.SetBuffer(bunch.get(),bytes<<3);
            out.Serialize(&reliable,1);
            out.Serialize(&partial,1);
            out.Serialize(&firstP,1);
            out.Serialize(&finalP,1);
            out.Serialize(bitsize);
            out.Serialize(bunchInd);
            out.Serialize(buf+offset,bitsize);
            //放入队列
            BunchSendInfo bsi;
            bsi.bits=out.Bits();
            bsi.header=header;
            bsi.bunch=bunch;
            bsi.close=close;
            SendInfo si;
            si.info=bsi;
            si.acked=false;
            //放入队列
            sendQueue.push_back(si);
            BunchSend(sendQueue.back().info);
        }
        //
        offset+=bit_size>>3;
    }
    //
}

/********************************************************************不可靠bunch管理器************************************ */

BunchUnReliableManager::BunchUnReliableManager()
{
    globalBunchInd=0;
    recvTotalBits=0;
    recvLatestBunchInd=0;
    recvLastProgressTime=0;
    sendAckedCount=0;
    sendNotifyCallback=nullptr;
}


void BunchUnReliableManager::notify(bool ack, BunchHeader &header)
{
    bool legal=(
        !header.bReliable&&
        !sendQueue.empty()&&
        header.bunchInd>=sendQueue.front().info.header.bunchInd&&
        header.bunchInd<=sendQueue.back().info.header.bunchInd);
    if(!legal)return;
    //不可靠Bunch只要有一个分片NAK，整个Bunch就发送失败，不做重传。
    if(!ack){
        finishSend(false);
        return;
    }
    for(auto&info:sendQueue){
        if(info.info.header.bunchInd!=header.bunchInd)continue;
        if(!info.acked){
            info.acked=true;
            ++sendAckedCount;
        }
        break;
    }
    if(sendAckedCount==sendQueue.size())finishSend(true);
}

void BunchUnReliableManager::postsend()
{
}

void BunchUnReliableManager::send(const Byte *buf, U32 bit_size)
{
    sendWithNotify(buf,bit_size,nullptr);
}

void BunchUnReliableManager::sendWithNotify(const Byte *buf, U32 bit_size, BunchUnReliableManager::SendNotifyCallback notify)
{
    if(buf==nullptr||bit_size==0||bit_size>(BUNCH_MAX_SIZE<<3)){
        if(notify)notify(false);
        return;
    }
    std::shared_ptr<Byte[]>data(new Byte[(bit_size+7)>>3]);
    memcpy(data.get(),buf,(bit_size+7)>>3);
    waitQueue.push(std::remove_reference_t<decltype(waitQueue.front())>{data,bit_size,notify});
}

bool BunchUnReliableManager::receive(FArchiveIn &archive)
{
    bool breliable=false;
    bool bPartial=false;
    bool bFirstPartial=false;
    bool bFinalPartial=false;
    U16 bitSize=0;
    U64 bunchInd=0;
    archive.Serialize(&breliable,1);
    archive.Serialize(&bPartial,1);
    archive.Serialize(&bFirstPartial,1);
    archive.Serialize(&bFinalPartial,1);
    archive.Serialize(bitSize,bunchInd);
    std::shared_ptr<Byte[]>data(new Byte[(bitSize+7)>>3]);
    archive.Serialize(data.get(),bitSize);
    if(archive.IsError()||breliable)return false;
    if(bitSize==0)return true;
    //已经处理或已经确定丢弃的旧分片无需再次进入队列。
    if(bunchInd<=recvLatestBunchInd)return true;

    //新的完整Bunch已经到达时，之前未拼完的不可靠Bunch不再等待。
    if(!bPartial){
        if(!recvQueue.empty())clearRecvQueue(recvQueue.back().ind);
        FArchiveIn bunchArchive;
        bunchArchive.SetBuffer(data.get(),bitSize);
        bool ret=processReceivedBunch(bunchArchive);
        if(ret)recvLatestBunchInd=std::max(recvLatestBunchInd,bunchInd);
        return ret;
    }

    if(!recvQueue.empty()){
        auto&tail=recvQueue.back();
        //已经见到上一个Bunch的final，却仍缺少中间分片；更大的ind说明它已经丢包。
        //新的first同样代表下一个Bunch开始，旧队列可直接放弃。
        if(bunchInd>tail.ind&&(tail.tail||bFirstPartial)){
            clearRecvQueue(tail.ind);
        }
    }

    auto insertPos=recvQueue.end();
    for(auto itr=recvQueue.begin();itr!=recvQueue.end();++itr){
        if(itr->ind==bunchInd){
            //重复分片虽然不重复入队，但说明这条不可靠Bunch仍有网络活动。
            recvLastProgressTime=GetTimeMS();
            return true;
        }
        if(itr->ind>bunchInd){
            insertPos=itr;
            break;
        }
    }
    RecvInfo info;
    info.data=data;
    info.ind=bunchInd;
    info.bit_size=bitSize;
    info.head=bFirstPartial;
    info.tail=bFinalPartial;
    recvTotalBits+=bitSize;
    recvQueue.insert(insertPos,info);
    recvLastProgressTime=GetTimeMS();

    if(recvTotalBits>(BUNCH_MAX_SIZE<<3)||
    recvQueue.size()>((BUNCH_MAX_SIZE+PACKET_MAX_SIZE-1)/PACKET_MAX_SIZE*2)
    ){
        clearRecvQueue(recvQueue.back().ind);
        return false;
    }

    if(recvQueue.front().head&&
       recvQueue.back().tail&&
       recvQueue.size()==recvQueue.back().ind-recvQueue.front().ind+1){
        std::unique_ptr<Byte[]>merged(new Byte[(recvTotalBits+7)>>3]());
        U32 bitOffset=0;
        for(auto&fragment:recvQueue){
            BitCopy(merged.get(),bitOffset,fragment.data.get(),0,fragment.bit_size);
            bitOffset+=fragment.bit_size;
        }
        FArchiveIn bunchArchive;
        bunchArchive.SetBuffer(merged.get(),recvTotalBits);
        U64 latestInd=recvQueue.back().ind;
        bool ret=processReceivedBunch(bunchArchive);
        clearRecvQueue(ret?latestInd:0);
        return ret;
    }
    return true;
}

std::pair<bool,BunchSendInfo> BunchUnReliableManager::getBunch()
{
    if(needSendQueue.empty()){
        if(!sendQueue.empty())return {false,BunchSendInfo{}};
        if(waitQueue.empty())return {false,BunchSendInfo{}};
        pushFromWaitQueue();
    }
    auto ret=needSendQueue.front();
    needSendQueue.pop();
    return {true,ret};
}

void BunchUnReliableManager::postreceive()
{
    if(recvQueue.empty())return;
    bool stalled=recvLastProgressTime&&
        GetTimeMS()-recvLastProgressTime>UNRELIABLE_BUNCH_RECV_STALL_TIMEOUT_MS;
    if(stalled||recvGapTooLarge()){
        clearRecvQueue(recvQueue.back().ind);
    }
}

void BunchUnReliableManager::BunchSend(const BunchSendInfo &bunch)
{
    needSendQueue.push(bunch);
}

void BunchUnReliableManager::finishSend(bool success)
{
    sendAckedCount=0;
    sendQueue.clear();
    while(!needSendQueue.empty())needSendQueue.pop();
    auto notify=sendNotifyCallback;
    sendNotifyCallback=nullptr;
    if(notify)notify(success);
}

void BunchUnReliableManager::clearRecvQueue(U64 latestInd)
{
    if(latestInd)recvLatestBunchInd=std::max(recvLatestBunchInd,latestInd);
    recvQueue.clear();
    recvTotalBits=0;
    recvLastProgressTime=0;
}

bool BunchUnReliableManager::recvGapTooLarge() const
{
    if(recvQueue.size()<2)return false;
    U64 totalSpan=recvQueue.back().ind-recvQueue.front().ind+1;
    U64 maxMissing=totalSpan/UNRELIABLE_BUNCH_RECV_GAP_RATIO_DENOMINATOR;
    auto previous=recvQueue.begin();
    for(auto current=std::next(previous);current!=recvQueue.end();++current,++previous){
        //只计算相邻两个已收到分片之间连续缺失的数量。
        //例如100100110的最大缺口是2，而不是所有0的总数。
        U64 missing=current->ind-previous->ind-1;
        if(missing>maxMissing)return true;
    }
    return false;
}

bool BunchUnReliableManager::processReceivedBunch(FArchiveIn &archive)
{
    std::unique_ptr<Byte[]>data(new Byte[archive.ByteSize()+1]());
    archive.Serialize(data.get(),archive.Bits());
    if(archive.IsError())return false;
    std::cout<<reinterpret_cast<const char*>(data.get())<<std::endl;
    return true;
}

void BunchUnReliableManager::pushFromWaitQueue()
{
    auto info=waitQueue.front();
    waitQueue.pop();
    constexpr U32 limit=PACKET_MAX_SIZE-(sizeof(PacketNotificationHeader)+sizeof(Packet::packID)+sizeof(Packet::cookie)+sizeof(BunchHeader)+1);
    constexpr U32 limitBits=limit<<3;
    U32 offset=0;
    U32 bitsLeft=std::get<1>(info);
    Byte*buf=std::get<0>(info).get();
    sendNotifyCallback=std::get<2>(info);
    bool partial=bitsLeft>limitBits;
    while(bitsLeft){
        U32 bitSize=std::min(limitBits,bitsLeft);
        if(bitSize!=bitsLeft)bitSize=bitSize>>3<<3;
        bitsLeft-=bitSize;

        BunchHeader header;
        header.bReliable=false;
        header.bPartial=partial;
        header.bFirstPartial=partial&&offset==0;
        header.bFinalPartial=partial&&bitsLeft==0;
        header.bitSize=static_cast<U16>(bitSize);
        header.bunchInd=++globalBunchInd;

        U32 bytes=sizeof(BunchHeader)+((bitSize+7)>>3);
        std::shared_ptr<Byte[]>bunch(new Byte[bytes]);
        FArchiveOut out;
        out.SetBuffer(bunch.get(),bytes<<3);
        bool reliable=false;
        bool first=header.bFirstPartial;
        bool final=header.bFinalPartial;
        U16 serializedBitSize=header.bitSize;
        U64 bunchInd=header.bunchInd;
        out.Serialize(&reliable,1);
        out.Serialize(&partial,1);
        out.Serialize(&first,1);
        out.Serialize(&final,1);
        out.Serialize(serializedBitSize);
        out.Serialize(bunchInd);
        out.Serialize(buf+offset,bitSize);

        BunchSendInfo sendInfo;
        sendInfo.bits=out.Bits();
        sendInfo.close=false;
        sendInfo.bunch=bunch;
        sendInfo.header=header;
        sendQueue.push_back(SendInfo{sendInfo,false});
        BunchSend(sendInfo);
        offset+=bitSize>>3;
    }
}
