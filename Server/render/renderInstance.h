#pragma once

struct SDL_Window;

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

private:
    void shutdown() noexcept;

    SDL_Window* window_ = nullptr;
    void* glContext_ = nullptr;
    bool sdlReady_ = false;
    bool sokolReady_ = false;
};
