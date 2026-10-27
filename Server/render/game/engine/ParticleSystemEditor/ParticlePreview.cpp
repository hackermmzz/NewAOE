#include "Server/render/game/engine/ParticleSystemEditor/ParticlePreview.h"

#include <algorithm>
#include <cmath>

#include "imgui.h"

namespace newaoe::render::editor {
namespace {

Vec3 evaluatePosition(const particle::GpuParticleRecord& record,
                      Vec3 gravity,
                      float age) noexcept {
    const Vec3 initialPosition{record.positionSpawnTime[0],
                               record.positionSpawnTime[1],
                               record.positionSpawnTime[2]};
    const Vec3 initialVelocity{record.velocityLifetime[0],
                               record.velocityLifetime[1],
                               record.velocityLifetime[2]};
    const Vec3 acceleration{record.accelerationDrag[0] + gravity.x,
                            record.accelerationDrag[1] + gravity.y,
                            record.accelerationDrag[2] + gravity.z};
    const float drag = std::max(record.accelerationDrag[3], 0.0F);
    if (drag < 0.00001F) {
        const float halfTimeSquared = 0.5F * age * age;
        return {initialPosition.x + initialVelocity.x * age +
                    acceleration.x * halfTimeSquared,
                initialPosition.y + initialVelocity.y * age +
                    acceleration.y * halfTimeSquared,
                initialPosition.z + initialVelocity.z * age +
                    acceleration.z * halfTimeSquared};
    }

    const float decay = std::exp(-drag * age);
    const Vec3 terminal{acceleration.x / drag, acceleration.y / drag,
                        acceleration.z / drag};
    const float integral = (1.0F - decay) / drag;
    return {initialPosition.x + terminal.x * age +
                (initialVelocity.x - terminal.x) * integral,
            initialPosition.y + terminal.y * age +
                (initialVelocity.y - terminal.y) * integral,
            initialPosition.z + terminal.z * age +
                (initialVelocity.z - terminal.z) * integral};
}

ImU32 packColor(Color color, float opacity) noexcept {
    const auto channel = [](float value) {
        return static_cast<int>(std::clamp(value, 0.0F, 1.0F) * 255.0F +
                                0.5F);
    };
    return IM_COL32(channel(color.r), channel(color.g), channel(color.b),
                    channel(color.a * opacity));
}

}  // namespace

void ParticlePreview::setDefinition(
    const particle::ParticleSystemDefinition& definition) {
    const bool wasPlaying = playing();
    definition_ =
        std::make_shared<particle::ParticleSystemDefinition>(definition);
    instance_ = std::make_unique<particle::ParticleEmitterInstance>(definition_);
    if (wasPlaying) instance_->play();
}

void ParticlePreview::play() noexcept {
    if (instance_) instance_->play();
}

void ParticlePreview::pause() noexcept {
    if (instance_) instance_->pause();
}

void ParticlePreview::restart() noexcept {
    if (instance_) instance_->restart();
}

bool ParticlePreview::playing() const noexcept {
    return instance_ &&
           instance_->state() == particle::PlaybackState::Playing;
}

void ParticlePreview::update(
    const particle::ParticleSystemDefinition& definition,
    float deltaSeconds) {
    if (!instance_) setDefinition(definition);
    if (instance_) instance_->update(std::clamp(deltaSeconds, 0.0F, 0.1F));
}

void ParticlePreview::draw(
    ImDrawList* drawList,
    ImVec2 topLeft,
    ImVec2 size,
    const particle::ParticleSystemDefinition& definition,
    std::uint64_t textureId) const {
    if (!drawList) return;

    const ImVec2 bottomRight{topLeft.x + size.x, topLeft.y + size.y};
    drawList->AddRectFilled(topLeft, bottomRight, IM_COL32(14, 17, 23, 255));

    const ImVec2 center{topLeft.x + size.x * 0.5F,
                        topLeft.y + size.y * 0.5F};
    drawList->AddLine(ImVec2(topLeft.x, center.y),
                      ImVec2(bottomRight.x, center.y),
                      IM_COL32(45, 49, 58, 255));
    drawList->AddLine(ImVec2(center.x, topLeft.y),
                      ImVec2(center.x, bottomRight.y),
                      IM_COL32(45, 49, 58, 255));

    if (!instance_) return;
    const float now = instance_->gpuTime();
    for (const particle::GpuParticleRecord& record : instance_->particles()) {
        const float age = now - record.positionSpawnTime[3];
        const float lifetime = record.velocityLifetime[3];
        if (age < 0.0F || age >= lifetime || lifetime <= 0.0F) continue;

        const float normalizedAge = std::clamp(age / lifetime, 0.0F, 1.0F);
        const Vec3 position = evaluatePosition(
            record, definition.emitter.gravity, age);
        const Vec2 curveSize = definition.emitter.size.sample(normalizedAge);
        const float halfWidth =
            std::max(0.5F, 0.5F * curveSize.x * record.variation[0]);
        const float halfHeight =
            std::max(0.5F, 0.5F * curveSize.y * record.variation[2]);
        const float angle =
            definition.emitter.rotation.sample(normalizedAge) +
            record.variation[1];
        const float cosine = std::cos(angle);
        const float sine = std::sin(angle);
        const float screenX = center.x + position.x;
        const float screenY = center.y + position.y;

        const auto corner = [&](float x, float y) {
            return ImVec2{screenX + x * cosine - y * sine,
                          screenY + x * sine + y * cosine};
        };
        Color color = definition.emitter.color.sample(normalizedAge);
        color.r *= definition.material.tint.r;
        color.g *= definition.material.tint.g;
        color.b *= definition.material.tint.b;
        color.a *= definition.material.tint.a;
        const ImU32 packed = packColor(color, definition.material.opacity);
        const ImVec2 upperLeft = corner(-halfWidth, -halfHeight);
        const ImVec2 upperRight = corner(halfWidth, -halfHeight);
        const ImVec2 lowerRight = corner(halfWidth, halfHeight);
        const ImVec2 lowerLeft = corner(-halfWidth, halfHeight);
        if (textureId != 0) {
            drawList->AddImageQuad(textureId, upperLeft, upperRight,
                                   lowerRight, lowerLeft, ImVec2(0.0F, 0.0F),
                                   ImVec2(1.0F, 0.0F), ImVec2(1.0F, 1.0F),
                                   ImVec2(0.0F, 1.0F), packed);
        } else {
            drawList->AddQuadFilled(upperLeft, upperRight, lowerRight,
                                    lowerLeft, packed);
        }
    }

    drawList->AddRect(topLeft, bottomRight, IM_COL32(74, 80, 92, 255));
}

std::size_t ParticlePreview::particleCount() const noexcept {
    return instance_ ? instance_->particles().size() : 0;
}

double ParticlePreview::time() const noexcept {
    return instance_ ? instance_->time() : 0.0;
}

}  // namespace newaoe::render::editor

