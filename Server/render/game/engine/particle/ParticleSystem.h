#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "Server/render/game/engine/RenderTypes.h"

namespace newaoe::render::particle {

inline constexpr std::uint32_t kParticleManifestVersion = 1;
inline constexpr std::size_t kMaxCurveKeys = 8;
inline constexpr std::uint32_t kMaximumParticleCapacity = 1'000'000;
inline constexpr float kMaximumEmissionRate = 250'000.0F;
inline constexpr std::uint32_t kMaximumContinuousEventsPerUpdate = 65'536;
inline constexpr float kMinimumTimedDuration = 0.001F;
inline constexpr float kMaximumTimedDuration = 86'400.0F;
inline constexpr float kMinimumParticleLifetime = 0.001F;
inline constexpr float kMaximumParticleLifetime = 86'400.0F;
inline constexpr std::size_t kMaximumParticleBursts = 1024;
inline constexpr std::uint32_t kMaximumBurstParticleCount = 65'536;
inline constexpr std::uint32_t kMaximumBurstCycles = 4096;
inline constexpr float kMinimumBurstInterval = 0.001F;
inline constexpr float kMaximumBurstInterval = 86'400.0F;
inline constexpr std::size_t kMaximumBurstEventsPerUpdate = 4096;
inline constexpr std::uint32_t kMaximumBurstParticleAttemptsPerUpdate = 65'536;

enum class RandomDistribution : std::uint8_t {
    Uniform,
    Normal,
};

enum class EmitterShape : std::uint8_t {
    Point,
    Box,
    Circle,
};

enum class SimulationSpace : std::uint8_t {
    World,
    Local,
};

struct FloatRange final {
    float minimum = 0.0F;
    float maximum = 0.0F;
    RandomDistribution distribution = RandomDistribution::Uniform;
};

struct Vec3Range final {
    Vec3 minimum{};
    Vec3 maximum{};
    RandomDistribution distribution = RandomDistribution::Uniform;
};

struct ScalarCurveKey final {
    float time = 0.0F;
    float value = 0.0F;
};

struct SizeCurveKey final {
    float time = 0.0F;
    Vec2 value{1.0F, 1.0F};
};

struct ColorCurveKey final {
    float time = 0.0F;
    Color value{};
};

struct ScalarCurve final {
    std::array<ScalarCurveKey, kMaxCurveKeys> keys{};
    std::uint32_t count = 0;

    [[nodiscard]] float sample(float normalizedAge) const noexcept;
};

struct SizeCurve final {
    std::array<SizeCurveKey, kMaxCurveKeys> keys{};
    std::uint32_t count = 0;

    [[nodiscard]] Vec2 sample(float normalizedAge) const noexcept;
};

struct ColorCurve final {
    std::array<ColorCurveKey, kMaxCurveKeys> keys{};
    std::uint32_t count = 0;

    [[nodiscard]] Color sample(float normalizedAge) const noexcept;
};

struct MaterialDefinition final {
    // Paths are UTF-8 and relative to the particle-system directory.  Empty
    // optional texture paths select the engine's built-in defaults.
    std::string baseTexture;
    std::string normalTexture;
    std::string emissiveTexture;
    Color tint{};
    Color emissive{0.0F, 0.0F, 0.0F, 1.0F};
    float opacity = 1.0F;
    float alphaCutoff = 0.0F;
    float roughness = 1.0F;
    float metallic = 0.0F;
    ParticleBlendMode blendMode = ParticleBlendMode::PremultipliedAlpha;
};

struct Burst final {
    float time = 0.0F;
    std::uint32_t countMinimum = 1;
    std::uint32_t countMaximum = 1;
    std::uint32_t cycles = 1;
    float interval = 0.0F;
};

struct EmitterDefinition final {
    std::uint32_t maxParticles = 4096;
    float emissionRate = 32.0F;
    // Zero means an infinite non-looping emitter.  Looping emitters require a
    // positive duration.
    float duration = 5.0F;
    bool looping = true;
    // World-space particles retain the transform at birth and form trails.
    // Local-space particles remain attached to the current instance transform.
    SimulationSpace simulationSpace = SimulationSpace::World;
    // Fraction of emitter linear velocity added to world-space births.
    float inheritVelocity = 0.0F;

