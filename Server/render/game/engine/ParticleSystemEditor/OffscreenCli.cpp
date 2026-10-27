#include "Server/render/game/engine/ParticleSystemEditor/OffscreenCli.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "Server/render/game/engine/particle/ParticleSystem.h"

namespace newaoe::render::editor {
namespace {

namespace particle = newaoe::render::particle;

constexpr int kSuccess = 0;
constexpr int kUsageError = 2;
constexpr int kReadError = 3;
constexpr int kValidationError = 4;
constexpr int kWriteError = 5;

struct Arguments final {
    std::vector<std::string> values;
    bool json = false;
};

std::string_view trim(std::string_view value) {
    while (!value.empty() &&
           (value.front() == ' ' || value.front() == '\t' ||
            value.front() == '\r' || value.front() == '\n')) {
        value.remove_prefix(1);
    }
    while (!value.empty() &&
           (value.back() == ' ' || value.back() == '\t' ||
            value.back() == '\r' || value.back() == '\n')) {
        value.remove_suffix(1);
    }
    return value;
}

std::string lower(std::string_view value) {
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char character) {
                       if (character >= 'A' && character <= 'Z') {
                           return static_cast<char>(character - 'A' + 'a');
                       }
                       return static_cast<char>(character);
                   });
    return result;
}

std::string jsonString(std::string_view value) {
    std::ostringstream output;
    output << '"';
    for (const unsigned char character : value) {
        switch (character) {
            case '"': output << "\\\""; break;
            case '\\': output << "\\\\"; break;
            case '\b': output << "\\b"; break;
            case '\f': output << "\\f"; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                if (character < 0x20U) {
                    static constexpr char digits[] = "0123456789abcdef";
                    output << "\\u00" << digits[(character >> 4U) & 0xFU]
                           << digits[character & 0xFU];
                } else {
                    output << static_cast<char>(character);
                }
        }
    }
    output << '"';
    return output.str();
}

std::filesystem::path pathFromUtf8(std::string_view value) {
    return std::filesystem::u8path(value.begin(), value.end());
}

int fail(bool json, int code, std::string_view command,
         std::string_view message) {
    if (json) {
        std::cout << "{\"ok\":false,\"command\":" << jsonString(command)
                  << ",\"error\":" << jsonString(message)
                  << ",\"exitCode\":" << code << "}\n";
    } else {
        std::cerr << "ParticleSystemEditor: " << message << '\n';
    }
    return code;
}

void succeed(bool json, std::string_view command,
             const std::filesystem::path& directory,
             std::string_view detail = {}) {
    if (json) {
        std::cout << "{\"ok\":true,\"command\":" << jsonString(command)
                  << ",\"directory\":"
                  << jsonString(directory.lexically_normal().u8string());
        if (!detail.empty()) {
            std::cout << ",\"detail\":" << jsonString(detail);
        }
        std::cout << "}\n";
    } else {
        std::cout << command << " succeeded: "
                  << directory.lexically_normal().u8string();
        if (!detail.empty()) std::cout << " (" << detail << ')';
        std::cout << '\n';
    }
}

void printHelp() {
    std::cout
        << "NewAOE Particle System Editor - offscreen commands\n\n"
        << "Usage:\n"
        << "  ParticleSystemEditor --offscreen help\n"
        << "  ParticleSystemEditor --offscreen create <directory> "
           "[--preset default|fire|smoke] [--force] [--json]\n"
        << "  ParticleSystemEditor --offscreen preset <default|fire|smoke> "
           "<directory> [--force] [--json]\n"
        << "  ParticleSystemEditor --offscreen inspect <directory> "
           "[--json]\n"
        << "  ParticleSystemEditor --offscreen validate <directory> "
           "[--json]\n"
        << "  ParticleSystemEditor --offscreen set <directory> <path> "
           "<value> [--json]\n"
        << "  ParticleSystemEditor --offscreen apply <directory> "
           "<path=value>... [--json]\n"
        << "  ParticleSystemEditor --offscreen apply <directory> "
           "--set <path> <value> [--set <path> <value> ...] [--json]\n\n"
        << "apply validates all changes together and saves once, so range "
           "endpoints can be changed atomically.\n"
        << "Vector values use commas, for example: 0,-125,0.\n"
        << "Curve values use semicolon-separated time:value keys:\n"
        << "  emitter.sizeCurve=0:8,12;0.3:20,34;1:3,6\n"
        << "  emitter.colorCurve=0:1,1,0.4,1;1:0.2,0,0,0\n"
        << "  emitter.rotationCurve=0:-0.1;1:0.5\n"
        << "Bursts use time,min,max,cycles,interval entries separated by ';'.\n"
        << "Use quotes around values containing ';' in a shell.\n\n"
        << "Editable paths:\n"
        << "  name\n"
        << "  material.baseTexture|normalTexture|emissiveTexture|tint|"
           "emissive|opacity|alphaCutoff|roughness|metallic|blendMode\n"
        << "  emitter.maxParticles|emissionRate|duration|looping|"
           "simulationSpace|inheritVelocity|shape|shapeExtents|shapeRadius|"
           "shapeDistribution|gravity|drag\n"
        << "  emitter.<lifetime|sizeMultiplier|rotationOffset>."
           "<minimum|maximum|distribution>\n"
        << "  emitter.<position|velocity|acceleration>."
           "<minimum|maximum|distribution>\n"
        << "  emitter.sizeCurve|colorCurve|rotationCurve|bursts\n";
}

