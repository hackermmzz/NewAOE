#include "Server/render/game/engine/particle/ParticleSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <type_traits>

namespace newaoe::render::particle {
namespace {

constexpr float kPi = 3.14159265358979323846F;

float clampAge(float value) noexcept {
    return std::max(0.0F, std::min(value, 1.0F));
}

float lerp(float lhs, float rhs, float amount) noexcept {
    return lhs + ((rhs - lhs) * amount);
}

float lerpAngle(float lhs, float rhs, float amount) noexcept {
    float delta = std::fmod(rhs - lhs, 2.0F * kPi);
    if (delta > kPi) delta -= 2.0F * kPi;
    if (delta < -kPi) delta += 2.0F * kPi;
    return lhs + delta * amount;
}

ParticleTransform2D lerpTransform(const ParticleTransform2D& lhs,
                                  const ParticleTransform2D& rhs,
                                  float amount) noexcept {
    return {{lerp(lhs.position.x, rhs.position.x, amount),
             lerp(lhs.position.y, rhs.position.y, amount)},
            lerpAngle(lhs.rotation, rhs.rotation, amount),
            {lerp(lhs.scale.x, rhs.scale.x, amount),
             lerp(lhs.scale.y, rhs.scale.y, amount)}};
}

struct TransformBasis final {
    float xx = 1.0F;
    float xy = 0.0F;
    float yx = 0.0F;
    float yy = 1.0F;
};

TransformBasis makeTransformBasis(
    const ParticleTransform2D& transform) noexcept {
    const float cosine = std::cos(transform.rotation);
    const float sine = std::sin(transform.rotation);
    return {transform.scale.x * cosine,
            -transform.scale.y * sine,
            transform.scale.x * sine,
            transform.scale.y * cosine};
}

Vec2 transformVector(const TransformBasis& basis, Vec2 value) noexcept {
    return {value.x * basis.xx + value.y * basis.xy,
            value.x * basis.yx + value.y * basis.yy};
}

Vec2 inverseTransformVector(const ParticleTransform2D& transform,
                            Vec2 value) noexcept {
    const float cosine = std::cos(transform.rotation);
    const float sine = std::sin(transform.rotation);
    return {(value.x * cosine + value.y * sine) / transform.scale.x,
            (-value.x * sine + value.y * cosine) / transform.scale.y};
}

template <typename Curve, typename Value, typename Interpolator>
Value sampleCurve(const Curve& curve, float age, Value fallback,
                  Interpolator interpolate) noexcept {
    if (curve.count == 0) return fallback;
    const float time = clampAge(age);
    if (time <= curve.keys[0].time) return curve.keys[0].value;
    for (std::uint32_t index = 1; index < curve.count; ++index) {
        const auto& right = curve.keys[index];
        if (time <= right.time) {
            const auto& left = curve.keys[index - 1];
            const float width = right.time - left.time;
            const float amount = width > 0.0F ? (time - left.time) / width
                                               : 0.0F;
            return interpolate(left.value, right.value, amount);
        }
    }
    return curve.keys[curve.count - 1].value;
}

bool finite(float value) noexcept {
    return std::isfinite(value);
}

bool finite(Vec2 value) noexcept {
    return finite(value.x) && finite(value.y);
}

bool finite(Vec3 value) noexcept {
    return finite(value.x) && finite(value.y) && finite(value.z);
}

bool finite(Color value) noexcept {
    return finite(value.r) && finite(value.g) && finite(value.b) &&
           finite(value.a);
}

void addDroppedSaturated(std::uint64_t& target,
                         std::uint64_t amount) noexcept {
    const std::uint64_t room = std::numeric_limits<std::uint64_t>::max() -
                               target;
    target += std::min(room, amount);
}

bool safeAssetPath(const std::string& text) {
    if (text.empty()) return true;
    std::string portable = text;
    std::replace(portable.begin(), portable.end(), '\\', '/');
    // Give separators and roots the same meaning on every host.  A Windows
    // style `..\\` must not bypass validation on Linux or Web builds.
    if (portable.front() == '/' || portable.find(':') != std::string::npos) {
        return false;
    }
    const std::filesystem::path path = std::filesystem::u8path(portable);
    if (path.is_absolute() || path.has_root_name() || path.has_root_directory()) {
        return false;
    }
    for (const auto& component : path) {
        if (component == "..") return false;
    }
    return true;
}

bool validateRange(const FloatRange& range, const char* name,
                   std::string& error) {
    if (!finite(range.minimum) || !finite(range.maximum) ||
        range.minimum > range.maximum) {
        error = std::string{name} + " must contain finite minimum <= maximum";
        return false;
    }
    return true;
}

bool validateRange(const Vec3Range& range, const char* name,
                   std::string& error) {
    if (!finite(range.minimum) || !finite(range.maximum) ||
        range.minimum.x > range.maximum.x ||
        range.minimum.y > range.maximum.y ||
        range.minimum.z > range.maximum.z) {
        error = std::string{name} +
                " must contain finite component-wise minimum <= maximum";
        return false;
    }
    return true;
}

template <typename Curve, typename ValueValidator>
bool validateCurve(const Curve& curve, const char* name,
                   ValueValidator validValue, std::string& error) {
    if (curve.count == 0 || curve.count > kMaxCurveKeys) {
        error = std::string{name} + " must contain between 1 and " +
                std::to_string(kMaxCurveKeys) + " keys";
        return false;
    }
    float previous = -1.0F;
    for (std::uint32_t index = 0; index < curve.count; ++index) {
        const auto& key = curve.keys[index];
        if (!finite(key.time) || key.time < 0.0F || key.time > 1.0F ||
            key.time < previous || !validValue(key.value)) {
            error = std::string{name} +
                    " keys must have ascending finite times in [0,1] and "
                    "finite values";
            return false;
        }
        previous = key.time;
    }
    return true;
}

}  // namespace

float ScalarCurve::sample(float normalizedAge) const noexcept {
    return sampleCurve(*this, normalizedAge, 0.0F,
                       [](float lhs, float rhs, float amount) {
                           return lerp(lhs, rhs, amount);
                       });
}

Vec2 SizeCurve::sample(float normalizedAge) const noexcept {
    return sampleCurve(*this, normalizedAge, Vec2{1.0F, 1.0F},
                       [](Vec2 lhs, Vec2 rhs, float amount) {
                           return Vec2{lerp(lhs.x, rhs.x, amount),
                                       lerp(lhs.y, rhs.y, amount)};
                       });
}

Color ColorCurve::sample(float normalizedAge) const noexcept {
    return sampleCurve(*this, normalizedAge, Color{},
                       [](Color lhs, Color rhs, float amount) {
                           return Color{lerp(lhs.r, rhs.r, amount),
                                        lerp(lhs.g, rhs.g, amount),
                                        lerp(lhs.b, rhs.b, amount),
                                        lerp(lhs.a, rhs.a, amount)};
                       });
}

ParticleSystemDefinition makeDefaultParticleSystem() {
    ParticleSystemDefinition result;
    result.emitter.size.count = 2;
    result.emitter.size.keys[0] = {0.0F, {8.0F, 8.0F}};
    result.emitter.size.keys[1] = {1.0F, {0.0F, 0.0F}};
    result.emitter.color.count = 2;
    result.emitter.color.keys[0] = {0.0F, Color{}};
    result.emitter.color.keys[1] = {1.0F, Color{1.0F, 1.0F, 1.0F, 0.0F}};
    result.emitter.rotation.count = 2;
    result.emitter.rotation.keys[0] = {0.0F, 0.0F};
    result.emitter.rotation.keys[1] = {1.0F, 0.0F};
    return result;
}

bool validateParticleSystem(const ParticleSystemDefinition& definition,
                            std::string& error) {
    error.clear();
    if (definition.version != kParticleManifestVersion) {
        error = "unsupported particle manifest version " +
                std::to_string(definition.version);
        return false;
    }
    if (definition.name.empty()) {
        error = "particle-system name cannot be empty";
        return false;
    }
    const auto& material = definition.material;
    if (!safeAssetPath(material.baseTexture) ||
        !safeAssetPath(material.normalTexture) ||
        !safeAssetPath(material.emissiveTexture)) {
        error = "material texture paths must be portable relative paths and "
                "cannot contain a root, drive name or '..'";
        return false;
    }
    if (!finite(material.tint) || !finite(material.emissive) ||
        !finite(material.opacity) || !finite(material.alphaCutoff) ||
        !finite(material.roughness) || !finite(material.metallic) ||
        material.opacity < 0.0F || material.opacity > 1.0F ||
        material.alphaCutoff < 0.0F || material.alphaCutoff > 1.0F ||
        material.roughness < 0.0F || material.roughness > 1.0F ||
        material.metallic < 0.0F || material.metallic > 1.0F) {
        error = "material values must be finite; opacity, alphaCutoff, "
                "roughness and metallic must be in [0,1]";
        return false;
    }

    const auto& emitter = definition.emitter;
    if (emitter.maxParticles == 0 ||
        emitter.maxParticles > kMaximumParticleCapacity) {
        error = "maxParticles must be in [1,1000000]";
        return false;
    }
    if (!finite(emitter.emissionRate) || emitter.emissionRate < 0.0F ||
        emitter.emissionRate > kMaximumEmissionRate ||
        !finite(emitter.duration) || emitter.duration < 0.0F ||
        emitter.duration > kMaximumTimedDuration ||
        (emitter.duration > 0.0F &&
         emitter.duration < kMinimumTimedDuration) ||
        (emitter.looping &&
         emitter.duration < kMinimumTimedDuration)) {
        error = "emissionRate must be in [0,250000]; duration must be zero "
                "for an infinite non-looping emitter or in [0.001,86400]";
        return false;
    }
    if (!finite(emitter.inheritVelocity) ||
        emitter.inheritVelocity < 0.0F || emitter.inheritVelocity > 1.0F) {
        error = "inheritVelocity must be finite and in [0,1]";
        return false;
    }
    if (!finite(emitter.shapeExtents) || emitter.shapeExtents.x < 0.0F ||
        emitter.shapeExtents.y < 0.0F || emitter.shapeExtents.z < 0.0F ||
        !finite(emitter.shapeRadius) || emitter.shapeRadius < 0.0F ||
        !finite(emitter.gravity) || !finite(emitter.drag) ||
        emitter.drag < 0.0F) {
        error = "shape extents/radius and drag must be finite and "
                "non-negative; gravity must be finite";
        return false;
    }
    if (!validateRange(emitter.lifetime, "lifetime", error) ||
        emitter.lifetime.minimum < kMinimumParticleLifetime ||
        emitter.lifetime.maximum > kMaximumParticleLifetime ||
        !validateRange(emitter.position, "position", error) ||
        !validateRange(emitter.velocity, "velocity", error) ||
        !validateRange(emitter.acceleration, "acceleration", error) ||
        !validateRange(emitter.sizeMultiplier, "sizeMultiplier", error) ||
        emitter.sizeMultiplier.minimum < 0.0F ||
        !validateRange(emitter.rotationOffset, "rotationOffset", error)) {
        if (error.empty()) {
            error = "lifetime must be in [0.001,86400] and sizeMultiplier "
                    "must be non-negative";
        }
        return false;
    }
    if (!validateCurve(emitter.size, "size curve",
                       [](Vec2 value) {
                           return finite(value) && value.x >= 0.0F &&
                                  value.y >= 0.0F;
                       },
                       error) ||
        !validateCurve(emitter.color, "color curve",
                       [](Color value) { return finite(value); }, error) ||
        !validateCurve(emitter.rotation, "rotation curve",
                       [](float value) { return finite(value); }, error)) {
        return false;
    }
    if (emitter.bursts.size() > kMaximumParticleBursts) {
        error = "a particle emitter cannot contain more than 1024 bursts";
        return false;
    }
    for (std::size_t index = 0; index < emitter.bursts.size(); ++index) {
        const auto& burst = emitter.bursts[index];
        const double lastTime = burst.cycles == 0
                                    ? static_cast<double>(burst.time)
                                    : static_cast<double>(burst.time) +
                                          static_cast<double>(burst.cycles - 1U) *
                                              burst.interval;
        if (!finite(burst.time) || burst.time < 0.0F ||
            burst.time > kMaximumTimedDuration ||
            burst.countMinimum > burst.countMaximum || burst.cycles == 0 ||
            burst.countMaximum > kMaximumBurstParticleCount ||
            burst.cycles > kMaximumBurstCycles ||
            !finite(burst.interval) || burst.interval < 0.0F ||
            burst.interval > kMaximumBurstInterval ||
            (burst.cycles > 1 &&
             burst.interval < kMinimumBurstInterval) ||
            (emitter.duration > 0.0F && lastTime > emitter.duration)) {
            error = "burst " + std::to_string(index) +
                    " has invalid time/count/cycle/interval values";
            return false;
        }
    }
    return true;
}

ParticleEmitterInstance::ParticleEmitterInstance(
    std::shared_ptr<const ParticleSystemDefinition> definition,
    std::uint64_t randomSeed)
    : definition_(std::move(definition)),
      randomState_(randomSeed != 0 ? randomSeed : 0x9E3779B97F4A7C15ULL) {
    if (definition_) {
        // A valid manifest may request one million particles. Grow on demand
        // instead of reserving 64 MiB for an instance which may never play.
        constexpr std::size_t initialReserve = 256;
        particles_.reserve(std::min<std::size_t>(
            std::min(definition_->emitter.maxParticles,
                     kMaximumParticleCapacity),
            initialReserve));
        burstCursors_.resize(std::min(definition_->emitter.bursts.size(),
                                      kMaximumParticleBursts));
    }
}

void ParticleEmitterInstance::enableParticleRecordChangeTracking() {
    if (trackParticleRecordChanges_) return;
    trackParticleRecordChanges_ = true;
    markParticleRecordsChanged(0, particles_.size());
}

void ParticleEmitterInstance::play() noexcept {
    if (!definition_) return;
    if (state_ == PlaybackState::Paused) {
        state_ = PlaybackState::Playing;
        return;
    }
    if (state_ != PlaybackState::Playing) {
        resetPlayback();
        state_ = PlaybackState::Playing;
        emitBurstsThrough(0.0);
    }
}

void ParticleEmitterInstance::setOrigin(Vec3 origin) noexcept {
    ParticleTransform2D value = targetTransform_;
    value.position = {origin.x, origin.y};
    originZ_ = origin.z;
    setTransform(value);
}

void ParticleEmitterInstance::setTransform(
    ParticleTransform2D transform) noexcept {
    const ParticleTransform2D previous = simulatedTransform_;
    const bool initialPlacement = !transformInitialized_ ||
                                  state_ != PlaybackState::Playing ||
                                  !hasUpdatedTransform_;
    targetTransform_ = transform;
    // A transform supplied before playback (or while paused) is a placement,
    // not a movement segment. This prevents the first frame from emitting a
    // trail from the world origin and prevents paused teleports from streaking.
    if (initialPlacement) {
        if (transformInitialized_ &&
            state_ == PlaybackState::Playing &&
            !hasUpdatedTransform_ && definition_ &&
            definition_->emitter.simulationSpace ==
                SimulationSpace::World) {
            const TransformBasis newBasis = makeTransformBasis(transform);
            for (GpuParticleRecord& record : particles_) {
                const Vec2 localPosition = inverseTransformVector(
                    previous,
                    {record.positionSpawnTime[0] - previous.position.x,
                     record.positionSpawnTime[1] - previous.position.y});
                const Vec2 localVelocity = inverseTransformVector(
                    previous,
                    {record.velocityLifetime[0], record.velocityLifetime[1]});
                const Vec2 localAcceleration = inverseTransformVector(
                    previous,
                    {record.accelerationDrag[0], record.accelerationDrag[1]});
                const Vec2 position = transformVector(newBasis, localPosition);
                const Vec2 velocity = transformVector(newBasis, localVelocity);
                const Vec2 acceleration =
                    transformVector(newBasis, localAcceleration);
                record.positionSpawnTime[0] = transform.position.x + position.x;
                record.positionSpawnTime[1] = transform.position.y + position.y;
                record.velocityLifetime[0] = velocity.x;
                record.velocityLifetime[1] = velocity.y;
                record.accelerationDrag[0] = acceleration.x;
                record.accelerationDrag[1] = acceleration.y;
                record.variation[0] =
                    (record.variation[0] / previous.scale.x) * transform.scale.x;
                record.variation[1] += transform.rotation - previous.rotation;
                record.variation[2] =
                    (record.variation[2] / previous.scale.y) * transform.scale.y;
            }
            markParticleRecordsChanged(0, particles_.size());
        }
        simulatedTransform_ = transform;
        frameStartTransform_ = transform;
        frameEndTransform_ = transform;
        transformInitialized_ = true;
    }
}

void ParticleEmitterInstance::pause() noexcept {
    if (state_ == PlaybackState::Playing) state_ = PlaybackState::Paused;
}

void ParticleEmitterInstance::stop() noexcept {
    particles_.clear();
    changedParticleRecordRanges_.clear();
    time_ = 0.0;
    gpuTimeOrigin_ = 0.0;
    emissionAccumulator_ = 0.0F;
    state_ = PlaybackState::Stopped;
}

void ParticleEmitterInstance::restart() noexcept {
    if (!definition_) return;
    resetPlayback();
    state_ = PlaybackState::Playing;
    emitBurstsThrough(0.0);
}

void ParticleEmitterInstance::update(float deltaSeconds) noexcept {
    if (state_ != PlaybackState::Playing || !definition_ ||
        !finite(deltaSeconds) || deltaSeconds <= 0.0F) {
        return;
    }

    const auto& emitter = definition_->emitter;
    const double previousTime = time_;
    const double nextTime = previousTime + static_cast<double>(deltaSeconds);
    if (!transformInitialized_) {
        simulatedTransform_ = targetTransform_;
        transformInitialized_ = true;
    }
    frameStartTransform_ = simulatedTransform_;
    frameEndTransform_ = targetTransform_;
    frameStartTime_ = previousTime;
    frameEndTime_ = nextTime;
    const float inverseDelta = deltaSeconds > 0.0F ? 1.0F / deltaSeconds : 0.0F;
    frameEmitterVelocity_ = {
        (frameEndTransform_.position.x - frameStartTransform_.position.x) *
            inverseDelta,
        (frameEndTransform_.position.y - frameStartTransform_.position.y) *
            inverseDelta};
    double emissionEnd = nextTime;
    if (!emitter.looping && emitter.duration > 0.0F) {
        emissionEnd = std::min(emissionEnd,
                               static_cast<double>(emitter.duration));
    }
    const double emissionStart =
        std::min(previousTime, emissionEnd);

    time_ = std::isfinite(nextTime) ? nextTime
                                    : std::numeric_limits<double>::max();
    // Capacity decisions below must not include particles which expired during
    // this frame.  Render-time analytic motion still uses each particle's
    // original spawn timestamp.
    expireParticles();
    rebaseGpuClockIfNeeded();

    if (finite(emitter.emissionRate) && emitter.emissionRate > 0.0F &&
        emissionEnd > emissionStart) {
        const float previousAccumulator = emissionAccumulator_;
        const double exact = static_cast<double>(previousAccumulator) +
                             (emissionEnd - emissionStart) *
                                 static_cast<double>(emitter.emissionRate);
        const double integralCount = std::floor(exact);
        const auto count = static_cast<std::uint32_t>(
            std::min(integralCount,
                     static_cast<double>(
                         kMaximumContinuousEventsPerUpdate)));
        emissionAccumulator_ =
            static_cast<float>(exact - integralCount);
        if (count > 0) {
            const double spacing = 1.0 / emitter.emissionRate;
            // When a caller supplies an unusually large delta, retain the
            // most recent births: those are the only ones which can still be
            // visible at render time.  The normal RenderEngine delta clamp
            // keeps validated emitters below this defensive work budget.
            const double skippedEvents = integralCount - count;
            const double first = emissionStart +
                                 ((1.0 - previousAccumulator + skippedEvents) *
                                  spacing);
            emit(count, first, spacing);
        }
    }
    emitBurstsThrough(emissionEnd);
    simulatedTransform_ = targetTransform_;
    hasUpdatedTransform_ = true;

    if (!emitter.looping && emitter.duration > 0.0F &&
        time_ >= emitter.duration && particles_.empty()) {
        state_ = PlaybackState::Finished;
    }
}

ParticleTransform2D ParticleEmitterInstance::transformAt(
    double spawnTime) const noexcept {
    if (frameEndTime_ <= frameStartTime_) return targetTransform_;
    const double normalized =
        (spawnTime - frameStartTime_) / (frameEndTime_ - frameStartTime_);
    const float amount = static_cast<float>(
        std::max(0.0, std::min(normalized, 1.0)));
    return lerpTransform(frameStartTransform_, frameEndTransform_, amount);
}

std::uint64_t ParticleEmitterInstance::nextRandom() noexcept {
    // xorshift64*: compact, deterministic and sufficient for visual effects.
    std::uint64_t value = randomState_;
    value ^= value >> 12U;
    value ^= value << 25U;
    value ^= value >> 27U;
    randomState_ = value;
    return value * 0x2545F4914F6CDD1DULL;
}

float ParticleEmitterInstance::uniform01() noexcept {
    const std::uint32_t high = static_cast<std::uint32_t>(nextRandom() >> 40U);
    return static_cast<float>(high) * (1.0F / 16777216.0F);
}

float ParticleEmitterInstance::sample(const FloatRange& range) noexcept {
    if (range.minimum == range.maximum) return range.minimum;
    if (range.distribution == RandomDistribution::Uniform) {
        return lerp(range.minimum, range.maximum, uniform01());
    }
    const float first = std::max(uniform01(), 1.0F / 16777216.0F);
    const float second = uniform01();
    const float standardNormal =
        std::sqrt(-2.0F * std::log(first)) *
        std::cos(2.0F * kPi * second);
    const float mean = (range.minimum + range.maximum) * 0.5F;
    const float sigma = (range.maximum - range.minimum) / 6.0F;
    return std::max(range.minimum,
                    std::min(mean + standardNormal * sigma, range.maximum));
}

Vec3 ParticleEmitterInstance::sample(const Vec3Range& range) noexcept {
    const FloatRange x{range.minimum.x, range.maximum.x, range.distribution};
    const FloatRange y{range.minimum.y, range.maximum.y, range.distribution};
    const FloatRange z{range.minimum.z, range.maximum.z, range.distribution};
    return Vec3{sample(x), sample(y), sample(z)};
}

Vec3 ParticleEmitterInstance::sampleShape() noexcept {
    const auto& emitter = definition_->emitter;
    if (emitter.shape == EmitterShape::Point) return {};
    if (emitter.shape == EmitterShape::Box) {
        return sample(Vec3Range{
            {-emitter.shapeExtents.x, -emitter.shapeExtents.y,
             -emitter.shapeExtents.z},
            emitter.shapeExtents, emitter.shapeDistribution});
    }
    const float angle = 2.0F * kPi * uniform01();
    float radius = 0.0F;
    if (emitter.shapeDistribution == RandomDistribution::Uniform) {
        radius = std::sqrt(uniform01()) * emitter.shapeRadius;
    } else {
        radius = std::abs(sample(FloatRange{-emitter.shapeRadius,
                                            emitter.shapeRadius,
                                            RandomDistribution::Normal}));
    }
    return Vec3{std::cos(angle) * radius, std::sin(angle) * radius, 0.0F};
}

std::uint32_t ParticleEmitterInstance::sampleCount(
    std::uint32_t minimum, std::uint32_t maximum) noexcept {
    if (minimum >= maximum) return minimum;
    const std::uint64_t span = static_cast<std::uint64_t>(maximum) - minimum + 1U;
    return minimum + static_cast<std::uint32_t>(nextRandom() % span);
}

void ParticleEmitterInstance::resetPlayback() noexcept {
    particles_.clear();
    changedParticleRecordRanges_.clear();
    time_ = 0.0;
    gpuTimeOrigin_ = 0.0;
    emissionAccumulator_ = 0.0F;
    nextBurstCursor_ = 0;
    droppedParticles_ = 0;
    simulatedTransform_ = targetTransform_;
    frameStartTransform_ = targetTransform_;
    frameEndTransform_ = targetTransform_;
    frameEmitterVelocity_ = {};
    frameStartTime_ = 0.0;
    frameEndTime_ = 0.0;
    transformInitialized_ = true;
    hasUpdatedTransform_ = false;
    const std::size_t burstCount = std::min(
        burstCursors_.size(), definition_->emitter.bursts.size());
    for (std::size_t index = 0; index < burstCount; ++index) {
        auto& cursor = burstCursors_[index];
        cursor = {};
        cursor.nextTime = definition_->emitter.bursts[index].time;
    }
    for (std::size_t index = burstCount; index < burstCursors_.size(); ++index) {
        burstCursors_[index] = {};
        burstCursors_[index].finished = true;
    }
}

void ParticleEmitterInstance::emit(std::uint32_t count,
                                   double firstSpawnTime,
                                   double spawnInterval) noexcept {
    const auto& emitter = definition_->emitter;
    const std::size_t effectiveCapacity =
        std::min<std::size_t>(emitter.maxParticles,
                              kMaximumParticleCapacity);
    const std::size_t firstChanged = particles_.size();
    for (std::uint32_t index = 0; index < count; ++index) {
        const Vec3 position = sample(emitter.position);
        const Vec3 shape = sampleShape();
        const Vec3 velocity = sample(emitter.velocity);
        const Vec3 acceleration = sample(emitter.acceleration);
        const float sizeMultiplier = sample(emitter.sizeMultiplier);
        GpuParticleRecord record{};
        const double absoluteSpawn = std::min(
            firstSpawnTime + index * spawnInterval, time_);
        if (emitter.simulationSpace == SimulationSpace::World) {
            const ParticleTransform2D birthTransform =
                transformAt(absoluteSpawn);
            const TransformBasis birthBasis =
                makeTransformBasis(birthTransform);
            const Vec2 transformedPosition = transformVector(
                birthBasis,
                {position.x + shape.x, position.y + shape.y});
            const Vec2 transformedVelocity = transformVector(
                birthBasis, {velocity.x, velocity.y});
            const Vec2 transformedAcceleration = transformVector(
                birthBasis, {acceleration.x, acceleration.y});
            record.positionSpawnTime[0] =
                birthTransform.position.x + transformedPosition.x;
            record.positionSpawnTime[1] =
                birthTransform.position.y + transformedPosition.y;
            record.velocityLifetime[0] = transformedVelocity.x +
                frameEmitterVelocity_.x * emitter.inheritVelocity;
            record.velocityLifetime[1] = transformedVelocity.y +
                frameEmitterVelocity_.y * emitter.inheritVelocity;
            record.accelerationDrag[0] = transformedAcceleration.x;
            record.accelerationDrag[1] = transformedAcceleration.y;
            record.variation[0] = sizeMultiplier * birthTransform.scale.x;
            record.variation[1] =
                sample(emitter.rotationOffset) + birthTransform.rotation;
            record.variation[2] = sizeMultiplier * birthTransform.scale.y;
        } else {
            record.positionSpawnTime[0] = position.x + shape.x;
            record.positionSpawnTime[1] = position.y + shape.y;
            record.velocityLifetime[0] = velocity.x;
            record.velocityLifetime[1] = velocity.y;
            record.accelerationDrag[0] = acceleration.x;
            record.accelerationDrag[1] = acceleration.y;
            record.variation[0] = sizeMultiplier;
            record.variation[1] = sample(emitter.rotationOffset);
            record.variation[2] = sizeMultiplier;
        }
        record.positionSpawnTime[2] = originZ_ + position.z + shape.z;
        record.positionSpawnTime[3] =
            static_cast<float>(absoluteSpawn - gpuTimeOrigin_);
        record.velocityLifetime[2] = velocity.z;
        record.velocityLifetime[3] = sample(emitter.lifetime);
        record.accelerationDrag[2] = acceleration.z;
        record.accelerationDrag[3] = emitter.drag;
        const double age = time_ - absoluteSpawn;
        if (!finite(record.velocityLifetime[3]) ||
            record.velocityLifetime[3] <= 0.0F ||
            age >= record.velocityLifetime[3]) {
            // A long frame can contain births which have already completed
            // their lifetime by the frame's render time.  They neither occupy
            // the live pool nor count as capacity drops.
            continue;
        }
        if (particles_.size() >= effectiveCapacity) {
            addDroppedSaturated(droppedParticles_, std::uint64_t{1});
            continue;
        }
        particles_.push_back(record);
    }
    markParticleRecordsChanged(firstChanged,
                               particles_.size() - firstChanged);
}

void ParticleEmitterInstance::emitBurstsThrough(double endTime) noexcept {
    const auto& emitter = definition_->emitter;
    constexpr double epsilon = 1.0e-7;
    std::size_t emittedEvents = 0;
    std::uint32_t attemptedParticles = 0;
    const std::size_t burstCount =
        std::min(emitter.bursts.size(), burstCursors_.size());
    if (burstCount == 0) return;

    std::size_t index = nextBurstCursor_ % burstCount;
    std::size_t cursorsWithoutDueEvent = 0;
    while (emittedEvents < kMaximumBurstEventsPerUpdate &&
           cursorsWithoutDueEvent < burstCount) {
        const auto& burst = emitter.bursts[index];
        auto& cursor = burstCursors_[index];
        if (!cursor.finished && cursor.nextTime <= endTime + epsilon) {
            const double previousEventTime = cursor.nextTime;
            const std::uint32_t requested =
                sampleCount(burst.countMinimum, burst.countMaximum);
            const std::uint32_t remainingAttempts =
                kMaximumBurstParticleAttemptsPerUpdate - attemptedParticles;
            const std::uint32_t processed =
                std::min(requested, remainingAttempts);
            emit(processed, cursor.nextTime, 0.0);
            attemptedParticles += processed;
            ++emittedEvents;
            cursorsWithoutDueEvent = 0;
            ++cursor.cycle;
            if (cursor.cycle < burst.cycles) {
                cursor.nextTime += burst.interval;
            } else if (emitter.looping &&
                       emitter.duration >= kMinimumTimedDuration) {
                ++cursor.loop;
                cursor.cycle = 0;
                cursor.nextTime =
                    static_cast<double>(cursor.loop) * emitter.duration +
                    burst.time;
            } else {
                cursor.finished = true;
            }
            // Invalid in-memory definitions can bypass JSON validation (the
            // editor previews such values while they are being typed).  Never
            // spin if an interval, duration or loop counter failed to advance.
            if (!cursor.finished &&
                (!std::isfinite(cursor.nextTime) ||
                 cursor.nextTime <= previousEventTime)) {
                cursor.finished = true;
            }
        } else {
            ++cursorsWithoutDueEvent;
        }

        index = (index + 1U) % burstCount;
    }
    nextBurstCursor_ = index;
}

void ParticleEmitterInstance::expireParticles() noexcept {
    std::size_t index = 0;
    while (index < particles_.size()) {
        const auto& particle = particles_[index];
        const double absoluteSpawn =
            gpuTimeOrigin_ + particle.positionSpawnTime[3];
        const double age = time_ - absoluteSpawn;
        if (age >= particle.velocityLifetime[3]) {
            const std::size_t last = particles_.size() - 1U;
            if (index != last) {
                particles_[index] = particles_.back();
                markParticleRecordsChanged(index, 1);
            }
            particles_.pop_back();
        } else {
            ++index;
        }
    }
}

void ParticleEmitterInstance::rebaseGpuClockIfNeeded() noexcept {
    // Keeping both shader operands near zero avoids the six-day float-clock
    // freeze while preserving the public monotonic double-precision clock.
    constexpr double rebaseThreshold = 4096.0;
    constexpr double rebaseQuantum = 2048.0;
    const double localTime = time_ - gpuTimeOrigin_;
    if (!std::isfinite(localTime) || localTime < rebaseThreshold) return;

    const double shift = std::floor(localTime / rebaseQuantum) *
                         rebaseQuantum;
    if (!(shift > 0.0) || !std::isfinite(shift)) return;
    for (GpuParticleRecord& particle : particles_) {
        particle.positionSpawnTime[3] -= static_cast<float>(shift);
    }
    gpuTimeOrigin_ += shift;
    markParticleRecordsChanged(0, particles_.size());
}

void ParticleEmitterInstance::markParticleRecordsChanged(
    std::size_t first, std::size_t count) noexcept {
    if (!trackParticleRecordChanges_ || count == 0) return;

    // A renderer consumes this list once per frame. If rendering is suspended,
    // collapse excessive bookkeeping to one recovery upload.
    constexpr std::size_t maximumPendingRanges = 4096;
    if (changedParticleRecordRanges_.size() >= maximumPendingRanges) {
        changedParticleRecordRanges_.clear();
        changedParticleRecordRanges_.push_back({0, particles_.size()});
        return;
    }

    if (!changedParticleRecordRanges_.empty()) {
        ParticleRecordRange& previous = changedParticleRecordRanges_.back();
        const std::size_t previousEnd = previous.first + previous.count;
        const std::size_t currentEnd = first + count;
        if (first <= previousEnd && currentEnd >= previous.first) {
            const std::size_t mergedFirst = std::min(previous.first, first);
            previous.first = mergedFirst;
            previous.count = std::max(previousEnd, currentEnd) - mergedFirst;
            return;
        }
    }
    changedParticleRecordRanges_.push_back({first, count});
}

}  // namespace newaoe::render::particle
