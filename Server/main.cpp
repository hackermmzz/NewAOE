#include <iostream>
#include <thread>
#include <chrono>
#include "Public/profile/profile.h"
#include "Public/profile/conf.h"
using namespace std;

void ModuleA_Func1()
{
    PROFILE_TRACE();
    std::this_thread::sleep_for(std::chrono::microseconds(200));
}

void ModuleA_Func2()
{
    PROFILE_TRACE();
    ModuleA_Func1();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
}

void ModuleB_Func1()
{
    PROFILE_TRACE();
    std::this_thread::sleep_for(std::chrono::microseconds(300));
}

void ModuleB_Nested()
{
    PROFILE_TRACE();
    std::this_thread::sleep_for(std::chrono::microseconds(150));
}

void Network_Recv()
{
    PROFILE_TRACE();
    ModuleB_Nested();
    std::this_thread::sleep_for(std::chrono::microseconds(80));
}

void WorkerThread()
{
    PROFILE_TRACE();
    while ((true))
    {
        PROFILE_TRACE();
        ModuleA_Func2();
        Network_Recv();
        // 这里控制压力：注释掉就是极限压测
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
}

void run(){
    while(true){
        ProfileWorker::instance().run();
    }
}
int main(){
    PROFILE_TRACE();

    std::thread t1(WorkerThread);
    std::thread t2(WorkerThread);
    std::thread t3(run);
    // 主线程也死循环打点
    while (true)
    {
        PROFILE_TRACE();
        ModuleA_Func2();
        ModuleB_Func1();
        Network_Recv();
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }

    t1.join();
    t2.join();
    t3.join();
    return 0;
}
