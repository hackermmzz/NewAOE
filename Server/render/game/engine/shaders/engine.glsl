@module aoe_engine

// NewAOE's renderer uses one portable shader source for every Sokol backend.
// Keep resource slots stable: the runtime binds views by the generated constants.

@vs sprite_gbuffer_vs
// Camera matrices use the D3D/Metal [0,+w] clip-depth convention. OpenGL
// needs the generated shader to expand that interval to [-w,+w].
@glsl_options fixup_clipspace
layout(binding=0) uniform sprite_camera {
    mat4 world_to_clip;
};

in vec2 sprite_position;
in vec2 sprite_uv;
in vec4 sprite_rect;
in vec4 sprite_tint;
in vec4 sprite_emissive_cutoff;
in vec4 sprite_surface_depth;

out vec2 sprite_fs_uv;
out vec4 sprite_fs_tint;
out vec4 sprite_fs_emissive_cutoff;
out vec4 sprite_fs_surface_depth;

void main() {
    vec2 world_position = sprite_rect.xy + sprite_position * sprite_rect.zw;
    gl_Position = world_to_clip * vec4(world_position, sprite_surface_depth.z, 1.0);
    sprite_fs_uv = sprite_uv;
    sprite_fs_tint = sprite_tint;
    sprite_fs_emissive_cutoff = sprite_emissive_cutoff;
    sprite_fs_surface_depth = sprite_surface_depth;
}
@end

@fs sprite_gbuffer_fs
layout(binding=0) uniform texture2D sprite_base_texture;
layout(binding=1) uniform texture2D sprite_normal_texture;
layout(binding=2) uniform texture2D sprite_emissive_texture;
layout(binding=0) uniform sampler sprite_sampler;

in vec2 sprite_fs_uv;
in vec4 sprite_fs_tint;
in vec4 sprite_fs_emissive_cutoff;
in vec4 sprite_fs_surface_depth;

layout(location=0) out vec4 gbuffer_albedo;
layout(location=1) out vec4 gbuffer_normal_roughness;
layout(location=2) out vec4 gbuffer_emissive_metallic;

void main() {
    vec4 base_sample = texture(sampler2D(sprite_base_texture, sprite_sampler), sprite_fs_uv);
    vec4 albedo = base_sample * sprite_fs_tint;
    if (albedo.a <= sprite_fs_emissive_cutoff.a) {
        discard;
    }

    vec3 sampled_normal = texture(sampler2D(sprite_normal_texture, sprite_sampler), sprite_fs_uv).xyz;
    sampled_normal = normalize(sampled_normal * 2.0 - 1.0);
    float normal_strength = clamp(sprite_fs_surface_depth.w, 0.0, 1.0);
    vec3 normal = normalize(mix(vec3(0.0, 0.0, 1.0), sampled_normal, normal_strength));
    float roughness = clamp(sprite_fs_surface_depth.x, 0.04, 1.0);
    float metallic = clamp(sprite_fs_surface_depth.y, 0.0, 1.0);
    vec3 emissive_sample = texture(sampler2D(sprite_emissive_texture, sprite_sampler), sprite_fs_uv).rgb;

    gbuffer_albedo = vec4(albedo.rgb, 1.0);
    gbuffer_normal_roughness = vec4(normal * 0.5 + 0.5, roughness);
    gbuffer_emissive_metallic = vec4(emissive_sample * sprite_fs_emissive_cutoff.rgb, metallic);
}
@end

@program sprite_gbuffer sprite_gbuffer_vs sprite_gbuffer_fs

@vs fullscreen_vs
// Preserve attachment orientation across the complete GBuffer, Bloom and
// presentation chain. Applying flip_vert_y at each GL/GLES full-screen pass
// makes the result depend on pass-count parity and mirrors forward particles
// relative to both deferred sprites and Bloom.
out vec2 fullscreen_uv;

