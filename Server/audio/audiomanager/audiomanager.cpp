#include "Server/audio/audiomanager/audiomanager.h"

#include "Public/third-party/miniaudio/miniaudio.h"

#include <memory>
#include <new>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

constexpr Float32 kEffectMinDistance = 0.01f;
constexpr Float32 kEffectMaxDistance = 1.41421356237f;

std::string makeResultMessage(const char* operation, ma_result result)
{
    std::string message = operation;
    message += ": ";
    message += ma_result_description(result);
    return message;
}

struct SoundOwner final {
    ma_sound sound{};
    bool initialized = false;

    SoundOwner() = default;
    ~SoundOwner()
    {
        reset();
    }

    SoundOwner(const SoundOwner&) = delete;
    SoundOwner& operator=(const SoundOwner&) = delete;
    SoundOwner(SoundOwner&&) = delete;
    SoundOwner& operator=(SoundOwner&&) = delete;

    void reset() noexcept
    {
        if (initialized) {
            ma_sound_uninit(&sound);
            initialized = false;
        }
    }
};

} // namespace

class AudioManager::Impl final {
public:
    Impl()
    {
        initialize();
    }

    ~Impl()
    {
        shutdown();
    }

    void playeffect(
        const char* file,
        Float32 posx,
        Float32 posz,
        Float32 velocityX,
        Float32 velocityZ
    )
    {
        if (!requireReady()) {
            return;
        }
        clearError();
        if (!validateFile(file)) {
            return;
        }

        releaseFinishedEffects();

        SoundOwner* cachedSound = findOrLoadEffect(file);
        if (cachedSound == nullptr) {
            return;
        }

        auto instance = std::unique_ptr<SoundOwner>(new (std::nothrow) SoundOwner());
        if (!instance) {
            error_ = "playeffect: out of memory";
            return;
        }

        ma_result result = ma_sound_init_copy(
            &engine_,
            &cachedSound->sound,
            0,
            nullptr,
            &instance->sound
        );
        if (result != MA_SUCCESS) {
            error_ = makeResultMessage("playeffect could not create an instance", result);
            return;
        }
        instance->initialized = true;

        ma_sound_set_spatialization_enabled(&instance->sound, MA_TRUE);
        ma_sound_set_positioning(&instance->sound, ma_positioning_absolute);
        ma_sound_set_attenuation_model(&instance->sound, ma_attenuation_model_linear);
        ma_sound_set_min_distance(&instance->sound, kEffectMinDistance);
        ma_sound_set_max_distance(&instance->sound, kEffectMaxDistance);
        ma_sound_set_min_gain(&instance->sound, 0.0f);
        ma_sound_set_max_gain(&instance->sound, 1.0f);
        ma_sound_set_rolloff(&instance->sound, 1.0f);
        ma_sound_set_position(&instance->sound, posx, 0.0f, posz);
        ma_sound_set_velocity(&instance->sound, velocityX, 0.0f, velocityZ);

        result = ma_sound_start(&instance->sound);
        if (result != MA_SUCCESS) {
            error_ = makeResultMessage("playeffect could not start", result);
            return;
        }

        activeEffects_.push_back(std::move(instance));
    }

    void playerSound(const char* file)
    {
        if (!requireReady()) {
            return;
        }
        clearError();
        if (!validateFile(file)) {
            return;
        }

        auto nextSound = std::unique_ptr<SoundOwner>(new (std::nothrow) SoundOwner());
        if (!nextSound) {
            error_ = "playerSound: out of memory";
            return;
        }

        const ma_uint32 flags =
            MA_SOUND_FLAG_STREAM |
            MA_SOUND_FLAG_NO_SPATIALIZATION |
            MA_SOUND_FLAG_NO_PITCH;

        ma_result result = ma_sound_init_from_file(
            &engine_,
            file,
            flags,
            nullptr,
            nullptr,
            &nextSound->sound
        );
        if (result != MA_SUCCESS) {
            error_ = makeResultMessage("playerSound could not open the stream", result);
            return;
        }
        nextSound->initialized = true;

        if (streamedSound_) {
            ma_sound_stop(&streamedSound_->sound);
            streamedSound_.reset();
        }

        result = ma_sound_start(&nextSound->sound);
        if (result != MA_SUCCESS) {
            error_ = makeResultMessage("playerSound could not start", result);
            return;
        }

        streamedSound_ = std::move(nextSound);
    }

    void setListenerPos(Float32 posx, Float32 posz)
    {
        if (!requireReady()) {
            return;
        }
        clearError();

        ma_engine_listener_set_position(&engine_, 0, posx, 0.0f, posz);
        ma_engine_listener_set_velocity(&engine_, 0, 0.0f, 0.0f, 0.0f);
    }

    void pause()
    {
        if (!requireReady()) {
            return;
        }
        clearError();
        if (paused_) {
            return;
        }

        const ma_result result = ma_engine_stop(&engine_);
        if (result != MA_SUCCESS) {
            error_ = makeResultMessage("pause failed", result);
            return;
        }

        paused_ = true;
    }

