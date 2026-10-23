#include "Public/util/util.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <thread>
#include<iostream>
#include <chrono>

U64 GetTimeMS()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>
    (std::chrono::steady_clock::now().time_since_epoch()).count();
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
    I64 totalMs=seconds*1000;
    constexpr I32 step=10;
    while(totalMs>=0){
        totalMs-=step;
        std::this_thread::sleep_for(std::chrono::milliseconds(step));
        if(cond&&cond()){
            return true;
        }
    }
    return false;
}

uint64_t GetCurrentPid()
{
    #if defined(_WIN32)
        #include <windows.h>
        return static_cast<uint64_t>(GetCurrentProcessId());
    #elif defined(__ANDROID__) || defined(__linux__) || defined(__APPLE__)
        #include <unistd.h>
        return static_cast<uint64_t>(getpid());
    #else
        return 0;
    #endif
}