void main() {
    vec2 position;
    if (gl_VertexIndex == 0) {
        position = vec2(-1.0, -1.0);
    } else if (gl_VertexIndex == 1) {
        position = vec2(3.0, -1.0);
    } else {
        position = vec2(-1.0, 3.0);
    }
    gl_Position = vec4(position, 0.0, 1.0);
    fullscreen_uv = position * 0.5 + 0.5;
}
@end

@fs deferred_lighting_fs
layout(binding=0) uniform deferred_lighting_params {
    vec4 light_direction_intensity;
    vec4 light_color_ambient;
    vec4 ambient_color;
};

layout(binding=0) uniform texture2D deferred_albedo_texture;
layout(binding=1) uniform texture2D deferred_normal_texture;
layout(binding=2) uniform texture2D deferred_emissive_texture;
layout(binding=0) uniform sampler deferred_sampler;

in vec2 fullscreen_uv;
out vec4 deferred_color;

void main() {
    vec3 albedo = texture(sampler2D(deferred_albedo_texture, deferred_sampler), fullscreen_uv).rgb;
    vec4 normal_roughness = texture(sampler2D(deferred_normal_texture, deferred_sampler), fullscreen_uv);
    vec4 emissive_metallic = texture(sampler2D(deferred_emissive_texture, deferred_sampler), fullscreen_uv);

    vec3 normal = normalize(normal_roughness.xyz * 2.0 - 1.0);
    vec3 light_direction = normalize(light_direction_intensity.xyz);
    float diffuse = max(dot(normal, -light_direction), 0.0) * max(light_direction_intensity.w, 0.0);
    vec3 diffuse_light = light_color_ambient.rgb * diffuse;
    vec3 ambient_light = ambient_color.rgb * max(light_color_ambient.a, 0.0);

    // A compact 2D-friendly metallic response. The normal and material buffers
    // remain useful when richer lights are added without changing sprite data.
    float metallic = clamp(emissive_metallic.a, 0.0, 1.0);
    vec3 lit = albedo * (ambient_light + diffuse_light * (1.0 - metallic));
    lit += diffuse_light * mix(vec3(0.04), albedo, metallic) * (1.0 - normal_roughness.a) * 0.25;
    lit += emissive_metallic.rgb;
    deferred_color = vec4(lit, 1.0);
}
@end

@program deferred_lighting fullscreen_vs deferred_lighting_fs

@vs particle_vs
@glsl_options fixup_clipspace
layout(binding=0) uniform particle_camera {
    mat4 particle_world_to_clip;
    // x: system time, yzw: XYZ gravity
    vec4 particle_time_gravity;
};

layout(binding=1) uniform particle_system {
    // xy: base world size, z: base rotation, w: reserved
    vec4 particle_base_size_rotation;
    vec4 particle_system_tint;
    // xy: live local-space position, z: rotation, w: local-space flag
    vec4 particle_local_position_rotation;
    // xy: live local-space scale
    vec4 particle_local_scale;
};

@image_sample_type particle_motion_curve_texture unfilterable_float
layout(binding=3) uniform texture2D particle_motion_curve_texture;
@sampler_type particle_curve_sampler nonfiltering
layout(binding=1) uniform sampler particle_curve_sampler;

in vec2 particle_position;
in vec2 particle_uv;
in vec4 particle_position_spawn_time;
in vec4 particle_velocity_lifetime;
in vec4 particle_acceleration_drag;
in vec4 particle_variation;

out vec2 particle_fs_uv;
out vec4 particle_fs_tint;
out float particle_fs_age;
out vec2 particle_fs_rotation;

vec4 sample_motion_curve(float normalized_age) {
    ivec2 dimensions = textureSize(sampler2D(particle_motion_curve_texture, particle_curve_sampler), 0);
    float coordinate = clamp(normalized_age, 0.0, 1.0) * float(max(dimensions.x - 1, 0));
    int lower_index = int(floor(coordinate));
    int upper_index = min(lower_index + 1, dimensions.x - 1);
    vec4 lower_value = texelFetch(
        sampler2D(particle_motion_curve_texture, particle_curve_sampler),
        ivec2(lower_index, 0), 0);
    vec4 upper_value = texelFetch(
        sampler2D(particle_motion_curve_texture, particle_curve_sampler),
        ivec2(upper_index, 0), 0);
    return mix(lower_value, upper_value, fract(coordinate));
}

