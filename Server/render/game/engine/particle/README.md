# Particle system runtime format

`particle.json` is the editable asset manifest. `loadParticleSystem()` validates
it and produces an immutable `ParticleSystemDefinition`; any number of
`ParticleEmitterInstance` objects can share that definition while retaining
independent time, random state and live particles.

Each playback instance also owns a `ParticleTransform2D`. With
`simulationSpace: "world"`, transforms are interpolated at each sub-frame
birth time and baked into immutable records, so moving emitters produce a
continuous trail while older particles remain behind. `inheritVelocity`
controls how much emitter linear velocity a new world-space particle receives.
With `simulationSpace: "local"`, records remain local and every live particle
follows the current position, rotation and non-uniform scale.

The manifest uses UTF-8 JSON and version `1`. Texture paths are relative to the
directory containing the manifest. Absolute paths and `..` components are
rejected so a packaged particle system cannot escape its asset directory.
Both `/` and legacy `\\` separators are interpreted portably; manifests are
best authored with `/` so the JSON reads the same on every host.
Calling `saveParticleSystem()` is the canonical way to create a complete
manifest and provides a useful example of the schema.

## GPU motion

Each live particle is represented by one 64-byte `GpuParticleRecord`. No
position integration is performed on the CPU. Given age `t`, initial velocity
`v0`, combined acceleration `a = record.acceleration + emitter.gravity`, and
drag `d`, the vertex shader evaluates:

```text
d == 0: position = p0 + v0*t + 0.5*a*t*t
d >  0: terminal = a/d
         position = p0 + terminal*t + (v0-terminal)*(1-exp(-d*t))/d
```

The system's size, color and rotation curves contain at most eight normalized
keys. The shared definition bakes them into 256-sample `RGBA32F` LUT textures;
the record's variation vector supplies per-particle size, rotation and random
values. The shader manually interpolates neighboring LUT texels so the same
path works on WebGL2 implementations where float filtering is unavailable.

`ParticleEmitterInstance::update()` only expires records, schedules continuous
emission and bursts, and creates immutable birth records. The CPU vector starts
with at most 256 reserved records and grows on demand, so loading a manifest
whose per-emitter maximum is one million does not immediately reserve 64 MiB.

The renderer stores those records in persistent Sokol vertex-buffer pages.
Pages are allocated lazily with a minimum of 256 records; the first page may
grow to 4096 when the initial live set needs it, and later pages grow
geometrically. Dirty ranges track births and the occasional swap needed to
compact expired records. Only those ranges pass through a transient staging
buffer and `sg_copy_buffer_to_buffer`; an unchanged or paused emitter performs
no particle-record upload. Empty and unused tail pages are released.

Playback time accumulates in double precision.  The float clock and birth
times sent to shaders are rebased in exact 2048-second quanta, with live birth
records updated at each rare rebase.  Long-running infinite effects therefore
do not freeze when a single-precision absolute clock loses frame-scale
resolution.

All playback instances share a hard budget of 1,000,000 resident GPU record
slots. Allocated page capacity (including its small unused tail), rather than
only the live count, consumes this budget. Exceeding it reports a render error
instead of overflowing buffer sizes or Sokol's integer draw arguments.

