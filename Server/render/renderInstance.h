#pragma once

#include <string>
#include <string_view>

#include "Server/render/game/engine/RenderEngine.h"

// WinUser.h exposes LoadImage as an A/W preprocessor macro.  The renderer's
// public API intentionally uses the engine-level name from the specification.
#if defined(LoadImage)
#undef LoadImage
#endif

struct sapp_event;
struct sapp_desc;

class RenderInstance {
public:
    RenderInstance() = default;
    ~RenderInstance();

    RenderInstance(const RenderInstance&) = delete;
    RenderInstance& operator=(const RenderInstance&) = delete;
    RenderInstance(RenderInstance&&) = delete;
    RenderInstance& operator=(RenderInstance&&) = delete;

    bool init();
    bool run();

    [[nodiscard]] newaoe::render::TextureHandle LoadImage(
        std::string_view path);
    [[nodiscard]] newaoe::render::TextureHandle LoadTexture(
        std::string_view path);
    [[nodiscard]] newaoe::render::TextureHandle LoadLinearImage(
        std::string_view path);
    bool ReleaseImage(newaoe::render::TextureHandle texture) noexcept;
    void DrawImage(newaoe::render::TextureHandle texture,
                   const newaoe::render::WorldRect& destination);
    void DrawImage(newaoe::render::TextureHandle texture,
                   const newaoe::render::WorldRect& destination,
                   const newaoe::render::SpriteMaterial& material);

    [[nodiscard]] newaoe::render::ParticleSystemHandle LoadParticleSystem(
        std::string_view directory);
    bool ReleaseParticleSystem(
        newaoe::render::ParticleSystemHandle system) noexcept;
    bool PlayParticleSystem(
        newaoe::render::ParticleSystemHandle system) noexcept;
    bool PauseParticleSystem(
        newaoe::render::ParticleSystemHandle system) noexcept;
    bool StopParticleSystem(
        newaoe::render::ParticleSystemHandle system) noexcept;
    bool SetParticleSystemPosition(
        newaoe::render::ParticleSystemHandle system,
        newaoe::render::Vec2 position) noexcept;
    bool SetParticleSystemTransform(
        newaoe::render::ParticleSystemHandle system,
        const newaoe::render::ParticleTransform2D& transform) noexcept;

    [[nodiscard]] newaoe::render::Camera2D& Camera() noexcept;
    [[nodiscard]] const newaoe::render::Camera2D& Camera() const noexcept;
    void SetLight(const newaoe::render::DirectionalLight& light) noexcept;
    void SetPostProcess(
        const newaoe::render::PostProcessSettings& settings) noexcept;
    [[nodiscard]] const newaoe::render::RenderStats& Stats() const noexcept;
    [[nodiscard]] const newaoe::render::RenderCapabilities& Capabilities()
        const noexcept;
    [[nodiscard]] const std::string& LastError() const noexcept;

private:
    static void appInit(void* userData);
    static void appFrame(void* userData);
    static void appCleanup(void* userData);
    static void appEvent(const sapp_event* event, void* userData);

    void initializeGraphics();
    void renderFrame();
    void handleEvent(const sapp_event* event);
    void shutdown() noexcept;

    bool configured_ = false;
    bool appRunning_ = false;
    bool sokolReady_ = false;
    bool imguiReady_ = false;
    newaoe::render::RenderEngine engine_;
};