void main() {
    float age = particle_time_gravity.x - particle_position_spawn_time.w;
    float lifetime = max(particle_velocity_lifetime.w, 0.0001);
    float normalized_age = clamp(age / lifetime, 0.0, 1.0);
    float alive = float(age >= 0.0 && age < lifetime);

    vec3 acceleration = particle_acceleration_drag.xyz;
    acceleration += particle_time_gravity.yzw;
    vec3 initial_velocity = particle_velocity_lifetime.xyz;
    float drag = max(particle_acceleration_drag.w, 0.0);
    vec3 displacement;
    if (drag > 0.0001) {
        vec3 terminal_velocity = acceleration / drag;
        displacement = terminal_velocity * age
            + (initial_velocity - terminal_velocity) * (1.0 - exp(-drag * age)) / drag;
    } else {
        displacement = initial_velocity * age + acceleration * (0.5 * age * age);
    }

    vec4 motion_curve = sample_motion_curve(normalized_age);
    vec2 size = particle_base_size_rotation.xy
        * vec2(particle_variation.x, particle_variation.z)
        * max(motion_curve.rg, vec2(0.0));
    float rotation = particle_base_size_rotation.z + particle_variation.y + motion_curve.b;
    float sine_rotation = sin(rotation);
    float cosine_rotation = cos(rotation);
    vec2 local = (particle_position - vec2(0.5)) * size;
    vec2 rotated = vec2(
        local.x * cosine_rotation - local.y * sine_rotation,
        local.x * sine_rotation + local.y * cosine_rotation);

    vec3 center_position = particle_position_spawn_time.xyz + displacement;
    if (particle_local_position_rotation.w > 0.5) {
        vec2 scaled_center = center_position.xy * particle_local_scale.xy;
        float system_sine = sin(particle_local_position_rotation.z);
        float system_cosine = cos(particle_local_position_rotation.z);
        center_position.xy = particle_local_position_rotation.xy + vec2(
            scaled_center.x * system_cosine - scaled_center.y * system_sine,
            scaled_center.x * system_sine + scaled_center.y * system_cosine);
    }
    vec3 world_position = center_position + vec3(rotated, 0.0);
    vec4 clip_position = particle_world_to_clip * vec4(world_position, 1.0);
    if (alive < 0.5) {
        clip_position = vec4(2.0, 2.0, 2.0, 1.0);
    }
    gl_Position = clip_position;
    particle_fs_uv = particle_uv;
    particle_fs_tint = particle_system_tint;
    particle_fs_age = normalized_age * alive;
    particle_fs_rotation = vec2(cosine_rotation, sine_rotation);
}
@end

@fs particle_fs
layout(binding=2) uniform particle_material {
    // rgb: emissive tint, a: opacity
    vec4 particle_emissive_opacity;
    // x: alpha cutoff, y: normal strength, z: roughness, w: metallic
    vec4 particle_surface;
};

layout(binding=3) uniform particle_lighting {
    vec4 particle_light_direction_intensity;
    vec4 particle_light_color_ambient;
    vec4 particle_ambient_color;
};

layout(binding=0) uniform texture2D particle_base_texture;
layout(binding=1) uniform texture2D particle_normal_texture;
layout(binding=2) uniform texture2D particle_emissive_texture;
@image_sample_type particle_color_curve_texture unfilterable_float
layout(binding=4) uniform texture2D particle_color_curve_texture;
layout(binding=0) uniform sampler particle_sampler;
@sampler_type particle_color_curve_sampler nonfiltering
layout(binding=2) uniform sampler particle_color_curve_sampler;

in vec2 particle_fs_uv;
in vec4 particle_fs_tint;
in float particle_fs_age;
in vec2 particle_fs_rotation;
out vec4 particle_color;