bool parseFloat(std::string_view text, float& output) {
    std::istringstream input{std::string(trim(text))};
    input.imbue(std::locale::classic());
    float value = 0.0F;
    if (!(input >> value) || !std::isfinite(value)) return false;
    input >> std::ws;
    if (input.peek() != std::char_traits<char>::eof()) return false;
    output = value;
    return true;
}

bool parseUnsigned(std::string_view text, std::uint32_t& output) {
    text = trim(text);
    if (text.empty()) return false;
    std::uint32_t value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(),
                                        value, 10);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
        return false;
    }
    output = value;
    return true;
}

bool parseBool(std::string_view text, bool& output) {
    const std::string value = lower(trim(text));
    if (value == "true" || value == "1" || value == "yes") {
        output = true;
        return true;
    }
    if (value == "false" || value == "0" || value == "no") {
        output = false;
        return true;
    }
    return false;
}

std::vector<std::string_view> split(std::string_view source, char delimiter) {
    std::vector<std::string_view> result;
    std::size_t begin = 0;
    while (begin <= source.size()) {
        const std::size_t end = source.find(delimiter, begin);
        result.push_back(trim(source.substr(
            begin, end == std::string_view::npos ? source.size() - begin
                                                 : end - begin)));
        if (end == std::string_view::npos) break;
        begin = end + 1;
    }
    return result;
}

template <std::size_t Count>
bool parseFloatList(std::string_view text, std::array<float, Count>& output) {
    text = trim(text);
    if (text.size() >= 2 &&
        ((text.front() == '[' && text.back() == ']') ||
         (text.front() == '(' && text.back() == ')'))) {
        text = trim(text.substr(1, text.size() - 2));
    }
    const auto values = split(text, ',');
    if (values.size() != Count) return false;
    for (std::size_t index = 0; index < Count; ++index) {
        if (!parseFloat(values[index], output[index])) return false;
    }
    return true;
}

bool parseVec2(std::string_view text, Vec2& output) {
    std::array<float, 2> values{};
    if (!parseFloatList(text, values)) return false;
    output = {values[0], values[1]};
    return true;
}

bool parseVec3(std::string_view text, Vec3& output) {
    std::array<float, 3> values{};
    if (!parseFloatList(text, values)) return false;
    output = {values[0], values[1], values[2]};
    return true;
}

bool parseColor(std::string_view text, Color& output) {
    std::array<float, 4> values{};
    if (!parseFloatList(text, values)) return false;
    output = {values[0], values[1], values[2], values[3]};
    return true;
}

bool parseDistribution(std::string_view text,
                       particle::RandomDistribution& output) {
    const std::string value = lower(trim(text));
    if (value == "uniform") {
        output = particle::RandomDistribution::Uniform;
        return true;
    }
    if (value == "normal") {
        output = particle::RandomDistribution::Normal;
        return true;
    }
    return false;
}

bool parseShape(std::string_view text, particle::EmitterShape& output) {
    const std::string value = lower(trim(text));
    if (value == "point") output = particle::EmitterShape::Point;
    else if (value == "box") output = particle::EmitterShape::Box;
    else if (value == "circle") output = particle::EmitterShape::Circle;
    else return false;
    return true;
}

