#include "Server/render/game/engine/RenderEngine.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "Server/render/game/engine/DeferredRenderer.h"
#include "Server/render/game/engine/ParticleManager.h"
#include "Server/render/game/engine/SpriteBatcher.h"
#include "Server/render/game/engine/TextureCache.h"
#include "sokol_gfx.h"

namespace newaoe::render {

class RenderEngine::Impl final {
public:
    ~Impl() { shutdown(); }

    bool initialize() {
        if (initialized) return true;
        lastError.clear();
        if (!sg_isvalid()) {
            lastError = "RenderEngine::Initialize requires a valid sg_setup";
            return false;
        }
        if (!textures.initialize(lastError) ||
            !particles.initialize(textures, lastError) ||
            !renderer.initialize(lastError)) {
            shutdown();
            return false;
        }
        capabilities.hdrScene = renderer.hdrSceneEnabled();
        capabilities.hdrEmissiveGBuffer = renderer.hdrEmissiveEnabled();
        initialized = true;
        return true;
    }

    void shutdown() noexcept {
        if (sg_isvalid()) {
            particles.shutdown();
            renderer.shutdown();
            textures.shutdown();
        }
        sprites.clear();
        particleBatches.clear();
        initialized = false;
        width = 0;
        height = 0;
        frameReady = false;
        capabilities = {};
    }

