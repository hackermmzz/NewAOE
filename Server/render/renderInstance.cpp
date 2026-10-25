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
    ImGui::End();

    simgui_flush();

    sg_pass pass{};
    pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass.action.colors[0].store_action = SG_STOREACTION_STORE;
    pass.action.colors[0].clear_value = {0.03F, 0.05F, 0.05F, 1.0F};
    pass.swapchain = sglue_swapchain();
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
    if (sokolReady_) {
        sg_shutdown();
        sokolReady_ = false;
    }
    appRunning_ = false;
}