bool parseSimulationSpace(std::string_view text,
                          particle::SimulationSpace& output) {
    const std::string value = lower(trim(text));
    if (value == "world") output = particle::SimulationSpace::World;
    else if (value == "local") output = particle::SimulationSpace::Local;
    else return false;
    return true;
}

bool parseBlendMode(std::string_view text, ParticleBlendMode& output) {
    const std::string value = lower(trim(text));
    if (value == "additive") output = ParticleBlendMode::Additive;
    else if (value == "premultipliedalpha" || value == "premultiplied") {
        output = ParticleBlendMode::PremultipliedAlpha;
    } else {
        return false;
    }
    return true;
}

template <typename Curve, typename ValueParser>
bool parseCurve(std::string_view text, Curve& output, ValueParser parseValue,
                std::string& error) {
    const auto entries = split(trim(text), ';');
    if (entries.empty() || entries.size() > particle::kMaxCurveKeys ||
        (entries.size() == 1 && entries[0].empty())) {
        error = "curve must contain between 1 and " +
                std::to_string(particle::kMaxCurveKeys) + " keys";
        return false;
    }
    Curve parsed{};
    parsed.count = static_cast<std::uint32_t>(entries.size());
    for (std::size_t index = 0; index < entries.size(); ++index) {
        const std::size_t colon = entries[index].find(':');
        if (colon == std::string_view::npos ||
            !parseFloat(entries[index].substr(0, colon),
                        parsed.keys[index].time) ||
            !parseValue(entries[index].substr(colon + 1),
                        parsed.keys[index].value)) {
            error = "invalid curve key " + std::to_string(index) +
                    "; expected time:value";
            return false;
        }
    }
    output = parsed;
    return true;
}

bool parseBursts(std::string_view text, std::vector<particle::Burst>& output,
                 std::string& error) {
    text = trim(text);
    if (text.empty() || text == "[]") {
        output.clear();
        return true;
    }
    const auto entries = split(text, ';');
    if (entries.size() > particle::kMaximumParticleBursts) {
        error = "burst list exceeds the 1024 entry limit";
        return false;
    }
    std::vector<particle::Burst> parsed;
    parsed.reserve(entries.size());
    for (std::size_t index = 0; index < entries.size(); ++index) {
        const auto fields = split(entries[index], ',');
        particle::Burst burst;
        if (fields.size() != 5 || !parseFloat(fields[0], burst.time) ||
            !parseUnsigned(fields[1], burst.countMinimum) ||
            !parseUnsigned(fields[2], burst.countMaximum) ||
            !parseUnsigned(fields[3], burst.cycles) ||
            !parseFloat(fields[4], burst.interval)) {
            error = "invalid burst " + std::to_string(index) +
                    "; expected time,min,max,cycles,interval";
            return false;
        }
        parsed.push_back(burst);
    }
    output = std::move(parsed);
    return true;
}

bool setFloatRange(particle::FloatRange& range, std::string_view component,
                   std::string_view value) {
    if (component == "minimum") return parseFloat(value, range.minimum);
    if (component == "maximum") return parseFloat(value, range.maximum);
    if (component == "distribution") {
        return parseDistribution(value, range.distribution);
    }
    return false;
}

bool setVec3Range(particle::Vec3Range& range, std::string_view component,
                  std::string_view value) {
    if (component == "minimum") return parseVec3(value, range.minimum);
    if (component == "maximum") return parseVec3(value, range.maximum);
    if (component == "distribution") {
        return parseDistribution(value, range.distribution);
    }
    return false;
}