    void resume()
    {
        if (!requireReady()) {
            return;
        }
        clearError();
        if (!paused_) {
            return;
        }

        const ma_result result = ma_engine_start(&engine_);
        if (result != MA_SUCCESS) {
            error_ = makeResultMessage("resume failed", result);
            return;
        }

        paused_ = false;
    }

    void update()
    {
        if (!ready_) {
            return;
        }

        releaseFinishedEffects();
    }

    bool isReady() const noexcept
    {
        return ready_;
    }

    bool isPaused() const noexcept
    {
        return paused_;
    }

    const char* lastError() const noexcept
    {
        return error_.c_str();
    }

private:
    void initialize()
    {
        ma_engine_config engineConfig = ma_engine_config_init();
        engineConfig.sampleRate = 48000;

        const ma_result result = ma_engine_init(&engineConfig, &engine_);
        if (result != MA_SUCCESS) {
            error_ = makeResultMessage("audio engine initialization failed", result);
            return;
        }
        engineInitialized_ = true;
        ready_ = true;

        ma_engine_listener_set_world_up(&engine_, 0, 0.0f, 1.0f, 0.0f);
        ma_engine_listener_set_direction(&engine_, 0, 0.0f, 0.0f, -1.0f);
        ma_engine_listener_set_velocity(&engine_, 0, 0.0f, 0.0f, 0.0f);
    }

    void shutdown() noexcept
    {
        ready_ = false;

        if (engineInitialized_) {
            ma_engine_stop(&engine_);
        }

        streamedSound_.reset();
        activeEffects_.clear();
        effectCache_.clear();

        if (engineInitialized_) {
            ma_engine_uninit(&engine_);
            engineInitialized_ = false;
        }

        paused_ = false;
    }

    SoundOwner* findOrLoadEffect(const char* file)
    {
        const auto found = effectCache_.find(file);
        if (found != effectCache_.end()) {
            return found->second.get();
        }

        auto cachedSound = std::unique_ptr<SoundOwner>(new (std::nothrow) SoundOwner());
        if (!cachedSound) {
            error_ = "playeffect cache: out of memory";
            return nullptr;
        }

        const ma_uint32 flags = MA_SOUND_FLAG_DECODE;
        const ma_result result = ma_sound_init_from_file(
            &engine_,
            file,
            flags,
            nullptr,
            nullptr,
            &cachedSound->sound
        );
        if (result != MA_SUCCESS) {
            error_ = makeResultMessage("playeffect could not cache the file", result);
            return nullptr;
        }
        cachedSound->initialized = true;

        SoundOwner* resultSound = cachedSound.get();
        effectCache_.emplace(file, std::move(cachedSound));
        return resultSound;
    }

    void releaseFinishedEffects()
    {
        auto current = activeEffects_.begin();
        while (current != activeEffects_.end()) {
            if (ma_sound_at_end(&(*current)->sound)) {
                current = activeEffects_.erase(current);
            } else {
                ++current;
            }
        }
    }

    bool validateFile(const char* file)
    {
        if (file == nullptr || file[0] == '\0') {
            error_ = "audio file path is empty";
            return false;
        }
        return true;
    }

    bool requireReady()
    {
        if (!ready_) {
            if (error_.empty()) {
                error_ = "AudioManager is not ready";
            }
            return false;
        }
        return true;
    }

    void clearError()
    {
        error_.clear();
    }

    ma_engine engine_{};
    bool engineInitialized_ = false;
    bool ready_ = false;
    bool paused_ = false;
    std::string error_;

    std::unordered_map<std::string, std::unique_ptr<SoundOwner>> effectCache_;
    std::vector<std::unique_ptr<SoundOwner>> activeEffects_;
    std::unique_ptr<SoundOwner> streamedSound_;
};

AudioManager::AudioManager()
    : impl_(new (std::nothrow) Impl())
{
}

AudioManager::~AudioManager() noexcept
{
    delete impl_;
}

void AudioManager::playeffect(
    const char* file,
    Float32 posx,
    Float32 posz,
    Float32 velocityX,
    Float32 velocityZ
)
{
    if (impl_ != nullptr) {
        impl_->playeffect(file, posx, posz, velocityX, velocityZ);
    }
}

void AudioManager::playerSound(const char* file)
{
    if (impl_ != nullptr) {
        impl_->playerSound(file);
    }
}

void AudioManager::setListenerPos(Float32 posx, Float32 posz)
{
    if (impl_ != nullptr) {
        impl_->setListenerPos(posx, posz);
    }
}

void AudioManager::pause()
{
    if (impl_ != nullptr) {
        impl_->pause();
    }
}

void AudioManager::resume()
{
    if (impl_ != nullptr) {
        impl_->resume();
    }
}

void AudioManager::update()
{
    if (impl_ != nullptr) {
        impl_->update();
    }
}

bool AudioManager::isReady() const noexcept
{
    return impl_ != nullptr && impl_->isReady();
}

bool AudioManager::isPaused() const noexcept
{
    return impl_ != nullptr && impl_->isPaused();
}

const char* AudioManager::lastError() const noexcept
{
    static const char allocationFailure[] = "AudioManager allocation failed";
    return impl_ != nullptr ? impl_->lastError() : allocationFailure;
}

