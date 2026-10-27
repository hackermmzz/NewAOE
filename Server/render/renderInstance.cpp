#include "renderInstance.h"

#include <cstdio>

#include "imgui.h"

#define SOKOL_NO_ENTRY

#define SOKOL_APP_IMPL
#include "sokol_app.h"

#define SOKOL_GFX_IMPL
#include "sokol_gfx.h"

#define SOKOL_GLUE_IMPL
#include "sokol_glue.h"

#define SOKOL_LOG_IMPL
#include "sokol_log.h"

#define SOKOL_IMGUI_IMPL
#include "util/sokol_imgui.h"

#if defined(LoadImage)
#undef LoadImage
#endif

#ifndef ENABLE_LOGGER
#define ENABLE_LOGGER 0
#endif

namespace {

constexpr int kInitialWindowWidth = 1280;
constexpr int kInitialWindowHeight = 720;

}  // namespace

RenderInstance::~RenderInstance() {
    shutdown();
}

bool RenderInstance::init() {
    configured_ = true;
    return true;
}

bool RenderInstance::run() {
    if (!configured_ || appRunning_) {
        return false;
    }

    appRunning_ = true;

    sapp_desc descriptor{};
    descriptor.user_data = this;
    descriptor.init_userdata_cb = &RenderInstance::appInit;
    descriptor.frame_userdata_cb = &RenderInstance::appFrame;
    descriptor.cleanup_userdata_cb = &RenderInstance::appCleanup;
    descriptor.event_userdata_cb = &RenderInstance::appEvent;
    descriptor.width = kInitialWindowWidth;
    descriptor.height = kInitialWindowHeight;
    descriptor.depth_format = SAPP_PIXELFORMAT_DEPTH_STENCIL;
    descriptor.sample_count = 1;
    descriptor.high_dpi = true;
    descriptor.window_title = "NewAOE";
#if ENABLE_LOGGER
    descriptor.logger.func = slog_func;
#endif

    sapp_run(&descriptor);

#ifdef __EMSCRIPTEN__
    // sapp_run() returns immediately on the web while Sokol keeps driving its
    // own browser main loop.
    return true;
#else
    // On desktop sapp_run() returns only after the window has closed.
    return false;
#endif
}

newaoe::render::TextureHandle RenderInstance::LoadImage(
    std::string_view path) {
    return LoadTexture(path);
}

newaoe::render::TextureHandle RenderInstance::LoadTexture(
    std::string_view path) {
    return engine_.LoadTexture(path);
}

newaoe::render::TextureHandle RenderInstance::LoadLinearImage(
    std::string_view path) {
    return engine_.LoadLinearImage(path);
}

bool RenderInstance::ReleaseImage(
    newaoe::render::TextureHandle texture) noexcept {
    return engine_.ReleaseImage(texture);
}

void RenderInstance::DrawImage(
    newaoe::render::TextureHandle texture,
    const newaoe::render::WorldRect& destination) {
    engine_.DrawImage(texture, destination);
}

void RenderInstance::DrawImage(
    newaoe::render::TextureHandle texture,
    const newaoe::render::WorldRect& destination,
    const newaoe::render::SpriteMaterial& material) {
    engine_.DrawImage(texture, destination, material);
}

newaoe::render::ParticleSystemHandle RenderInstance::LoadParticleSystem(
    std::string_view directory) {
    return engine_.LoadParticleSystem(directory);
}

bool RenderInstance::ReleaseParticleSystem(
    newaoe::render::ParticleSystemHandle system) noexcept {
    return engine_.ReleaseParticleSystem(system);
}

bool RenderInstance::PlayParticleSystem(
    newaoe::render::ParticleSystemHandle system) noexcept {
    return engine_.PlayParticleSystem(system);
}

bool RenderInstance::PauseParticleSystem(
    newaoe::render::ParticleSystemHandle system) noexcept {
    return engine_.PauseParticleSystem(system);
}

bool RenderInstance::StopParticleSystem(
    newaoe::render::ParticleSystemHandle system) noexcept {
    return engine_.StopParticleSystem(system);
}

bool RenderInstance::SetParticleSystemPosition(
    newaoe::render::ParticleSystemHandle system,
    newaoe::render::Vec2 position) noexcept {
    return engine_.SetParticleSystemPosition(system, position);
}

bool RenderInstance::SetParticleSystemTransform(
    newaoe::render::ParticleSystemHandle system,
    const newaoe::render::ParticleTransform2D& transform) noexcept {
    return engine_.SetParticleSystemTransform(system, transform);
}

newaoe::render::Camera2D& RenderInstance::Camera() noexcept {
    return engine_.Camera();
}

const newaoe::render::Camera2D& RenderInstance::Camera() const noexcept {
    return engine_.Camera();
}

void RenderInstance::SetLight(
    const newaoe::render::DirectionalLight& light) noexcept {
    engine_.SetLight(light);
}

void RenderInstance::SetPostProcess(
    const newaoe::render::PostProcessSettings& settings) noexcept {
    engine_.SetPostProcess(settings);
}

