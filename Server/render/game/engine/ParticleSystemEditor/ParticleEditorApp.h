#pragma once

#include <array>
#include <cstddef>
#include <string>

#include "Server/render/game/engine/ParticleSystemEditor/ParticlePreview.h"
#include "Server/render/game/engine/ParticleSystemEditor/PreviewTexture.h"
#include "Server/render/game/engine/particle/ParticleSystem.h"

namespace newaoe::render::editor {

class ParticleEditorApp final {
public:
    ParticleEditorApp();

    void frame(float deltaSeconds);

private:
    void drawToolbar();
    void drawProperties();
    void drawMaterial();
    void drawEmitter();
    void drawMotion();
    void drawCurves();
    void drawBursts();
    void drawPreview(float deltaSeconds);
    void drawStatusBar();

    void newDocument();
    void openDocument();
    void saveDocument();
    void reloadMaterialTextures();
    void markChanged() noexcept;

    particle::ParticleSystemDefinition definition_{};
    ParticlePreview preview_{};
    PreviewTexture baseTexture_{};
    PreviewTexture normalTexture_{};
    PreviewTexture emissiveTexture_{};
    std::array<char, 1024> directory_{};
    std::string status_;
    bool dirty_ = false;
    bool previewNeedsSync_ = true;
};

}  // namespace newaoe::render::editor

