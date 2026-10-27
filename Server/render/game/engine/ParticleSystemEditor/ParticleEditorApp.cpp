#include "Server/render/game/engine/ParticleSystemEditor/ParticleEditorApp.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <string>
#include <utility>

#include "imgui.h"

namespace newaoe::render::editor {
namespace {

constexpr float kPropertyPanelWidth = 520.0F;

bool inputString(const char* label, std::string& value) {
    std::array<char, 1024> buffer{};
    const std::size_t length = std::min(value.size(), buffer.size() - 1U);
    std::memcpy(buffer.data(), value.data(), length);
    if (!ImGui::InputText(label, buffer.data(), buffer.size())) return false;
    value.assign(buffer.data());
    return true;
}

bool distributionCombo(const char* label,
                       particle::RandomDistribution& distribution) {
    const char* options[] = {"Uniform", "Normal"};
    int selected =
        distribution == particle::RandomDistribution::Uniform ? 0 : 1;
    if (!ImGui::Combo(label, &selected, options, IM_ARRAYSIZE(options))) {
        return false;
    }
    distribution = selected == 0 ? particle::RandomDistribution::Uniform
                                 : particle::RandomDistribution::Normal;
    return true;
}

bool editFloatRange(const char* id,
                    const char* label,
                    particle::FloatRange& range,
                    float speed = 0.05F) {
    bool changed = false;
    ImGui::PushID(id);
    ImGui::TextUnformatted(label);
    ImGui::SetNextItemWidth(125.0F);
    changed |= ImGui::DragFloat("Minimum", &range.minimum, speed);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(125.0F);
    changed |= ImGui::DragFloat("Maximum", &range.maximum, speed);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0F);
    changed |= distributionCombo("Distribution", range.distribution);
    ImGui::PopID();
    return changed;
}

bool editVec3Range(const char* id,
                   const char* label,
                   particle::Vec3Range& range,
                   float speed = 0.1F) {
    bool changed = false;
    ImGui::PushID(id);
    ImGui::TextUnformatted(label);
    ImGui::SetNextItemWidth(300.0F);
    changed |= ImGui::DragFloat3("Minimum", &range.minimum.x, speed);
    ImGui::SetNextItemWidth(300.0F);
    changed |= ImGui::DragFloat3("Maximum", &range.maximum.x, speed);
    ImGui::SetNextItemWidth(150.0F);
    changed |= distributionCombo("Distribution", range.distribution);
    ImGui::PopID();
    return changed;
}

template <typename Curve>
void eraseCurveKey(Curve& curve, std::uint32_t index) {
    for (std::uint32_t i = index + 1; i < curve.count; ++i) {
        curve.keys[i - 1] = curve.keys[i];
    }
    if (curve.count > 0) --curve.count;
}

template <typename Curve>
void sortCurve(Curve& curve) {
    std::sort(curve.keys.begin(), curve.keys.begin() + curve.count,
              [](const auto& lhs, const auto& rhs) {
                  return lhs.time < rhs.time;
              });
}

bool editSizeCurve(particle::SizeCurve& curve) {
    bool changed = false;
    int remove = -1;
    if (ImGui::BeginTable("size-curve", 4,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed,
                                100.0F);
        ImGui::TableSetupColumn("Width");
        ImGui::TableSetupColumn("Height");
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30.0F);
        ImGui::TableHeadersRow();
        for (std::uint32_t i = 0; i < curve.count; ++i) {
            auto& key = curve.keys[i];
            ImGui::PushID(static_cast<int>(i));
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::SetNextItemWidth(-1.0F);
            changed |= ImGui::DragFloat("##time", &key.time, 0.01F, 0.0F,
                                        1.0F, "%.3f");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1.0F);
            changed |= ImGui::DragFloat("##width", &key.value.x, 0.05F, 0.0F,
                                        10000.0F);
            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(-1.0F);
            changed |= ImGui::DragFloat("##height", &key.value.y, 0.05F,
                                        0.0F, 10000.0F);
            ImGui::TableSetColumnIndex(3);
            if (ImGui::SmallButton("x")) remove = static_cast<int>(i);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (remove >= 0) {
        eraseCurveKey(curve, static_cast<std::uint32_t>(remove));
        changed = true;
    }
    if (curve.count < particle::kMaxCurveKeys && ImGui::Button("Add size key")) {
        auto& key = curve.keys[curve.count++];
        key.time = curve.count == 1 ? 0.0F : 1.0F;
        key.value = curve.count > 1 ? curve.keys[curve.count - 2].value
                                    : Vec2{16.0F, 16.0F};
        changed = true;
    }
    if (changed) sortCurve(curve);
    return changed;
}

bool editColorCurve(particle::ColorCurve& curve) {
    bool changed = false;
    int remove = -1;
    if (ImGui::BeginTable("color-curve", 3,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed,
                                100.0F);
        ImGui::TableSetupColumn("Color");
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30.0F);
        ImGui::TableHeadersRow();
        for (std::uint32_t i = 0; i < curve.count; ++i) {
            auto& key = curve.keys[i];
            ImGui::PushID(static_cast<int>(i));
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::SetNextItemWidth(-1.0F);
            changed |= ImGui::DragFloat("##time", &key.time, 0.01F, 0.0F,
                                        1.0F, "%.3f");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1.0F);
            changed |= ImGui::ColorEdit4("##color", &key.value.r,
                                         ImGuiColorEditFlags_AlphaBar);
            ImGui::TableSetColumnIndex(2);
            if (ImGui::SmallButton("x")) remove = static_cast<int>(i);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (remove >= 0) {
        eraseCurveKey(curve, static_cast<std::uint32_t>(remove));
        changed = true;
    }
    if (curve.count < particle::kMaxCurveKeys &&
        ImGui::Button("Add color key")) {
        auto& key = curve.keys[curve.count++];
        key.time = curve.count == 1 ? 0.0F : 1.0F;
        key.value = curve.count > 1 ? curve.keys[curve.count - 2].value
                                    : Color{};
        changed = true;
    }
    if (changed) sortCurve(curve);
    return changed;
}

bool editRotationCurve(particle::ScalarCurve& curve) {
    bool changed = false;
    int remove = -1;
    if (ImGui::BeginTable("rotation-curve", 3,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed,
                                100.0F);
        ImGui::TableSetupColumn("Radians");
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30.0F);
        ImGui::TableHeadersRow();
        for (std::uint32_t i = 0; i < curve.count; ++i) {
            auto& key = curve.keys[i];
            ImGui::PushID(static_cast<int>(i));
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::SetNextItemWidth(-1.0F);
            changed |= ImGui::DragFloat("##time", &key.time, 0.01F, 0.0F,
                                        1.0F, "%.3f");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1.0F);
            changed |= ImGui::DragFloat("##angle", &key.value, 0.01F);
            ImGui::TableSetColumnIndex(2);
            if (ImGui::SmallButton("x")) remove = static_cast<int>(i);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (remove >= 0) {
        eraseCurveKey(curve, static_cast<std::uint32_t>(remove));
        changed = true;
    }
    if (curve.count < particle::kMaxCurveKeys &&
        ImGui::Button("Add rotation key")) {
        auto& key = curve.keys[curve.count++];
        key.time = curve.count == 1 ? 0.0F : 1.0F;
        key.value = curve.count > 1 ? curve.keys[curve.count - 2].value : 0.0F;
        changed = true;
    }
    if (changed) sortCurve(curve);
    return changed;
}

}  // namespace

