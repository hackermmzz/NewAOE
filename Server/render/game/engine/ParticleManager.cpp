#include "Server/render/game/engine/ParticleManager.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <limits>
#include <system_error>

namespace newaoe::render::detail {
namespace {

constexpr int kCurveResolution = 256;
constexpr std::size_t kInitialGpuChunkCapacity = 4096;

template <typename Handle>
bool valid(Handle handle) noexcept {
    return handle.id != 0;
}

std::string lookupKey(std::string path) {
#if defined(_WIN32)
    std::transform(path.begin(), path.end(), path.begin(),
                   [](unsigned char character) {
                       return character >= 'A' && character <= 'Z'
                                  ? static_cast<char>(character - 'A' + 'a')
                                  : static_cast<char>(character);
                   });
#endif
    return path;
}

bool validTransform(const ParticleTransform2D& transform) noexcept {
    return std::isfinite(transform.position.x) &&
           std::isfinite(transform.position.y) &&
           std::isfinite(transform.rotation) &&
           std::isfinite(transform.scale.x) &&
           std::isfinite(transform.scale.y) &&
           std::abs(transform.scale.x) >= 0.0001F &&
           std::abs(transform.scale.y) >= 0.0001F;
}

Vec2 inverseTransformVector(const ParticleTransform2D& transform,
                            Vec2 value) noexcept {
    const float cosine = std::cos(transform.rotation);
    const float sine = std::sin(transform.rotation);
    const float x = value.x * cosine + value.y * sine;
    const float y = -value.x * sine + value.y * cosine;
    return {x / transform.scale.x, y / transform.scale.y};
}

bool createCurveTexture(const std::array<float, kCurveResolution * 4>& pixels,
                        const char* label, sg_image& image, sg_view& view,
                        std::string& error) {
    const sg_pixelformat_info format =
        sg_query_pixelformat(SG_PIXELFORMAT_RGBA32F);
    if (!format.sample) {
        error = "RGBA32F sampled textures are required for particle curves";
        return false;
    }
    sg_image_desc descriptor{};
    descriptor.width = kCurveResolution;
    descriptor.height = 1;
    descriptor.pixel_format = SG_PIXELFORMAT_RGBA32F;
    descriptor.data.mip_levels[0] = {pixels.data(), sizeof(pixels)};
    descriptor.label = label;
    image = sg_make_image(&descriptor);
    if (sg_query_image_state(image) != SG_RESOURCESTATE_VALID) {
        error = std::string("failed to create particle curve texture: ") +
                label;
        return false;
    }
    sg_view_desc viewDescriptor{};
    viewDescriptor.texture.image = image;
    viewDescriptor.label = label;
    view = sg_make_view(&viewDescriptor);
    if (sg_query_view_state(view) != SG_RESOURCESTATE_VALID) {
        error = std::string("failed to create particle curve view: ") + label;
        return false;
    }
    return true;
}

}  // namespace

bool ParticleManager::initialize(TextureCache& textures, std::string& error) {
    error.clear();
    textures_ = &textures;
    return true;
}

void ParticleManager::shutdown() noexcept {
    for (Slot& slot : slots_) destroyGpuChunks(slot);
    if (valid(uploadStaging_)) sg_destroy_buffer(uploadStaging_);
    uploadStaging_ = {};
    uploadStagingCapacity_ = 0;
    residentGpuCapacity_ = 0;
    uploadRecords_.clear();
    pendingCopies_.clear();
    rangeScratch_.clear();
    slots_.clear();
    freeSlots_.clear();
    for (auto& entry : assets_) destroyAsset(*entry.second);
    assets_.clear();
    textures_ = nullptr;
}

ParticleSystemHandle ParticleManager::load(std::string_view directory,
                                           std::string& error) {
    if (textures_ == nullptr) {
        error = "particle manager is not initialized";
        return {};
    }
    std::shared_ptr<Asset> asset = loadAsset(directory, error);
    if (!asset) return {};

    std::uint32_t index = 0;
    if (freeSlots_.empty()) {
        index = static_cast<std::uint32_t>(slots_.size());
        slots_.emplace_back();
    } else {
        index = freeSlots_.back();
        freeSlots_.pop_back();
    }
    Slot& slot = slots_[index];
    if (slot.generation == 0) slot.generation = 1;
    slot.asset = std::move(asset);
    slot.emitter = std::make_unique<particle::ParticleEmitterInstance>(
        slot.asset->definition,
        (static_cast<std::uint64_t>(slot.generation) << 32U) | index);
    slot.emitter->enableParticleRecordChangeTracking();
    slot.occupied = true;
    return ParticleSystemHandle{index, slot.generation};
}

bool ParticleManager::release(ParticleSystemHandle handle) noexcept {
    Slot* slot = resolve(handle);
    if (slot == nullptr) return false;
    std::shared_ptr<Asset> releasedAsset = std::move(slot->asset);
    destroyGpuChunks(*slot);
    slot->emitter.reset();
    slot->occupied = false;
    ++slot->generation;
    if (slot->generation == 0) ++slot->generation;
    freeSlots_.push_back(handle.index);

    // The cache exists to share immutable data between live playback
    // instances, not to retain every effect ever loaded until shutdown.  Once
    // the final instance goes away, release its textures and curve LUTs too.
    const bool stillInUse = std::any_of(
        slots_.begin(), slots_.end(),
        [&releasedAsset](const Slot& candidate) {
            return candidate.occupied && candidate.asset == releasedAsset;
        });
    if (releasedAsset && !stillInUse) {
        const auto cached = std::find_if(
            assets_.begin(), assets_.end(),
            [&releasedAsset](const auto& entry) {
                return entry.second == releasedAsset;
            });
        destroyAsset(*releasedAsset);
        if (cached != assets_.end()) assets_.erase(cached);
    }
    return true;
}

bool ParticleManager::play(ParticleSystemHandle handle) noexcept {
    Slot* slot = resolve(handle);
    if (slot == nullptr) return false;
    slot->emitter->play();
    return true;
}

bool ParticleManager::pause(ParticleSystemHandle handle) noexcept {
    Slot* slot = resolve(handle);
    if (slot == nullptr) return false;
    slot->emitter->pause();
    return true;
}

bool ParticleManager::stop(ParticleSystemHandle handle) noexcept {
    Slot* slot = resolve(handle);
    if (slot == nullptr) return false;
    slot->emitter->stop();
    destroyGpuChunks(*slot);
    return true;
}

bool ParticleManager::setPosition(ParticleSystemHandle handle,
                                  Vec2 position) noexcept {
    Slot* slot = resolve(handle);
    if (slot == nullptr) return false;
    ParticleTransform2D transform = slot->emitter->transform();
    transform.position = position;
    return setTransform(handle, transform);
}

bool ParticleManager::setTransform(
    ParticleSystemHandle handle,
    const ParticleTransform2D& transform) noexcept {
    Slot* slot = resolve(handle);
    if (slot == nullptr || !validTransform(transform)) return false;
    slot->emitter->setTransform(transform);
    return true;
}

void ParticleManager::update(float deltaSeconds) noexcept {
    for (Slot& slot : slots_) {
        if (slot.occupied) slot.emitter->update(deltaSeconds);
    }
}

bool ParticleManager::ensureGpuChunks(Slot& slot, std::size_t required,
                                      std::string& error) {
    const std::size_t emitterMaximum = std::min<std::size_t>(
        slot.asset->definition->emitter.maxParticles,
        particle::kMaximumParticleCapacity);
    if (required > emitterMaximum) {
        error = "particle instance exceeds its validated capacity";
        return false;
    }

    const std::size_t originalChunkCount = slot.gpuChunks.size();
    const auto rollbackNewChunks = [&]() noexcept {
        while (slot.gpuChunks.size() > originalChunkCount) {
            const GpuChunk& chunk = slot.gpuChunks.back();
            if (valid(chunk.buffer)) sg_destroy_buffer(chunk.buffer);
            residentGpuCapacity_ =
                residentGpuCapacity_ >= chunk.capacity
                    ? residentGpuCapacity_ - chunk.capacity
                    : 0;
            slot.gpuChunks.pop_back();
        }
    };

    std::size_t current = slot.gpuChunks.empty()
                              ? 0
                              : slot.gpuChunks.back().firstRecord +
                                    slot.gpuChunks.back().capacity;
    while (current < required) {
        const std::size_t remainingForEmitter = emitterMaximum - current;
        std::size_t planned = 0;
        if (slot.gpuChunks.empty()) {
            // Small systems must not each pin a 4096-record page. A large
            // first burst may still take one page and one draw immediately.
            planned = std::min(
                std::min(kInitialGpuChunkCapacity,
                         std::max<std::size_t>(required, 256)),
                remainingForEmitter);
        } else {
            planned = std::min(slot.gpuChunks.back().capacity * 2U,
                               remainingForEmitter);
        }
        const std::size_t needed = required - current;
        const std::size_t available =
            kMaximumResidentGpuParticles - residentGpuCapacity_;
        if (needed > available || planned == 0) {
            error = "particle GPU pages exceed the global resident budget of " +
                    std::to_string(kMaximumResidentGpuParticles) +
                    " records";
            rollbackNewChunks();
            return false;
        }
        const std::size_t capacity =
            planned <= available ? planned : needed;
        if (capacity >
            std::numeric_limits<std::size_t>::max() /
                sizeof(particle::GpuParticleRecord)) {
            error = "particle GPU buffer size overflow";
            rollbackNewChunks();
            return false;
        }

        sg_buffer_desc descriptor{};
        descriptor.size = capacity * sizeof(particle::GpuParticleRecord);
        descriptor.usage.vertex_buffer = true;
        descriptor.usage.copy_dst = true;
        descriptor.label = "NewAOE persistent particle page";
        const sg_buffer buffer = sg_make_buffer(&descriptor);
        if (sg_query_buffer_state(buffer) != SG_RESOURCESTATE_VALID) {
            if (valid(buffer)) sg_destroy_buffer(buffer);
            error = "failed to allocate a persistent particle GPU page";
            rollbackNewChunks();
            return false;
        }
        slot.gpuChunks.push_back({buffer, current, capacity});
        residentGpuCapacity_ += capacity;
        current += capacity;
    }
    return true;
}

bool ParticleManager::ensureUploadStaging(std::size_t required,
                                          std::string& error) {
    if (required == 0) return true;
    if (required <= uploadStagingCapacity_ && valid(uploadStaging_)) {
        return true;
    }
    if (required > kMaximumResidentGpuParticles) {
        error = "particle upload exceeds the global GPU record budget";
        return false;
    }

    std::size_t capacity = std::max<std::size_t>(
        uploadStagingCapacity_, 1024);
    while (capacity < required) {
        capacity = std::min(capacity * 2U,
                            kMaximumResidentGpuParticles);
    }
    sg_buffer_desc descriptor{};
    descriptor.size = capacity * sizeof(particle::GpuParticleRecord);
    descriptor.usage.staging_buffer = true;
    descriptor.usage.write_transient = true;
    descriptor.usage.copy_src = true;
    descriptor.label = "NewAOE particle upload staging";
    const sg_buffer replacement = sg_make_buffer(&descriptor);
    if (sg_query_buffer_state(replacement) != SG_RESOURCESTATE_VALID) {
        if (valid(replacement)) sg_destroy_buffer(replacement);
        error = "failed to allocate the particle upload staging buffer";
        return false;
    }
    if (valid(uploadStaging_)) sg_destroy_buffer(uploadStaging_);
    uploadStaging_ = replacement;
    uploadStagingCapacity_ = capacity;
    return true;
}

void ParticleManager::trimGpuChunks(Slot& slot,
                                    std::size_t required) noexcept {
    // Empty and unused tail pages must not pin the global budget. Low-rate
    // emitters may recreate their first page, but active particles can then
    // always consume the advertised one-million-record budget.
    while (!slot.gpuChunks.empty() &&
           slot.gpuChunks.back().firstRecord >= required) {
        const GpuChunk& chunk = slot.gpuChunks.back();
        if (valid(chunk.buffer)) sg_destroy_buffer(chunk.buffer);
        residentGpuCapacity_ =
            residentGpuCapacity_ >= chunk.capacity
                ? residentGpuCapacity_ - chunk.capacity
                : 0;
        slot.gpuChunks.pop_back();
    }
}

void ParticleManager::destroyGpuChunks(Slot& slot) noexcept {
    for (const GpuChunk& chunk : slot.gpuChunks) {
        if (valid(chunk.buffer)) sg_destroy_buffer(chunk.buffer);
        residentGpuCapacity_ =
            residentGpuCapacity_ >= chunk.capacity
                ? residentGpuCapacity_ - chunk.capacity
                : 0;
    }
    slot.gpuChunks.clear();
}

void ParticleManager::appendUploadRange(Slot& slot, std::size_t first,
                                        std::size_t count) {
    const auto& source = slot.emitter->particles();
    const std::size_t end = first + count;
    for (const GpuChunk& chunk : slot.gpuChunks) {
        const std::size_t chunkEnd = chunk.firstRecord + chunk.capacity;
        const std::size_t overlapFirst = std::max(first, chunk.firstRecord);
        const std::size_t overlapEnd = std::min(end, chunkEnd);
        if (overlapFirst >= overlapEnd) continue;

        const std::size_t sourceFirst = uploadRecords_.size();
        uploadRecords_.insert(uploadRecords_.end(),
                              source.begin() +
                                  static_cast<std::ptrdiff_t>(overlapFirst),
                              source.begin() +
                                  static_cast<std::ptrdiff_t>(overlapEnd));
        pendingCopies_.push_back(
            {chunk.buffer, sourceFirst,
             overlapFirst - chunk.firstRecord,
             overlapEnd - overlapFirst});
    }
}

void ParticleManager::appendRenderBatches(
    const Slot& slot, std::vector<ParticleRenderBatch>& batches) const {
    const std::size_t active = slot.emitter->particles().size();
    const Asset& asset = *slot.asset;
    const auto& definition = *asset.definition;
    for (const GpuChunk& chunk : slot.gpuChunks) {
        if (chunk.firstRecord >= active) break;

        ParticleRenderBatch batch{};
        batch.instanceBuffer = chunk.buffer;
        batch.instanceCount =
            std::min(chunk.capacity, active - chunk.firstRecord);
        if (const auto* texture = textures_->resolve(asset.baseTexture))
            batch.baseTexture = texture->view;
        if (const auto* texture = textures_->resolve(asset.normalTexture))
            batch.normalTexture = texture->view;
        if (const auto* texture = textures_->resolve(asset.emissiveTexture))
            batch.emissiveTexture = texture->view;
        batch.motionCurveTexture = asset.motionCurveView;
        batch.colorCurveTexture = asset.colorCurveView;
        batch.blendMode = definition.material.blendMode;
        batch.systemTime = slot.emitter->gpuTime();
        batch.gravity = definition.emitter.gravity;
        batch.baseSize = {1.0F, 1.0F};
        if (definition.emitter.simulationSpace ==
            particle::SimulationSpace::Local) {
            batch.localSpace = true;
            batch.localTransform = slot.emitter->transform();
            batch.baseSize = batch.localTransform.scale;
            batch.baseRotation = batch.localTransform.rotation;
            const Vec2 localGravity = inverseTransformVector(
                batch.localTransform,
                {definition.emitter.gravity.x,
                 definition.emitter.gravity.y});
            batch.gravity.x = localGravity.x;
            batch.gravity.y = localGravity.y;
        }
        batch.tint = definition.material.tint;
        batch.emissive = definition.material.emissive;
        batch.opacity = definition.material.opacity;
        batch.alphaCutoff = definition.material.alphaCutoff;
        batch.normalStrength =
            definition.material.normalTexture.empty() ? 0.0F : 1.0F;
        batch.roughness = definition.material.roughness;
        batch.metallic = definition.material.metallic;
        batches.push_back(batch);
    }
}

bool ParticleManager::buildRenderData(
    std::vector<ParticleRenderBatch>& batches,
    std::size_t& visibleParticleCount,
    std::string& error) {
    error.clear();
    batches.clear();
    visibleParticleCount = 0;
    uploadRecords_.clear();
    pendingCopies_.clear();

    std::vector<Slot*> visible;
    visible.reserve(slots_.size());
    std::size_t total = 0;
    for (Slot& slot : slots_) {
        if (!slot.occupied) continue;
        const std::size_t active = slot.emitter->particles().size();
        trimGpuChunks(slot, active);
        if (active == 0) {
            slot.emitter->clearParticleRecordChanges();
            continue;
        }
        if (active > kMaximumResidentGpuParticles - total) {
            error = "active particles exceed the global GPU budget of " +
                    std::to_string(kMaximumResidentGpuParticles);
            return false;
        }
        visible.push_back(&slot);
        total += active;
    }
    // Premultiplied alpha remains order dependent.  Render far systems first;
    // additive systems are order independent but keeping one deterministic
    // ordering makes captures and editor previews reproducible.
    std::stable_sort(visible.begin(), visible.end(),
                     [](const Slot* lhs, const Slot* rhs) {
                         return lhs->emitter->origin().z >
                                rhs->emitter->origin().z;
                     });

    for (Slot* slot : visible) {
        if (!ensureGpuChunks(*slot, slot->emitter->particles().size(), error)) {
            batches.clear();
            return false;
        }

        const std::size_t active = slot->emitter->particles().size();
        rangeScratch_.clear();
        for (const particle::ParticleRecordRange& changed :
             slot->emitter->changedParticleRecordRanges()) {
            if (changed.count == 0 || changed.first >= active) continue;
            const std::size_t count =
                std::min(changed.count, active - changed.first);
            rangeScratch_.push_back({changed.first, count});
        }
        std::sort(rangeScratch_.begin(), rangeScratch_.end(),
                  [](const auto& lhs, const auto& rhs) {
                      return lhs.first < rhs.first;
                  });
        std::size_t merged = 0;
        for (const particle::ParticleRecordRange& range : rangeScratch_) {
            if (merged == 0) {
                rangeScratch_[merged++] = range;
                continue;
            }
            auto& previous = rangeScratch_[merged - 1U];
            const std::size_t previousEnd = previous.first + previous.count;
            const std::size_t rangeEnd = range.first + range.count;
            if (range.first <= previousEnd) {
                previous.count = std::max(previousEnd, rangeEnd) -
                                 previous.first;
            } else {
                rangeScratch_[merged++] = range;
            }
        }
        rangeScratch_.resize(merged);
        for (const particle::ParticleRecordRange& range : rangeScratch_) {
            appendUploadRange(*slot, range.first, range.count);
        }
        appendRenderBatches(*slot, batches);
    }

    if (!ensureUploadStaging(uploadRecords_.size(), error)) {
        batches.clear();
        return false;
    }
    if (!uploadRecords_.empty()) {
        sg_write_buffer_desc write{};
        write.src.data = {uploadRecords_.data(),
                          uploadRecords_.size() *
                              sizeof(particle::GpuParticleRecord)};
        write.dst.buffer = uploadStaging_;
        sg_write_buffer_transient(&write);

        for (const PendingCopy& pending : pendingCopies_) {
            sg_copy_buffer_to_buffer_desc copy{};
            copy.src.buffer = uploadStaging_;
            copy.src.offset = pending.sourceFirst *
                              sizeof(particle::GpuParticleRecord);
            copy.dst.buffer = pending.destination;
            copy.dst.offset = pending.destinationFirst *
                              sizeof(particle::GpuParticleRecord);
            copy.size = pending.count *
                        sizeof(particle::GpuParticleRecord);
            sg_copy_buffer_to_buffer(&copy);
        }
    }

    for (Slot* slot : visible) {
        slot->emitter->clearParticleRecordChanges();
    }
    visibleParticleCount = total;
    return true;
}

std::shared_ptr<ParticleManager::Asset> ParticleManager::loadAsset(
    std::string_view directory, std::string& error) {
    error.clear();
    if (directory.empty()) {
        error = "particle-system directory is empty";
        return {};
    }
    const std::string canonicalDirectory = canonicalize(directory);
    const std::string cacheKey = lookupKey(canonicalDirectory);
    const auto found = assets_.find(cacheKey);
    if (found != assets_.end()) return found->second;

    auto definition = std::make_shared<particle::ParticleSystemDefinition>();
    const std::filesystem::path path =
        std::filesystem::u8path(canonicalDirectory);
    if (!particle::loadParticleSystem(path, *definition, error)) return {};

    auto asset = std::make_shared<Asset>();
    asset->canonicalDirectory = canonicalDirectory;
    asset->definition = definition;
    const auto loadTexture = [&](const std::string& relative, bool srgb,
                                 TextureHandle& output) -> bool {
        if (relative.empty()) return true;
        std::string portable = relative;
        std::replace(portable.begin(), portable.end(), '\\', '/');
        output = textures_->load(
            (path / std::filesystem::u8path(portable)).generic_u8string(),
            error, srgb);
        return output.valid();
    };
    if (!loadTexture(definition->material.baseTexture, true,
                     asset->baseTexture) ||
        !loadTexture(definition->material.normalTexture, false,
                     asset->normalTexture) ||
        !loadTexture(definition->material.emissiveTexture, true,
                     asset->emissiveTexture) ||
        !createCurveTextures(*asset, error)) {
        destroyAsset(*asset);
        return {};
    }
    assets_.emplace(cacheKey, asset);
    return asset;
}

bool ParticleManager::createCurveTextures(Asset& asset, std::string& error) {
    std::array<float, kCurveResolution * 4> motion{};
    std::array<float, kCurveResolution * 4> color{};
    for (int index = 0; index < kCurveResolution; ++index) {
        const float age = static_cast<float>(index) /
                          static_cast<float>(kCurveResolution - 1);
        const Vec2 size = asset.definition->emitter.size.sample(age);
        const float rotation = asset.definition->emitter.rotation.sample(age);
        const Color tint = asset.definition->emitter.color.sample(age);
        const std::size_t offset = static_cast<std::size_t>(index) * 4U;
        motion[offset + 0U] = size.x;
        motion[offset + 1U] = size.y;
        motion[offset + 2U] = rotation;
        motion[offset + 3U] = 0.0F;
        color[offset + 0U] = tint.r;
        color[offset + 1U] = tint.g;
        color[offset + 2U] = tint.b;
        color[offset + 3U] = tint.a;
    }
    if (!createCurveTexture(motion, "NewAOE particle motion curve",
                            asset.motionCurveImage, asset.motionCurveView,
                            error) ||
        !createCurveTexture(color, "NewAOE particle color curve",
                            asset.colorCurveImage, asset.colorCurveView,
                            error)) {
        return false;
    }
    return true;
}

void ParticleManager::destroyAsset(Asset& asset) noexcept {
    if (textures_ != nullptr) {
        if (asset.baseTexture.valid()) textures_->release(asset.baseTexture);
        if (asset.normalTexture.valid()) textures_->release(asset.normalTexture);
        if (asset.emissiveTexture.valid())
            textures_->release(asset.emissiveTexture);
    }
    if (valid(asset.motionCurveView)) sg_destroy_view(asset.motionCurveView);
    if (valid(asset.motionCurveImage)) sg_destroy_image(asset.motionCurveImage);
    if (valid(asset.colorCurveView)) sg_destroy_view(asset.colorCurveView);
    if (valid(asset.colorCurveImage)) sg_destroy_image(asset.colorCurveImage);
    asset = {};
}

ParticleManager::Slot* ParticleManager::resolve(
    ParticleSystemHandle handle) noexcept {
    if (!handle.valid() || handle.index >= slots_.size()) return nullptr;
    Slot& slot = slots_[handle.index];
    return slot.occupied && slot.generation == handle.generation ? &slot
                                                                 : nullptr;
}

const ParticleManager::Slot* ParticleManager::resolve(
    ParticleSystemHandle handle) const noexcept {
    if (!handle.valid() || handle.index >= slots_.size()) return nullptr;
    const Slot& slot = slots_[handle.index];
    return slot.occupied && slot.generation == handle.generation ? &slot
                                                                 : nullptr;
}

std::string ParticleManager::canonicalize(std::string_view directory) {
    std::filesystem::path value =
        std::filesystem::u8path(std::string(directory));
    std::error_code error;
    value = std::filesystem::absolute(value, error);
    if (error) {
        error.clear();
        value = std::filesystem::u8path(std::string(directory));
    }
    const std::filesystem::path canonical =
        std::filesystem::weakly_canonical(value, error);
    if (!error) value = canonical;
    value = value.lexically_normal();
    return value.generic_u8string();
}

}  // namespace newaoe::render::detail
