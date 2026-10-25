#pragma once

#include "Server/audio/audiomanager/audiomanager.h"


class AudioInstance{
private:
    AudioManager manager;
public:
    AudioInstance();
    void run();
    ~AudioInstance();
};