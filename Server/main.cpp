#include <iostream>
#include "Server/conf.h"
#include "Public/logger/conf.h"
#include "Public/profile/conf.h"
#include <functional>
#include <string>
#include <thread>
#include <utility>
#include "Server/core/coreInstance.h"

#if ENABLE_LOGGER
#include "Public/logger/logger_worker.h"
#endif

#if ENABLE_PROFILE_TRACE
#include "Public/profile/profile_worker.h"
#endif

#if ENABLE_AUDIO_PLAY   //
#include "Server/audio/audioInstance.h"
#endif

#if ENABLE_GAME_RENDER  //
#include "Server/render/renderInstance.h"
#endif

using namespace std;


void ParseCMDInput(int argc, char *argv[]);
void StartLoggerAndProfilThread();
void StartGamePlayThread();
void StartAudioPlayThread();
void StartGameRender();

int main(int argc, char *argv[]){
    //读取命令行配置
    ParseCMDInput(argc, argv);
    //开启日志和性能追踪
    StartLoggerAndProfilThread();
    //开启游戏线程
    StartGamePlayThread();
    //开启音频播放线程
    StartAudioPlayThread();
    //开启画面渲染
    StartGameRender();
    return 0;
}

/*****************************************录入参数***************** */

void ParseCMDInput(int argc, char *argv[]){
    CMD.clear();

    for (int i = 1; i < argc; ++i) {
        string argument = argv[i] == nullptr ? string{} : string{argv[i]};

        // --key、-key 和 key 使用同一个键名。
        const string::size_type firstKeyCharacter = argument.find_first_not_of('-');
        if (firstKeyCharacter == string::npos) {
            continue;
        }
        argument.erase(0, firstKeyCharacter);

        string key;
        string value;
        const string::size_type equalPosition = argument.find('=');
        if (equalPosition != string::npos) {
            key = argument.substr(0, equalPosition);
            value = argument.substr(equalPosition + 1);
        } else {
            key = argument;

            // 支持 --key value。下一个以 '-' 开头的参数视为另一个选项；
            // 负数等特殊值可以使用 --key=-1 的形式传入。
            if (i + 1 < argc && argv[i + 1] != nullptr && argv[i + 1][0] != '-') {
                value = argv[++i];
            } else {
                value = "true";
            }
        }

        if (!key.empty()) {
            // 重复参数以后出现的值为准。
            CMD.insert_or_assign(std::move(key), std::move(value));
        }
    }
}

/*****************************************开启性能追踪以及日志记录线程***************** */

void StartLoggerAndProfilThread(){
    #if ENABLE_LOGGER||ENABLE_PROFILE_TRACE
    //单独开一个线程
    std::function<void(void)>job=[&](){
        while(ProgramRun.load()){
            //开启日志
            #if ENABLE_LOGGER
            LoggerWorker::instance().run();
            #endif
            //开启性能追踪
            #if ENABLE_PROFILE_TRACE
            ProfileWorker::instance().run();
            #endif
        }
    };
    //
    std::thread worker(job);
    worker.detach();
    #endif
}


/*****************************************开启游戏内核线程***************** */

void StartGamePlayThread(){
    //单独开一个线程
    std::function<void(void)>job=[&](){
        const bool registered = RegistInstance<CoreInstance>();
        Assert(!registered,"注册Core失败!");
        auto*ins=GetInstance<CoreInstance>();
        Assert(ins==nullptr,"CoreInsntance怎么可能是null?");
        if (ins == nullptr) return;
        while(ProgramRun.load()){
            ins->run();
        }
    };
    //
    std::thread worker(job);
    worker.detach();
}

/*****************************************开启音频播放线程***************** */

void StartAudioPlayThread(){
    #if ENABLE_AUDIO_PLAY
    //单独开一个线程
    std::function<void(void)>job=[&](){
        const bool registered = RegistInstance<AudioInstance>();
        Assert(!registered,"注册Audio失败!");
        auto*ins=GetInstance<AudioInstance>();
        Assert(ins==nullptr,"AudioInstance怎么可能是null?");
        if (ins == nullptr) return;
        while(ProgramRun.load()){
            ins->run();
        }
    };
    //
    std::thread worker(job);
    worker.detach();
    #endif
}


/****************************************开启游戏画面的渲染*********************/
void StartGameRender(){
    #if ENABLE_GAME_RENDER
        //注册实例对象
        const bool registered = RegistInstance<RenderInstance>();
        Assert(!registered,"注册Render失败!");
        auto*ins=GetInstance<RenderInstance>();
        Assert(ins==nullptr,"RenderInstance怎么可能是null?");
        if (ins == nullptr || !ins->init()) {
            ProgramRun.store(false);
            return;
        }
        //
        // sokol_app owns the native/browser event loop. On desktop this call
        // blocks until the window closes; on WebAssembly it installs the
        // browser callback and returns immediately.
        if (!ins->run()) {
            ProgramRun.store(false);
        }
    #endif
}
