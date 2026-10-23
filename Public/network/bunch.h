#pragma once

#include "Public/type/type.h"
#include "Public/serializer/FarchiveIn.h"
#include <memory>
#include <list>
#include <queue>
#include<functional>

struct BunchHeader{
    //是否是可靠Bunch
    bool bReliable :1;
    //是否位分片
    bool bPartial :1;
    //是否是第一个分片
    bool bFirstPartial :1;
    //是否是最后一个分片
    bool bFinalPartial :1;
    //bunch大小(bit)
    U16 bitSize;
    //bunch的编号
    U64 bunchInd;
};

struct Bunch{
    BunchHeader header;
    Byte data[1];
};


struct BunchSendInfo{
    U32 bits;
    bool close;
    std::shared_ptr<Byte[]> bunch;
    BunchHeader header;
};

/***********************************管理Bunch*********************************/
class BunchManager{
private:
    
public:
    virtual void notify(bool ack,BunchHeader&header)=0;
    virtual void postsend()=0;
    virtual bool receive(FArchiveIn&archive)=0;
    virtual std::pair<bool,BunchSendInfo> getBunch()=0;
};


/***********************************可靠Bunch***************************** */
class BunchReliableManager:public BunchManager{
private:
    U64 globalBunchInd;
    //
    struct SendInfo{
        BunchSendInfo info; 
        bool acked;
    };

    struct RecvInfo{
        std::shared_ptr<Byte[]>data;
        U64 ind;
        U32 bit_size;
        bool head;
        bool tail;
    };
    //是否关闭连接
    bool bclose;
    //接收可靠数据
    std::list<RecvInfo>recvQueue;
    U32 recvTotalBits;
    U64 recvLatestBunchInd;
    //发送可靠数据
    std::list<SendInfo>sendQueue;
    U16 sendAckedCount;
    using SendNotifyCallback = std::function<void()>;
    BunchReliableManager::SendNotifyCallback sendNotifyCallback; //发送成功的回调
    std::queue<BunchSendInfo>needSendQueue;
    std::queue<std::tuple<std::shared_ptr<Byte[]>,U32,BunchReliableManager::SendNotifyCallback,bool>> waitQueue;
public:
    BunchReliableManager();
    virtual void notify(bool ack,BunchHeader&header);
    void send(const Byte*data,U32 bit_size);
    void sendWithNotify(const Byte*data,U32 bit_size,BunchReliableManager::SendNotifyCallback callback);
    void close(BunchReliableManager::SendNotifyCallback callback);
    virtual void postsend();
    virtual bool receive(FArchiveIn&archive);
    virtual std::pair<bool,BunchSendInfo> getBunch();

private:
    void BunchSend(const BunchSendInfo&bunch);
    void NotifySendOK();
    bool processReceivedBunch(FArchiveIn&archive);
    void pushFromWaitQueue();
}; 


/***********************************不可靠Bunch***************************** */
class BunchUnReliableManager:public BunchManager{
private:
    U64 globalBunchInd;
    
public:
    BunchUnReliableManager();
    virtual void notify(bool ack,BunchHeader&header);
    virtual void postsend();
    virtual bool receive(FArchiveIn&archive);
    virtual std::pair<bool,BunchSendInfo> getBunch();
};