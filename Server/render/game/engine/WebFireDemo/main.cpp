#include <algorithm>
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
constexpr const char* kFireAssetDirectory = "assets/fire";

struct FireDemoState final {
    newaoe::render::RenderEngine engine;
    newaoe::render::ParticleSystemHandle fire{};
    bool graphicsReady = false;
    bool paused = false;
    std::string lastReportedError;
};

FireDemoState state;

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
    std::fprintf(stderr, "SokolFireDemo: %s\n", message);
    sapp_request_quit();
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
    postProcess.bloom = true;
    postProcess.exposure = 1.12F;
    postProcess.bloomThreshold = 0.28F;
    postProcess.bloomIntensity = 0.72F;
    state.engine.SetPostProcess(postProcess);

    state.fire = state.engine.LoadParticleSystem(kFireAssetDirectory);
    if (!state.fire.valid()) {
        fail(state.engine.LastError().c_str());
        return;
    }
    if (!state.engine.PlayParticleSystem(state.fire)) {
        fail("could not start the fire particle system");
        return;
    }

    std::printf("NewAOE fire particle loaded from %s (%s)\n",
                kFireAssetDirectory, backendLabel());
    std::puts("Space pauses/resumes; R restarts; Escape closes the demo.");
}

void frame() {
    if (!state.graphicsReady || !state.engine.IsInitialized()) return;

    const float deltaSeconds = std::clamp(
        static_cast<float>(sapp_frame_duration()), 0.0F, 0.1F);
    const int width = std::max(sapp_width(), 1);
    const int height = std::max(sapp_height(), 1);

    state.engine.BeginFrame(deltaSeconds, width, height);
    state.engine.SetParticleSystemPosition(
        state.fire,
        {static_cast<float>(width) * 0.5F,
         static_cast<float>(height) * 0.76F});
    state.engine.Render(sglue_swapchain());
    sg_commit();

    const std::string& error = state.engine.LastError();
    if (!error.empty() && error != state.lastReportedError) {
        std::fprintf(stderr, "SokolFireDemo renderer: %s\n", error.c_str());
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
                state.engine.PauseParticleSystem(state.fire);
            } else {
                state.engine.PlayParticleSystem(state.fire);
            }
            break;
        case SAPP_KEYCODE_R:
            state.engine.StopParticleSystem(state.fire);
            state.engine.PlayParticleSystem(state.fire);
            state.paused = false;
            break;
        default:
            break;
    }
}

void cleanup() {
    if (!state.graphicsReady) return;
    if (state.fire.valid()) {
        state.engine.ReleaseParticleSystem(state.fire);
        state.fire = {};
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
    descriptor.window_title = "NewAOE - Fire Particle Web Demo";
    descriptor.logger.func = slog_func;
    return descriptor;
}