    bool initialized = false;
    bool frameReady = false;
    int width = 0;
    int height = 0;
    detail::TextureCache textures;
    detail::ParticleManager particles;
    detail::DeferredRenderer renderer;
    detail::SpriteBatcher sprites;
    std::vector<detail::ParticleRenderBatch> particleBatches;
    Camera2D camera;
    DirectionalLight light;
    PostProcessSettings postProcess;
    RenderStats stats;
    RenderCapabilities capabilities;
    std::string lastError;
};

RenderEngine::RenderEngine() : impl_(std::make_unique<Impl>()) {}

RenderEngine::~RenderEngine() { Shutdown(); }

RenderEngine::RenderEngine(RenderEngine&&) noexcept = default;
RenderEngine& RenderEngine::operator=(RenderEngine&&) noexcept = default;

bool RenderEngine::Initialize() { return impl_->initialize(); }

void RenderEngine::Shutdown() noexcept {
    if (impl_) impl_->shutdown();
}

bool RenderEngine::IsInitialized() const noexcept {
    return impl_ && impl_->initialized;
}

void RenderEngine::BeginFrame(float deltaSeconds, int framebufferWidth,
                              int framebufferHeight) {
    if (!impl_->initialized) return;
    impl_->frameReady = false;
    impl_->width = std::max(framebufferWidth, 1);
    impl_->height = std::max(framebufferHeight, 1);
    impl_->camera.setDefaultPosition(
        {static_cast<float>(impl_->width) * 0.5F,
         static_cast<float>(impl_->height) * 0.5F});
    if (!impl_->renderer.resize(impl_->width, impl_->height,
                                impl_->lastError)) {
        return;
    }
    impl_->frameReady = true;
    // Avoid a debugger pause emitting millions of particles on the next frame.
    const float safeDelta = std::clamp(deltaSeconds, 0.0F, 0.25F);
    impl_->particles.update(safeDelta);
}

void RenderEngine::Render(const sg_swapchain& swapchain) {
    if (!impl_->initialized) return;
    if (!impl_->frameReady || impl_->width <= 0 || impl_->height <= 0) {
        // Submissions belong to one frame even when render-target allocation
        // failed.  Drop them without replacing the more useful resize error.
        impl_->sprites.clear();
        impl_->stats = {};
        return;
    }
    impl_->stats = {};
    impl_->stats.submittedSprites = impl_->sprites.submittedCount();
    impl_->sprites.build(impl_->textures);
    std::string particleError;
    const bool particlesReady = impl_->particles.buildRenderData(
        impl_->particleBatches, impl_->stats.visibleParticles,
        particleError);
    if (!particlesReady) {
        impl_->lastError = std::move(particleError);
    }
    const Matrix4 matrix = impl_->camera.viewProjection(
        static_cast<float>(impl_->width), static_cast<float>(impl_->height));
    std::string renderError;
    if (!impl_->renderer.render(
            swapchain, matrix, impl_->light, impl_->postProcess,
            impl_->textures, impl_->sprites, impl_->particleBatches,
            impl_->stats, renderError)) {
        impl_->lastError = std::move(renderError);
    } else if (particlesReady) {
        impl_->lastError.clear();
    }
    impl_->sprites.clear();
}

TextureHandle RenderEngine::LoadImage(std::string_view path) {
    return LoadTexture(path);
}

TextureHandle RenderEngine::LoadTexture(std::string_view path) {
    if (!impl_->initialized) {
        impl_->lastError = "LoadImage/LoadTexture called before "
                           "RenderEngine::Initialize";
        return {};
    }
    return impl_->textures.load(path, impl_->lastError, true);
}

TextureHandle RenderEngine::LoadLinearImage(std::string_view path) {
    if (!impl_->initialized) {
        impl_->lastError =
            "LoadLinearImage called before RenderEngine::Initialize";
        return {};
    }
    return impl_->textures.load(path, impl_->lastError, false);
}

bool RenderEngine::ReleaseImage(TextureHandle texture) noexcept {
    return impl_->initialized && impl_->textures.release(texture);
}

void RenderEngine::DrawImage(TextureHandle texture,
                             const WorldRect& destination) {
    DrawImage(texture, destination, SpriteMaterial{});
}

void RenderEngine::DrawImage(TextureHandle texture,
                             const WorldRect& destination,
                             const SpriteMaterial& material) {
    if (!impl_->initialized) return;
    impl_->sprites.submit(texture, destination, material);
}

ParticleSystemHandle RenderEngine::LoadParticleSystem(
    std::string_view directory) {
    if (!impl_->initialized) {
        impl_->lastError =
            "LoadParticleSystem called before RenderEngine::Initialize";
        return {};
    }
    return impl_->particles.load(directory, impl_->lastError);
}

bool RenderEngine::ReleaseParticleSystem(
    ParticleSystemHandle system) noexcept {
    return impl_->initialized && impl_->particles.release(system);
}

bool RenderEngine::PlayParticleSystem(ParticleSystemHandle system) noexcept {
    return impl_->initialized && impl_->particles.play(system);
}

bool RenderEngine::PauseParticleSystem(ParticleSystemHandle system) noexcept {
    return impl_->initialized && impl_->particles.pause(system);
}

bool RenderEngine::StopParticleSystem(ParticleSystemHandle system) noexcept {
    return impl_->initialized && impl_->particles.stop(system);
}

bool RenderEngine::SetParticleSystemPosition(ParticleSystemHandle system,
                                             Vec2 position) noexcept {
    return impl_->initialized && impl_->particles.setPosition(system, position);
}

bool RenderEngine::SetParticleSystemTransform(
    ParticleSystemHandle system,
    const ParticleTransform2D& transform) noexcept {
    return impl_->initialized && impl_->particles.setTransform(system,
                                                               transform);
}

Camera2D& RenderEngine::Camera() noexcept { return impl_->camera; }

const Camera2D& RenderEngine::Camera() const noexcept { return impl_->camera; }

void RenderEngine::SetLight(const DirectionalLight& light) noexcept {
    impl_->light = light;
}

void RenderEngine::SetPostProcess(
    const PostProcessSettings& settings) noexcept {
    impl_->postProcess = settings;
}

const RenderStats& RenderEngine::Stats() const noexcept { return impl_->stats; }

const RenderCapabilities& RenderEngine::Capabilities() const noexcept {
    return impl_->capabilities;
}

const std::string& RenderEngine::LastError() const noexcept {
    return impl_->lastError;
}

}  // namespace newaoe::render
