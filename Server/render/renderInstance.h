#pragma once

struct sapp_event;

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
};
