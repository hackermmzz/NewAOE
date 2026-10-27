#pragma once

#include <cstdint>
#include <memory>
#include <random>
#include <vector>

#include "Server/render/game/engine/particle/ParticleSystem.h"

struct ImDrawList;
struct ImVec2;

namespace newaoe::render::editor {

// Lightweight editor-only analytic preview.  It mirrors the shipping
// vertex-shader motion formula on the CPU so editing stays useful without
// linking the complete deferred runtime renderer.
class ParticlePreview final {
public:
    void setDefinition(const particle::ParticleSystemDefinition& definition);
    void play() noexcept;
    void pause() noexcept;
    void restart() noexcept;
    [[nodiscard]] bool playing() const noexcept;

    void update(const particle::ParticleSystemDefinition& definition,
                float deltaSeconds);
    void draw(ImDrawList* drawList,
              ImVec2 topLeft,
              ImVec2 size,
              const particle::ParticleSystemDefinition& definition,
              std::uint64_t textureId) const;

    [[nodiscard]] std::size_t particleCount() const noexcept;
    [[nodiscard]] double time() const noexcept;

private:
    std::shared_ptr<particle::ParticleSystemDefinition> definition_;
    std::unique_ptr<particle::ParticleEmitterInstance> instance_;
};

}  // namespace newaoe::render::editor

