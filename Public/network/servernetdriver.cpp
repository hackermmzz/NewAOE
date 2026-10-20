#include "Public/network/servernetdriver.h"
#include "Public/util/util.h"
#include "Public/serializer/FarchiveOut.h"
#include<algorithm>

ServerNetDriver::ServerNetDriver(std::shared_ptr<Sock> sock_)
{
    sock=sock_;
    Assert(sock==nullptr,"传入的sock不能为空!");
}

void ServerNetDriver::receive()
{
    char buf[PACKET_MAX_SIZE*2];
    for(;;){
        auto info=sock->newInfo();
        auto siz=sock->read(buf,PACKET_MAX_SIZE,*info);
        if(siz==0){
            break;
        }
        parsePacket(buf,siz,info);
    }
    //更新Tick
    tick();
}

std::shared_ptr<Connection> ServerNetDriver::getConnection(const std::string&cookie)
{
    //
    auto itr=cookieToconnection.find(cookie);
    if(itr==cookieToconnection.end()){
        return nullptr;
    }
    return itr->second.connection;
}

bool ServerNetDriver::keepConnectionAlive(const std::string&cookie)
{
    auto itr=cookieToconnection.find(cookie);
    if(itr==cookieToconnection.end()){
        return false;
    }
    //cookie存在记录
    ConnectionInfo&info=itr->second;
    info.preAliveTime=GetTimeMS();
    return true;
}

ServerNetDriver::~ServerNetDriver()
{
}

bool ServerNetDriver::connectionPushStatus(const std::string&cookie, ServerNetDriver::ConnectionInfo::ProcessStateInfo info)
{
    //获取对应的connection记录
    auto itr=cookieToconnection.find(cookie);
    if(itr==cookieToconnection.end())return false;
    auto&m_info=itr->second;
    auto&m_queue=m_info.ProcessStateQueue;
    if(m_queue.size()>64){
        auto info_front=m_queue.front();
        if(info_front.status==false){
            --m_info.ProcessStateErrorCount;
        }
        m_queue.pop_front();
    }
    m_queue.push_back(info);
    if(info.status==false)++m_info.ProcessStateErrorCount;
    return true;
}

void ServerNetDriver::closeConnection(const std::string&cookie)
{   
    auto connection=getConnection(cookie);
    if(connection==nullptr)return;
    //关闭连接
    connection->close();
}

void ServerNetDriver::tick()
{
    //处理长时间未建立连接的信息
    processConnectionEstablishTimeout();
    //处理超时连接
    processTimeoutConnection();
    //处理短时间发送多次错误包的连接
    processPacketErrorConnection();
    //处理关闭连接的队列
    processConnectionClose();
}

void ServerNetDriver::processConnectionEstablishTimeout()
{
    std::vector<std::string>nr;
    auto curTime=GetTimeMS();
    for(auto&ele:connectionEstablishPhase){
        if((curTime-ele.second.updateTime)/1000>CONNECTION_ESTABLISH_TIMEOUT){
            nr.push_back(ele.first);
        }
    }
    //
    for(auto&info:nr){
        connectionEstablishPhase.erase(info);
    }
}

void ServerNetDriver::processTimeoutConnection()
{
    std::vector<std::string>needRemove;
    auto curTime=GetTimeMS();
    for(auto itr=cookieToconnection.begin();itr!=cookieToconnection.end();++itr){
        auto&info=itr->second;
        if((curTime-info.preAliveTime)/1000>CONNECTION_TIMEOUT){
            needRemove.push_back(itr->first);
        }
    }
    //移除并且关闭连接
    for(auto&cookie:needRemove){
        closeConnection(cookie);
    }
}

void ServerNetDriver::processConnectionClose()
{
    std::vector<std::shared_ptr<Connection>>nr;
    for(auto&connection:connections){
        if(connection->closed()){
            nr.push_back(connection);
        }
    }
    //移除已经关闭连接的连接
    for(auto&conn:nr){
        auto itr=std::find(connections.begin(),connections.end(),conn);
        connections.erase(itr);
        cookieToconnection.erase(conn->getCookie());
    }
}