vec4 sample_color_curve(float normalized_age) {
    ivec2 dimensions = textureSize(sampler2D(particle_color_curve_texture, particle_color_curve_sampler), 0);
    float coordinate = clamp(normalized_age, 0.0, 1.0) * float(max(dimensions.x - 1, 0));
    int lower_index = int(floor(coordinate));
    int upper_index = min(lower_index + 1, dimensions.x - 1);
    vec4 lower_value = texelFetch(
        sampler2D(particle_color_curve_texture, particle_color_curve_sampler),
        ivec2(lower_index, 0), 0);
    vec4 upper_value = texelFetch(
        sampler2D(particle_color_curve_texture, particle_color_curve_sampler),
        ivec2(upper_index, 0), 0);
    return mix(lower_value, upper_value, fract(coordinate));
}

void main() {
    vec4 base_sample = texture(sampler2D(particle_base_texture, particle_sampler), particle_fs_uv);
    vec4 color_curve = sample_color_curve(particle_fs_age);
    vec4 color = base_sample * particle_fs_tint * color_curve;
    float alpha = clamp(color.a * particle_emissive_opacity.a, 0.0, 1.0);
    if (alpha <= particle_surface.x || particle_fs_age <= 0.0) {
        discard;
    }

    // Premultiplied-alpha output. Additive and regular-alpha pipelines use
    // the same shader and differ only in their Sokol blend state.
    vec3 sampled_normal = texture(sampler2D(particle_normal_texture, particle_sampler), particle_fs_uv).xyz;
    sampled_normal = normalize(sampled_normal * 2.0 - 1.0);
    // The normal texture is authored in the particle quad's tangent space.
    // Rotate its XY axes by the same world rotation used for the quad. Camera
    // rotation is intentionally excluded because lighting vectors are world-space.
    sampled_normal.xy = vec2(
        sampled_normal.x * particle_fs_rotation.x - sampled_normal.y * particle_fs_rotation.y,
        sampled_normal.x * particle_fs_rotation.y + sampled_normal.y * particle_fs_rotation.x);
    vec3 normal = normalize(mix(
        vec3(0.0, 0.0, 1.0),
        sampled_normal,
        clamp(particle_surface.y, 0.0, 1.0)));
    vec3 light_direction = normalize(particle_light_direction_intensity.xyz);
    float diffuse = max(dot(normal, -light_direction), 0.0)
        * max(particle_light_direction_intensity.w, 0.0);
    float roughness = clamp(particle_surface.z, 0.04, 1.0);
    float metallic = clamp(particle_surface.w, 0.0, 1.0);
    vec3 diffuse_light = particle_light_color_ambient.rgb * diffuse;
    vec3 ambient_light = particle_ambient_color.rgb * max(particle_light_color_ambient.a, 0.0);
    vec3 emissive_sample = texture(sampler2D(particle_emissive_texture, particle_sampler), particle_fs_uv).rgb;
    vec3 shaded = color.rgb * (ambient_light + diffuse_light * (1.0 - metallic));
    shaded += diffuse_light * mix(vec3(0.04), color.rgb, metallic) * (1.0 - roughness) * 0.25;
    shaded += emissive_sample * max(particle_emissive_opacity.rgb, vec3(0.0));
    particle_color = vec4(shaded * alpha, alpha);
}
@end

@program particle particle_vs particle_fs

@fs bloom_extract_fs
layout(binding=0) uniform bloom_extract_params {
    vec4 bloom_threshold_knee;
};
layout(binding=0) uniform texture2D bloom_source_texture;
layout(binding=0) uniform sampler bloom_source_sampler;
in vec2 fullscreen_uv;
out vec4 bloom_extract_color;

void main() {
    vec3 color = texture(sampler2D(bloom_source_texture, bloom_source_sampler), fullscreen_uv).rgb;
    float brightness = max(max(color.r, color.g), color.b);
    float threshold = max(bloom_threshold_knee.x, 0.0);
    float knee = max(bloom_threshold_knee.y, 0.0001);
    float soft = clamp((brightness - threshold + knee) / (2.0 * knee), 0.0, 1.0);
    soft = soft * soft * knee;
    float contribution = max(brightness - threshold, soft) / max(brightness, 0.0001);
    bloom_extract_color = vec4(color * contribution, 1.0);
}
@end

