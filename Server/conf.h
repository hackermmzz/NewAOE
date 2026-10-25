#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "Public/vec/cast.hpp"

//全局开关，控制所有线程是否继续Run还是退出
inline std::atomic<bool> ProgramRun{true};

//命令行输入全局Map
inline std::unordered_map<std::string,Cast> CMD;

//全局实例池
inline std::unordered_map<std::type_index, std::shared_ptr<void>> ObjectInstancePool;
inline std::mutex ObjectInstancePoolMutex;

template<class T, class... ARG>
bool RegistInstance(ARG&&... arg){
    auto instance = std::make_shared<T>(std::forward<ARG>(arg)...);
    const auto key = std::type_index(typeid(T));
    std::lock_guard<std::mutex> lock(ObjectInstancePoolMutex);
    return ObjectInstancePool.emplace(key, std::move(instance)).second;
}

template<class T>
T* GetInstance(){
    const auto key = std::type_index(typeid(T));
    std::lock_guard<std::mutex> lock(ObjectInstancePoolMutex);
    const auto itr = ObjectInstancePool.find(key);
    if (itr == ObjectInstancePool.end()) return nullptr;
    return static_cast<T*>(itr->second.get());
}



//画面渲染开关
#ifndef ENABLE_GAME_RENDER
#define ENABLE_GAME_RENDER 1
#endif

//音效播放开关
#ifndef ENABLE_AUDIO_PLAY
#define ENABLE_AUDIO_PLAY 1
#endif