void ServerNetDriver::processPacketErrorConnection()
{
    std::vector<std::string>nr;
    for(auto&ele:cookieToconnection){
        auto&info=ele.second;
        //短时间超过一半错误的直接关闭连接
        if(info.ProcessStateErrorCount>=32){
            nr.push_back(ele.first);
        }
    }
    //
    for(auto&cookie:nr){
        closeConnection(cookie);
    }
}

void ServerNetDriver::parsePacket(void *buf, U32 size, std::shared_ptr<AddrInfo> info)
{
    FArchiveIn archive;
    archive.SetBuffer(buf,size<<3);
    //判断是否是连接包
    bool bConnectionRequest=false;
    archive.Serialize(&bConnectionRequest,1);
    //判断archive是否error了，如果error了，那么就不处理了
    if(archive.IsError())return;
    //处理连接请求
    if(bConnectionRequest){
        parseConnectionRequest(archive,info);
        return;
    }
    //处理正常的数据包
    parseConnectionPacket(archive,info);
}

bool ServerNetDriver::parseConnectionHello(FArchiveIn &archive, std::shared_ptr<AddrInfo> info)
{
    static std::string resp_buf("Perfect, no falsification at all!");
    //拿到hello数据
    constexpr U8 buf_len = std::char_traits<char>::length(CONNECTION_HELLO_BUF);
    char buf[buf_len+1]={0};
    archive.Serialize(buf,buf_len<<3);
    if(!archive.IsError()){
        if(!strcmp(CONNECTION_HELLO_BUF,buf)){
                return sendClientConnectionEstablishInfo(
                    true,
                    CONN_REQ_HELLO,
                    resp_buf.size(),
                    (const Byte*)(resp_buf.c_str()),
                    *info
                );
            }
    }
    return false;
}

void ServerNetDriver::parseConnectionRequest(FArchiveIn&archive,std::shared_ptr<AddrInfo> info)
{
    U64 curTime=GetTimeMS();
    U8 phase=0;
    archive.Serialize(phase);
    if(archive.IsError())return;
    //获取前面记录的阶段
    U8 prePhase=U8(CONN_REQ_HELLO)-1;
    std::string key(info->Info());
    auto itr=connectionEstablishPhase.find(key);
    if(itr!=connectionEstablishPhase.end()){
        prePhase=itr->second.phase;
    }
    //如果和当前服务器记录的阶段匹配，进行处理(允许回滚)
    if(U8(prePhase+1)>=phase&&CONN_REQ_HELLO<=phase &&phase<=CONN_REQ_OK){
        //更新信息维护表
        if(itr==connectionEstablishPhase.end())
        {
            connectionEstablishPhase[key].createTime=curTime;
            itr=connectionEstablishPhase.find(key);
        }
        //阶段相同，可能是丢包引起，也可能是短时间发了大量包
        if(prePhase>=phase){
            //只有超过Ns钟才算重新发的请求，不然直接丢弃
            if((curTime-itr->second.updateTime)/1000>=itr->second.backoff){
                //重置backoff
                itr->second.backoff=std::min(I32(itr->second.backoff*2),I32(16));
            }else{
                return;
            }
        }else{
            //阶段前进，backoff变为1
            itr->second.backoff=1;
        }
        //
        bool status=false;
        switch (phase)
        {
        case CONN_REQ_HELLO:
            status=parseConnectionHello(archive,info);
            break;
        case CONN_REQ_CONN:
            parseRespNewConnection(archive,info);
            status=true;
            break;
        case CONN_REQ_OK:
            if(status=parseAddNewConnection(archive,info)){
                //移除信息，直接返回
                connectionEstablishPhase.erase(key);
                return;
            }
            break;
        }
        //
        itr->second.updateTime=curTime;
        U8 curPhase=status?phase:prePhase;
        itr->second.phase=curPhase;
        if(status){
            return;
        }
    }
    //失败或者阶段不匹配
    {
        static std::string buf("You bastard! How dare you fool me!");
        sendClientConnectionEstablishInfo(
            false,
            prePhase,
            buf.size(),
            (const Byte*)(buf.data()),
            *info); 
    }
}