bool setProperty(particle::ParticleSystemDefinition& definition,
                 std::string_view path, std::string_view value,
                 std::string& error) {
    path = trim(path);
    value = trim(value);
    bool parsed = false;

    if (path == "name") {
        definition.name = std::string(value);
        parsed = !definition.name.empty();
    } else if (path == "material.baseTexture") {
        definition.material.baseTexture = std::string(value);
        parsed = true;
    } else if (path == "material.normalTexture") {
        definition.material.normalTexture = std::string(value);
        parsed = true;
    } else if (path == "material.emissiveTexture") {
        definition.material.emissiveTexture = std::string(value);
        parsed = true;
    } else if (path == "material.tint") {
        parsed = parseColor(value, definition.material.tint);
    } else if (path == "material.emissive") {
        parsed = parseColor(value, definition.material.emissive);
    } else if (path == "material.opacity") {
        parsed = parseFloat(value, definition.material.opacity);
    } else if (path == "material.alphaCutoff") {
        parsed = parseFloat(value, definition.material.alphaCutoff);
    } else if (path == "material.roughness") {
        parsed = parseFloat(value, definition.material.roughness);
    } else if (path == "material.metallic") {
        parsed = parseFloat(value, definition.material.metallic);
    } else if (path == "material.blendMode") {
        parsed = parseBlendMode(value, definition.material.blendMode);
    } else if (path == "emitter.maxParticles") {
        parsed = parseUnsigned(value, definition.emitter.maxParticles);
    } else if (path == "emitter.emissionRate") {
        parsed = parseFloat(value, definition.emitter.emissionRate);
    } else if (path == "emitter.duration") {
        parsed = parseFloat(value, definition.emitter.duration);
    } else if (path == "emitter.looping") {
        parsed = parseBool(value, definition.emitter.looping);
    } else if (path == "emitter.simulationSpace") {
        parsed = parseSimulationSpace(value,
                                      definition.emitter.simulationSpace);
    } else if (path == "emitter.inheritVelocity") {
        parsed = parseFloat(value, definition.emitter.inheritVelocity);
    } else if (path == "emitter.shape") {
        parsed = parseShape(value, definition.emitter.shape);
    } else if (path == "emitter.shapeExtents") {
        parsed = parseVec3(value, definition.emitter.shapeExtents);
    } else if (path == "emitter.shapeRadius") {
        parsed = parseFloat(value, definition.emitter.shapeRadius);
    } else if (path == "emitter.shapeDistribution") {
        parsed = parseDistribution(value,
                                   definition.emitter.shapeDistribution);
    } else if (path == "emitter.gravity") {
        parsed = parseVec3(value, definition.emitter.gravity);
    } else if (path == "emitter.drag") {
        parsed = parseFloat(value, definition.emitter.drag);
    } else if (path == "emitter.sizeCurve") {
        parsed = parseCurve(value, definition.emitter.size, parseVec2, error);
    } else if (path == "emitter.colorCurve") {
        parsed = parseCurve(value, definition.emitter.color, parseColor, error);
    } else if (path == "emitter.rotationCurve") {
        parsed = parseCurve(value, definition.emitter.rotation, parseFloat,
                            error);
    } else if (path == "emitter.bursts") {
        parsed = parseBursts(value, definition.emitter.bursts, error);
    } else {
        constexpr std::string_view prefix = "emitter.";
        if (path.substr(0, prefix.size()) == prefix) {
            const std::string_view suffix = path.substr(prefix.size());
            const std::size_t dot = suffix.find('.');
            if (dot != std::string_view::npos) {
                const std::string_view rangeName = suffix.substr(0, dot);
                const std::string_view component = suffix.substr(dot + 1);
                if (rangeName == "lifetime") {
                    parsed = setFloatRange(definition.emitter.lifetime,
                                           component, value);
                } else if (rangeName == "sizeMultiplier") {
                    parsed = setFloatRange(definition.emitter.sizeMultiplier,
                                           component, value);
                } else if (rangeName == "rotationOffset") {
                    parsed = setFloatRange(definition.emitter.rotationOffset,
                                           component, value);
                } else if (rangeName == "position") {
                    parsed = setVec3Range(definition.emitter.position,
                                         component, value);
                } else if (rangeName == "velocity") {
                    parsed = setVec3Range(definition.emitter.velocity,
                                         component, value);
                } else if (rangeName == "acceleration") {
                    parsed = setVec3Range(definition.emitter.acceleration,
                                         component, value);
                }
            }
        }
    }

    if (!parsed && error.empty()) {
        error = "unknown property or invalid value for '" + std::string(path) +
                "'";
    }
    return parsed;
}

