#pragma once

#include <array>
#include <cstdint>
#include <limits>

namespace newaoe::render {

struct Vec2 final {
    float x = 0.0F;
    float y = 0.0F;
};

struct Vec3 final {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
};

// Instance transform for 2D particle systems. Position is in world units,
// rotation is in radians, and scale is component-wise. Callers should update
// it before BeginFrame so births during that frame can be distributed along
// the emitter's complete motion segment.
struct ParticleTransform2D final {
    Vec2 position{};
    float rotation = 0.0F;
    Vec2 scale{1.0F, 1.0F};
};

struct Color final {
    float r = 1.0F;
    float g = 1.0F;
    float b = 1.0F;
    float a = 1.0F;
};

// Absolute world-space destination rectangle.  The complete source image is
// mapped to this rectangle; individual images intentionally have no rotation.
struct WorldRect final {
    float x = 0.0F;
    float y = 0.0F;
    float width = 0.0F;
    float height = 0.0F;
};

struct TextureHandle final {
    static constexpr std::uint32_t InvalidIndex =
        std::numeric_limits<std::uint32_t>::max();

    std::uint32_t index = InvalidIndex;
    std::uint32_t generation = 0;

    [[nodiscard]] constexpr bool valid() const noexcept {
        return index != InvalidIndex && generation != 0;
    }

    friend constexpr bool operator==(TextureHandle lhs,
                                     TextureHandle rhs) noexcept {
        return lhs.index == rhs.index && lhs.generation == rhs.generation;
    }
    friend constexpr bool operator!=(TextureHandle lhs,
                                     TextureHandle rhs) noexcept {
        return !(lhs == rhs);
    }
};

struct ParticleSystemHandle final {
    static constexpr std::uint32_t InvalidIndex =
        std::numeric_limits<std::uint32_t>::max();

    std::uint32_t index = InvalidIndex;
    std::uint32_t generation = 0;

    [[nodiscard]] constexpr bool valid() const noexcept {
        return index != InvalidIndex && generation != 0;
    }

    friend constexpr bool operator==(ParticleSystemHandle lhs,
                                     ParticleSystemHandle rhs) noexcept {
        return lhs.index == rhs.index && lhs.generation == rhs.generation;
    }
};

enum class ParticleBlendMode : std::uint8_t {
    PremultipliedAlpha,
    Additive,
};

// Optional sprite material.  DrawImage(texture, rect) uses these defaults.
// Alpha is a cutout test only: surviving image fragments are fully opaque.
struct SpriteMaterial final {
    TextureHandle normalTexture{};
    TextureHandle emissiveTexture{};
    Color tint{};
    Color emissiveColor{0.0F, 0.0F, 0.0F, 1.0F};
    float roughness = 1.0F;
    float metallic = 0.0F;
    float alphaCutoff = 0.0F;
};

struct DirectionalLight final {
    Vec3 direction{-0.35F, 0.45F, -0.82F};
    Color color{1.0F, 0.98F, 0.92F, 1.0F};
    float intensity = 1.0F;
    float ambient = 0.18F;
};

struct PostProcessSettings final {
    bool enabled = true;
    bool fxaa = true;
    bool bloom = true;
    float exposure = 1.0F;
    float bloomThreshold = 1.0F;
    float bloomIntensity = 0.15F;
};

struct RenderCapabilities final {
    // False means the portable RGBA8 fallback is active.  Rendering remains
    // functional, but values above one are clamped before bloom extraction.
    bool hdrScene = false;
    bool hdrEmissiveGBuffer = false;
};

using Matrix4 = std::array<float, 16>;

}  // namespace newaoe::render
