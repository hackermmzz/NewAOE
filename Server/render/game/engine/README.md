# NewAOE lightweight render engine

The public facade is `RenderEngine.h`; `RenderInstance` exposes the same game
facing operations.  All calls are thread-affine and must happen on the Sokol
render thread after `sg_setup()`.

`RenderInstance` configures larger Sokol resource pools for the particle
library.  A custom host which owns `sg_setup()` should likewise size
`buffer_pool_size`, `image_pool_size`, and `view_pool_size` for its maximum
number of simultaneously loaded effects; `RenderEngine` never reaches into a
platform API or recreates the host's Sokol context.  Supporting the full
one-million-record resident budget with the minimum 256-record pages requires
at least 4096 buffer slots; the built-in host uses 8192 for headroom.

```cpp
auto texture = renderer.LoadImage("assets/units/archer.png");
renderer.DrawImage(texture, {100.0F, 80.0F, 96.0F, 128.0F});

auto normal = renderer.LoadLinearImage("assets/units/archer_normal.png");

auto fire = renderer.LoadParticleSystem("assets/particles/fire");
renderer.SetParticleSystemPosition(fire, {400.0F, 300.0F});
renderer.PlayParticleSystem(fire);
```

`LoadImage` is the requested game-facing name.  Windows' `WinUser.h` also
defines a `LoadImage` macro; the engine header undefines an already-visible
macro, and additionally exposes the identical `LoadTexture` spelling for code
that includes `WinUser.h` afterwards.

Color and emissive images use `SRGB8A8` sampling when supported so deferred
lighting operates in linear space.  Use `LoadLinearImage` for normal maps and
other numeric/data textures; the cache keeps sRGB and linear views of the same
file as distinct GPU resources.

`WorldRect` is an absolute world-space destination.  The entire source image
is scaled into it.  Individual sprites do not rotate; camera position,
rotation and zoom provide the scene transform.  A default image draw discards
only zero-alpha texels and writes every surviving texel as opaque.
Until game code explicitly sets or translates the camera position, the engine
keeps its centre aligned with the framebuffer so `(0, 0)` remains the
top-left of the visible world, including after a window resize.

## Frame graph

1. Instanced opaque/cutout sprites write albedo, normal/roughness and
   emissive/metallic GBuffer targets plus depth.  Emissive uses `RGBA16F`
   when the backend can render and sample it, with an `RGBA8` fallback.
2. Deferred directional/ambient lighting writes an HDR target when the device
   supports blendable `RGBA16F`, otherwise a portable `RGBA8` target.
3. Analytically simulated particles render forward with depth testing and no
   depth writes.  Premultiplied-alpha and additive pipelines are separate.
4. Half-resolution bloom, optional FXAA, ACES tone mapping and gamma correction
   produce the swapchain image.
5. `RenderInstance` loads that image and draws Dear ImGui over it.

Use `Capabilities().hdrScene` and `Capabilities().hdrEmissiveGBuffer` to detect
the SDR fallback explicitly.  In fallback mode values above one are clamped
before bloom extraction, while the rest of the frame graph remains enabled.

Sprite submission order is encoded in depth before sorting by texture, so
equal textures collapse into one instanced draw call without changing overlap.

## Particle assets

`LoadParticleSystem(directory)` reads `directory/particle.json`.  Parsed
definitions, textures and curve LUTs are cached by canonical directory, while
every returned handle owns independent playback time, particles and random
state.  CPU code schedules births and bursts; the vertex shader evaluates
velocity, acceleration, gravity and drag directly from birth records.

Birth records live in lazily allocated persistent GPU pages.  A frame uploads
only newly born records and records relocated while compacting the live pool;
paused or otherwise unchanged systems perform no particle-buffer upload.
Pages grow geometrically (a default 4096-particle system remains a single draw
when its first burst needs that capacity), and empty/tail pages are released.
The runtime enforces a global budget of 1,000,000 resident record slots across
all playback instances.  Page slack counts toward that bound, and allocation
failure is reported through `LastError()` instead of overflowing Sokol's
integer draw arguments.

Build the Windows/Linux editor without Server or Client modules:

```text
cmake -S . -B build-engine -DAOE_BUILD=Engine
cmake --build build-engine --config Release
```

The editor saves manifests and texture references.  Portable shaders are
pre-generated with `sokol-shdc`; see `shaders/README.md` for regeneration.