particle::ParticleSystemDefinition makeFirePreset() {
    auto result = particle::makeDefaultParticleSystem();
    result.name = "Fire";
    result.material.tint = {1.0F, 0.82F, 0.35F, 1.0F};
    result.material.emissive = {1.5F, 0.35F, 0.02F, 1.0F};
    result.material.opacity = 0.9F;
    result.material.roughness = 0.8F;
    result.material.blendMode = ParticleBlendMode::Additive;

    auto& emitter = result.emitter;
    emitter.maxParticles = 512;
    emitter.emissionRate = 65.0F;
    emitter.duration = 2.0F;
    emitter.looping = true;
    emitter.shape = particle::EmitterShape::Circle;
    emitter.shapeExtents = {8.0F, 4.0F, 0.0F};
    emitter.shapeRadius = 8.0F;
    emitter.lifetime = {0.65F, 1.15F,
                        particle::RandomDistribution::Uniform};
    emitter.position = {{-2.0F, 0.0F, 0.0F}, {2.0F, 4.0F, 0.0F},
                        particle::RandomDistribution::Uniform};
    emitter.velocity = {{-18.0F, -125.0F, 0.0F},
                        {18.0F, -80.0F, 0.0F},
                        particle::RandomDistribution::Uniform};
    emitter.acceleration = {{-4.0F, 0.0F, 0.0F}, {4.0F, 0.0F, 0.0F},
                            particle::RandomDistribution::Uniform};
    emitter.gravity = {0.0F, 45.0F, 0.0F};
    emitter.drag = 0.25F;
    emitter.sizeMultiplier = {0.8F, 1.4F,
                              particle::RandomDistribution::Uniform};
    emitter.rotationOffset = {-0.35F, 0.35F,
                              particle::RandomDistribution::Uniform};

    emitter.size.count = 4;
    emitter.size.keys[0] = {0.0F, {8.0F, 12.0F}};
    emitter.size.keys[1] = {0.3F, {20.0F, 34.0F}};
    emitter.size.keys[2] = {0.75F, {14.0F, 24.0F}};
    emitter.size.keys[3] = {1.0F, {3.0F, 6.0F}};
    emitter.color.count = 4;
    emitter.color.keys[0] = {0.0F, {1.0F, 0.95F, 0.45F, 1.0F}};
    emitter.color.keys[1] = {0.25F, {1.0F, 0.5F, 0.08F, 0.95F}};
    emitter.color.keys[2] = {0.65F, {0.9F, 0.12F, 0.01F, 0.55F}};
    emitter.color.keys[3] = {1.0F, {0.2F, 0.01F, 0.0F, 0.0F}};
    emitter.rotation.count = 3;
    emitter.rotation.keys[0] = {0.0F, -0.15F};
    emitter.rotation.keys[1] = {0.5F, 0.2F};
    emitter.rotation.keys[2] = {1.0F, 0.55F};
    emitter.bursts = {{0.0F, 18, 28, 1, 0.0F}};
    return result;
}

particle::ParticleSystemDefinition makeSmokePreset() {
    auto result = particle::makeDefaultParticleSystem();
    result.name = "Smoke Grenade Trail";
    result.material.baseTexture = "smoke.png";
    result.material.tint = {0.42F, 0.46F, 0.44F, 1.0F};
    result.material.emissive = {0.0F, 0.0F, 0.0F, 1.0F};
    result.material.opacity = 0.68F;
    result.material.alphaCutoff = 0.002F;
    result.material.roughness = 1.0F;
    result.material.metallic = 0.0F;
    result.material.blendMode = ParticleBlendMode::PremultipliedAlpha;

    auto& emitter = result.emitter;
    emitter.maxParticles = 2048;
    emitter.emissionRate = 44.0F;
    emitter.duration = 3.0F;
    emitter.looping = true;
    emitter.simulationSpace = particle::SimulationSpace::World;
    emitter.inheritVelocity = 0.09F;
    emitter.shape = particle::EmitterShape::Circle;
    emitter.shapeExtents = {3.0F, 3.0F, 0.0F};
    emitter.shapeRadius = 3.0F;
    emitter.shapeDistribution = particle::RandomDistribution::Normal;
    emitter.lifetime = {2.8F, 4.6F,
                        particle::RandomDistribution::Uniform};
    // The instance rotation points +X along the grenade trajectory. Spawn
    // behind the body so fresh smoke does not cover the grenade sprite.
    // Start just behind a roughly 96 px projectile.  This keeps the first
    // smoke puffs adjacent to the grenade without covering its silhouette.
    emitter.position = {{-70.0F, -2.0F, 0.0F}, {-58.0F, 2.0F, 0.0F},
                        particle::RandomDistribution::Normal};
    emitter.velocity = {{-10.0F, -28.0F, 0.0F},
                        {10.0F, -8.0F, 0.0F},
                        particle::RandomDistribution::Normal};
    emitter.acceleration = {{-2.0F, -5.0F, 0.0F}, {2.0F, 1.0F, 0.0F},
                            particle::RandomDistribution::Normal};
    emitter.gravity = {0.0F, -6.0F, 0.0F};
    emitter.drag = 0.9F;
    emitter.sizeMultiplier = {0.75F, 1.4F,
                              particle::RandomDistribution::Uniform};
    emitter.rotationOffset = {-3.14159265F, 3.14159265F,
                              particle::RandomDistribution::Uniform};

    emitter.size.count = 4;
    emitter.size.keys[0] = {0.0F, {12.0F, 12.0F}};
    emitter.size.keys[1] = {0.15F, {26.0F, 26.0F}};
    emitter.size.keys[2] = {0.65F, {62.0F, 62.0F}};
    emitter.size.keys[3] = {1.0F, {90.0F, 90.0F}};
    emitter.color.count = 4;
    emitter.color.keys[0] = {0.0F, {0.58F, 0.62F, 0.64F, 0.05F}};
    emitter.color.keys[1] = {0.12F, {0.72F, 0.74F, 0.75F, 0.72F}};
    emitter.color.keys[2] = {0.7F, {0.42F, 0.44F, 0.45F, 0.42F}};
    emitter.color.keys[3] = {1.0F, {0.2F, 0.21F, 0.22F, 0.0F}};
    emitter.rotation.count = 3;
    emitter.rotation.keys[0] = {0.0F, -0.08F};
    emitter.rotation.keys[1] = {0.5F, 0.12F};
    emitter.rotation.keys[2] = {1.0F, 0.35F};
    emitter.bursts = {{0.0F, 8, 12, 1, 0.0F}};
    return result;
}

