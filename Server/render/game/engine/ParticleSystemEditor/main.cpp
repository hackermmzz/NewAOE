#include "Server/render/game/engine/ParticleSystemEditor/ParticleEditorApp.h"
#include "Server/render/game/engine/ParticleSystemEditor/OffscreenCli.h"

#include <cstdio>
#include <cstring>
#include <memory>

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

namespace {

std::unique_ptr<newaoe::render::editor::ParticleEditorApp> editor;
bool graphicsReady = false;
bool imguiReady = false;

void initialize() {
    sg_desc graphicsDescriptor{};
    graphicsDescriptor.environment = sglue_environment();
    graphicsDescriptor.logger.func = slog_func;
    sg_setup(&graphicsDescriptor);
    if (!sg_isvalid()) {
        std::fputs("ParticleSystemEditor: Sokol initialization failed\n", stderr);
        sapp_request_quit();
        return;
    }
    graphicsReady = true;

    simgui_desc_t imguiDescriptor{};
    imguiDescriptor.logger.func = slog_func;
    simgui_setup(&imguiDescriptor);
    imguiReady = true;

    ImGui::GetIO().IniFilename = "particle_editor.ini";
    ImGui::StyleColorsDark();
    editor = std::make_unique<newaoe::render::editor::ParticleEditorApp>();
}

void frame() {
    if (!graphicsReady || !imguiReady || !editor) return;

    simgui_frame_desc_t frameDescriptor{};
    frameDescriptor.width = sapp_width();
    frameDescriptor.height = sapp_height();
    frameDescriptor.delta_time = sapp_frame_duration();
    frameDescriptor.dpi_scale = sapp_dpi_scale();
    simgui_new_frame(&frameDescriptor);

    editor->frame(static_cast<float>(sapp_frame_duration()));
    simgui_flush();

    sg_pass pass{};
    pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass.action.colors[0].store_action = SG_STOREACTION_STORE;
    pass.action.colors[0].clear_value = {0.025F, 0.028F, 0.035F, 1.0F};
    pass.swapchain = sglue_swapchain();
    sg_begin_pass(&pass);
    simgui_draw();
    sg_end_pass();
    sg_commit();
}

void event(const sapp_event* inputEvent) {
    if (imguiReady) simgui_handle_event(inputEvent);
}

void cleanup() {
    editor.reset();
    if (imguiReady) {
        simgui_shutdown();
        imguiReady = false;
    }
    if (graphicsReady) {
        sg_shutdown();
        graphicsReady = false;
    }
}

}  // namespace

sapp_desc makeGuiDescriptor() {
    sapp_desc descriptor{};
    descriptor.init_cb = initialize;
    descriptor.frame_cb = frame;
    descriptor.cleanup_cb = cleanup;
    descriptor.event_cb = event;
    descriptor.width = 1440;
    descriptor.height = 900;
    descriptor.sample_count = 1;
    descriptor.high_dpi = true;
    descriptor.window_title = "NewAOE Particle System Editor";
    descriptor.icon.sokol_default = true;
    descriptor.logger.func = slog_func;
    return descriptor;
}

int runApplication(int argc, char** argv) {
    for (int index = 1; index < argc; ++index) {
        if (argv[index] != nullptr &&
            std::strcmp(argv[index], "--offscreen") == 0) {
            return newaoe::render::editor::runOffscreenCli(argc, argv);
        }
    }

    const sapp_desc descriptor = makeGuiDescriptor();
    sapp_run(&descriptor);
    return 0;
}

int main(int argc, char** argv) {
#if defined(_WIN32)
    (void)argc;
    (void)argv;
    // Sokol's normal WinMain entry converts the UTF-16 Windows command line
    // to UTF-8 before calling sokol_main.  SOKOL_NO_ENTRY is required here so
    // --offscreen can return without ever entering the window loop, therefore
    // reuse that same conversion for our explicit main function.
    int utf8ArgumentCount = 0;
    char** utf8Arguments = _sapp_win32_command_line_to_utf8_argv(
        GetCommandLineW(), &utf8ArgumentCount);
    const int result = runApplication(utf8ArgumentCount, utf8Arguments);
    _sapp_free(utf8Arguments);
    return result;
#else
    return runApplication(argc, argv);
#endif
}

