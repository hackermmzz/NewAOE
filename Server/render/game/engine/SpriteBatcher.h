#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Server/render/game/engine/RenderTypes.h"
#include "Server/render/game/engine/TextureCache.h"

namespace newaoe::render::detail {

// Layout is intentionally four-float aligned so every backend produced by
// sokol-shdc consumes the same instance stride.
struct SpriteGpuInstance final {
    float rectangle[4]{};       // x, y, width, height
    float tint[4]{};
    float emissiveCutoff[4]{};  // emissive rgb, alpha cutoff
    float surfaceDepth[4]{};    // roughness, metallic, depth, normal strength
};

struct SpriteBatch final {
    sg_view baseColor{};
    sg_view normal{};
    sg_view emissive{};
    std::size_t firstInstance = 0;
    std::size_t instanceCount = 0;
};

class SpriteBatcher final {
public:
    void submit(TextureHandle texture, const WorldRect& destination,
                const SpriteMaterial& material);

    // Resolves handles, groups equal texture bindings, and encodes original
    // submission order into depth so batching never changes 2D overlap.
    void build(const TextureCache& textures);
    void clear() noexcept;

    [[nodiscard]] const std::vector<SpriteGpuInstance>& instances() const {
        return gpuInstances_;
    }
    [[nodiscard]] const std::vector<SpriteBatch>& batches() const {
        return batches_;
    }
    [[nodiscard]] std::size_t submittedCount() const noexcept {
        return submissions_.size();
    }

private:
    struct Submission final {
        TextureHandle texture{};
        WorldRect destination{};
        SpriteMaterial material{};
        std::uint32_t order = 0;
    };

    struct Resolved final {
        const Submission* submission = nullptr;
        sg_view baseColor{};
        sg_view normal{};
        sg_view emissive{};
    };

    std::vector<Submission> submissions_;
    std::vector<Resolved> resolved_;
    std::vector<SpriteGpuInstance> gpuInstances_;
    std::vector<SpriteBatch> batches_;
};

}  // namespace newaoe::render::detail
