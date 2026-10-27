# NewAOE portable shaders

`engine.glsl` is the editable Sokol Shader Language source. `engine.glsl.h`
is checked in so application builds never need a host shader compiler and can
target Windows, Linux, WebGL2, Android, macOS, and iOS from the same source.

Regenerate the header with a current `sokol-shdc` from
<https://github.com/floooh/sokol-tools-bin>:

```text
sokol-shdc -i engine.glsl -o engine.glsl.h -l glsl410:glsl300es:hlsl5:metal_macos:metal_ios:metal_sim --ifdef --reflection --no-log-cmdline
```

The generated header must be compiled after `sokol_gfx.h` is included. The
runtime should use the generated `ATTR_*`, `UB_*`, `VIEW_*`, and `SMP_*`
constants instead of duplicating binding numbers.

## Coordinate conventions

Camera matrices use a D3D/Metal-style `[0, 1]` clip-depth interval. Sprite and
particle vertex shaders request sokol-shdc's GLSL `fixup_clipspace`, which maps
that interval to OpenGL's `[-1, 1]` interval without changing application-side
matrices. Full-screen render-target passes deliberately preserve their input
UV orientation. Applying `flip_vert_y` at every GL/GLES pass makes orientation
depend on the number of passes: forward particles, deferred sprites and Bloom
then land on different vertical sides of the scene.

Asset UVs are intentionally not backend-flipped. `stb_image`'s first decoded
row is uploaded unchanged, and `(0,0)` remains the first texel row on each
backend. Quad UVs therefore use the same top-left convention everywhere;
only render-target sampling needs the sokol-shdc framebuffer-origin correction.

## Passes

- `sprite_gbuffer`: instanced, opaque/cutout sprites into three GBuffer targets.
- `deferred_lighting`: directional plus ambient lighting into the HDR scene.
- `particle`: analytic GPU motion and premultiplied-alpha forward shading.
- `bloom_extract` and `bloom_blur`: separable bloom preparation.
- `postprocess`: optional FXAA, bloom combine, ACES tone mapping, and gamma.

The particle motion LUT uses `R/G` for X/Y size multipliers and `B` for a
rotation offset in radians. Its color LUT is RGBA. Both LUTs are RGBA32F,
declared unfilterable, and manually interpolated so WebGL2 does not require
floating-point linear-filter extensions.

Each GPU particle instance is exactly four `vec4` values:

- `position_spawn_time`: world XYZ and system-local spawn time.
- `velocity_lifetime`: initial XYZ velocity and lifetime.
- `acceleration_drag`: constant XYZ acceleration and non-negative drag.
- `variation`: X size multiplier, rotation offset, Y size multiplier, reserved.

World-space effects bake the interpolated instance rotation and non-uniform
scale into each birth record. Local-space effects retain local records and use
the live `ParticleTransform2D` uniform so existing particles remain attached.

The particle camera block contains the world-to-clip matrix followed by
`particle_time_gravity = {systemTime, gravityX, gravityY, gravityZ}`. Particle
depth is the simulated world-space Z value; there is no separate depth offset.
