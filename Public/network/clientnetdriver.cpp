#include "Public/network/clientnetdriver.h"
#include "Public/serializer/FarchiveOut.h"
#include "Public/util/util.h"
#include "Public/profile/profile.h"

ClientNetDriver::ClientNetDriver(std::shared_ptr<Sock> sock_, const AddrInfo &addrInfo)
{
     sock=sock_;
     server_addr=sock_->newInfo();
     server_addr->copy(addrInfo);
     Assert(sock==nullptr,"sock不能为空!");

}

void ClientNetDriver::receive()
{
    PROFILE_TRACE();

    char buf[PACKET_MAX_SIZE*2];
    auto recv_addr=sock->newInfo();
    for(I32 i=0;i<DRIVER_RECEIVE_MAX_PACKAGE_NUM;i+=1){
        auto siz=sock->read(buf,PACKET_MAX_SIZE,*recv_addr);
        if(siz==0){
            break;
        }
        //检测是否是恶意包
        if(strcmp(recv_addr->Info(),server_addr->Info()))continue;
        parsePacket(buf,siz);
    }
    //
    if(server!=nullptr)server->postRecevive();
    //
    tick();
}

void ClientNetDriver::connect(bool reconnect)
{
    //关闭现有连接
    if(server!=nullptr){
        if(reconnect){
            server->close();
            while(server!=nullptr&&server->closed()==false){
                receive();
                send();
            }
        }else{
            return;//因为现在已经有连接了
        }
    }
    //
    char buf[PACKET_MAX_SIZE*2];
    FArchiveIn archive;
    U32 siz=0;
    I32 backoff=1;
    U8 prePhase=U8(-1);
    U8 curPhase=prePhase;
    auto recv_addr=sock->newInfo();
    for(;curPhase!=CONN_REQ_OK;){
        //睡眠N秒钟
        bool ret=wait_for(backoff,[&]()->bool{
            siz=sock->read(buf,PACKET_MAX_SIZE,*recv_addr);
            //检查是否是恶意包
            if(strcmp(recv_addr->Info(),server_addr->Info()))return false;
            if(siz){
                archive.SetBuffer(buf,siz<<3);
                return true;
            }
            return false;
        });
        //
        if(ret){
            curPhase=connectImpl(archive);
        }else{
            connectImpl(archive,U8(curPhase+1));
        }
        //如果阶段变更了，说明网络还行，恢复回溯
        if(curPhase!=prePhase)backoff=1;
        else backoff=std::min(I32(backoff*2),I32(16));
        prePhase=curPhase;
        archive.SetBuffer(buf,0);
    }
}

void ClientNetDriver::close()
{
    if(server==nullptr)return;
    //关闭连接
    server->close();
}

void ClientNetDriver::send()
{
    if(server==nullptr)return;
    server->send(*sock);
}

Connection*  ClientNetDriver::getserver()
{
    return server.get();
}

ClientNetDriver::~ClientNetDriver()
{
}

U8 ClientNetDriver::connectImpl(FArchiveIn &archive,U8 try_phase)
{
    archive.SetPos(0);
    //取出connect位
    bool connect=true;
    archive.Serialize(&connect,1);
    if(!connect)return false;
    //连接
    U8 phase=0;
    bool ok=false;
    U16 buf_len=0;
    std::vector<Byte> buf;
    archive.Serialize(&ok,1);
    archive.Serialize(phase);
    archive.Serialize(buf_len);
    //取出数据
    if(buf_len>0&&buf_len<=PACKET_COOKIE_SIZE){
        buf.resize(buf_len);
        archive.Serialize(&buf[0],buf_len<<3);
    }
    //如果读取出错或者拒绝连接，那么重HELLO开始重新连接
    if(archive.IsError() || !ok){
        phase=U8(try_phase-1);
    }
    //
    bool status=false;
    U8 nxtPhase=phase+1;
    switch (nxtPhase)
    {
    case CONN_REQ_HELLO:
        status=parseConnectionHello();
        break;
    case CONN_REQ_CONN:
        status=parseRespNewConnection();
        break;
    case CONN_REQ_OK:
        status=parseAddNewConnection(buf);
        break;
    }
    //
    if(status==true){
        return nxtPhase;
    }else{
        return phase;
    }
    //
    return status;
}

void ClientNetDriver::tick()
{
    PROFILE_TRACE();

    //关闭连接
    if(server!=nullptr&&server->closed()){
        server.reset(nullptr);
    }
}

void ClientNetDriver::parsePacket(void *buf, U32 siz)
{
    PROFILE_TRACE();

    FArchiveIn archive;
    archive.SetBuffer(buf,siz<<3);
    //判断是否是连接包
    bool bConnectionRequest=false;
    archive.Serialize(&bConnectionRequest,1);
    if(archive.IsError())return;
    //重新建立连接
    if(bConnectionRequest){
        server.reset(nullptr);
        connect(false);
        return;
    }
    //普通包
    if(server!=nullptr){
        //检查cookie是否存在且匹配
        char cookie[PACKET_COOKIE_SIZE];
        archive.Serialize(&cookie,PACKET_COOKIE_SIZE<<3);
        if(archive.IsError() || memcmp(cookie,server->getCookie().data(),PACKET_COOKIE_SIZE))return;
        bool status=server->receive(archive);
        //按道理都应该处理成功的，所以处理失败的情况直接assert
        Assert(!status,"按道理ClientDriver不可能处理包返回false的!");
    }
}


bool ClientNetDriver::parseConnectionHello()
{
    FArchiveOut out;
    bool connect=true;
    U8 phase=CONN_REQ_HELLO;
    std::string hello(CONNECTION_HELLO_BUF);
    out.Serialize(&connect,1);
    out.Serialize(phase);
    out.Serialize(&hello[0],hello.size()<<3);
    return sock->write(out.GetData(),out.ByteSize(),*server_addr)==out.ByteSize();
}

bool ClientNetDriver::parseRespNewConnection()
{
    FArchiveOut out;
    bool connect=true;
    U8 phase=CONN_REQ_CONN;
    out.Serialize(&connect,1);
    out.Serialize(phase);
    return sock->write(out.GetData(),out.ByteSize(),*server_addr)==out.ByteSize();
}

bool ClientNetDriver::parseAddNewConnection(const std::vector<Byte>&data)
{
    if(data.size()!=PACKET_COOKIE_SIZE)return false;
    //拿到cookie
    std::string cookie(data.begin(),data.end());
    //创建连接
    server.reset(new Connection(server_addr,cookie));
    //告诉服务器创建好了
    FArchiveOut out;
    bool connect=true;
    U8 phase=CONN_REQ_OK;
    out.Serialize(&connect,1);
    out.Serialize(phase);
    out.Serialize(cookie.data(),data.size()<<3);
    auto ret=sock->write(out.GetData(),out.ByteSize(),*server_addr)==out.ByteSize();
    //
    if(ret==false){
        server.reset(nullptr);
    }
    return ret;
}
