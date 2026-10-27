#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Server/render/game/engine/DeferredRenderer.h"
#include "Server/render/game/engine/TextureCache.h"

namespace newaoe::render::detail {

class ParticleManager final {
public:
    bool initialize(TextureCache& textures, std::string& error);
    void shutdown() noexcept;

    ParticleSystemHandle load(std::string_view directory, std::string& error);
    bool release(ParticleSystemHandle handle) noexcept;
    bool play(ParticleSystemHandle handle) noexcept;
    bool pause(ParticleSystemHandle handle) noexcept;
    bool stop(ParticleSystemHandle handle) noexcept;
    bool setPosition(ParticleSystemHandle handle, Vec2 position) noexcept;
    bool setTransform(ParticleSystemHandle handle,
                      const ParticleTransform2D& transform) noexcept;
    void update(float deltaSeconds) noexcept;

    // Uploads only newly born or compacted records into persistent GPU pages.
    // Across all playback instances at most kMaximumResidentGpuParticles
    // records may be resident at once.
    [[nodiscard]] bool buildRenderData(
        std::vector<ParticleRenderBatch>& batches,
        std::size_t& visibleParticleCount,
        std::string& error);

    static constexpr std::size_t kMaximumResidentGpuParticles =
        particle::kMaximumParticleCapacity;

private:
    struct Asset final {
        std::string canonicalDirectory;
        std::shared_ptr<const particle::ParticleSystemDefinition> definition;
        TextureHandle baseTexture{};
        TextureHandle normalTexture{};
        TextureHandle emissiveTexture{};
        sg_image motionCurveImage{};
        sg_view motionCurveView{};
        sg_image colorCurveImage{};
        sg_view colorCurveView{};
    };

    struct GpuChunk final {
        sg_buffer buffer{};
        std::size_t firstRecord = 0;
        std::size_t capacity = 0;
    };

    struct Slot final {
        std::shared_ptr<Asset> asset;
        std::unique_ptr<particle::ParticleEmitterInstance> emitter;
        std::vector<GpuChunk> gpuChunks;
        std::uint32_t generation = 1;
        bool occupied = false;
    };

    struct PendingCopy final {
        sg_buffer destination{};
        std::size_t sourceFirst = 0;
        std::size_t destinationFirst = 0;
        std::size_t count = 0;
    };

    std::shared_ptr<Asset> loadAsset(std::string_view directory,
                                    std::string& error);
    bool createCurveTextures(Asset& asset, std::string& error);
    void destroyAsset(Asset& asset) noexcept;
    bool ensureGpuChunks(Slot& slot, std::size_t required,
                         std::string& error);
    bool ensureUploadStaging(std::size_t required, std::string& error);
    void trimGpuChunks(Slot& slot, std::size_t required) noexcept;
    void destroyGpuChunks(Slot& slot) noexcept;
    void appendUploadRange(Slot& slot, std::size_t first,
                           std::size_t count);
    void appendRenderBatches(const Slot& slot,
                             std::vector<ParticleRenderBatch>& batches) const;
    Slot* resolve(ParticleSystemHandle handle) noexcept;
    const Slot* resolve(ParticleSystemHandle handle) const noexcept;
    static std::string canonicalize(std::string_view directory);

    TextureCache* textures_ = nullptr;
    std::vector<Slot> slots_;
    std::vector<std::uint32_t> freeSlots_;
    std::unordered_map<std::string, std::shared_ptr<Asset>> assets_;
    sg_buffer uploadStaging_{};
    std::size_t uploadStagingCapacity_ = 0;
    std::size_t residentGpuCapacity_ = 0;
    std::vector<particle::GpuParticleRecord> uploadRecords_;
    std::vector<PendingCopy> pendingCopies_;
    std::vector<particle::ParticleRecordRange> rangeScratch_;
};

}  // namespace newaoe::render::detail