ParticleEditorApp::ParticleEditorApp()
    : definition_(particle::makeDefaultParticleSystem()) {
    const char* initialDirectory = "particle_system";
    std::memcpy(directory_.data(), initialDirectory,
                std::strlen(initialDirectory) + 1U);
    preview_.setDefinition(definition_);
    preview_.play();
    previewNeedsSync_ = false;
    status_ = "Ready. particle.json is saved inside the selected directory.";
}

void ParticleEditorApp::frame(float deltaSeconds) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("Particle System Editor", nullptr, flags);

    drawToolbar();
    ImGui::Separator();

    const float bodyHeight = std::max(100.0F,
        ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing());
    ImGui::BeginChild("properties", ImVec2(kPropertyPanelWidth, bodyHeight),
                      ImGuiChildFlags_Borders);
    drawProperties();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("preview", ImVec2(0.0F, bodyHeight),
                      ImGuiChildFlags_Borders);
    drawPreview(deltaSeconds);
    ImGui::EndChild();

    drawStatusBar();
    ImGui::End();
}

void ParticleEditorApp::drawToolbar() {
    if (ImGui::Button("New")) newDocument();
    ImGui::SameLine();
    if (ImGui::Button("Open")) openDocument();
    ImGui::SameLine();
    if (ImGui::Button("Save")) saveDocument();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(480.0F);
    ImGui::InputText("Directory", directory_.data(), directory_.size());
    ImGui::SameLine();
    if (ImGui::Button("Reload textures")) reloadMaterialTextures();

    ImGui::SameLine(0.0F, 24.0F);
    if (preview_.playing()) {
        if (ImGui::Button("Pause")) preview_.pause();
    } else if (ImGui::Button("Play")) {
        preview_.play();
    }
    ImGui::SameLine();
    if (ImGui::Button("Restart")) preview_.restart();
}

