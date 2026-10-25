#pragma once

#include "Public/type/type.h"

// AudioManager does not create a thread of its own. Construct it, call every
// member (including update()), and destroy it from the caller-owned audio
// thread. miniaudio may still use the platform's required device callback and
// resource streaming threads.
class AudioManager final {
public:
    AudioManager();
    ~AudioManager() noexcept;

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    AudioManager(AudioManager&&) = delete;
    AudioManager& operator=(AudioManager&&) = delete;

    // Plays a spatialized effect in the normalized X/Z world. The decoded
    // effect is retained in the cache until this manager is destroyed.
    void playeffect(
        const char* file,
        Float32 posx,
        Float32 posz,
        Float32 velocityX,
        Float32 velocityZ
    );

    // Replaces the current non-spatialized streamed sound and starts it.
    void playerSound(const char* file);

    void setListenerPos(Float32 posx, Float32 posz);

    // Pauses and resumes the complete engine without changing play cursors.
    void pause();
    void resume();

    // Call regularly from the caller-owned audio thread to release completed
    // effect instances. Streaming itself continues on miniaudio's worker.
    void update();

    bool isReady() const noexcept;
    bool isPaused() const noexcept;
    const char* lastError() const noexcept;

private:
    class Impl;
    Impl* impl_;
};

