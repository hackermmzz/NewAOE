#include "Server/render/game/engine/SpriteBatcher.h"

#include <algorithm>
#include <tuple>

namespace newaoe::render::detail {
namespace {

bool sameBindings(const SpriteBatch& batch, const SpriteBatch& other) noexcept {
    return batch.baseColor.id == other.baseColor.id &&
           batch.normal.id == other.normal.id &&
           batch.emissive.id == other.emissive.id;
}

}  // namespace

void SpriteBatcher::submit(TextureHandle texture,
                           const WorldRect& destination,
                           const SpriteMaterial& material) {
    if (!texture.valid() || destination.width == 0.0F ||
        destination.height == 0.0F) {
        return;
    }
    submissions_.push_back(Submission{texture, destination, material,
                                      static_cast<std::uint32_t>(
                                          submissions_.size())});
}

void SpriteBatcher::build(const TextureCache& textures) {
    resolved_.clear();
    gpuInstances_.clear();
    batches_.clear();
    resolved_.reserve(submissions_.size());

    for (const Submission& submission : submissions_) {
        const TextureGpuResource* base = textures.resolve(submission.texture);
        if (base == nullptr) continue;
        const TextureGpuResource* normal =
            textures.resolve(submission.material.normalTexture);
        const TextureGpuResource* emissive =
            textures.resolve(submission.material.emissiveTexture);
        resolved_.push_back(Resolved{
            &submission,
            base->view,
            normal != nullptr ? normal->view : textures.flatNormalView(),
            emissive != nullptr ? emissive->view : textures.whiteView(),
        });
    }

    // Group by actual GPU bindings.  The encoded depth below preserves the
    // visual result even though this changes submission order.
    std::stable_sort(resolved_.begin(), resolved_.end(),
                     [](const Resolved& lhs, const Resolved& rhs) {
                         return std::tie(lhs.baseColor.id, lhs.normal.id,
                                         lhs.emissive.id) <
                                std::tie(rhs.baseColor.id, rhs.normal.id,
                                         rhs.emissive.id);
                     });

    gpuInstances_.reserve(resolved_.size());
    batches_.reserve(resolved_.size());
    const float denominator =
        static_cast<float>(std::max<std::size_t>(submissions_.size() + 1U, 2U));

    for (const Resolved& resolved : resolved_) {
        const Submission& source = *resolved.submission;
        SpriteGpuInstance instance{};
        instance.rectangle[0] = source.destination.x;
        instance.rectangle[1] = source.destination.y;
        instance.rectangle[2] = source.destination.width;
        instance.rectangle[3] = source.destination.height;
        instance.tint[0] = source.material.tint.r;
        instance.tint[1] = source.material.tint.g;
        instance.tint[2] = source.material.tint.b;
        instance.tint[3] = source.material.tint.a;
        instance.emissiveCutoff[0] = source.material.emissiveColor.r;
        instance.emissiveCutoff[1] = source.material.emissiveColor.g;
        instance.emissiveCutoff[2] = source.material.emissiveColor.b;
        instance.emissiveCutoff[3] =
            std::max(source.material.alphaCutoff, 0.0F);
        instance.surfaceDepth[0] =
            std::clamp(source.material.roughness, 0.0F, 1.0F);
        instance.surfaceDepth[1] =
            std::clamp(source.material.metallic, 0.0F, 1.0F);
        // Later submissions get a smaller depth and win SG_COMPAREFUNC_LESS,
        // independent of texture-batch order.
        instance.surfaceDepth[2] =
            1.0F - (static_cast<float>(source.order) + 1.0F) / denominator;
        instance.surfaceDepth[3] = 1.0F;
        gpuInstances_.push_back(instance);

        SpriteBatch candidate{};
        candidate.baseColor = resolved.baseColor;
        candidate.normal = resolved.normal;
        candidate.emissive = resolved.emissive;
        if (batches_.empty() || !sameBindings(batches_.back(), candidate)) {
            candidate.firstInstance = gpuInstances_.size() - 1U;
            candidate.instanceCount = 1;
            batches_.push_back(candidate);
        } else {
            ++batches_.back().instanceCount;
        }
    }
}

void SpriteBatcher::clear() noexcept {
    submissions_.clear();
    resolved_.clear();
    gpuInstances_.clear();
    batches_.clear();
}

}  // namespace newaoe::render::detail
