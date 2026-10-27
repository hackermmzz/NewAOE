# ParticleSystemEditor

`ParticleSystemEditor` is one executable with two frontends over the same
particle definition, validation and transactional serializer:

- Run it without `--offscreen` to open the Dear ImGui editor.
- Add `--offscreen` to run a command without creating a Sokol window or GPU
  context. This mode is suitable for scripts, CI and AI tools on Windows or
  Linux, including Linux hosts without `DISPLAY`.

Build from the repository root:

```powershell
cmake -S . -B build-engine-mingw -G "MinGW Makefiles" `
  -DCMAKE_MAKE_PROGRAM=C:/ProgramData/chocolatey/bin/make.exe `
  -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe `
  -DAOE_BUILD=Engine
cmake --build build-engine-mingw --parallel 4
```

The complete command reference is always available from the executable:

```powershell
build-engine-mingw/bin/ParticleSystemEditor.exe --offscreen help
```

Typical automation flow:

```powershell
$editor = "build-engine-mingw/bin/ParticleSystemEditor.exe"
& $editor --offscreen preset fire fire --force --json
& $editor --offscreen apply fire --json `
  --set material.baseTexture fire.png `
  --set emitter.emissionRate 88 `
  --set emitter.gravity "0,58,0"
& $editor --offscreen validate fire --json
& $editor --offscreen inspect fire --json
```

The smoke preset is authored for a moving grenade emitter. It uses world
simulation so particles already born remain behind as a trail, while new
particles inherit a small fraction of the grenade's linear velocity:

```powershell
& $editor --offscreen preset smoke smoke --force --json
& $editor --offscreen apply smoke --json `
  --set emitter.simulationSpace world `
  --set emitter.inheritVelocity 0.09
```

`emitter.simulationSpace=local` is the attached alternative: existing
particles continue to follow the instance's current translation, rotation and
scale. `world` is the appropriate mode for smoke, missile and spark trails.

`apply` changes all requested properties in memory, validates the complete
result, and saves only once. Invalid input leaves the existing
`particle.json` unchanged. Exit codes distinguish command-line errors, read
errors, validation errors and write errors; `--json` makes status output
machine-readable.

