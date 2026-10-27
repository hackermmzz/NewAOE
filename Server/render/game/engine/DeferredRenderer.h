#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Server/render/game/engine/RenderTypes.h"
#include "Server/render/game/engine/SpriteBatcher.h"
#include "Server/render/game/engine/particle/ParticleSystem.h"
#include "sokol_gfx.h"

namespace newaoe::render {
struct RenderStats;
}

namespace newaoe::render::detail {

struct ParticleRenderBatch final {
    sg_buffer instanceBuffer{};
    std::size_t instanceCount = 0;
    sg_view baseTexture{};
    sg_view normalTexture{};
    sg_view emissiveTexture{};
    sg_view motionCurveTexture{};
    sg_view colorCurveTexture{};
    ParticleBlendMode blendMode = ParticleBlendMode::PremultipliedAlpha;
    float systemTime = 0.0F;
    Vec3 gravity{};
    Vec2 baseSize{1.0F, 1.0F};
    float baseRotation = 0.0F;
    ParticleTransform2D localTransform{};
    bool localSpace = false;
    Color tint{};
    Color emissive{0.0F, 0.0F, 0.0F, 1.0F};
    float opacity = 1.0F;
    float alphaCutoff = 0.0F;
    float normalStrength = 1.0F;
    float roughness = 1.0F;
    float metallic = 0.0F;
};

class DeferredRenderer final {
public:
    bool initialize(std::string& error);
    void shutdown() noexcept;
    bool resize(int width, int height, std::string& error);
    [[nodiscard]] bool hdrSceneEnabled() const noexcept {
        return sceneFormat_ == SG_PIXELFORMAT_RGBA16F;
    }
    [[nodiscard]] bool hdrEmissiveEnabled() const noexcept {
        return emissiveFormat_ == SG_PIXELFORMAT_RGBA16F;
    }

    [[nodiscard]] bool render(
        const sg_swapchain& swapchain,
        const Matrix4& worldToClip,
        const DirectionalLight& light,
        const PostProcessSettings& postProcess,
        const TextureCache& textures,
        const SpriteBatcher& sprites,
        const std::vector<ParticleRenderBatch>& particleBatches,
        RenderStats& stats,
        std::string& error);

    // Internal target bundles are public only so translation-unit helpers can
    // construct and destroy them; DeferredRenderer itself is not public API.
    struct ColorTarget final {
        sg_image image{};
        sg_view attachment{};
        sg_view texture{};
    };

    struct DepthTarget final {
        sg_image image{};
        sg_view attachment{};
    };

private:

    bool createPipelines(std::string& error);
    bool createTargets(int width, int height, std::string& error);
    bool ensureSpriteCapacity(std::size_t count, std::string& error);
    void destroyTargets() noexcept;
    void destroyPipelines() noexcept;

    void drawGBuffer(const Matrix4& worldToClip,
                     const TextureCache& textures,
                     const SpriteBatcher& sprites,
                     RenderStats& stats);
    void drawLighting(const DirectionalLight& light);
    void drawParticles(
        const Matrix4& worldToClip,
        const DirectionalLight& light,
        const TextureCache& textures,
        const std::vector<ParticleRenderBatch>& batches,
        RenderStats& stats);
    void drawBloom(const PostProcessSettings& settings);
    void drawPostProcess(const sg_swapchain& swapchain,
                         const TextureCache& textures,
                         const PostProcessSettings& settings);

    int width_ = 0;
    int height_ = 0;
    sg_pixel_format sceneFormat_ = SG_PIXELFORMAT_RGBA8;
    sg_pixel_format emissiveFormat_ = SG_PIXELFORMAT_RGBA8;

    sg_buffer quadBuffer_{};
    sg_buffer spriteInstanceBuffer_{};
    std::size_t spriteCapacity_ = 0;
    sg_sampler linearSampler_{};
    sg_sampler curveSampler_{};

    sg_shader spriteShader_{};
    sg_shader lightingShader_{};
    sg_shader particleShader_{};
    sg_shader bloomExtractShader_{};
    sg_shader bloomBlurShader_{};
    sg_shader postShader_{};
    sg_pipeline spritePipeline_{};
    sg_pipeline lightingPipeline_{};
    sg_pipeline particleAlphaPipeline_{};
    sg_pipeline particleAdditivePipeline_{};
    sg_pipeline bloomExtractPipeline_{};
    sg_pipeline bloomBlurPipeline_{};
    sg_pipeline postPipeline_{};

    ColorTarget gAlbedo_{};
    ColorTarget gNormal_{};
    ColorTarget gEmissive_{};
    DepthTarget depth_{};
    ColorTarget scene_{};
    ColorTarget bloomA_{};
    ColorTarget bloomB_{};
};

}  // namespace newaoe::render::detail