@program bloom_extract fullscreen_vs bloom_extract_fs

@fs bloom_blur_fs
layout(binding=0) uniform bloom_blur_params {
    vec4 bloom_texel_direction;
};
layout(binding=0) uniform texture2D bloom_blur_texture;
layout(binding=0) uniform sampler bloom_blur_sampler;
in vec2 fullscreen_uv;
out vec4 bloom_blur_color;

void main() {
    vec2 offset = bloom_texel_direction.xy * bloom_texel_direction.zw;
    vec3 color = texture(sampler2D(bloom_blur_texture, bloom_blur_sampler), fullscreen_uv).rgb * 0.227027;
    color += texture(sampler2D(bloom_blur_texture, bloom_blur_sampler), fullscreen_uv + offset * 1.384615).rgb * 0.316216;
    color += texture(sampler2D(bloom_blur_texture, bloom_blur_sampler), fullscreen_uv - offset * 1.384615).rgb * 0.316216;
    color += texture(sampler2D(bloom_blur_texture, bloom_blur_sampler), fullscreen_uv + offset * 3.230769).rgb * 0.070270;
    color += texture(sampler2D(bloom_blur_texture, bloom_blur_sampler), fullscreen_uv - offset * 3.230769).rgb * 0.070270;
    bloom_blur_color = vec4(color, 1.0);
}
@end

@program bloom_blur fullscreen_vs bloom_blur_fs

@fs postprocess_fs
layout(binding=0) uniform postprocess_params {
    vec4 post_exposure_bloom_fxaa;
    vec4 post_inverse_size;
};
layout(binding=0) uniform texture2D post_scene_texture;
layout(binding=1) uniform texture2D post_bloom_texture;
layout(binding=0) uniform sampler post_sampler;
in vec2 fullscreen_uv;
out vec4 post_color;

float post_luma(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

vec3 sample_scene(vec2 uv) {
    return texture(sampler2D(post_scene_texture, post_sampler), uv).rgb;
}

vec3 fxaa_scene(vec2 uv) {
    vec2 inverse_size = post_inverse_size.xy;
    vec3 center = sample_scene(uv);
    float center_luma = post_luma(center);
    float north = post_luma(sample_scene(uv + vec2(0.0, -inverse_size.y)));
    float south = post_luma(sample_scene(uv + vec2(0.0, inverse_size.y)));
    float west = post_luma(sample_scene(uv + vec2(-inverse_size.x, 0.0)));
    float east = post_luma(sample_scene(uv + vec2(inverse_size.x, 0.0)));
    float minimum_luma = min(center_luma, min(min(north, south), min(west, east)));
    float maximum_luma = max(center_luma, max(max(north, south), max(west, east)));
    if (maximum_luma - minimum_luma < max(0.0312, maximum_luma * 0.125)) {
        return center;
    }
    vec2 direction = vec2(-(north - south), west - east);
    direction = clamp(direction / max(abs(direction.x) + abs(direction.y), 0.0001), vec2(-1.0), vec2(1.0));
    return 0.5 * (sample_scene(uv + direction * inverse_size * 0.5)
        + sample_scene(uv - direction * inverse_size * 0.5));
}

vec3 aces_tonemap(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

void main() {
    vec3 scene = post_exposure_bloom_fxaa.z > 0.5 ? fxaa_scene(fullscreen_uv) : sample_scene(fullscreen_uv);
    vec3 bloom = texture(sampler2D(post_bloom_texture, post_sampler), fullscreen_uv).rgb;
    vec3 hdr = scene + bloom * max(post_exposure_bloom_fxaa.y, 0.0);
    vec3 mapped = aces_tonemap(hdr * max(post_exposure_bloom_fxaa.x, 0.0));
    post_color = vec4(pow(mapped, vec3(1.0 / 2.2)), 1.0);
}
@end

@program postprocess fullscreen_vs postprocess_fs
