#include "Server/render/game/engine/particle/ParticleSystem.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <string>

namespace particle = newaoe::render::particle;

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

bool near(float lhs, float rhs) {
    return std::abs(lhs - rhs) < 0.0001F;
}

bool near(double lhs, double rhs) {
    return std::abs(lhs - rhs) < 0.0000001;
}

class CommaDecimalPoint final : public std::numpunct<char> {
protected:
    char do_decimal_point() const override { return ','; }
};

}  // namespace

int main(int argc, char** argv) {
    particle::ParticleSystemDefinition definition =
        particle::makeDefaultParticleSystem();
    definition.name = "Serialization \xE6\xB5\x8B\xE8\xAF\x95";
    definition.material.baseTexture = "textures/particle.png";
    definition.material.roughness = 0.4F;
    definition.emitter.shape = particle::EmitterShape::Circle;
    definition.emitter.shapeRadius = 12.0F;
    definition.emitter.simulationSpace = particle::SimulationSpace::Local;
    definition.emitter.inheritVelocity = 0.25F;
    definition.emitter.lifetime = {1.0F, 2.0F,
                                   particle::RandomDistribution::Normal};
    definition.emitter.bursts.push_back({0.25F, 2, 5, 2, 0.5F});

    std::string error;
    expect(particle::validateParticleSystem(definition, error),
           "default definition should validate");
    const auto sampledSize = definition.emitter.size.sample(0.5F);
    expect(near(sampledSize.x, 4.0F) && near(sampledSize.y, 4.0F),
           "size curve should interpolate linearly");
    const auto sampledColor = definition.emitter.color.sample(0.5F);
    expect(near(sampledColor.a, 0.5F),
           "color curve should interpolate alpha linearly");

    {
        auto invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.maxParticles = 0;
        expect(!particle::validateParticleSystem(invalid, error),
               "zero particle capacity must be rejected");
        invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.inheritVelocity = 1.01F;
        expect(!particle::validateParticleSystem(invalid, error),
               "inherit velocity above one must be rejected");
        invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.maxParticles =
            particle::kMaximumParticleCapacity + 1U;
        expect(!particle::validateParticleSystem(invalid, error),
               "particle capacity above the runtime limit must be rejected");
        invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.emissionRate = particle::kMaximumEmissionRate + 1.0F;
        expect(!particle::validateParticleSystem(invalid, error),
               "emission rate above the runtime limit must be rejected");
        invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.duration = particle::kMinimumTimedDuration * 0.5F;
        expect(!particle::validateParticleSystem(invalid, error),
               "sub-millisecond looping durations must be rejected");
        invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.duration = particle::kMaximumTimedDuration + 1.0F;
        expect(!particle::validateParticleSystem(invalid, error),
               "overlong emitter durations must be rejected");
        invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.lifetime.minimum =
            particle::kMinimumParticleLifetime * 0.5F;
        expect(!particle::validateParticleSystem(invalid, error),
               "too-short particle lifetimes must be rejected");
        invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.lifetime.maximum =
            particle::kMaximumParticleLifetime + 1.0F;
        expect(!particle::validateParticleSystem(invalid, error),
               "too-long particle lifetimes must be rejected");
        invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.bursts.push_back(
            {0.0F, 1, particle::kMaximumBurstParticleCount + 1U, 1, 0.0F});
        expect(!particle::validateParticleSystem(invalid, error),
               "oversized burst counts must be rejected");
        invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.duration = 10.0F;
        invalid.emitter.bursts.push_back(
            {0.0F, 1, 1, particle::kMaximumBurstCycles + 1U,
             particle::kMinimumBurstInterval});
        expect(!particle::validateParticleSystem(invalid, error),
               "oversized burst cycle counts must be rejected");
        invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.bursts.push_back(
            {0.0F, 1, 1, 2, particle::kMinimumBurstInterval * 0.5F});
        expect(!particle::validateParticleSystem(invalid, error),
               "too-short repeated burst intervals must be rejected");
        invalid = particle::makeDefaultParticleSystem();
        invalid.material.baseTexture = "..\\outside.png";
        expect(!particle::validateParticleSystem(invalid, error),
               "Windows-style traversal must be rejected on every host");
        invalid = particle::makeDefaultParticleSystem();
        invalid.material.baseTexture = "C:\\absolute.png";
        expect(!particle::validateParticleSystem(invalid, error),
               "drive-relative material paths must not be portable assets");
        invalid = particle::makeDefaultParticleSystem();
        invalid.emitter.bursts.push_back(
            {0.0F, 1, 1, 1, particle::kMaximumBurstInterval + 1.0F});
        expect(!particle::validateParticleSystem(invalid, error),
               "overlong burst intervals must be rejected");
    }

    if (argc == 2) {
        const std::filesystem::path directory =
            std::filesystem::u8path(argv[1]);
        const std::locale previousLocale = std::locale();
        std::locale::global(std::locale(
            previousLocale, new CommaDecimalPoint));
        expect(particle::saveParticleSystem(directory, definition, error),
               error.c_str());
        {
            std::ifstream manifest(directory / "particle.json",
                                   std::ios::binary);
            const std::string json{std::istreambuf_iterator<char>{manifest},
                                   std::istreambuf_iterator<char>{}};
            expect(json.find("\"roughness\": 0.400000006") !=
                       std::string::npos,
                   "JSON output must use a locale-independent decimal point");
        }
        particle::ParticleSystemDefinition loaded;
        expect(particle::loadParticleSystem(directory, loaded, error),
               error.c_str());
        expect(loaded.name == definition.name, "UTF-8 name should round-trip");
        expect(loaded.material.baseTexture == definition.material.baseTexture,
               "texture path should round-trip");
        expect(near(loaded.material.roughness, 0.4F),
               "material scalar should round-trip");
        expect(loaded.emitter.bursts.size() == 1 &&
                   loaded.emitter.bursts[0].countMaximum == 5,
               "burst should round-trip");
        expect(loaded.emitter.simulationSpace ==
                   particle::SimulationSpace::Local &&
                   near(loaded.emitter.inheritVelocity, 0.25F),
               "simulation space and inherited velocity should round-trip");
        std::locale::global(previousLocale);
    }

    auto runtimeDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            particle::makeDefaultParticleSystem());
    runtimeDefinition->emitter.maxParticles = 8;
    runtimeDefinition->emitter.emissionRate = 10.0F;
    runtimeDefinition->emitter.duration = 0.5F;
    runtimeDefinition->emitter.looping = false;
    runtimeDefinition->emitter.lifetime = {
        1.0F, 1.0F, particle::RandomDistribution::Uniform};
    runtimeDefinition->emitter.bursts = {{0.0F, 2, 2, 1, 0.0F}};
    particle::ParticleEmitterInstance instance(runtimeDefinition, 1234);
    instance.enableParticleRecordChangeTracking();
    instance.play();
    expect(instance.particles().size() == 2,
           "time-zero burst should fire on play");
    expect(instance.changedParticleRecordRanges().size() == 1 &&
               instance.changedParticleRecordRanges()[0].first == 0 &&
               instance.changedParticleRecordRanges()[0].count == 2,
           "time-zero births should produce one contiguous GPU update");
    instance.clearParticleRecordChanges();
    instance.update(0.1F);
    expect(instance.particles().size() == 3,
           "continuous emission should add one particle");
    expect(instance.changedParticleRecordRanges().size() == 1 &&
               instance.changedParticleRecordRanges()[0].first == 2 &&
               instance.changedParticleRecordRanges()[0].count == 1,
           "only a newly born record should be marked for GPU upload");
    instance.pause();
    const double pausedTime = instance.time();
    instance.update(10.0F);
    expect(near(instance.time(), pausedTime), "pause should freeze local time");
    instance.play();
    instance.update(0.4F);
    expect(instance.particles().size() == 7,
           "finite emitter should produce the expected count");
    instance.update(1.1F);
    expect(instance.particles().empty(), "expired particles should be removed");
    expect(instance.state() == particle::PlaybackState::Finished,
           "finite emitter should finish after its particles expire");

    auto largeDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            particle::makeDefaultParticleSystem());
    largeDefinition->emitter.maxParticles =
        particle::kMaximumParticleCapacity;
    particle::ParticleEmitterInstance lazyLarge(largeDefinition, 4321);
    expect(lazyLarge.particles().capacity() <= 256,
           "a million-particle manifest must not reserve 64 MiB at load");

    runtimeDefinition->emitter.maxParticles = 4;
    runtimeDefinition->emitter.emissionRate = 0.0F;
    runtimeDefinition->emitter.bursts = {{0.0F, 10, 10, 1, 0.0F}};
    particle::ParticleEmitterInstance capped(runtimeDefinition, 5678);
    capped.play();
    expect(capped.particles().size() == 4,
           "particle pool should respect its fixed capacity");
    expect(capped.droppedParticleCount() == 6,
           "capacity overflow should be observable");

    auto recyclingDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            particle::makeDefaultParticleSystem());
    recyclingDefinition->emitter.maxParticles = 1;
    recyclingDefinition->emitter.emissionRate = 10.0F;
    recyclingDefinition->emitter.duration = 0.0F;
    recyclingDefinition->emitter.looping = false;
    recyclingDefinition->emitter.lifetime = {
        0.05F, 0.05F, particle::RandomDistribution::Uniform};
    particle::ParticleEmitterInstance recycling(recyclingDefinition, 1111);
    recycling.play();
    recycling.update(0.1F);
    expect(recycling.particles().size() == 1,
           "first capacity-limited particle should spawn");
    const float firstSpawn = recycling.particles().empty()
                                 ? 0.0F
                                 : recycling.particles()[0].positionSpawnTime[3];
    recycling.update(0.1F);
    expect(recycling.particles().size() == 1 &&
               recycling.particles()[0].positionSpawnTime[3] > firstSpawn,
           "expired particles must free capacity before new emission");
    expect(recycling.droppedParticleCount() == 0,
           "reusing expired capacity must not count as a dropped particle");

    auto withinFrameDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            particle::makeDefaultParticleSystem());
    withinFrameDefinition->emitter.maxParticles = 1;
    withinFrameDefinition->emitter.emissionRate = 20.0F;
    withinFrameDefinition->emitter.duration = 0.0F;
    withinFrameDefinition->emitter.looping = false;
    withinFrameDefinition->emitter.lifetime = {
        0.01F, 0.01F, particle::RandomDistribution::Uniform};
    particle::ParticleEmitterInstance withinFrame(withinFrameDefinition, 1212);
    withinFrame.play();
    withinFrame.update(0.1F);
    expect(withinFrame.particles().size() == 1 &&
               near(withinFrame.particles()[0].positionSpawnTime[3], 0.1F),
           "already-dead early births must not block a live later birth");
    expect(withinFrame.droppedParticleCount() == 0,
           "already-dead births must not count as capacity drops");

    auto extremeDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            particle::makeDefaultParticleSystem());
    extremeDefinition->emitter.maxParticles = 4;
    extremeDefinition->emitter.emissionRate =
        std::numeric_limits<float>::max();
    extremeDefinition->emitter.duration = 0.0F;
    extremeDefinition->emitter.looping = false;
    particle::ParticleEmitterInstance extreme(extremeDefinition, 2222);
    extreme.play();
    extreme.update(std::numeric_limits<float>::max());
    expect(extreme.particles().size() == 4,
           "out-of-contract emission rates must saturate without integer UB");
    expect(extreme.droppedParticleCount() ==
               particle::kMaximumContinuousEventsPerUpdate - 4U,
           "continuous work limiting must retain the newest births");

    auto zeroDurationDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            particle::makeDefaultParticleSystem());
    zeroDurationDefinition->emitter.maxParticles = 4;
    zeroDurationDefinition->emitter.emissionRate = 0.0F;
    zeroDurationDefinition->emitter.duration = 0.0F;
    zeroDurationDefinition->emitter.looping = true;
    zeroDurationDefinition->emitter.bursts = {{0.0F, 1, 1, 1, 0.0F}};
    particle::ParticleEmitterInstance zeroDuration(zeroDurationDefinition,
                                                    3333);
    zeroDuration.play();
    zeroDuration.update(0.1F);
    expect(zeroDuration.particles().size() == 1,
           "invalid zero-duration loops must terminate instead of spinning");

    auto budgetDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            particle::makeDefaultParticleSystem());
    budgetDefinition->emitter.maxParticles = 1;
    budgetDefinition->emitter.emissionRate = 0.0F;
    budgetDefinition->emitter.duration = 0.0F;
    budgetDefinition->emitter.looping = false;
    budgetDefinition->emitter.lifetime = {
        1000.0F, 1000.0F, particle::RandomDistribution::Uniform};
    budgetDefinition->emitter.bursts = {{
        0.0F,
        1,
        1,
        static_cast<std::uint32_t>(
            particle::kMaximumBurstEventsPerUpdate + 10U),
        particle::kMinimumBurstInterval}};
    particle::ParticleEmitterInstance budgeted(budgetDefinition, 4444);
    budgeted.play();
    budgeted.update(100.0F);
    expect(budgeted.droppedParticleCount() ==
               particle::kMaximumBurstEventsPerUpdate,
           "burst processing must stop at the per-update event budget");

    auto longClockDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            particle::makeDefaultParticleSystem());
    longClockDefinition->emitter.emissionRate = 0.0F;
    longClockDefinition->emitter.duration = 0.0F;
    longClockDefinition->emitter.looping = false;
    longClockDefinition->emitter.lifetime = {
        particle::kMaximumParticleLifetime,
        particle::kMaximumParticleLifetime,
        particle::RandomDistribution::Uniform};
    longClockDefinition->emitter.bursts = {{0.0F, 1, 1, 1, 0.0F}};
    particle::ParticleEmitterInstance longClock(longClockDefinition, 5555);
    longClock.play();
    longClock.update(4097.0F);
    expect(longClock.particles().size() == 1,
           "GPU clock rebasing must retain live long-lived particles");
    if (!longClock.particles().empty()) {
        const float gpuAge =
            longClock.gpuTime() -
            longClock.particles()[0].positionSpawnTime[3];
        expect(std::abs(gpuAge - 4097.0F) < 0.01F,
               "rebased GPU time must preserve particle age");
    }
    longClock.stop();
    longClock.play();
    longClock.update(524288.0F);
    const double oldLongTime = longClock.time();
    longClock.update(1.0F / 60.0F);
    expect(longClock.time() > oldLongTime,
           "double playback clock must continue advancing after six days");
    expect(std::abs(longClock.gpuTime()) < 4096.0F,
           "GPU-facing float clock must remain inside the rebase window");

    auto transformDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            particle::makeDefaultParticleSystem());
    transformDefinition->emitter.maxParticles = 8;
    transformDefinition->emitter.emissionRate = 0.0F;
    transformDefinition->emitter.duration = 0.0F;
    transformDefinition->emitter.looping = false;
    transformDefinition->emitter.lifetime = {
        5.0F, 5.0F, particle::RandomDistribution::Uniform};
    transformDefinition->emitter.position = {
        {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F},
        particle::RandomDistribution::Uniform};
    transformDefinition->emitter.velocity = {};
    transformDefinition->emitter.acceleration = {};
    transformDefinition->emitter.gravity = {};
    transformDefinition->emitter.bursts.clear();
    particle::ParticleEmitterInstance interpolated(transformDefinition, 6666);
    interpolated.setTransform({{0.0F, 0.0F}, 0.0F, {1.0F, 1.0F}});
    interpolated.play();
    interpolated.update(0.25F);
    transformDefinition->emitter.emissionRate = 2.0F;
    interpolated.setTransform({{100.0F, 0.0F}, 0.0F, {1.0F, 1.0F}});
    interpolated.update(1.0F);
    expect(interpolated.particles().size() == 2 &&
               near(interpolated.particles()[0].positionSpawnTime[0], 50.0F) &&
               near(interpolated.particles()[1].positionSpawnTime[0], 100.0F),
           "world births should interpolate across the frame transform");

    auto birthTransformDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            particle::makeDefaultParticleSystem());
    birthTransformDefinition->emitter.emissionRate = 0.0F;
    birthTransformDefinition->emitter.position = {
        {1.0F, 0.0F, 0.0F}, {1.0F, 0.0F, 0.0F},
        particle::RandomDistribution::Uniform};
    birthTransformDefinition->emitter.velocity = {};
    birthTransformDefinition->emitter.acceleration = {};
    birthTransformDefinition->emitter.gravity = {};
    birthTransformDefinition->emitter.bursts = {{0.0F, 1, 1, 1, 0.0F}};
    particle::ParticleEmitterInstance transformedBirth(
        birthTransformDefinition, 7777);
    transformedBirth.setTransform({{10.0F, 20.0F},
                                   1.57079632679F,
                                   {2.0F, 3.0F}});
    transformedBirth.play();
    expect(transformedBirth.particles().size() == 1 &&
               near(transformedBirth.particles()[0].positionSpawnTime[0], 10.0F) &&
               near(transformedBirth.particles()[0].positionSpawnTime[1], 22.0F) &&
               near(transformedBirth.particles()[0].variation[0], 2.0F) &&
               near(transformedBirth.particles()[0].variation[2], 3.0F),
           "world births should bake rotation and non-uniform scale");

    particle::ParticleEmitterInstance placedAfterPlay(
        birthTransformDefinition, 7878);
    placedAfterPlay.play();
    placedAfterPlay.setTransform(
        {{40.0F, 50.0F}, 0.0F, {2.0F, 3.0F}});
    expect(placedAfterPlay.particles().size() == 1 &&
               near(placedAfterPlay.particles()[0].positionSpawnTime[0],
                    42.0F) &&
               near(placedAfterPlay.particles()[0].positionSpawnTime[1],
                    50.0F),
           "placement before the first update must not streak from origin");

    auto inheritedDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            *transformDefinition);
    inheritedDefinition->emitter.inheritVelocity = 0.5F;
    inheritedDefinition->emitter.emissionRate = 0.0F;
    particle::ParticleEmitterInstance inherited(inheritedDefinition, 8888);
    inherited.setTransform({{0.0F, 0.0F}, 0.0F, {1.0F, 1.0F}});
    inherited.play();
    inherited.update(0.25F);
    inheritedDefinition->emitter.emissionRate = 2.0F;
    inherited.setTransform({{100.0F, 0.0F}, 0.0F, {1.0F, 1.0F}});
    inherited.update(1.0F);
    expect(!inherited.particles().empty() &&
               near(inherited.particles()[0].velocityLifetime[0], 50.0F),
           "world births should inherit configured emitter velocity");

    auto localDefinition =
        std::make_shared<particle::ParticleSystemDefinition>(
            *birthTransformDefinition);
    localDefinition->emitter.simulationSpace =
        particle::SimulationSpace::Local;
    particle::ParticleEmitterInstance local(localDefinition, 9999);
    local.setTransform({{100.0F, 200.0F}, 0.5F, {2.0F, 3.0F}});
    local.play();
    expect(local.particles().size() == 1 &&
               near(local.particles()[0].positionSpawnTime[0], 1.0F) &&
               near(local.particles()[0].variation[0], 1.0F) &&
               near(local.particles()[0].variation[2], 1.0F),
           "local-space births should remain in emitter-local coordinates");

    if (failures != 0) {
        std::cerr << failures << " particle tests failed\n";
        return 1;
    }
    std::cout << "particle tests passed\n";
    return 0;
}
