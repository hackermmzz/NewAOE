#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

#include "Server/render/game/engine/Camera2D.h"
#include "Server/render/game/engine/RenderTypes.h"

// WinUser.h defines LoadImage as an A/W macro.  The engine API deliberately
// owns this unqualified name, so make the header safe regardless of include
// order for Windows consumers.
#if defined(LoadImage)
#undef LoadImage
#endif

struct sg_swapchain;

namespace newaoe::render {

struct RenderStats final {
    std::size_t submittedSprites = 0;
    std::size_t spriteDrawCalls = 0;
    std::size_t visibleParticles = 0;
    std::size_t particleDrawCalls = 0;
};

// Lightweight UE-style scene renderer.  This object is deliberately
// thread-affine: initialize, resource APIs, submission and rendering must all
// happen on the thread which owns Sokol's graphics context.
class RenderEngine final {
public:
    RenderEngine();
    ~RenderEngine();

    RenderEngine(const RenderEngine&) = delete;
    RenderEngine& operator=(const RenderEngine&) = delete;
    RenderEngine(RenderEngine&&) noexcept;
    RenderEngine& operator=(RenderEngine&&) noexcept;

    // sg_setup() must have succeeded before Initialize is called.
    [[nodiscard]] bool Initialize();
    void Shutdown() noexcept;
    [[nodiscard]] bool IsInitialized() const noexcept;

    // Advances engine time and performs resource-size maintenance.  Submitted
    // sprites are consumed and cleared by Render(), not by BeginFrame().
    void BeginFrame(float deltaSeconds, int framebufferWidth,
                    int framebufferHeight);
    void Render(const sg_swapchain& swapchain);

    [[nodiscard]] TextureHandle LoadImage(std::string_view path);
    // Macro-safe synonym for Windows consumers which include WinUser.h after
    // this header (WinUser.h may subsequently redefine the token LoadImage).
    [[nodiscard]] TextureHandle LoadTexture(std::string_view path);
    // For data textures such as normal maps. Color images should use
    // LoadImage/LoadTexture so hardware performs sRGB-to-linear sampling.
    [[nodiscard]] TextureHandle LoadLinearImage(std::string_view path);
    bool ReleaseImage(TextureHandle texture) noexcept;

    void DrawImage(TextureHandle texture, const WorldRect& destination);
    void DrawImage(TextureHandle texture, const WorldRect& destination,
                   const SpriteMaterial& material);

    // Each call returns a new playback instance.  Parsed configuration and
    // immutable textures are cached by canonical directory path.
    [[nodiscard]] ParticleSystemHandle LoadParticleSystem(
        std::string_view directory);
    bool ReleaseParticleSystem(ParticleSystemHandle system) noexcept;
    bool PlayParticleSystem(ParticleSystemHandle system) noexcept;
    bool PauseParticleSystem(ParticleSystemHandle system) noexcept;
    bool StopParticleSystem(ParticleSystemHandle system) noexcept;
    bool SetParticleSystemPosition(ParticleSystemHandle system,
                                   Vec2 position) noexcept;
    // Update before BeginFrame. World-space effects interpolate births from
    // the previously simulated transform to this value; local-space effects
    // render every live particle relative to this value.
    bool SetParticleSystemTransform(
        ParticleSystemHandle system,
        const ParticleTransform2D& transform) noexcept;

    [[nodiscard]] Camera2D& Camera() noexcept;
    [[nodiscard]] const Camera2D& Camera() const noexcept;
    void SetLight(const DirectionalLight& light) noexcept;
    void SetPostProcess(const PostProcessSettings& settings) noexcept;

    [[nodiscard]] const RenderStats& Stats() const noexcept;
    [[nodiscard]] const RenderCapabilities& Capabilities() const noexcept;
    [[nodiscard]] const std::string& LastError() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace newaoe::render
