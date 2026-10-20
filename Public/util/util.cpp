#include "Public/util/util.h"
#include <boost/chrono.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <thread>
#include<iostream>

U64 GetTimeMS()
{
    auto tp = boost::chrono::system_clock::now();
    auto dur = tp.time_since_epoch();
    return boost::chrono::duration_cast<boost::chrono::milliseconds>(dur).count();
}


boost::uuids::random_generator uuid_makeId;
std::string UUID()
{
    const boost::uuids::uuid id=uuid_makeId();
    if(id.is_nil()){
        //通过时间生成16位随机数
        constexpr const char* buf="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-";
        constexpr U8 buf_len = std::char_traits<char>::length(buf);
        std::string ret;
        ret.resize(16);
        for(auto i=0;i<16;i+=1){
            ret[i]=buf[rand()%buf_len];
        }
        std::cerr<<"UUID get nil!"<<std::endl;
        return ret;
    }
    return std::string(reinterpret_cast<const char*>(id.begin()), 16);
}

bool wait_for(U32 seconds, const std::function<bool()> &cond)
{
    U32 totalMs=seconds*1000;
    constexpr U32 step=10;
    while(totalMs>=0){
        totalMs-=step;
        std::this_thread::sleep_for(std::chrono::milliseconds(step));
        if(cond){
            return true;
        }
    }
    return false;
}