void ServerNetDriver::parseRespNewConnection(FArchiveIn &archive,std::shared_ptr<AddrInfo> info)
{
    //对于已经生成了cookie的连接，不需要重新生成cookie，防止客户端一直窃取cookie
    //添加到本地待连接表
    auto&wait_info=connectionEstablishPhase[info->Info()];
    auto&cookie=wait_info.cookie;
    for(;cookie.size()==0;){
        //给客户端生成一个cookie
        std::string cookie_=UUID();
        if(connectionEstablishPhase.find(cookie_)!=connectionEstablishPhase.end())continue;
        cookie=cookie_;
    }
    //发送数据
    sendClientConnectionEstablishInfo(
        true,
        CONN_REQ_CONN,
        PACKET_COOKIE_SIZE,
        (const Byte*)(cookie.c_str()),
        *info);
}

bool ServerNetDriver::parseAddNewConnection(FArchiveIn & archive,std::shared_ptr<AddrInfo> info)
{
    std::shared_ptr<Connection>connection;
    //获取cookie
    std::string cookie(PACKET_COOKIE_SIZE,0);
    archive.Serialize(&cookie[0],PACKET_COOKIE_SIZE<<3);
    if(archive.IsError())return false;
    //不存在则违法返回
    auto itr=connectionEstablishPhase.find(info->Info());
    if(itr==connectionEstablishPhase.end() || 
        cookie!=itr->second.cookie
    )return false;
    connectionEstablishPhase.erase(itr);
    //生成Connection
    connection=std::make_shared<Connection>(info,cookie);
    //写入记录
    connections.push_back(connection);

    ServerNetDriver::ConnectionInfo conninfo;
    conninfo.preAliveTime=GetTimeMS();
    conninfo.connection=connection;
    cookieToconnection[cookie]=conninfo;
    //
    return true;
}

void ServerNetDriver::parseConnectionPacket(FArchiveIn &archive, std::shared_ptr<AddrInfo> info)
{
    auto curTime=GetTimeMS();
    //获取cookie
    std::string cookie(PACKET_COOKIE_SIZE,0);
    archive.Serialize(&cookie[0],PACKET_COOKIE_SIZE<<3);
    if(archive.IsError())return;
    //判断连接是否存在，如果不存在告诉客户端进行连接
    if(cookieToconnection.find(cookie)==cookieToconnection.end()){
        sendClientConnectionEstablishInfo(false,U8(CONN_REQ_HELLO-1),0,0,*info);
        return;
    }
    //
    bool status=false;
    //保活
    if(keepConnectionAlive(cookie)){
        //获取对应的connection
        auto connection=getConnection(cookie);
        if(connection){
            //交给connection处理包
            status=connection->parsePacket(archive);
        }
    }
    //记录结果
    ServerNetDriver::ConnectionInfo::ProcessStateInfo info_;
    info_.status=status;
    info_.timeStamp=curTime;
    connectionPushStatus(cookie,info_);
}

bool ServerNetDriver::sendClientConnectionEstablishInfo(bool ok, U8 phase, U16 data_len,const Byte *data, const AddrInfo &addrinfo)
{
    bool connect=true;
    FArchiveOut out;
    out.Serialize(&connect,1);
    out.Serialize(&ok,1);
    out.Serialize(phase);
    out.Serialize(data_len);
    if(data){
        out.Serialize(const_cast<Byte*>(data),data_len<<3);
    }
    return sock->write(out.GetData(),out.ByteSize(),addrinfo)==out.ByteSize();
}