void ParticleEditorApp::drawProperties() {
    bool changed = inputString("Name", definition_.name);
    if (changed) markChanged();
    drawMaterial();
    drawEmitter();
    drawMotion();
    drawCurves();
    drawBursts();
}

void ParticleEditorApp::drawMaterial() {
    if (!ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    auto& material = definition_.material;
    bool changed = false;
    changed |= inputString("Base texture", material.baseTexture);
    changed |= inputString("Normal texture", material.normalTexture);
    changed |= inputString("Emissive texture", material.emissiveTexture);
    changed |= ImGui::ColorEdit4("Tint", &material.tint.r,
                                 ImGuiColorEditFlags_AlphaBar);
    changed |= ImGui::ColorEdit4("Emissive", &material.emissive.r,
                                 ImGuiColorEditFlags_AlphaBar |
                                     ImGuiColorEditFlags_HDR);
    changed |= ImGui::SliderFloat("Opacity", &material.opacity, 0.0F, 1.0F);
    changed |= ImGui::SliderFloat("Alpha cutoff", &material.alphaCutoff,
                                  0.0F, 1.0F);
    changed |= ImGui::SliderFloat("Roughness", &material.roughness, 0.0F,
                                  1.0F);
    changed |= ImGui::SliderFloat("Metallic", &material.metallic, 0.0F, 1.0F);
    const char* blendModes[] = {"Premultiplied alpha", "Additive"};
    int blend = material.blendMode == ParticleBlendMode::PremultipliedAlpha
                    ? 0
                    : 1;
    if (ImGui::Combo("Blend mode", &blend, blendModes,
                     IM_ARRAYSIZE(blendModes))) {
        material.blendMode = blend == 0 ? ParticleBlendMode::PremultipliedAlpha
                                        : ParticleBlendMode::Additive;
        changed = true;
    }
    if (changed) markChanged();
}

void ParticleEditorApp::drawEmitter() {
    if (!ImGui::CollapsingHeader("Emitter", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    auto& emitter = definition_.emitter;
    bool changed = false;
    int capacity = static_cast<int>(emitter.maxParticles);
    if (ImGui::DragInt(
            "Maximum particles", &capacity, 16.0F, 1,
            static_cast<int>(particle::kMaximumParticleCapacity))) {
        emitter.maxParticles = static_cast<std::uint32_t>(std::max(1, capacity));
        changed = true;
    }
    changed |= ImGui::DragFloat("Emission rate", &emitter.emissionRate, 0.1F,
                                0.0F, particle::kMaximumEmissionRate);
    changed |= ImGui::DragFloat("Duration", &emitter.duration, 0.05F, 0.0F,
                                particle::kMaximumTimedDuration);
    changed |= ImGui::Checkbox("Looping", &emitter.looping);

    const char* simulationSpaces[] = {"World (trail)", "Local (attached)"};
    int simulationSpace =
        emitter.simulationSpace == particle::SimulationSpace::World ? 0 : 1;
    if (ImGui::Combo("Simulation space", &simulationSpace,
                     simulationSpaces, IM_ARRAYSIZE(simulationSpaces))) {
        emitter.simulationSpace =
            simulationSpace == 0 ? particle::SimulationSpace::World
                                 : particle::SimulationSpace::Local;
        changed = true;
    }
    changed |= ImGui::SliderFloat("Inherit emitter velocity",
                                  &emitter.inheritVelocity, 0.0F, 1.0F,
                                  "%.2f");
    if (emitter.simulationSpace == particle::SimulationSpace::Local) {
        ImGui::TextDisabled(
            "Velocity inheritance affects world-space births only.");
    }

    const char* shapes[] = {"Point", "Box", "Circle"};
    int shape = static_cast<int>(emitter.shape);
    if (ImGui::Combo("Shape", &shape, shapes, IM_ARRAYSIZE(shapes))) {
        emitter.shape = static_cast<particle::EmitterShape>(shape);
        changed = true;
    }
    if (emitter.shape == particle::EmitterShape::Box) {
        changed |= ImGui::DragFloat3("Shape extents", &emitter.shapeExtents.x,
                                     0.1F, 0.0F, 100000.0F);
    } else if (emitter.shape == particle::EmitterShape::Circle) {
        changed |= ImGui::DragFloat("Shape radius", &emitter.shapeRadius, 0.1F,
                                    0.0F, 100000.0F);
    }
    changed |= distributionCombo("Shape distribution",
                                 emitter.shapeDistribution);
    changed |= editFloatRange("lifetime", "Lifetime (seconds)",
                              emitter.lifetime);
    changed |= editFloatRange("size-multiplier", "Size multiplier",
                              emitter.sizeMultiplier);
    changed |= editFloatRange("rotation-offset", "Rotation offset (radians)",
                              emitter.rotationOffset);
    if (changed) markChanged();
}

void ParticleEditorApp::drawMotion() {
    if (!ImGui::CollapsingHeader("Position and motion",
                                 ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    auto& emitter = definition_.emitter;
    bool changed = false;
    changed |= editVec3Range("position", "Initial position", emitter.position);
    changed |= editVec3Range("velocity", "Initial velocity", emitter.velocity);
    changed |= editVec3Range("acceleration", "Acceleration",
                             emitter.acceleration);
    changed |= ImGui::DragFloat3("Gravity", &emitter.gravity.x, 0.1F);
    changed |= ImGui::DragFloat("Drag", &emitter.drag, 0.01F, 0.0F, 10000.0F);
    if (changed) markChanged();
}

void ParticleEditorApp::drawCurves() {
    if (!ImGui::CollapsingHeader("Curves", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    bool changed = false;
    if (ImGui::TreeNodeEx("Size", ImGuiTreeNodeFlags_DefaultOpen)) {
        changed |= editSizeCurve(definition_.emitter.size);
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
        changed |= editColorCurve(definition_.emitter.color);
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Rotation", ImGuiTreeNodeFlags_DefaultOpen)) {
        changed |= editRotationCurve(definition_.emitter.rotation);
        ImGui::TreePop();
    }
    if (changed) markChanged();
}

void ParticleEditorApp::drawBursts() {
    if (!ImGui::CollapsingHeader("Burst emission",
                                 ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    auto& bursts = definition_.emitter.bursts;
    bool changed = false;
    int remove = -1;
    for (std::size_t i = 0; i < bursts.size(); ++i) {
        auto& burst = bursts[i];
        ImGui::PushID(static_cast<int>(i));
        const std::string label = "Burst " + std::to_string(i + 1U);
        if (ImGui::TreeNodeEx("burst", ImGuiTreeNodeFlags_DefaultOpen, "%s",
                              label.c_str())) {
            changed |= ImGui::DragFloat("Time", &burst.time, 0.01F, 0.0F,
                                        86400.0F);
            int minimum = static_cast<int>(burst.countMinimum);
            int maximum = static_cast<int>(burst.countMaximum);
            int cycles = static_cast<int>(burst.cycles);
            if (ImGui::DragInt(
                    "Minimum count", &minimum, 1.0F, 0,
                    static_cast<int>(particle::kMaximumBurstParticleCount))) {
                burst.countMinimum =
                    static_cast<std::uint32_t>(std::max(0, minimum));
                changed = true;
            }
            if (ImGui::DragInt(
                    "Maximum count", &maximum, 1.0F, 0,
                    static_cast<int>(particle::kMaximumBurstParticleCount))) {
                burst.countMaximum =
                    static_cast<std::uint32_t>(std::max(0, maximum));
                changed = true;
            }
            if (ImGui::DragInt(
                    "Cycles", &cycles, 1.0F, 1,
                    static_cast<int>(particle::kMaximumBurstCycles))) {
                burst.cycles = static_cast<std::uint32_t>(std::max(1, cycles));
                changed = true;
            }
            changed |= ImGui::DragFloat("Interval", &burst.interval, 0.01F,
                                        0.0F,
                                        particle::kMaximumBurstInterval);
            if (ImGui::Button("Remove burst")) remove = static_cast<int>(i);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    if (remove >= 0) {
        bursts.erase(bursts.begin() + remove);
        changed = true;
    }
    if (bursts.size() < particle::kMaximumParticleBursts &&
        ImGui::Button("Add burst")) {
        bursts.emplace_back();
        changed = true;
    } else if (bursts.size() >= particle::kMaximumParticleBursts) {
        ImGui::TextDisabled("Burst limit reached (%zu)",
                            particle::kMaximumParticleBursts);
    }
    if (changed) markChanged();
}

void ParticleEditorApp::drawPreview(float deltaSeconds) {
    if (previewNeedsSync_) {
        std::string validationError;
        if (!particle::validateParticleSystem(definition_, validationError)) {
            ImGui::TextColored(ImVec4(1.0F, 0.35F, 0.3F, 1.0F),
                               "Preview unavailable: %s",
                               validationError.c_str());
            ImGui::TextWrapped(
                "Adjust the invalid particle parameters before previewing."
            );
            return;
        }
        preview_.setDefinition(definition_);
        previewNeedsSync_ = false;
    }
    preview_.update(definition_, deltaSeconds);

    ImGui::Text("Analytic editor preview | %.2f s | %zu particles",
                preview_.time(), preview_.particleCount());
    if (baseTexture_.imguiId() != 0) {
        ImGui::SameLine();
        ImGui::TextDisabled("| texture %dx%d", baseTexture_.width(),
                            baseTexture_.height());
    }
    ImGui::TextDisabled(
        "Motion/base-color preview: runtime also applies alpha cutoff, blend "
        "mode, normal/emissive lighting and HDR bloom.");
    const auto drawSample = [](const char* label,
                               const PreviewTexture& texture) {
        ImGui::BeginGroup();
        ImGui::TextUnformatted(label);
        if (texture.imguiId() != 0) {
            ImGui::Image(texture.imguiId(), ImVec2(64.0F, 64.0F));
            ImGui::TextDisabled("%dx%d", texture.width(), texture.height());
        } else {
            ImGui::Dummy(ImVec2(64.0F, 64.0F));
            ImGui::TextDisabled("not set");
        }
        ImGui::EndGroup();
    };
    drawSample("Base", baseTexture_);
    ImGui::SameLine();
    drawSample("Normal", normalTexture_);
    ImGui::SameLine();
    drawSample("Emissive", emissiveTexture_);
    ImGui::Separator();
    ImVec2 available = ImGui::GetContentRegionAvail();
    available.x = std::max(available.x, 64.0F);
    available.y = std::max(available.y, 64.0F);
    const ImVec2 topLeft = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("particle-preview-canvas", available);
    preview_.draw(ImGui::GetWindowDrawList(), topLeft, available, definition_,
                  baseTexture_.imguiId());
}

void ParticleEditorApp::drawStatusBar() {
    ImGui::Separator();
    if (dirty_) {
        ImGui::TextColored(ImVec4(1.0F, 0.72F, 0.2F, 1.0F), "Modified");
        ImGui::SameLine();
    }
    ImGui::TextUnformatted(status_.c_str());
}

void ParticleEditorApp::newDocument() {
    definition_ = particle::makeDefaultParticleSystem();
    dirty_ = false;
    preview_.setDefinition(definition_);
    preview_.play();
    previewNeedsSync_ = false;
    baseTexture_.reset();
    normalTexture_.reset();
    emissiveTexture_.reset();
    status_ = "Created a new particle system.";
}

void ParticleEditorApp::openDocument() {
    particle::ParticleSystemDefinition loaded;
    std::string error;
    if (!particle::loadParticleSystem(std::filesystem::u8path(directory_.data()),
                                      loaded, error)) {
        status_ = "Open failed: " + error;
        return;
    }
    definition_ = std::move(loaded);
    dirty_ = false;
    preview_.setDefinition(definition_);
    preview_.play();
    previewNeedsSync_ = false;
    status_ = "Loaded particle.json.";
    reloadMaterialTextures();
}

void ParticleEditorApp::saveDocument() {
    std::string error;
    if (!particle::saveParticleSystem(std::filesystem::u8path(directory_.data()),
                                      definition_, error)) {
        status_ = "Save failed: " + error;
        return;
    }
    dirty_ = false;
    status_ = "Saved particle.json.";
}

void ParticleEditorApp::reloadMaterialTextures() {
    const std::filesystem::path root =
        std::filesystem::u8path(directory_.data());
    std::string errors;
    std::size_t loaded = 0;
    const auto load = [&](const char* label, const std::string& relative,
                          PreviewTexture& texture) {
        if (relative.empty()) {
            texture.reset();
            return;
        }
        std::string portable = relative;
        std::replace(portable.begin(), portable.end(), '\\', '/');
        std::string error;
        if (!texture.load(root / std::filesystem::u8path(portable), error)) {
            texture.reset();
            if (!errors.empty()) errors += "; ";
            errors += label;
            errors += ": ";
            errors += error;
            return;
        }
        ++loaded;
    };
    load("base", definition_.material.baseTexture, baseTexture_);
    load("normal", definition_.material.normalTexture, normalTexture_);
    load("emissive", definition_.material.emissiveTexture,
         emissiveTexture_);
    status_ = errors.empty()
                  ? "Loaded " + std::to_string(loaded) +
                        " material texture sample(s)."
                  : "Texture load failed: " + errors;
}

void ParticleEditorApp::markChanged() noexcept {
    dirty_ = true;
    previewNeedsSync_ = true;
}

}  // namespace newaoe::render::editor