bool makePreset(std::string_view name,
                particle::ParticleSystemDefinition& definition,
                std::string& error) {
    const std::string normalized = lower(name);
    if (normalized == "default") {
        definition = particle::makeDefaultParticleSystem();
        return true;
    }
    if (normalized == "fire") {
        definition = makeFirePreset();
        return true;
    }
    if (normalized == "smoke" || normalized == "smokegrenade" ||
        normalized == "smoke-grenade") {
        definition = makeSmokePreset();
        return true;
    }
    error = "unknown preset '" + std::string(name) +
            "'; expected default, fire or smoke";
    return false;
}

bool manifestExists(const std::filesystem::path& directory,
                    std::string& error) {
    std::error_code filesystemError;
    const bool exists =
        std::filesystem::exists(directory / "particle.json", filesystemError);
    if (filesystemError) {
        error = "cannot inspect output directory: " +
                filesystemError.message();
        return false;
    }
    return exists;
}

int saveNew(const std::filesystem::path& directory, std::string_view preset,
            bool force, bool json, std::string_view command) {
    std::string error;
    if (manifestExists(directory, error) && !force) {
        return fail(json, kWriteError, command,
                    "particle.json already exists; pass --force to replace it");
    }
    if (!error.empty()) return fail(json, kWriteError, command, error);
    particle::ParticleSystemDefinition definition;
    if (!makePreset(preset, definition, error)) {
        return fail(json, kUsageError, command, error);
    }
    if (!particle::saveParticleSystem(directory, definition, error)) {
        return fail(json, kWriteError, command, error);
    }
    succeed(json, command, directory, std::string("preset=") +
                                      std::string(preset));
    return kSuccess;
}

int inspect(const std::filesystem::path& directory, bool json) {
    particle::ParticleSystemDefinition definition;
    std::string error;
    if (!particle::loadParticleSystem(directory, definition, error)) {
        return fail(json, kReadError, "inspect", error);
    }
    std::ifstream input(directory / "particle.json", std::ios::binary);
    if (!input) {
        return fail(json, kReadError, "inspect",
                    "cannot reopen validated particle.json");
    }
    std::cout << input.rdbuf();
    if (!std::cout) {
        return fail(json, kReadError, "inspect", "cannot write stdout");
    }
    return kSuccess;
}

int validate(const std::filesystem::path& directory, bool json) {
    particle::ParticleSystemDefinition definition;
    std::string error;
    if (!particle::loadParticleSystem(directory, definition, error)) {
        return fail(json, kValidationError, "validate", error);
    }
    succeed(json, "validate", directory, "valid");
    return kSuccess;
}

