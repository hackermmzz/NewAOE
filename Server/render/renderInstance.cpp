#include "renderInstance.h"

#include <SDL.h>

#define SOKOL_GFX_IMPL
#include "sokol_gfx.h"

#define SOKOL_LOG_IMPL
#include "sokol_log.h"

#ifndef AOE_GFX_GLES
#define AOE_GFX_GLES 0
#endif

#ifndef ENABLE_LOGGER
#define ENABLE_LOGGER 0
#endif

namespace {

constexpr int kInitialWindowWidth = 1280;
constexpr int kInitialWindowHeight = 720;

bool setGlAttribute(SDL_GLattr attribute, int value) {
    if (SDL_GL_SetAttribute(attribute, value) == 0) {
        return true;
    }
    SDL_LogError(
        SDL_LOG_CATEGORY_VIDEO,
        "SDL_GL_SetAttribute(%d, %d) failed: %s",
        static_cast<int>(attribute),
        value,
        SDL_GetError()
    );
    return false;
}

}  // namespace

RenderInstance::~RenderInstance() {
    shutdown();
}

bool RenderInstance::init() {
    if (sokolReady_) {
        return true;
    }

    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL_Init failed: %s",
            SDL_GetError()
        );
        return false;
    }
    sdlReady_ = true;

    SDL_GL_ResetAttributes();

#if AOE_GFX_GLES
    const bool profileReady =
        setGlAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_ES
        ) &&
        setGlAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) &&
        setGlAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    const bool profileReady =
        setGlAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE
        ) &&
        setGlAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4) &&
        setGlAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

    const bool framebufferReady =
        setGlAttribute(SDL_GL_DOUBLEBUFFER, 1) &&
        setGlAttribute(SDL_GL_RED_SIZE, 8) &&
        setGlAttribute(SDL_GL_GREEN_SIZE, 8) &&
        setGlAttribute(SDL_GL_BLUE_SIZE, 8) &&
        setGlAttribute(SDL_GL_ALPHA_SIZE, 8) &&
        setGlAttribute(SDL_GL_DEPTH_SIZE, 24) &&
        setGlAttribute(SDL_GL_STENCIL_SIZE, 8);

    if (!profileReady || !framebufferReady) {
        shutdown();
        return false;
    }

    window_ = SDL_CreateWindow(
        "NewAOE",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kInitialWindowWidth,
        kInitialWindowHeight,
        SDL_WINDOW_OPENGL |
            SDL_WINDOW_SHOWN |
            SDL_WINDOW_RESIZABLE |
            SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (window_ == nullptr) {
        SDL_LogError(
            SDL_LOG_CATEGORY_VIDEO,
            "SDL_CreateWindow failed: %s",
            SDL_GetError()
        );
        shutdown();
        return false;
    }

    glContext_ = SDL_GL_CreateContext(window_);
    if (glContext_ == nullptr) {
        SDL_LogError(
            SDL_LOG_CATEGORY_VIDEO,
            "SDL_GL_CreateContext failed: %s",
            SDL_GetError()
        );
        shutdown();
        return false;
    }

    if (SDL_GL_MakeCurrent(window_, glContext_) != 0) {
        SDL_LogError(
            SDL_LOG_CATEGORY_VIDEO,
            "SDL_GL_MakeCurrent failed: %s",
            SDL_GetError()
        );
        shutdown();
        return false;
    }

#ifndef __EMSCRIPTEN__
    if (SDL_GL_SetSwapInterval(1) != 0) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_VIDEO,
            "VSync is unavailable: %s",
            SDL_GetError()
        );
    }
#endif

    sg_desc descriptor{};
    descriptor.environment.defaults.color_format = SG_PIXELFORMAT_RGBA8;
    descriptor.environment.defaults.depth_format =
        SG_PIXELFORMAT_DEPTH_STENCIL;
    descriptor.environment.defaults.sample_count = 1;
#if ENABLE_LOGGER
    descriptor.logger.func = slog_func;
#endif

    sg_setup(&descriptor);
    if (!sg_isvalid()) {
        SDL_LogError(
            SDL_LOG_CATEGORY_RENDER,
            "sokol_gfx initialization failed"
        );
        shutdown();
        return false;
    }

    sokolReady_ = true;
    return true;
}

bool RenderInstance::run() {
    if (!sokolReady_) {
        return false;
    }

    SDL_Event event{};
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }

    int drawableWidth = 0;
    int drawableHeight = 0;
    SDL_GL_GetDrawableSize(window_, &drawableWidth, &drawableHeight);
    if (drawableWidth <= 0 || drawableHeight <= 0) {
        return true;
    }

    sg_pass pass{};
    pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass.action.colors[0].store_action = SG_STOREACTION_STORE;
    pass.action.colors[0].clear_value.r = 0.03F;
    pass.action.colors[0].clear_value.g = 0.05F;
    pass.action.colors[0].clear_value.b = 0.05F;
    pass.action.colors[0].clear_value.a = 1.0F;
    pass.swapchain.width = drawableWidth;
    pass.swapchain.height = drawableHeight;
    pass.swapchain.sample_count = 1;
    pass.swapchain.color_format = SG_PIXELFORMAT_RGBA8;
    pass.swapchain.depth_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    pass.swapchain.gl.framebuffer = 0;

    sg_begin_pass(&pass);
    sg_end_pass();
    sg_commit();
    SDL_GL_SwapWindow(window_);
    return true;
}

void RenderInstance::shutdown() noexcept {
    if (sokolReady_) {
        sg_shutdown();
        sokolReady_ = false;
    }
    if (glContext_ != nullptr) {
        SDL_GL_DeleteContext(glContext_);
        glContext_ = nullptr;
    }
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    if (sdlReady_) {
        SDL_Quit();
        sdlReady_ = false;
    }
}
