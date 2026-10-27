#include "Server/render/game/engine/particle/ParticleSystem.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string_view>
#include <system_error>
#include <utility>

namespace newaoe::render::particle {
namespace {

constexpr std::uintmax_t kMaximumManifestBytes = 4U * 1024U * 1024U;
constexpr std::size_t kMaximumJsonDepth = 64;
constexpr std::size_t kMaximumJsonNodes = 65'536;
constexpr std::size_t kMaximumJsonArrayElements = 16'384;
constexpr std::size_t kMaximumJsonObjectMembers = 256;

enum class JsonType : std::uint8_t { Null, Boolean, Number, String, Array, Object };

struct JsonValue final {
    JsonType type = JsonType::Null;
    bool boolean = false;
    double number = 0.0;
    std::string string;
    std::vector<JsonValue> array;
    std::vector<std::pair<std::string, JsonValue>> object;
};

bool validUtf8(std::string_view text) noexcept {
    std::size_t index = 0;
    while (index < text.size()) {
        const auto first = static_cast<unsigned char>(text[index]);
        if (first <= 0x7FU) {
            ++index;
            continue;
        }
        std::size_t continuationCount = 0;
        std::uint32_t codepoint = 0;
        std::uint32_t minimum = 0;
        if (first >= 0xC2U && first <= 0xDFU) {
            continuationCount = 1;
            codepoint = first & 0x1FU;
            minimum = 0x80U;
        } else if (first >= 0xE0U && first <= 0xEFU) {
            continuationCount = 2;
            codepoint = first & 0x0FU;
            minimum = 0x800U;
        } else if (first >= 0xF0U && first <= 0xF4U) {
            continuationCount = 3;
            codepoint = first & 0x07U;
            minimum = 0x10000U;
        } else {
            return false;
        }
        if (index + continuationCount >= text.size()) return false;
        for (std::size_t offset = 1; offset <= continuationCount; ++offset) {
            const auto next =
                static_cast<unsigned char>(text[index + offset]);
            if ((next & 0xC0U) != 0x80U) return false;
            codepoint = (codepoint << 6U) | (next & 0x3FU);
        }
        if (codepoint < minimum || codepoint > 0x10FFFFU ||
            (codepoint >= 0xD800U && codepoint <= 0xDFFFU)) {
            return false;
        }
        index += continuationCount + 1U;
    }
    return true;
}

void appendUtf8(std::string& output, std::uint32_t codepoint) {
    if (codepoint <= 0x7FU) {
        output.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FFU) {
        output.push_back(static_cast<char>(0xC0U | (codepoint >> 6U)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else if (codepoint <= 0xFFFFU) {
        output.push_back(static_cast<char>(0xE0U | (codepoint >> 12U)));
        output.push_back(
            static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else {
        output.push_back(static_cast<char>(0xF0U | (codepoint >> 18U)));
        output.push_back(
            static_cast<char>(0x80U | ((codepoint >> 12U) & 0x3FU)));
        output.push_back(
            static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    }
}

class JsonParser final {
public:
    explicit JsonParser(std::string_view source) : source_(source) {}

    bool parse(JsonValue& output, std::string& error) {
        skipWhitespace();
        if (!parseValue(output, 0)) {
            error = "particle.json:" + std::to_string(line_) + ":" +
                    std::to_string(column_) + ": " + error_;
            return false;
        }
        skipWhitespace();
        if (position_ != source_.size()) {
            fail("unexpected characters after the root value");
            error = "particle.json:" + std::to_string(line_) + ":" +
                    std::to_string(column_) + ": " + error_;
            return false;
        }
        return true;
    }

private:
    bool parseValue(JsonValue& output, std::size_t depth) {
        if (depth > kMaximumJsonDepth) return fail("JSON nesting is too deep");
        if (++nodeCount_ > kMaximumJsonNodes) {
            return fail("JSON value count exceeds the safety limit");
        }
        skipWhitespace();
        if (position_ >= source_.size()) return fail("expected a JSON value");
        const char next = source_[position_];
        if (next == '{') return parseObject(output, depth + 1);
        if (next == '[') return parseArray(output, depth + 1);
        if (next == '"') {
            output.type = JsonType::String;
            return parseString(output.string);
        }
        if (next == 't') return parseLiteral("true", JsonType::Boolean, output, true);
        if (next == 'f') return parseLiteral("false", JsonType::Boolean, output, false);
        if (next == 'n') return parseLiteral("null", JsonType::Null, output, false);
        if (next == '-' || (next >= '0' && next <= '9')) return parseNumber(output);
        return fail("unexpected character while reading a JSON value");
    }

    bool parseObject(JsonValue& output, std::size_t depth) {
        output.type = JsonType::Object;
        advance();
        skipWhitespace();
        if (consume('}')) return true;
        while (true) {
            if (output.object.size() >= kMaximumJsonObjectMembers) {
                return fail("JSON object has too many members");
            }
            if (position_ >= source_.size() || source_[position_] != '"') {
                return fail("expected an object member name");
            }
            std::string name;
            if (!parseString(name)) return false;
            for (const auto& member : output.object) {
                if (member.first == name) {
                    return fail("duplicate object member '" + name + "'");
                }
            }
            skipWhitespace();
            if (!consume(':')) return fail("expected ':' after object member name");
            JsonValue value;
            if (!parseValue(value, depth)) return false;
            output.object.emplace_back(std::move(name), std::move(value));
            skipWhitespace();
            if (consume('}')) return true;
            if (!consume(',')) return fail("expected ',' or '}' in object");
            skipWhitespace();
        }
    }

    bool parseArray(JsonValue& output, std::size_t depth) {
        output.type = JsonType::Array;
        advance();
        skipWhitespace();
        if (consume(']')) return true;
        while (true) {
            if (output.array.size() >= kMaximumJsonArrayElements) {
                return fail("JSON array has too many elements");
            }
            JsonValue value;
            if (!parseValue(value, depth)) return false;
            output.array.push_back(std::move(value));
            skipWhitespace();
            if (consume(']')) return true;
            if (!consume(',')) return fail("expected ',' or ']' in array");
            skipWhitespace();
        }
    }

    static int hexDigit(char value) noexcept {
        if (value >= '0' && value <= '9') return value - '0';
        if (value >= 'a' && value <= 'f') return value - 'a' + 10;
        if (value >= 'A' && value <= 'F') return value - 'A' + 10;
        return -1;
    }

    bool parseHex4(std::uint32_t& value) {
        value = 0;
        for (int index = 0; index < 4; ++index) {
            if (position_ >= source_.size()) return fail("incomplete Unicode escape");
            const int digit = hexDigit(source_[position_]);
            if (digit < 0) return fail("invalid Unicode escape");
            value = (value << 4U) | static_cast<std::uint32_t>(digit);
            advance();
        }
        return true;
    }

    bool parseString(std::string& output) {
        if (!consume('"')) return fail("expected a string");
        while (position_ < source_.size()) {
            const unsigned char character =
                static_cast<unsigned char>(source_[position_]);
            advance();
            if (character == '"') return true;
            if (character < 0x20U) return fail("unescaped control character in string");
            if (character != '\\') {
                output.push_back(static_cast<char>(character));
                continue;
            }
            if (position_ >= source_.size()) return fail("incomplete string escape");
            const char escaped = source_[position_];
            advance();
            switch (escaped) {
                case '"': output.push_back('"'); break;
                case '\\': output.push_back('\\'); break;
                case '/': output.push_back('/'); break;
                case 'b': output.push_back('\b'); break;
                case 'f': output.push_back('\f'); break;
                case 'n': output.push_back('\n'); break;
                case 'r': output.push_back('\r'); break;
                case 't': output.push_back('\t'); break;
                case 'u': {
                    std::uint32_t first = 0;
                    if (!parseHex4(first)) return false;
                    std::uint32_t codepoint = first;
                    if (first >= 0xD800U && first <= 0xDBFFU) {
                        if (position_ + 2 > source_.size() ||
                            source_[position_] != '\\' ||
                            source_[position_ + 1] != 'u') {
                            return fail("high surrogate without low surrogate");
                        }
                        advance();
                        advance();
                        std::uint32_t second = 0;
                        if (!parseHex4(second)) return false;
                        if (second < 0xDC00U || second > 0xDFFFU) {
                            return fail("invalid Unicode surrogate pair");
                        }
                        codepoint = 0x10000U + ((first - 0xD800U) << 10U) +
                                    (second - 0xDC00U);
                    } else if (first >= 0xDC00U && first <= 0xDFFFU) {
                        return fail("unexpected low Unicode surrogate");
                    }
                    appendUtf8(output, codepoint);
                    break;
                }
                default: return fail("invalid string escape");
            }
        }
        return fail("unterminated string");
    }

    bool parseNumber(JsonValue& output) {
        const std::size_t begin = position_;
        consume('-');
        if (consume('0')) {
            if (position_ < source_.size() && source_[position_] >= '0' &&
                source_[position_] <= '9') {
                return fail("leading zero in number");
            }
        } else {
            if (position_ >= source_.size() || source_[position_] < '1' ||
                source_[position_] > '9') return fail("invalid number");
            while (position_ < source_.size() && source_[position_] >= '0' &&
                   source_[position_] <= '9') advance();
        }
        if (consume('.')) {
            if (position_ >= source_.size() || source_[position_] < '0' ||
                source_[position_] > '9') return fail("invalid fraction");
            while (position_ < source_.size() && source_[position_] >= '0' &&
                   source_[position_] <= '9') advance();
        }
        if (position_ < source_.size() &&
            (source_[position_] == 'e' || source_[position_] == 'E')) {
            advance();
            if (position_ < source_.size() &&
                (source_[position_] == '+' || source_[position_] == '-')) advance();
            if (position_ >= source_.size() || source_[position_] < '0' ||
                source_[position_] > '9') return fail("invalid exponent");
            while (position_ < source_.size() && source_[position_] >= '0' &&
                   source_[position_] <= '9') advance();
        }
        const std::string text{source_.substr(begin, position_ - begin)};
        std::istringstream numberStream{text};
        numberStream.imbue(std::locale::classic());
        double number = 0.0;
        numberStream >> number;
        if (!numberStream || numberStream.peek() != std::char_traits<char>::eof() ||
            !std::isfinite(number)) {
            return fail("number is outside the finite range");
        }
        output.type = JsonType::Number;
        output.number = number;
        return true;
    }

    bool parseLiteral(std::string_view literal, JsonType type,
                      JsonValue& output, bool boolean) {
        if (source_.substr(position_, literal.size()) != literal) {
            return fail("invalid literal");
        }
        for (std::size_t index = 0; index < literal.size(); ++index) advance();
        output.type = type;
        output.boolean = boolean;
        return true;
    }

    bool consume(char expected) {
        if (position_ >= source_.size() || source_[position_] != expected) {
            return false;
        }
        advance();
        return true;
    }

    void skipWhitespace() {
        while (position_ < source_.size()) {
            const char character = source_[position_];
            if (character != ' ' && character != '\t' && character != '\r' &&
                character != '\n') break;
            advance();
        }
    }

    void advance() {
        if (source_[position_] == '\n') {
            ++line_;
            column_ = 1;
        } else {
            ++column_;
        }
        ++position_;
    }

    bool fail(std::string message) {
        if (error_.empty()) error_ = std::move(message);
        return false;
    }

    std::string_view source_;
    std::size_t position_ = 0;
    std::size_t line_ = 1;
    std::size_t column_ = 1;
    std::size_t nodeCount_ = 0;
    std::string error_;
};

const JsonValue* member(const JsonValue& object, std::string_view name) {
    if (object.type != JsonType::Object) return nullptr;
    for (const auto& entry : object.object) {
        if (entry.first == name) return &entry.second;
    }
    return nullptr;
}

bool expectType(const JsonValue& value, JsonType expected,
                std::string_view path, std::string& error) {
    if (value.type == expected) return true;
    error = std::string{path} + " has the wrong JSON type";
    return false;
}

bool readString(const JsonValue& object, std::string_view key,
                std::string_view path, std::string& output,
                std::string& error, bool required = false) {
    const JsonValue* value = member(object, key);
    if (!value) {
        if (required) error = std::string{path} + "." + std::string{key} + " is required";
        return !required;
    }
    if (!expectType(*value, JsonType::String,
                    std::string{path} + "." + std::string{key}, error)) return false;
    output = value->string;
    return true;
}

bool readBool(const JsonValue& object, std::string_view key,
              std::string_view path, bool& output, std::string& error) {
    const JsonValue* value = member(object, key);
    if (!value) return true;
    if (!expectType(*value, JsonType::Boolean,
                    std::string{path} + "." + std::string{key}, error)) return false;
    output = value->boolean;
    return true;
}

bool readFloat(const JsonValue& object, std::string_view key,
               std::string_view path, float& output, std::string& error) {
    const JsonValue* value = member(object, key);
    if (!value) return true;
    if (!expectType(*value, JsonType::Number,
                    std::string{path} + "." + std::string{key}, error) ||
        value->number < -std::numeric_limits<float>::max() ||
        value->number > std::numeric_limits<float>::max()) {
        if (error.empty()) error = std::string{path} + "." + std::string{key} + " is outside float range";
        return false;
    }
    output = static_cast<float>(value->number);
    return true;
}

bool readUnsigned(const JsonValue& object, std::string_view key,
                  std::string_view path, std::uint32_t& output,
                  std::string& error, bool required = false) {
    const JsonValue* value = member(object, key);
    if (!value) {
        if (required) error = std::string{path} + "." + std::string{key} + " is required";
        return !required;
    }
    if (!expectType(*value, JsonType::Number,
                    std::string{path} + "." + std::string{key}, error) ||
        value->number < 0.0 ||
        value->number > std::numeric_limits<std::uint32_t>::max() ||
        std::floor(value->number) != value->number) {
        if (error.empty()) error = std::string{path} + "." + std::string{key} + " must be an unsigned integer";
        return false;
    }
    output = static_cast<std::uint32_t>(value->number);
    return true;
}

bool readVec2Value(const JsonValue& value, Vec2& output,
                   std::string_view path, std::string& error) {
    if (!expectType(value, JsonType::Array, path, error) || value.array.size() != 2) {
        if (error.empty()) error = std::string{path} + " must be an array of 2 numbers";
        return false;
    }
    for (const auto& item : value.array) {
        if (item.type != JsonType::Number ||
            std::abs(item.number) > std::numeric_limits<float>::max()) {
            error = std::string{path} + " must contain finite float values";
            return false;
        }
    }
    output = {static_cast<float>(value.array[0].number),
              static_cast<float>(value.array[1].number)};
    return true;
}

bool readVec3Value(const JsonValue& value, Vec3& output,
                   std::string_view path, std::string& error) {
    if (!expectType(value, JsonType::Array, path, error) || value.array.size() != 3) {
        if (error.empty()) error = std::string{path} + " must be an array of 3 numbers";
        return false;
    }
    for (const auto& item : value.array) {
        if (item.type != JsonType::Number ||
            std::abs(item.number) > std::numeric_limits<float>::max()) {
            error = std::string{path} + " must contain finite float values";
            return false;
        }
    }
    output = {static_cast<float>(value.array[0].number),
              static_cast<float>(value.array[1].number),
              static_cast<float>(value.array[2].number)};
    return true;
}

bool readColorValue(const JsonValue& value, Color& output,
                    std::string_view path, std::string& error) {
    if (!expectType(value, JsonType::Array, path, error) || value.array.size() != 4) {
        if (error.empty()) error = std::string{path} + " must be an array of 4 numbers";
        return false;
    }
    for (const auto& item : value.array) {
        if (item.type != JsonType::Number ||
            std::abs(item.number) > std::numeric_limits<float>::max()) {
            error = std::string{path} + " must contain finite float values";
            return false;
        }
    }
    output = {static_cast<float>(value.array[0].number),
              static_cast<float>(value.array[1].number),
              static_cast<float>(value.array[2].number),
              static_cast<float>(value.array[3].number)};
    return true;
}

bool readVec3(const JsonValue& object, std::string_view key,
              std::string_view path, Vec3& output, std::string& error) {
    const JsonValue* value = member(object, key);
    return !value || readVec3Value(*value, output,
                                  std::string{path} + "." + std::string{key}, error);
}

bool readColor(const JsonValue& object, std::string_view key,
               std::string_view path, Color& output, std::string& error) {
    const JsonValue* value = member(object, key);
    return !value || readColorValue(*value, output,
                                    std::string{path} + "." + std::string{key}, error);
}

bool parseDistribution(std::string_view value, RandomDistribution& output) {
    if (value == "uniform") output = RandomDistribution::Uniform;
    else if (value == "normal") output = RandomDistribution::Normal;
    else return false;
    return true;
}

bool readDistribution(const JsonValue& object, std::string_view key,
                      std::string_view path, RandomDistribution& output,
                      std::string& error) {
    std::string text;
    if (!readString(object, key, path, text, error)) return false;
    if (!member(object, key)) return true;
    if (!parseDistribution(text, output)) {
        error = std::string{path} + "." + std::string{key} +
                " must be 'uniform' or 'normal'";
        return false;
    }
    return true;
}

bool readFloatRange(const JsonValue& object, std::string_view key,
                    std::string_view path, FloatRange& output,
                    std::string& error) {
    const JsonValue* value = member(object, key);
    if (!value) return true;
    const std::string child = std::string{path} + "." + std::string{key};
    if (!expectType(*value, JsonType::Object, child, error)) return false;
    return readFloat(*value, "minimum", child, output.minimum, error) &&
           readFloat(*value, "maximum", child, output.maximum, error) &&
           readDistribution(*value, "distribution", child,
                            output.distribution, error);
}

bool readVec3Range(const JsonValue& object, std::string_view key,
                   std::string_view path, Vec3Range& output,
                   std::string& error) {
    const JsonValue* value = member(object, key);
    if (!value) return true;
    const std::string child = std::string{path} + "." + std::string{key};
    if (!expectType(*value, JsonType::Object, child, error)) return false;
    return readVec3(*value, "minimum", child, output.minimum, error) &&
           readVec3(*value, "maximum", child, output.maximum, error) &&
           readDistribution(*value, "distribution", child,
                            output.distribution, error);
}

template <typename Curve, typename ValueReader>
bool readCurve(const JsonValue& object, std::string_view key,
               std::string_view path, Curve& output, ValueReader readValue,
               std::string& error) {
    const JsonValue* value = member(object, key);
    if (!value) return true;
    const std::string child = std::string{path} + "." + std::string{key};
    if (!expectType(*value, JsonType::Array, child, error) ||
        value->array.size() > kMaxCurveKeys) {
        if (error.empty()) error = child + " exceeds the curve key limit";
        return false;
    }
    output.count = static_cast<std::uint32_t>(value->array.size());
    for (std::size_t index = 0; index < value->array.size(); ++index) {
        const auto& item = value->array[index];
        const std::string itemPath = child + "[" + std::to_string(index) + "]";
        if (!expectType(item, JsonType::Object, itemPath, error) ||
            !readFloat(item, "time", itemPath, output.keys[index].time, error)) return false;
        const JsonValue* keyValue = member(item, "value");
        if (!keyValue) {
            error = itemPath + ".value is required";
            return false;
        }
        if (!readValue(*keyValue, output.keys[index].value,
                       itemPath + ".value", error)) return false;
    }
    return true;
}

bool readDefinition(const JsonValue& root, ParticleSystemDefinition& output,
                    std::string& error) {
    if (!expectType(root, JsonType::Object, "$", error)) return false;
    output = makeDefaultParticleSystem();
    if (!readUnsigned(root, "version", "$", output.version, error, true) ||
        !readString(root, "name", "$", output.name, error, true)) return false;

    if (const JsonValue* material = member(root, "material")) {
        if (!expectType(*material, JsonType::Object, "$.material", error)) return false;
        std::string blend;
        if (!readString(*material, "baseTexture", "$.material",
                        output.material.baseTexture, error) ||
            !readString(*material, "normalTexture", "$.material",
                        output.material.normalTexture, error) ||
            !readString(*material, "emissiveTexture", "$.material",
                        output.material.emissiveTexture, error) ||
            !readColor(*material, "tint", "$.material", output.material.tint, error) ||
            !readColor(*material, "emissive", "$.material", output.material.emissive, error) ||
            !readFloat(*material, "opacity", "$.material", output.material.opacity, error) ||
            !readFloat(*material, "alphaCutoff", "$.material", output.material.alphaCutoff, error) ||
            !readFloat(*material, "roughness", "$.material", output.material.roughness, error) ||
            !readFloat(*material, "metallic", "$.material", output.material.metallic, error) ||
            !readString(*material, "blendMode", "$.material", blend, error)) return false;
        if (member(*material, "blendMode")) {
            if (blend == "premultipliedAlpha") {
                output.material.blendMode = ParticleBlendMode::PremultipliedAlpha;
            } else if (blend == "additive") {
                output.material.blendMode = ParticleBlendMode::Additive;
            } else {
                error = "$.material.blendMode must be 'premultipliedAlpha' or 'additive'";
                return false;
            }
        }
    }

    const JsonValue* emitter = member(root, "emitter");
    if (!emitter || !expectType(*emitter, JsonType::Object, "$.emitter", error)) {
        if (!emitter) error = "$.emitter is required";
        return false;
    }
    auto& target = output.emitter;
    std::string shape;
    std::string simulationSpace;
    if (!readUnsigned(*emitter, "maxParticles", "$.emitter", target.maxParticles, error) ||
        !readFloat(*emitter, "emissionRate", "$.emitter", target.emissionRate, error) ||
        !readFloat(*emitter, "duration", "$.emitter", target.duration, error) ||
        !readBool(*emitter, "looping", "$.emitter", target.looping, error) ||
        !readString(*emitter, "simulationSpace", "$.emitter",
                    simulationSpace, error) ||
        !readFloat(*emitter, "inheritVelocity", "$.emitter",
                   target.inheritVelocity, error) ||
        !readString(*emitter, "shape", "$.emitter", shape, error) ||
        !readVec3(*emitter, "shapeExtents", "$.emitter", target.shapeExtents, error) ||
        !readFloat(*emitter, "shapeRadius", "$.emitter", target.shapeRadius, error) ||
        !readDistribution(*emitter, "shapeDistribution", "$.emitter",
                          target.shapeDistribution, error) ||
        !readFloatRange(*emitter, "lifetime", "$.emitter", target.lifetime, error) ||
        !readVec3Range(*emitter, "position", "$.emitter", target.position, error) ||
        !readVec3Range(*emitter, "velocity", "$.emitter", target.velocity, error) ||
        !readVec3Range(*emitter, "acceleration", "$.emitter", target.acceleration, error) ||
        !readVec3(*emitter, "gravity", "$.emitter", target.gravity, error) ||
        !readFloat(*emitter, "drag", "$.emitter", target.drag, error) ||
        !readFloatRange(*emitter, "sizeMultiplier", "$.emitter", target.sizeMultiplier, error) ||
        !readFloatRange(*emitter, "rotationOffset", "$.emitter", target.rotationOffset, error)) return false;
    if (member(*emitter, "simulationSpace")) {
        if (simulationSpace == "world") {
            target.simulationSpace = SimulationSpace::World;
        } else if (simulationSpace == "local") {
            target.simulationSpace = SimulationSpace::Local;
        } else {
            error = "$.emitter.simulationSpace must be 'world' or 'local'";
            return false;
        }
    }
    if (member(*emitter, "shape")) {
        if (shape == "point") target.shape = EmitterShape::Point;
        else if (shape == "box") target.shape = EmitterShape::Box;
        else if (shape == "circle") target.shape = EmitterShape::Circle;
        else {
            error = "$.emitter.shape must be 'point', 'box' or 'circle'";
            return false;
        }
    }
    if (!readCurve(*emitter, "sizeCurve", "$.emitter", target.size,
                   readVec2Value, error) ||
        !readCurve(*emitter, "colorCurve", "$.emitter", target.color,
                   readColorValue, error) ||
        !readCurve(*emitter, "rotationCurve", "$.emitter", target.rotation,
                   [](const JsonValue& value, float& result,
                      std::string_view path, std::string& localError) {
                       if (!expectType(value, JsonType::Number, path, localError) ||
                           std::abs(value.number) > std::numeric_limits<float>::max()) return false;
                       result = static_cast<float>(value.number);
                       return true;
                   }, error)) return false;

    if (const JsonValue* bursts = member(*emitter, "bursts")) {
        if (!expectType(*bursts, JsonType::Array, "$.emitter.bursts", error)) return false;
        if (bursts->array.size() > kMaximumParticleBursts) {
            error = "$.emitter.bursts exceeds the burst limit";
            return false;
        }
        target.bursts.clear();
        target.bursts.reserve(bursts->array.size());
        for (std::size_t index = 0; index < bursts->array.size(); ++index) {
            const auto& item = bursts->array[index];
            const std::string path = "$.emitter.bursts[" + std::to_string(index) + "]";
            if (!expectType(item, JsonType::Object, path, error)) return false;
            Burst burst;
            if (!readFloat(item, "time", path, burst.time, error) ||
                !readUnsigned(item, "countMinimum", path, burst.countMinimum, error) ||
                !readUnsigned(item, "countMaximum", path, burst.countMaximum, error) ||
                !readUnsigned(item, "cycles", path, burst.cycles, error) ||
                !readFloat(item, "interval", path, burst.interval, error)) return false;
            target.bursts.push_back(burst);
        }
    }
    return validateParticleSystem(output, error);
}

std::string escaped(std::string_view input) {
    std::ostringstream output;
    output << '"';
    for (const unsigned char character : input) {
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
                    output << "\\u" << std::hex << std::setw(4)
                           << std::setfill('0') << static_cast<unsigned>(character)
                           << std::dec << std::setfill(' ');
                } else {
                    output << static_cast<char>(character);
                }
        }
    }
    output << '"';
    return output.str();
}

const char* distributionName(RandomDistribution value) noexcept {
    return value == RandomDistribution::Normal ? "normal" : "uniform";
}

void writeVec2(std::ostream& output, Vec2 value) {
    output << '[' << value.x << ", " << value.y << ']';
}

void writeVec3(std::ostream& output, Vec3 value) {
    output << '[' << value.x << ", " << value.y << ", " << value.z << ']';
}

void writeColor(std::ostream& output, Color value) {
    output << '[' << value.r << ", " << value.g << ", " << value.b << ", "
           << value.a << ']';
}

void writeFloatRange(std::ostream& output, const FloatRange& value) {
    output << "{\"minimum\": " << value.minimum << ", \"maximum\": "
           << value.maximum << ", \"distribution\": \""
           << distributionName(value.distribution) << "\"}";
}

void writeVec3Range(std::ostream& output, const Vec3Range& value) {
    output << "{\"minimum\": ";
    writeVec3(output, value.minimum);
    output << ", \"maximum\": ";
    writeVec3(output, value.maximum);
    output << ", \"distribution\": \"" << distributionName(value.distribution)
           << "\"}";
}

template <typename Curve, typename ValueWriter>
void writeCurve(std::ostream& output, const Curve& curve,
                ValueWriter writeValue, std::string_view indent) {
    output << "[";
    for (std::uint32_t index = 0; index < curve.count; ++index) {
        if (index != 0) output << ',';
        output << "\n" << indent << "  {\"time\": " << curve.keys[index].time
               << ", \"value\": ";
        writeValue(output, curve.keys[index].value);
        output << '}';
    }
    if (curve.count != 0) output << "\n" << indent;
    output << ']';
}

bool writeDefinition(std::ostream& output,
                     const ParticleSystemDefinition& definition) {
    const auto& material = definition.material;
    const auto& emitter = definition.emitter;
    const char* shape = emitter.shape == EmitterShape::Box
                            ? "box"
                            : (emitter.shape == EmitterShape::Circle ? "circle"
                                                                     : "point");
    output.imbue(std::locale::classic());
    output << std::setprecision(std::numeric_limits<float>::max_digits10);
    output << "{\n  \"version\": " << definition.version
           << ",\n  \"name\": " << escaped(definition.name)
           << ",\n  \"material\": {\n"
           << "    \"baseTexture\": " << escaped(material.baseTexture) << ",\n"
           << "    \"normalTexture\": " << escaped(material.normalTexture) << ",\n"
           << "    \"emissiveTexture\": " << escaped(material.emissiveTexture) << ",\n"
           << "    \"tint\": ";
    writeColor(output, material.tint);
    output << ",\n    \"emissive\": ";
    writeColor(output, material.emissive);
    output << ",\n    \"opacity\": " << material.opacity
           << ",\n    \"alphaCutoff\": " << material.alphaCutoff
           << ",\n    \"roughness\": " << material.roughness
           << ",\n    \"metallic\": " << material.metallic
           << ",\n    \"blendMode\": \""
           << (material.blendMode == ParticleBlendMode::Additive
                   ? "additive"
                   : "premultipliedAlpha")
           << "\"\n  },\n  \"emitter\": {\n"
           << "    \"maxParticles\": " << emitter.maxParticles
           << ",\n    \"emissionRate\": " << emitter.emissionRate
           << ",\n    \"duration\": " << emitter.duration
           << ",\n    \"looping\": " << (emitter.looping ? "true" : "false")
           << ",\n    \"simulationSpace\": \""
           << (emitter.simulationSpace == SimulationSpace::Local
                   ? "local"
                   : "world")
           << "\""
           << ",\n    \"inheritVelocity\": " << emitter.inheritVelocity
           << ",\n    \"shape\": \"" << shape << "\",\n"
           << "    \"shapeExtents\": ";
    writeVec3(output, emitter.shapeExtents);
    output << ",\n    \"shapeRadius\": " << emitter.shapeRadius
           << ",\n    \"shapeDistribution\": \""
           << distributionName(emitter.shapeDistribution) << "\",\n"
           << "    \"lifetime\": ";
    writeFloatRange(output, emitter.lifetime);
    output << ",\n    \"position\": ";
    writeVec3Range(output, emitter.position);
    output << ",\n    \"velocity\": ";
    writeVec3Range(output, emitter.velocity);
    output << ",\n    \"acceleration\": ";
    writeVec3Range(output, emitter.acceleration);
    output << ",\n    \"gravity\": ";
    writeVec3(output, emitter.gravity);
    output << ",\n    \"drag\": " << emitter.drag
           << ",\n    \"sizeMultiplier\": ";
    writeFloatRange(output, emitter.sizeMultiplier);
    output << ",\n    \"rotationOffset\": ";
    writeFloatRange(output, emitter.rotationOffset);
    output << ",\n    \"sizeCurve\": ";
    writeCurve(output, emitter.size, writeVec2, "    ");
    output << ",\n    \"colorCurve\": ";
    writeCurve(output, emitter.color, writeColor, "    ");
    output << ",\n    \"rotationCurve\": ";
    writeCurve(output, emitter.rotation,
               [](std::ostream& stream, float value) { stream << value; },
               "    ");
    output << ",\n    \"bursts\": [";
    for (std::size_t index = 0; index < emitter.bursts.size(); ++index) {
        const auto& burst = emitter.bursts[index];
        if (index != 0) output << ',';
        output << "\n      {\"time\": " << burst.time
               << ", \"countMinimum\": " << burst.countMinimum
               << ", \"countMaximum\": " << burst.countMaximum
               << ", \"cycles\": " << burst.cycles
               << ", \"interval\": " << burst.interval << '}';
    }
    if (!emitter.bursts.empty()) output << "\n    ";
    output << "]\n  }\n}\n";
    return output.good();
}

}  // namespace

bool loadParticleSystem(const std::filesystem::path& directory,
                        ParticleSystemDefinition& definition,
                        std::string& error) {
    error.clear();
    const std::filesystem::path manifest = directory / "particle.json";
    std::error_code filesystemError;
    const std::uintmax_t size = std::filesystem::file_size(manifest, filesystemError);
    if (filesystemError) {
        error = "cannot inspect particle manifest '" + manifest.u8string() +
                "': " + filesystemError.message();
        return false;
    }
    if (size > kMaximumManifestBytes) {
        error = "particle manifest exceeds the 4 MiB size limit";
        return false;
    }
    std::ifstream input(manifest, std::ios::binary);
    if (!input) {
        error = "cannot open particle manifest '" + manifest.u8string() + "'";
        return false;
    }
    std::string source(static_cast<std::size_t>(size), '\0');
    if (!source.empty()) input.read(source.data(), static_cast<std::streamsize>(source.size()));
    if (!input && !source.empty()) {
        error = "cannot read complete particle manifest '" + manifest.u8string() + "'";
        return false;
    }
    if (!validUtf8(source)) {
        error = "particle manifest is not valid UTF-8";
        return false;
    }
    JsonValue root;
    JsonParser parser(source);
    ParticleSystemDefinition parsed;
    if (!parser.parse(root, error) || !readDefinition(root, parsed, error)) return false;
    definition = std::move(parsed);
    return true;
}

bool saveParticleSystem(const std::filesystem::path& directory,
                        const ParticleSystemDefinition& definition,
                        std::string& error) {
    if (!validateParticleSystem(definition, error)) return false;
    std::error_code filesystemError;
    std::filesystem::create_directories(directory, filesystemError);
    if (filesystemError) {
        error = "cannot create particle-system directory: " + filesystemError.message();
        return false;
    }
    const auto manifest = directory / "particle.json";
    const auto temporary = directory / "particle.json.tmp";
    const auto backup = directory / "particle.json.bak";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output || !writeDefinition(output, definition)) {
            error = "cannot write temporary particle manifest '" + temporary.u8string() + "'";
            return false;
        }
        output.flush();
        if (!output) {
            error = "cannot flush temporary particle manifest '" + temporary.u8string() + "'";
            return false;
        }
    }

    const bool hadManifest = std::filesystem::exists(manifest, filesystemError);
    if (filesystemError) {
        error = "cannot inspect existing particle manifest: " + filesystemError.message();
        return false;
    }
    if (hadManifest) {
        std::filesystem::remove(backup, filesystemError);
        filesystemError.clear();
        std::filesystem::rename(manifest, backup, filesystemError);
        if (filesystemError) {
            error = "cannot preserve existing particle manifest: " + filesystemError.message();
            return false;
        }
    }
    std::filesystem::rename(temporary, manifest, filesystemError);
    if (filesystemError) {
        if (hadManifest) {
            std::error_code restoreError;
            std::filesystem::rename(backup, manifest, restoreError);
        }
        error = "cannot install particle manifest: " + filesystemError.message();
        return false;
    }
    if (hadManifest) {
        std::filesystem::remove(backup, filesystemError);
    }
    error.clear();
    return true;
}

}  // namespace newaoe::render::particle