    EmitterShape shape = EmitterShape::Point;
    Vec3 shapeExtents{32.0F, 32.0F, 0.0F};
    float shapeRadius = 32.0F;
    RandomDistribution shapeDistribution = RandomDistribution::Uniform;

    FloatRange lifetime{1.0F, 1.0F, RandomDistribution::Uniform};
    Vec3Range position{};
    Vec3Range velocity{};
    Vec3Range acceleration{};
    Vec3 gravity{0.0F, 98.0F, 0.0F};
    float drag = 0.0F;

    FloatRange sizeMultiplier{1.0F, 1.0F,
                              RandomDistribution::Uniform};
    FloatRange rotationOffset{0.0F, 0.0F,
                              RandomDistribution::Uniform};
    SizeCurve size;
    ColorCurve color;
    ScalarCurve rotation;
    std::vector<Burst> bursts;
};

struct ParticleSystemDefinition final {
    std::uint32_t version = kParticleManifestVersion;
    std::string name = "Particle System";
    MaterialDefinition material;
    EmitterDefinition emitter;
};

// A tightly packed, backend-independent instance record.  It contains only
// birth state; the vertex shader evaluates motion analytically at render time.
// variation = {X size multiplier, rotation offset, Y size multiplier,
//              reserved}. World-space births bake transform scale/rotation
// into these values; local-space batches apply the live transform instead.
struct alignas(16) GpuParticleRecord final {
    float positionSpawnTime[4]{};
    float velocityLifetime[4]{};
    float accelerationDrag[4]{};
    float variation[4]{1.0F, 0.0F, 0.0F, 0.0F};
};

static_assert(sizeof(GpuParticleRecord) == 64,
              "GPU particle records must remain four float4 values");

struct ParticleRecordRange final {
    std::size_t first = 0;
    std::size_t count = 0;
};

enum class PlaybackState : std::uint8_t {
    Stopped,
    Playing,
    Paused,
    Finished,
};

// Creates the editor/runtime default, including valid two-key curves.
[[nodiscard]] ParticleSystemDefinition makeDefaultParticleSystem();

// Validates semantic limits, curve ordering, finite values and asset paths.
[[nodiscard]] bool validateParticleSystem(
    const ParticleSystemDefinition& definition,
    std::string& error);

// Reads/writes <directory>/particle.json.  Save is transactional: the new
// manifest is written beside the destination and then renamed into place.
[[nodiscard]] bool loadParticleSystem(
    const std::filesystem::path& directory,
    ParticleSystemDefinition& definition,
    std::string& error);
[[nodiscard]] bool saveParticleSystem(
    const std::filesystem::path& directory,
    const ParticleSystemDefinition& definition,
    std::string& error);

class ParticleEmitterInstance final {
public:
    explicit ParticleEmitterInstance(
        std::shared_ptr<const ParticleSystemDefinition> definition,
        std::uint64_t randomSeed = 0x9E3779B97F4A7C15ULL);

    ParticleEmitterInstance(const ParticleEmitterInstance&) = delete;
    ParticleEmitterInstance& operator=(const ParticleEmitterInstance&) =
        delete;
    ParticleEmitterInstance(ParticleEmitterInstance&&) noexcept = default;
    ParticleEmitterInstance& operator=(ParticleEmitterInstance&&) noexcept =
        default;

    // play() resumes a paused instance.  A stopped or finished instance is
    // restarted from time zero.  restart() always clears live particles.
    void play() noexcept;
    void pause() noexcept;
    void stop() noexcept;
    void restart() noexcept;
    void update(float deltaSeconds) noexcept;

