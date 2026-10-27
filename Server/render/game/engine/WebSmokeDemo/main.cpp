#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#define SOKOL_APP_IMPL
#include "sokol_app.h"

#define SOKOL_GFX_IMPL
#include "sokol_gfx.h"

#define SOKOL_GLUE_IMPL
#include "sokol_glue.h"

#define SOKOL_LOG_IMPL
#include "sokol_log.h"

#include "Server/render/game/engine/RenderEngine.h"

namespace {

constexpr int kInitialWidth = 1280;
constexpr int kInitialHeight = 720;
constexpr const char* kSmokeAssetDirectory = "assets/smoke";
constexpr const char* kGrenadeTexture = "assets/smoke/grenade_bright.png";
// Keep the low-resolution icon readable on high-DPI browser canvases.
constexpr float kGrenadeSize = 96.0F;

struct SmokeDemoState final {
    newaoe::render::RenderEngine engine;
    newaoe::render::ParticleSystemHandle smoke{};
    newaoe::render::TextureHandle grenade{};
    newaoe::render::ParticleTransform2D transform{};
    bool graphicsReady = false;
    bool paused = false;
    double time = 0.0;
    std::string lastReportedError;
};

SmokeDemoState state;

const char* backendLabel() noexcept {
#if defined(SOKOL_D3D11)
    return "D3D11 / HLSL 5";
#elif defined(SOKOL_METAL)
    return "Metal";
#elif defined(SOKOL_GLES3)
    return "WebGL2 / GLSL ES 3";
#elif defined(SOKOL_GLCORE)
    return "OpenGL Core";
#else
    return "Sokol";
#endif
}

void fail(const char* message) {
    std::fprintf(stderr, "SokolSmokeDemo: %s\n", message);
    sapp_request_quit();
}

newaoe::render::ParticleTransform2D trajectory(double seconds,
                                                int width,
                                                int height) noexcept {
    const float t = static_cast<float>(seconds);
    const float phase = t * 0.72F;
    const float sine = std::sin(phase);
    const float cosine = std::cos(phase);
    const float doubleSine = std::sin(phase * 2.0F);
    const float doubleCosine = std::cos(phase * 2.0F);

    newaoe::render::ParticleTransform2D result{};
    result.position = {
        static_cast<float>(width) * (0.50F + 0.34F * sine),
        static_cast<float>(height) * (0.58F - 0.23F * doubleSine),
    };

    // Tangent of the figure-eight path. Rotation affects the emitter shape
    // and local launch velocity while the grenade sprite intentionally stays
    // axis-aligned to match DrawImage's lightweight contract.
    const float dx = static_cast<float>(width) * 0.34F * 0.72F * cosine;
    const float dy = static_cast<float>(height) * -0.23F * 1.44F * doubleCosine;
    result.rotation = std::atan2(dy, dx);
    const float scalePulse = 1.0F + 0.16F * std::sin(phase * 1.7F);
    result.scale = {scalePulse, 0.90F + 0.12F * std::cos(phase * 1.3F)};
    return result;
}

void initialize() {
    sg_desc graphicsDescriptor{};
    graphicsDescriptor.environment = sglue_environment();
    graphicsDescriptor.buffer_pool_size = 8192;
    graphicsDescriptor.image_pool_size = 2048;
    graphicsDescriptor.sampler_pool_size = 256;
    graphicsDescriptor.shader_pool_size = 128;
    graphicsDescriptor.pipeline_pool_size = 256;
    graphicsDescriptor.view_pool_size = 4096;
    graphicsDescriptor.logger.func = slog_func;
    sg_setup(&graphicsDescriptor);
    if (!sg_isvalid()) {
        fail("sokol_gfx initialization failed");
        return;
    }
    state.graphicsReady = true;

    if (!state.engine.Initialize()) {
        fail(state.engine.LastError().c_str());
        return;
    }

    newaoe::render::PostProcessSettings postProcess{};
    postProcess.enabled = true;
    postProcess.fxaa = true;
    // Keep the small pixel-art projectile crisp; bloom would turn it into the
    // same soft highlight as the newborn smoke puffs.
    postProcess.bloom = false;
    postProcess.exposure = 1.0F;
    postProcess.bloomThreshold = 0.9F;
    postProcess.bloomIntensity = 0.08F;
    state.engine.SetPostProcess(postProcess);

    state.grenade = state.engine.LoadImage(kGrenadeTexture);
    if (!state.grenade.valid()) {
        fail(state.engine.LastError().c_str());
        return;
    }
    state.smoke = state.engine.LoadParticleSystem(kSmokeAssetDirectory);
    if (!state.smoke.valid()) {
        fail(state.engine.LastError().c_str());
        return;
    }

    // A time-zero burst is emitted by PlayParticleSystem. Install the first
    // transform before Play so it cannot spawn at the world origin.
    state.transform = trajectory(0.0, std::max(sapp_width(), 1),
                                 std::max(sapp_height(), 1));
    if (!state.engine.SetParticleSystemTransform(state.smoke,
                                                  state.transform) ||
        !state.engine.PlayParticleSystem(state.smoke)) {
        fail("could not initialize the smoke particle system");
        return;
    }

    std::printf("NewAOE smoke trail loaded from %s (%s)\n",
                kSmokeAssetDirectory, backendLabel());
    std::puts("World-space trail | Transform2D interpolation | Space pauses");
}

void frame() {
    if (!state.graphicsReady || !state.engine.IsInitialized()) return;

    const float deltaSeconds = std::clamp(
        static_cast<float>(sapp_frame_duration()), 0.0F, 0.1F);
    const int width = std::max(sapp_width(), 1);
    const int height = std::max(sapp_height(), 1);
    if (!state.paused) state.time += static_cast<double>(deltaSeconds);

    // Transform must precede BeginFrame: the particle update interpolates
    // from the previously simulated transform to this one for sub-frame births.
    state.transform = trajectory(state.time, width, height);
    state.engine.SetParticleSystemTransform(state.smoke, state.transform);
    state.engine.BeginFrame(deltaSeconds, width, height);

    newaoe::render::SpriteMaterial grenadeMaterial{};
    grenadeMaterial.tint = {1.0F, 1.0F, 1.0F, 1.0F};
    // Keep the projectile readable against the unlit deferred background.
    // Reusing the sprite as its emissive mask preserves the alpha-cutout
    // silhouette while making it independent from scene lighting.
    grenadeMaterial.emissiveTexture = state.grenade;
    grenadeMaterial.emissiveColor = {1.0F, 0.9F, 0.3F, 1.0F};
    grenadeMaterial.roughness = 0.8F;
    state.engine.DrawImage(
        state.grenade,
        {state.transform.position.x - kGrenadeSize * 0.5F,
         state.transform.position.y - kGrenadeSize * 0.5F,
         kGrenadeSize, kGrenadeSize},
        grenadeMaterial);
    state.engine.Render(sglue_swapchain());
    sg_commit();

    const std::string& error = state.engine.LastError();
    if (!error.empty() && error != state.lastReportedError) {
        std::fprintf(stderr, "SokolSmokeDemo renderer: %s\n", error.c_str());
        state.lastReportedError = error;
    }
}

void handleEvent(const sapp_event* event) {
    if (event->type != SAPP_EVENTTYPE_KEY_DOWN || event->key_repeat) return;

    switch (event->key_code) {
        case SAPP_KEYCODE_ESCAPE:
            sapp_request_quit();
            break;
        case SAPP_KEYCODE_SPACE:
            state.paused = !state.paused;
            if (state.paused) {
                state.engine.PauseParticleSystem(state.smoke);
            } else {
                state.engine.PlayParticleSystem(state.smoke);
            }
            break;
        case SAPP_KEYCODE_R:
            state.engine.StopParticleSystem(state.smoke);
            state.transform = trajectory(state.time, std::max(sapp_width(), 1),
                                         std::max(sapp_height(), 1));
            state.engine.SetParticleSystemTransform(state.smoke,
                                                     state.transform);
            state.engine.PlayParticleSystem(state.smoke);
            state.paused = false;
            break;
        default:
            break;
    }
}

void cleanup() {
    if (!state.graphicsReady) return;
    if (state.smoke.valid()) {
        state.engine.ReleaseParticleSystem(state.smoke);
        state.smoke = {};
    }
    if (state.grenade.valid()) {
        state.engine.ReleaseImage(state.grenade);
        state.grenade = {};
    }
    state.engine.Shutdown();
    sg_shutdown();
    state.graphicsReady = false;
}

}  // namespace

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    sapp_desc descriptor{};
    descriptor.init_cb = initialize;
    descriptor.frame_cb = frame;
    descriptor.cleanup_cb = cleanup;
    descriptor.event_cb = handleEvent;
    descriptor.width = kInitialWidth;
    descriptor.height = kInitialHeight;
    descriptor.depth_format = SAPP_PIXELFORMAT_DEPTH_STENCIL;
    descriptor.sample_count = 1;
    descriptor.high_dpi = true;
    descriptor.window_title = "NewAOE - Smoke Grenade Transform2D Demo";
    descriptor.logger.func = slog_func;
    return descriptor;
}