const newaoe::render::RenderStats& RenderInstance::Stats() const noexcept {
    return engine_.Stats();
}

const newaoe::render::RenderCapabilities& RenderInstance::Capabilities()
    const noexcept {
    return engine_.Capabilities();
}

const std::string& RenderInstance::LastError() const noexcept {
    return engine_.LastError();
}

void RenderInstance::appInit(void* userData) {
    static_cast<RenderInstance*>(userData)->initializeGraphics();
}

void RenderInstance::appFrame(void* userData) {
    static_cast<RenderInstance*>(userData)->renderFrame();
}

void RenderInstance::appCleanup(void* userData) {
    static_cast<RenderInstance*>(userData)->shutdown();
}

void RenderInstance::appEvent(const sapp_event* event, void* userData) {
    static_cast<RenderInstance*>(userData)->handleEvent(event);
}

void RenderInstance::initializeGraphics() {
    sg_desc graphicsDescriptor{};
    graphicsDescriptor.environment = sglue_environment();
    // Particle assets own several texture/LUT views and playback instances
    // own persistent birth-record pages.  Sokol's small defaults are ideal
    // for samples but would cap a real effect library long before the
    // engine's explicit particle budgets are reached.
    graphicsDescriptor.buffer_pool_size = 8192;
    graphicsDescriptor.image_pool_size = 2048;
    graphicsDescriptor.sampler_pool_size = 256;
    graphicsDescriptor.shader_pool_size = 128;
    graphicsDescriptor.pipeline_pool_size = 256;
    graphicsDescriptor.view_pool_size = 4096;
#if ENABLE_LOGGER
    graphicsDescriptor.logger.func = slog_func;
#endif
    sg_setup(&graphicsDescriptor);
    if (!sg_isvalid()) {
        std::fputs("sokol_gfx initialization failed\n", stderr);
        sapp_request_quit();
        return;
    }
    sokolReady_ = true;

    if (!engine_.Initialize()) {
        std::fprintf(stderr, "render engine initialization failed: %s\n",
                     engine_.LastError().c_str());
        sapp_request_quit();
        return;
    }

    simgui_desc_t imguiDescriptor{};
#if ENABLE_LOGGER
    imguiDescriptor.logger.func = slog_func;
#endif
    simgui_setup(&imguiDescriptor);
    imguiReady_ = true;
}

void RenderInstance::renderFrame() {
    if (!sokolReady_ || !imguiReady_) {
        return;
    }

    engine_.BeginFrame(static_cast<float>(sapp_frame_duration()), sapp_width(),
                       sapp_height());

    simgui_frame_desc_t frameDescriptor{};
    frameDescriptor.width = sapp_width();
    frameDescriptor.height = sapp_height();
    frameDescriptor.delta_time = sapp_frame_duration();
    frameDescriptor.dpi_scale = sapp_dpi_scale();
    simgui_new_frame(&frameDescriptor);

    ImGui::SetNextWindowPos(ImVec2(16.0F, 16.0F), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.88F);
    ImGui::Begin("NewAOE");
    ImGui::TextUnformatted("Sokol + Dear ImGui renderer");
    ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
    const newaoe::render::RenderStats& stats = engine_.Stats();
    ImGui::Text("Sprites: %zu (%zu draw calls)", stats.submittedSprites,
                stats.spriteDrawCalls);
    ImGui::Text("Particles: %zu (%zu draw calls)", stats.visibleParticles,
                stats.particleDrawCalls);
    if (!engine_.LastError().empty()) {
        ImGui::TextWrapped("Renderer: %s", engine_.LastError().c_str());
    }
    ImGui::End();

    simgui_flush();

    const sg_swapchain swapchain = sglue_swapchain();
    engine_.Render(swapchain);

    sg_pass pass{};
    // Preserve the tone-mapped engine output and composite tooling on top.
    pass.action.colors[0].load_action = SG_LOADACTION_LOAD;
    pass.action.colors[0].store_action = SG_STOREACTION_STORE;
    pass.action.depth.load_action = SG_LOADACTION_DONTCARE;
    pass.action.depth.store_action = SG_STOREACTION_DONTCARE;
    pass.action.stencil.load_action = SG_LOADACTION_DONTCARE;
    pass.action.stencil.store_action = SG_STOREACTION_DONTCARE;
    pass.swapchain = swapchain;
    sg_begin_pass(&pass);
    simgui_draw();
    sg_end_pass();
    sg_commit();
}

void RenderInstance::handleEvent(const sapp_event* event) {
    if (imguiReady_) {
        simgui_handle_event(event);
    }
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN && !event->key_repeat &&
        event->key_code == SAPP_KEYCODE_ESCAPE) {
        sapp_request_quit();
    }
}

void RenderInstance::shutdown() noexcept {
    if (imguiReady_) {
        simgui_shutdown();
        imguiReady_ = false;
    }
    engine_.Shutdown();
    if (sokolReady_) {
        sg_shutdown();
        sokolReady_ = false;
    }
    appRunning_ = false;
}