    void setOrigin(Vec3 origin) noexcept;
    [[nodiscard]] Vec3 origin() const noexcept {
        return {targetTransform_.position.x, targetTransform_.position.y,
                originZ_};
    }
    void setTransform(ParticleTransform2D transform) noexcept;
    [[nodiscard]] ParticleTransform2D transform() const noexcept {
        return targetTransform_;
    }
    [[nodiscard]] PlaybackState state() const noexcept { return state_; }
    [[nodiscard]] double time() const noexcept { return time_; }
    // Float clock paired with positionSpawnTime.w for the GPU.  It is
    // periodically rebased while time() remains monotonic for the full
    // playback lifetime.
    [[nodiscard]] float gpuTime() const noexcept {
        return static_cast<float>(time_ - gpuTimeOrigin_);
    }
    [[nodiscard]] std::uint64_t droppedParticleCount() const noexcept {
        return droppedParticles_;
    }
    [[nodiscard]] const std::shared_ptr<const ParticleSystemDefinition>&
    definition() const noexcept {
        return definition_;
    }
    [[nodiscard]] const std::vector<GpuParticleRecord>& particles() const
        noexcept {
        return particles_;
    }

    // Runtime renderers opt into change tracking so immutable birth records
    // can remain in persistent GPU buffers. CPU-only editor previews leave it
    // disabled and pay no tracking cost.
    void enableParticleRecordChangeTracking();
    [[nodiscard]] const std::vector<ParticleRecordRange>&
    changedParticleRecordRanges() const noexcept {
        return changedParticleRecordRanges_;
    }
    void clearParticleRecordChanges() noexcept {
        changedParticleRecordRanges_.clear();
    }

private:
    struct BurstCursor final {
        std::uint64_t loop = 0;
        std::uint32_t cycle = 0;
        double nextTime = 0.0;
        bool finished = false;
    };

    [[nodiscard]] std::uint64_t nextRandom() noexcept;
    [[nodiscard]] float uniform01() noexcept;
    [[nodiscard]] float sample(const FloatRange& range) noexcept;
    [[nodiscard]] Vec3 sample(const Vec3Range& range) noexcept;
    [[nodiscard]] Vec3 sampleShape() noexcept;
    [[nodiscard]] ParticleTransform2D transformAt(
        double spawnTime) const noexcept;
    [[nodiscard]] std::uint32_t sampleCount(std::uint32_t minimum,
                                            std::uint32_t maximum) noexcept;
    void resetPlayback() noexcept;
    void emit(std::uint32_t count, double firstSpawnTime,
              double spawnInterval) noexcept;
    void emitBurstsThrough(double endTime) noexcept;
    void expireParticles() noexcept;
    void rebaseGpuClockIfNeeded() noexcept;
    void markParticleRecordsChanged(std::size_t first,
                                    std::size_t count) noexcept;

    std::shared_ptr<const ParticleSystemDefinition> definition_;
    std::vector<GpuParticleRecord> particles_;
    std::vector<ParticleRecordRange> changedParticleRecordRanges_;
    std::vector<BurstCursor> burstCursors_;
    ParticleTransform2D targetTransform_{};
    ParticleTransform2D simulatedTransform_{};
    ParticleTransform2D frameStartTransform_{};
    ParticleTransform2D frameEndTransform_{};
    Vec2 frameEmitterVelocity_{};
    double frameStartTime_ = 0.0;
    double frameEndTime_ = 0.0;
    float originZ_ = 0.0F;
    bool transformInitialized_ = false;
    bool hasUpdatedTransform_ = false;
    PlaybackState state_ = PlaybackState::Stopped;
    double time_ = 0.0;
    double gpuTimeOrigin_ = 0.0;
    float emissionAccumulator_ = 0.0F;
    std::size_t nextBurstCursor_ = 0;
    std::uint64_t randomState_ = 0;
    std::uint64_t droppedParticles_ = 0;
    bool trackParticleRecordChanges_ = false;
};

}  // namespace newaoe::render::particle
