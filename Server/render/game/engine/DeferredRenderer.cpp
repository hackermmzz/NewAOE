#include "Server/render/game/engine/DeferredRenderer.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>

#include "Server/render/game/engine/RenderEngine.h"
#include "Server/render/game/engine/shaders/engine.glsl.h"

namespace newaoe::render::detail {
namespace {

template <typename Handle>
bool valid(Handle handle) noexcept {
    return handle.id != 0;
}

struct QuadVertex final {
    float position[2];
    float uv[2];
};

constexpr std::array<QuadVertex, 6> kQuadVertices{{
    {{0.0F, 0.0F}, {0.0F, 0.0F}},
    {{1.0F, 0.0F}, {1.0F, 0.0F}},
    {{1.0F, 1.0F}, {1.0F, 1.0F}},
    {{0.0F, 0.0F}, {0.0F, 0.0F}},
    {{1.0F, 1.0F}, {1.0F, 1.0F}},
    {{0.0F, 1.0F}, {0.0F, 1.0F}},
}};

template <typename Uniform>
void applyUniforms(int slot, const Uniform& uniform) {
    const sg_range range{&uniform, sizeof(uniform)};
    sg_apply_uniforms(slot, &range);
}

bool makeColorTarget(int width, int height, sg_pixel_format format,
                     DeferredRenderer::ColorTarget& target,
                     const char* label, std::string& error) {
    sg_image_desc imageDescriptor{};
    imageDescriptor.usage.color_attachment = true;
    imageDescriptor.width = width;
    imageDescriptor.height = height;
    imageDescriptor.pixel_format = format;
    imageDescriptor.sample_count = 1;
    imageDescriptor.label = label;
    target.image = sg_make_image(&imageDescriptor);
    if (sg_query_image_state(target.image) != SG_RESOURCESTATE_VALID) {
        error = std::string("failed to create render target: ") + label;
        return false;
    }

    sg_view_desc attachmentDescriptor{};
    attachmentDescriptor.color_attachment.image = target.image;
    attachmentDescriptor.label = label;
    target.attachment = sg_make_view(&attachmentDescriptor);

    sg_view_desc textureDescriptor{};
    textureDescriptor.texture.image = target.image;
    textureDescriptor.label = label;
    target.texture = sg_make_view(&textureDescriptor);
    if (sg_query_view_state(target.attachment) != SG_RESOURCESTATE_VALID ||
        sg_query_view_state(target.texture) != SG_RESOURCESTATE_VALID) {
        error = std::string("failed to create render-target views: ") + label;
        return false;
    }
    return true;
}

bool makeDepthTarget(int width, int height,
                     DeferredRenderer::DepthTarget& target,
                     std::string& error) {
    sg_image_desc imageDescriptor{};
    imageDescriptor.usage.depth_stencil_attachment = true;
    imageDescriptor.width = width;
    imageDescriptor.height = height;
    imageDescriptor.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    imageDescriptor.sample_count = 1;
    imageDescriptor.label = "NewAOE GBuffer depth";
    target.image = sg_make_image(&imageDescriptor);
    if (sg_query_image_state(target.image) != SG_RESOURCESTATE_VALID) {
        error = "failed to create the GBuffer depth target";
        return false;
    }

    sg_view_desc descriptor{};
    descriptor.depth_stencil_attachment.image = target.image;
    descriptor.label = "NewAOE GBuffer depth attachment";
    target.attachment = sg_make_view(&descriptor);
    if (sg_query_view_state(target.attachment) != SG_RESOURCESTATE_VALID) {
        error = "failed to create the GBuffer depth view";
        return false;
    }
    return true;
}

void destroyColorTarget(DeferredRenderer::ColorTarget& target) noexcept {
    if (valid(target.texture)) sg_destroy_view(target.texture);
    if (valid(target.attachment)) sg_destroy_view(target.attachment);
    if (valid(target.image)) sg_destroy_image(target.image);
    target = {};
}

void destroyDepthTarget(DeferredRenderer::DepthTarget& target) noexcept {
    if (valid(target.attachment)) sg_destroy_view(target.attachment);
    if (valid(target.image)) sg_destroy_image(target.image);
    target = {};
}

bool resourceValid(sg_shader shader) {
    return sg_query_shader_state(shader) == SG_RESOURCESTATE_VALID;
}

bool resourceValid(sg_pipeline pipeline) {
    return sg_query_pipeline_state(pipeline) == SG_RESOURCESTATE_VALID;
}

std::size_t nextCapacity(std::size_t requested) {
    std::size_t result = 1024;
    while (result < requested &&
           result <= std::numeric_limits<std::size_t>::max() / 2U) {
        result *= 2U;
    }
    return std::max(result, requested);
}

void configureQuadLayout(sg_pipeline_desc& descriptor,
                         bool spriteLayout) {
    descriptor.layout.buffers[0].stride = sizeof(QuadVertex);
    descriptor.layout.buffers[0].step_func = SG_VERTEXSTEP_PER_VERTEX;
    descriptor.layout.attrs[0].buffer_index = 0;
    descriptor.layout.attrs[0].offset = 0;
    descriptor.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
    descriptor.layout.attrs[1].buffer_index = 0;
    descriptor.layout.attrs[1].offset = 2 * sizeof(float);
    descriptor.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;

    descriptor.layout.buffers[1].stride = 4 * 4 * sizeof(float);
    descriptor.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;
    descriptor.layout.buffers[1].step_rate = 1;
    for (int attribute = 2; attribute <= 5; ++attribute) {
        descriptor.layout.attrs[attribute].buffer_index = 1;
        descriptor.layout.attrs[attribute].offset =
            (attribute - 2) * 4 * static_cast<int>(sizeof(float));
        descriptor.layout.attrs[attribute].format = SG_VERTEXFORMAT_FLOAT4;
    }
    (void)spriteLayout;
}

sg_pipeline makeFullscreenPipeline(sg_shader shader,
                                   sg_pixel_format colorFormat,
                                   sg_pixel_format depthFormat,
                                   const char* label) {
    sg_pipeline_desc descriptor{};
    descriptor.shader = shader;
    descriptor.color_count = 1;
    descriptor.colors[0].pixel_format = colorFormat;
    descriptor.depth.pixel_format = depthFormat;
    descriptor.depth.compare = SG_COMPAREFUNC_ALWAYS;
    descriptor.depth.write_enabled = false;
    descriptor.sample_count = 1;
    descriptor.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    descriptor.label = label;
    return sg_make_pipeline(&descriptor);
}

sg_pipeline makeSwapchainFullscreenPipeline(sg_shader shader,
                                              const char* label) {
    sg_pipeline_desc descriptor{};
    descriptor.shader = shader;
    descriptor.color_count = 1;
    // Leave the attachment formats default-initialized so Sokol resolves
    // them from sg_desc.environment.defaults for the current swapchain.
    descriptor.depth.compare = SG_COMPAREFUNC_ALWAYS;
    descriptor.depth.write_enabled = false;
    descriptor.sample_count = 1;
    descriptor.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    descriptor.label = label;
    return sg_make_pipeline(&descriptor);
}

}  // namespace

bool DeferredRenderer::initialize(std::string& error) {
    error.clear();
    if (valid(quadBuffer_)) return true;

    const sg_pixelformat_info hdr = sg_query_pixelformat(SG_PIXELFORMAT_RGBA16F);
    sceneFormat_ = hdr.render && hdr.sample && hdr.filter && hdr.blend
                       ? SG_PIXELFORMAT_RGBA16F
                       : SG_PIXELFORMAT_RGBA8;
    emissiveFormat_ = hdr.render && hdr.sample && hdr.filter
                          ? SG_PIXELFORMAT_RGBA16F
                          : SG_PIXELFORMAT_RGBA8;

    sg_buffer_desc quadDescriptor{};
    quadDescriptor.usage.vertex_buffer = true;
    quadDescriptor.data = {kQuadVertices.data(), sizeof(kQuadVertices)};
    quadDescriptor.label = "NewAOE unit quad";
    quadBuffer_ = sg_make_buffer(&quadDescriptor);
    if (sg_query_buffer_state(quadBuffer_) != SG_RESOURCESTATE_VALID) {
        error = "failed to create the shared unit-quad buffer";
        shutdown();
        return false;
    }

    sg_sampler_desc linearDescriptor{};
    linearDescriptor.min_filter = SG_FILTER_LINEAR;
    linearDescriptor.mag_filter = SG_FILTER_LINEAR;
    linearDescriptor.mipmap_filter = SG_FILTER_NEAREST;
    linearDescriptor.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    linearDescriptor.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    linearDescriptor.label = "NewAOE render-target sampler";
    linearSampler_ = sg_make_sampler(&linearDescriptor);

    sg_sampler_desc curveDescriptor{};
    curveDescriptor.min_filter = SG_FILTER_NEAREST;
    curveDescriptor.mag_filter = SG_FILTER_NEAREST;
    curveDescriptor.mipmap_filter = SG_FILTER_NEAREST;
    curveDescriptor.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    curveDescriptor.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    curveDescriptor.label = "NewAOE unfilterable curve sampler";
    curveSampler_ = sg_make_sampler(&curveDescriptor);
    if (sg_query_sampler_state(linearSampler_) != SG_RESOURCESTATE_VALID ||
        sg_query_sampler_state(curveSampler_) != SG_RESOURCESTATE_VALID) {
        error = "failed to create renderer samplers";
        shutdown();
        return false;
    }

    if (!createPipelines(error) || !ensureSpriteCapacity(1024, error)) {
        shutdown();
        return false;
    }
    return true;
}

void DeferredRenderer::shutdown() noexcept {
    destroyTargets();
    destroyPipelines();
    if (valid(spriteInstanceBuffer_))
        sg_destroy_buffer(spriteInstanceBuffer_);
    if (valid(quadBuffer_)) sg_destroy_buffer(quadBuffer_);
    if (valid(linearSampler_)) sg_destroy_sampler(linearSampler_);
    if (valid(curveSampler_)) sg_destroy_sampler(curveSampler_);
    quadBuffer_ = {};
    spriteInstanceBuffer_ = {};
    linearSampler_ = {};
    curveSampler_ = {};
    spriteCapacity_ = 0;
    width_ = 0;
    height_ = 0;
}

bool DeferredRenderer::resize(int width, int height, std::string& error) {
    width = std::max(width, 1);
    height = std::max(height, 1);
    if (width == width_ && height == height_) return true;
    destroyTargets();
    return createTargets(width, height, error);
}

bool DeferredRenderer::createPipelines(std::string& error) {
    spriteShader_ = sg_make_shader(
        aoe_engine_sprite_gbuffer_shader_desc(sg_query_backend()));
    lightingShader_ = sg_make_shader(
        aoe_engine_deferred_lighting_shader_desc(sg_query_backend()));
    particleShader_ =
        sg_make_shader(aoe_engine_particle_shader_desc(sg_query_backend()));
    bloomExtractShader_ = sg_make_shader(
        aoe_engine_bloom_extract_shader_desc(sg_query_backend()));
    bloomBlurShader_ = sg_make_shader(
        aoe_engine_bloom_blur_shader_desc(sg_query_backend()));
    postShader_ =
        sg_make_shader(aoe_engine_postprocess_shader_desc(sg_query_backend()));
    if (!resourceValid(spriteShader_) || !resourceValid(lightingShader_) ||
        !resourceValid(particleShader_) ||
        !resourceValid(bloomExtractShader_) ||
        !resourceValid(bloomBlurShader_) || !resourceValid(postShader_)) {
        error = "failed to create one or more portable renderer shaders";
        return false;
    }

    sg_pipeline_desc spriteDescriptor{};
    spriteDescriptor.shader = spriteShader_;
    configureQuadLayout(spriteDescriptor, true);
    spriteDescriptor.depth.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    spriteDescriptor.depth.compare = SG_COMPAREFUNC_LESS;
    spriteDescriptor.depth.write_enabled = true;
    spriteDescriptor.color_count = 3;
    spriteDescriptor.colors[0].pixel_format = SG_PIXELFORMAT_RGBA8;
    spriteDescriptor.colors[1].pixel_format = SG_PIXELFORMAT_RGBA8;
    spriteDescriptor.colors[2].pixel_format = emissiveFormat_;
    spriteDescriptor.sample_count = 1;
    spriteDescriptor.label = "NewAOE sprite GBuffer pipeline";
    spritePipeline_ = sg_make_pipeline(&spriteDescriptor);

    lightingPipeline_ = makeFullscreenPipeline(
        lightingShader_, sceneFormat_, SG_PIXELFORMAT_NONE,
        "NewAOE deferred-lighting pipeline");
    bloomExtractPipeline_ = makeFullscreenPipeline(
        bloomExtractShader_, sceneFormat_, SG_PIXELFORMAT_NONE,
        "NewAOE bloom-extract pipeline");
    bloomBlurPipeline_ = makeFullscreenPipeline(
        bloomBlurShader_, sceneFormat_, SG_PIXELFORMAT_NONE,
        "NewAOE bloom-blur pipeline");
    postPipeline_ = makeSwapchainFullscreenPipeline(
        postShader_, "NewAOE post-process pipeline");

    sg_pipeline_desc particleDescriptor{};
    particleDescriptor.shader = particleShader_;
    configureQuadLayout(particleDescriptor, false);
    particleDescriptor.depth.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    particleDescriptor.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    particleDescriptor.depth.write_enabled = false;
    particleDescriptor.color_count = 1;
    particleDescriptor.colors[0].pixel_format = sceneFormat_;
    particleDescriptor.colors[0].blend.enabled = true;
    particleDescriptor.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_ONE;
    particleDescriptor.colors[0].blend.dst_factor_rgb =
        SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    particleDescriptor.colors[0].blend.op_rgb = SG_BLENDOP_ADD;
    particleDescriptor.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
    particleDescriptor.colors[0].blend.dst_factor_alpha =
        SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    particleDescriptor.colors[0].blend.op_alpha = SG_BLENDOP_ADD;
    particleDescriptor.sample_count = 1;
    particleDescriptor.label = "NewAOE premultiplied particle pipeline";
    particleAlphaPipeline_ = sg_make_pipeline(&particleDescriptor);

    particleDescriptor.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE;
    particleDescriptor.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ONE;
    particleDescriptor.label = "NewAOE additive particle pipeline";
    particleAdditivePipeline_ = sg_make_pipeline(&particleDescriptor);

    if (!resourceValid(spritePipeline_) || !resourceValid(lightingPipeline_) ||
        !resourceValid(particleAlphaPipeline_) ||
        !resourceValid(particleAdditivePipeline_) ||
        !resourceValid(bloomExtractPipeline_) ||
        !resourceValid(bloomBlurPipeline_) || !resourceValid(postPipeline_)) {
        error = "failed to create one or more renderer pipelines";
        return false;
    }
    return true;
}

bool DeferredRenderer::createTargets(int width, int height,
                                     std::string& error) {
    error.clear();
    const int bloomWidth = std::max(width / 2, 1);
    const int bloomHeight = std::max(height / 2, 1);
    if (!makeColorTarget(width, height, SG_PIXELFORMAT_RGBA8, gAlbedo_,
                         "NewAOE GBuffer albedo", error) ||
        !makeColorTarget(width, height, SG_PIXELFORMAT_RGBA8, gNormal_,
                         "NewAOE GBuffer normal", error) ||
        !makeColorTarget(width, height, emissiveFormat_, gEmissive_,
                         "NewAOE GBuffer emissive", error) ||
        !makeDepthTarget(width, height, depth_, error) ||
        !makeColorTarget(width, height, sceneFormat_, scene_,
                         "NewAOE HDR scene", error) ||
        !makeColorTarget(bloomWidth, bloomHeight, sceneFormat_, bloomA_,
                         "NewAOE bloom A", error) ||
        !makeColorTarget(bloomWidth, bloomHeight, sceneFormat_, bloomB_,
                         "NewAOE bloom B", error)) {
        destroyTargets();
        return false;
    }
    width_ = width;
    height_ = height;
    return true;
}

bool DeferredRenderer::ensureSpriteCapacity(std::size_t count,
                                            std::string& error) {
    if (count <= spriteCapacity_ && valid(spriteInstanceBuffer_)) return true;
    if (valid(spriteInstanceBuffer_)) sg_destroy_buffer(spriteInstanceBuffer_);
    spriteInstanceBuffer_ = {};
    spriteCapacity_ = nextCapacity(count);
    sg_buffer_desc descriptor{};
    descriptor.size = spriteCapacity_ * sizeof(SpriteGpuInstance);
    descriptor.usage.vertex_buffer = true;
    descriptor.usage.write_transient = true;
    descriptor.label = "NewAOE sprite instance stream";
    spriteInstanceBuffer_ = sg_make_buffer(&descriptor);
    if (sg_query_buffer_state(spriteInstanceBuffer_) != SG_RESOURCESTATE_VALID) {
        error = "failed to allocate the sprite instance buffer";
        spriteCapacity_ = 0;
        return false;
    }
    return true;
}

bool DeferredRenderer::render(
    const sg_swapchain& swapchain, const Matrix4& worldToClip,
    const DirectionalLight& light, const PostProcessSettings& postProcess,
    const TextureCache& textures, const SpriteBatcher& sprites,
    const std::vector<ParticleRenderBatch>& particleBatches,
    RenderStats& stats, std::string& error) {
    error.clear();
    if (width_ <= 0 || height_ <= 0) {
        error = "deferred renderer has no valid framebuffer targets";
        return false;
    }
    if (!ensureSpriteCapacity(sprites.instances().size(), error)) {
        return false;
    }

    if (!sprites.instances().empty()) {
        sg_write_buffer_desc write{};
        write.src.data = {sprites.instances().data(),
                          sprites.instances().size() *
                              sizeof(SpriteGpuInstance)};
        write.dst.buffer = spriteInstanceBuffer_;
        sg_write_buffer_transient(&write);
    }
    drawGBuffer(worldToClip, textures, sprites, stats);
    drawLighting(light);
    drawParticles(worldToClip, light, textures, particleBatches, stats);
    drawBloom(postProcess);
    drawPostProcess(swapchain, textures, postProcess);
    return true;
}

void DeferredRenderer::drawGBuffer(const Matrix4& worldToClip,
                                   const TextureCache& textures,
                                   const SpriteBatcher& sprites,
                                   RenderStats& stats) {
    sg_pass pass{};
    pass.attachments.colors[0] = gAlbedo_.attachment;
    pass.attachments.colors[1] = gNormal_.attachment;
    pass.attachments.colors[2] = gEmissive_.attachment;
    pass.attachments.depth_stencil = depth_.attachment;
    for (int index = 0; index < 3; ++index) {
        pass.action.colors[index].load_action = SG_LOADACTION_CLEAR;
        pass.action.colors[index].store_action = SG_STOREACTION_STORE;
    }
    pass.action.colors[0].clear_value = {0.0F, 0.0F, 0.0F, 1.0F};
    pass.action.colors[1].clear_value = {0.5F, 0.5F, 1.0F, 1.0F};
    pass.action.colors[2].clear_value = {0.0F, 0.0F, 0.0F, 0.0F};
    pass.action.depth.load_action = SG_LOADACTION_CLEAR;
    pass.action.depth.store_action = SG_STOREACTION_STORE;
    pass.action.depth.clear_value = 1.0F;
    pass.label = "NewAOE GBuffer pass";
    sg_begin_pass(&pass);

    if (!sprites.instances().empty()) {
        sg_apply_pipeline(spritePipeline_);
        aoe_engine_sprite_camera_t camera{};
        std::memcpy(camera.world_to_clip, worldToClip.data(),
                    sizeof(camera.world_to_clip));
        applyUniforms(UB_aoe_engine_sprite_camera, camera);

        for (const SpriteBatch& batch : sprites.batches()) {
            sg_bindings bindings{};
            bindings.vertex_buffers[0] = quadBuffer_;
            bindings.vertex_buffers[1] = spriteInstanceBuffer_;
            bindings.vertex_buffer_offsets[1] = static_cast<int>(
                batch.firstInstance * sizeof(SpriteGpuInstance));
            bindings.views[VIEW_aoe_engine_sprite_base_texture] =
                batch.baseColor;
            bindings.views[VIEW_aoe_engine_sprite_normal_texture] = batch.normal;
            bindings.views[VIEW_aoe_engine_sprite_emissive_texture] =
                batch.emissive;
            bindings.samplers[SMP_aoe_engine_sprite_sampler] =
                textures.sampler();
            sg_apply_bindings(&bindings);
            sg_draw(0, static_cast<int>(kQuadVertices.size()),
                    static_cast<int>(batch.instanceCount));
            ++stats.spriteDrawCalls;
        }
    }
    sg_end_pass();
}

void DeferredRenderer::drawLighting(const DirectionalLight& light) {
    sg_pass pass{};
    pass.attachments.colors[0] = scene_.attachment;
    pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass.action.colors[0].store_action = SG_STOREACTION_STORE;
    pass.action.colors[0].clear_value = {0.0F, 0.0F, 0.0F, 1.0F};
    pass.label = "NewAOE deferred lighting pass";
    sg_begin_pass(&pass);
    sg_apply_pipeline(lightingPipeline_);

    sg_bindings bindings{};
    bindings.views[VIEW_aoe_engine_deferred_albedo_texture] = gAlbedo_.texture;
    bindings.views[VIEW_aoe_engine_deferred_normal_texture] = gNormal_.texture;
    bindings.views[VIEW_aoe_engine_deferred_emissive_texture] =
        gEmissive_.texture;
    bindings.samplers[SMP_aoe_engine_deferred_sampler] = linearSampler_;
    sg_apply_bindings(&bindings);

    aoe_engine_deferred_lighting_params_t parameters{};
    parameters.light_direction_intensity[0] = light.direction.x;
    parameters.light_direction_intensity[1] = light.direction.y;
    parameters.light_direction_intensity[2] = light.direction.z;
    parameters.light_direction_intensity[3] = light.intensity;
    parameters.light_color_ambient[0] = light.color.r;
    parameters.light_color_ambient[1] = light.color.g;
    parameters.light_color_ambient[2] = light.color.b;
    parameters.light_color_ambient[3] = light.ambient;
    parameters.ambient_color[0] = 1.0F;
    parameters.ambient_color[1] = 1.0F;
    parameters.ambient_color[2] = 1.0F;
    parameters.ambient_color[3] = 1.0F;
    applyUniforms(UB_aoe_engine_deferred_lighting_params, parameters);
    sg_draw(0, 3, 1);
    sg_end_pass();
}

void DeferredRenderer::drawParticles(
    const Matrix4& worldToClip, const DirectionalLight& light,
    const TextureCache& textures,
    const std::vector<ParticleRenderBatch>& batches, RenderStats& stats) {
    if (batches.empty()) return;

    sg_pass pass{};
    pass.attachments.colors[0] = scene_.attachment;
    pass.attachments.depth_stencil = depth_.attachment;
    pass.action.colors[0].load_action = SG_LOADACTION_LOAD;
    pass.action.colors[0].store_action = SG_STOREACTION_STORE;
    pass.action.depth.load_action = SG_LOADACTION_LOAD;
    pass.action.depth.store_action = SG_STOREACTION_DONTCARE;
    pass.label = "NewAOE transparent particle pass";
    sg_begin_pass(&pass);

    aoe_engine_particle_lighting_t lighting{};
    lighting.particle_light_direction_intensity[0] = light.direction.x;
    lighting.particle_light_direction_intensity[1] = light.direction.y;
    lighting.particle_light_direction_intensity[2] = light.direction.z;
    lighting.particle_light_direction_intensity[3] = light.intensity;
    lighting.particle_light_color_ambient[0] = light.color.r;
    lighting.particle_light_color_ambient[1] = light.color.g;
    lighting.particle_light_color_ambient[2] = light.color.b;
    lighting.particle_light_color_ambient[3] = light.ambient;
    lighting.particle_ambient_color[0] = 1.0F;
    lighting.particle_ambient_color[1] = 1.0F;
    lighting.particle_ambient_color[2] = 1.0F;
    lighting.particle_ambient_color[3] = 1.0F;

    for (const ParticleRenderBatch& batch : batches) {
        sg_apply_pipeline(batch.blendMode == ParticleBlendMode::Additive
                              ? particleAdditivePipeline_
                              : particleAlphaPipeline_);
        sg_bindings bindings{};
        bindings.vertex_buffers[0] = quadBuffer_;
        bindings.vertex_buffers[1] = batch.instanceBuffer;
        bindings.views[VIEW_aoe_engine_particle_base_texture] =
            valid(batch.baseTexture) ? batch.baseTexture : textures.whiteView();
        bindings.views[VIEW_aoe_engine_particle_normal_texture] =
            valid(batch.normalTexture) ? batch.normalTexture
                                       : textures.flatNormalView();
        bindings.views[VIEW_aoe_engine_particle_emissive_texture] =
            valid(batch.emissiveTexture) ? batch.emissiveTexture
                                         : textures.whiteView();
        bindings.views[VIEW_aoe_engine_particle_motion_curve_texture] =
            batch.motionCurveTexture;
        bindings.views[VIEW_aoe_engine_particle_color_curve_texture] =
            batch.colorCurveTexture;
        bindings.samplers[SMP_aoe_engine_particle_sampler] = textures.sampler();
        bindings.samplers[SMP_aoe_engine_particle_curve_sampler] = curveSampler_;
        bindings.samplers[SMP_aoe_engine_particle_color_curve_sampler] =
            curveSampler_;
        sg_apply_bindings(&bindings);

        aoe_engine_particle_camera_t camera{};
        std::memcpy(camera.particle_world_to_clip, worldToClip.data(),
                    sizeof(camera.particle_world_to_clip));
        camera.particle_time_gravity[0] = batch.systemTime;
        camera.particle_time_gravity[1] = batch.gravity.x;
        camera.particle_time_gravity[2] = batch.gravity.y;
        camera.particle_time_gravity[3] = batch.gravity.z;
        applyUniforms(UB_aoe_engine_particle_camera, camera);

        aoe_engine_particle_system_t system{};
        system.particle_base_size_rotation[0] = batch.baseSize.x;
        system.particle_base_size_rotation[1] = batch.baseSize.y;
        system.particle_base_size_rotation[2] = batch.baseRotation;
        system.particle_system_tint[0] = batch.tint.r;
        system.particle_system_tint[1] = batch.tint.g;
        system.particle_system_tint[2] = batch.tint.b;
        system.particle_system_tint[3] = batch.tint.a;
        system.particle_local_position_rotation[0] =
            batch.localTransform.position.x;
        system.particle_local_position_rotation[1] =
            batch.localTransform.position.y;
        system.particle_local_position_rotation[2] =
            batch.localTransform.rotation;
        system.particle_local_position_rotation[3] =
            batch.localSpace ? 1.0F : 0.0F;
        system.particle_local_scale[0] = batch.localTransform.scale.x;
        system.particle_local_scale[1] = batch.localTransform.scale.y;
        applyUniforms(UB_aoe_engine_particle_system, system);

        aoe_engine_particle_material_t material{};
        material.particle_emissive_opacity[0] = batch.emissive.r;
        material.particle_emissive_opacity[1] = batch.emissive.g;
        material.particle_emissive_opacity[2] = batch.emissive.b;
        material.particle_emissive_opacity[3] = batch.opacity;
        material.particle_surface[0] = batch.alphaCutoff;
        material.particle_surface[1] = batch.normalStrength;
        material.particle_surface[2] = batch.roughness;
        material.particle_surface[3] = batch.metallic;
        applyUniforms(UB_aoe_engine_particle_material, material);
        applyUniforms(UB_aoe_engine_particle_lighting, lighting);
        sg_draw(0, static_cast<int>(kQuadVertices.size()),
                static_cast<int>(batch.instanceCount));
        ++stats.particleDrawCalls;
    }
    sg_end_pass();
}

void DeferredRenderer::drawBloom(const PostProcessSettings& settings) {
    sg_pass extract{};
    extract.attachments.colors[0] = bloomA_.attachment;
    extract.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    extract.action.colors[0].store_action = SG_STOREACTION_STORE;
    extract.action.colors[0].clear_value = {0.0F, 0.0F, 0.0F, 1.0F};
    extract.label = "NewAOE bloom extract pass";
    sg_begin_pass(&extract);
    if (settings.enabled && settings.bloom) {
        sg_apply_pipeline(bloomExtractPipeline_);
        sg_bindings bindings{};
        bindings.views[VIEW_aoe_engine_bloom_source_texture] = scene_.texture;
        bindings.samplers[SMP_aoe_engine_bloom_source_sampler] = linearSampler_;
        sg_apply_bindings(&bindings);
        aoe_engine_bloom_extract_params_t parameters{};
        // An RGBA8 scene cannot represent values above one.  Lower the
        // extraction threshold just enough that bloom remains meaningful on
        // devices without a blendable floating-point render target.
        const float threshold =
            sceneFormat_ == SG_PIXELFORMAT_RGBA8
                ? std::min(settings.bloomThreshold, 0.8F)
                : settings.bloomThreshold;
        parameters.bloom_threshold_knee[0] = threshold;
        parameters.bloom_threshold_knee[1] =
            std::max(threshold * 0.5F, 0.0001F);
        applyUniforms(UB_aoe_engine_bloom_extract_params, parameters);
        sg_draw(0, 3, 1);
    }
    sg_end_pass();
    if (!settings.enabled || !settings.bloom) return;

    const float inverseWidth = 1.0F / static_cast<float>(std::max(width_ / 2, 1));
    const float inverseHeight =
        1.0F / static_cast<float>(std::max(height_ / 2, 1));
    for (int iteration = 0; iteration < 2; ++iteration) {
        sg_pass horizontal{};
        horizontal.attachments.colors[0] = bloomB_.attachment;
        horizontal.action.colors[0].load_action = SG_LOADACTION_CLEAR;
        horizontal.action.colors[0].store_action = SG_STOREACTION_STORE;
        horizontal.label = "NewAOE horizontal bloom blur";
        sg_begin_pass(&horizontal);
        sg_apply_pipeline(bloomBlurPipeline_);
        sg_bindings bindings{};
        bindings.views[VIEW_aoe_engine_bloom_blur_texture] = bloomA_.texture;
        bindings.samplers[SMP_aoe_engine_bloom_blur_sampler] = linearSampler_;
        sg_apply_bindings(&bindings);
        aoe_engine_bloom_blur_params_t parameters{};
        parameters.bloom_texel_direction[0] = 1.0F;
        parameters.bloom_texel_direction[2] = inverseWidth;
        parameters.bloom_texel_direction[3] = inverseHeight;
        applyUniforms(UB_aoe_engine_bloom_blur_params, parameters);
        sg_draw(0, 3, 1);
        sg_end_pass();

        sg_pass vertical{};
        vertical.attachments.colors[0] = bloomA_.attachment;
        vertical.action.colors[0].load_action = SG_LOADACTION_CLEAR;
        vertical.action.colors[0].store_action = SG_STOREACTION_STORE;
        vertical.label = "NewAOE vertical bloom blur";
        sg_begin_pass(&vertical);
        sg_apply_pipeline(bloomBlurPipeline_);
        bindings.views[VIEW_aoe_engine_bloom_blur_texture] = bloomB_.texture;
        sg_apply_bindings(&bindings);
        parameters = {};
        parameters.bloom_texel_direction[1] = 1.0F;
        parameters.bloom_texel_direction[2] = inverseWidth;
        parameters.bloom_texel_direction[3] = inverseHeight;
        applyUniforms(UB_aoe_engine_bloom_blur_params, parameters);
        sg_draw(0, 3, 1);
        sg_end_pass();
    }
}

void DeferredRenderer::drawPostProcess(
    const sg_swapchain& swapchain, const TextureCache&,
    const PostProcessSettings& settings) {
    sg_pass pass{};
    pass.swapchain = swapchain;
    pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass.action.colors[0].store_action = SG_STOREACTION_STORE;
    pass.action.colors[0].clear_value = {0.0F, 0.0F, 0.0F, 1.0F};
    pass.label = "NewAOE post-process pass";
    sg_begin_pass(&pass);
    sg_apply_pipeline(postPipeline_);
    sg_bindings bindings{};
    bindings.views[VIEW_aoe_engine_post_scene_texture] = scene_.texture;
    bindings.views[VIEW_aoe_engine_post_bloom_texture] = bloomA_.texture;
    bindings.samplers[SMP_aoe_engine_post_sampler] = linearSampler_;
    sg_apply_bindings(&bindings);
    aoe_engine_postprocess_params_t parameters{};
    parameters.post_exposure_bloom_fxaa[0] =
        settings.enabled ? settings.exposure : 1.0F;
    parameters.post_exposure_bloom_fxaa[1] =
        settings.enabled && settings.bloom ? settings.bloomIntensity : 0.0F;
    parameters.post_exposure_bloom_fxaa[2] =
        settings.enabled && settings.fxaa ? 1.0F : 0.0F;
    parameters.post_inverse_size[0] = 1.0F / static_cast<float>(width_);
    parameters.post_inverse_size[1] = 1.0F / static_cast<float>(height_);
    applyUniforms(UB_aoe_engine_postprocess_params, parameters);
    sg_draw(0, 3, 1);
    sg_end_pass();
}

void DeferredRenderer::destroyTargets() noexcept {
    destroyColorTarget(gAlbedo_);
    destroyColorTarget(gNormal_);
    destroyColorTarget(gEmissive_);
    destroyDepthTarget(depth_);
    destroyColorTarget(scene_);
    destroyColorTarget(bloomA_);
    destroyColorTarget(bloomB_);
    width_ = 0;
    height_ = 0;
}

void DeferredRenderer::destroyPipelines() noexcept {
    if (valid(postPipeline_)) sg_destroy_pipeline(postPipeline_);
    if (valid(bloomBlurPipeline_)) sg_destroy_pipeline(bloomBlurPipeline_);
    if (valid(bloomExtractPipeline_)) sg_destroy_pipeline(bloomExtractPipeline_);
    if (valid(particleAdditivePipeline_))
        sg_destroy_pipeline(particleAdditivePipeline_);
    if (valid(particleAlphaPipeline_))
        sg_destroy_pipeline(particleAlphaPipeline_);
    if (valid(lightingPipeline_)) sg_destroy_pipeline(lightingPipeline_);
    if (valid(spritePipeline_)) sg_destroy_pipeline(spritePipeline_);
    postPipeline_ = {};
    bloomBlurPipeline_ = {};
    bloomExtractPipeline_ = {};
    particleAdditivePipeline_ = {};
    particleAlphaPipeline_ = {};
    lightingPipeline_ = {};
    spritePipeline_ = {};

    if (valid(postShader_)) sg_destroy_shader(postShader_);
    if (valid(bloomBlurShader_)) sg_destroy_shader(bloomBlurShader_);
    if (valid(bloomExtractShader_)) sg_destroy_shader(bloomExtractShader_);
    if (valid(particleShader_)) sg_destroy_shader(particleShader_);
    if (valid(lightingShader_)) sg_destroy_shader(lightingShader_);
    if (valid(spriteShader_)) sg_destroy_shader(spriteShader_);
    postShader_ = {};
    bloomBlurShader_ = {};
    bloomExtractShader_ = {};
    particleShader_ = {};
    lightingShader_ = {};
    spriteShader_ = {};
}

}  // namespace newaoe::render::detail