int update(const std::filesystem::path& directory,
           const std::vector<std::pair<std::string, std::string>>& changes,
           bool json, std::string_view command) {
    particle::ParticleSystemDefinition definition;
    std::string error;
    if (!particle::loadParticleSystem(directory, definition, error)) {
        return fail(json, kReadError, command, error);
    }
    for (const auto& change : changes) {
        error.clear();
        if (!setProperty(definition, change.first, change.second, error)) {
            return fail(json, kUsageError, command, error);
        }
    }
    if (!particle::validateParticleSystem(definition, error)) {
        return fail(json, kValidationError, command, error);
    }
    if (!particle::saveParticleSystem(directory, definition, error)) {
        return fail(json, kWriteError, command, error);
    }
    succeed(json, command, directory,
            std::to_string(changes.size()) + " change(s)");
    return kSuccess;
}

Arguments collectArguments(int argc, char** argv) {
    Arguments result;
    for (int index = 1; index < argc; ++index) {
        const std::string value = argv[index] ? argv[index] : "";
        if (value == "--offscreen") continue;
        if (value == "--json") {
            result.json = true;
            continue;
        }
        result.values.push_back(value);
    }
    return result;
}

}  // namespace

int runOffscreenCli(int argc, char** argv) {
    const Arguments arguments = collectArguments(argc, argv);
    if (arguments.values.empty() || arguments.values[0] == "help" ||
        arguments.values[0] == "--help" || arguments.values[0] == "-h") {
        printHelp();
        return kSuccess;
    }

    const std::string& command = arguments.values[0];
    if (command == "create") {
        if (arguments.values.size() < 2) {
            return fail(arguments.json, kUsageError, command,
                        "create requires an output directory");
        }
        std::string preset = "default";
        bool force = false;
        for (std::size_t index = 2; index < arguments.values.size(); ++index) {
            if (arguments.values[index] == "--force") {
                force = true;
            } else if (arguments.values[index] == "--preset" &&
                       index + 1 < arguments.values.size()) {
                preset = arguments.values[++index];
            } else {
                return fail(arguments.json, kUsageError, command,
                            "unknown create option '" +
                                arguments.values[index] + "'");
            }
        }
        return saveNew(pathFromUtf8(arguments.values[1]), preset, force,
                       arguments.json, command);
    }

    if (command == "preset") {
        if (arguments.values.size() < 3 || arguments.values.size() > 4 ||
            (arguments.values.size() == 4 &&
             arguments.values[3] != "--force")) {
            return fail(arguments.json, kUsageError, command,
                        "usage: preset <default|fire|smoke> <directory> "
                        "[--force]");
        }
        return saveNew(pathFromUtf8(arguments.values[2]), arguments.values[1],
                       arguments.values.size() == 4, arguments.json, command);
    }

    if (command == "inspect" || command == "validate") {
        if (arguments.values.size() != 2) {
            return fail(arguments.json, kUsageError, command,
                        command + " requires exactly one directory");
        }
        return command == "inspect"
                   ? inspect(pathFromUtf8(arguments.values[1]), arguments.json)
                   : validate(pathFromUtf8(arguments.values[1]), arguments.json);
    }

    if (command == "set") {
        if (arguments.values.size() != 4) {
            return fail(arguments.json, kUsageError, command,
                        "usage: set <directory> <path> <value>");
        }
        return update(pathFromUtf8(arguments.values[1]),
                      {{arguments.values[2], arguments.values[3]}},
                      arguments.json, command);
    }

    if (command == "apply") {
        if (arguments.values.size() < 3) {
            return fail(arguments.json, kUsageError, command,
                        "apply requires at least one property change");
        }
        std::vector<std::pair<std::string, std::string>> changes;
        for (std::size_t index = 2; index < arguments.values.size(); ++index) {
            if (arguments.values[index] == "--set") {
                if (index + 2 >= arguments.values.size()) {
                    return fail(arguments.json, kUsageError, command,
                                "--set requires a path and value");
                }
                changes.emplace_back(arguments.values[index + 1],
                                     arguments.values[index + 2]);
                index += 2;
                continue;
            }
            const std::size_t equals = arguments.values[index].find('=');
            if (equals == std::string::npos || equals == 0) {
                return fail(arguments.json, kUsageError, command,
                            "expected path=value or --set path value, got '" +
                                arguments.values[index] + "'");
            }
            changes.emplace_back(arguments.values[index].substr(0, equals),
                                 arguments.values[index].substr(equals + 1));
        }
        return update(pathFromUtf8(arguments.values[1]), changes,
                      arguments.json, command);
    }

    return fail(arguments.json, kUsageError, command,
                "unknown command; use --offscreen help");
}

}  // namespace newaoe::render::editor
