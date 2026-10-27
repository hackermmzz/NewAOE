# NewAOE smoke-grenade Web demo

This integration demo uses the shipping `RenderEngine`, a particle directory
created by the single `ParticleSystemEditor --offscreen` executable, and the
instance-level `ParticleTransform2D` API. The emitter follows a moving grenade;
world-space particles remain behind and form a continuous interpolated trail.

Generate or update the asset:

```powershell
build-engine-mingw/bin/ParticleSystemEditor.exe --offscreen preset smoke smoke --force --json
build-engine-mingw/bin/ParticleSystemEditor.exe --offscreen validate smoke --json
```

Build with Emscripten in WSL:

```bash
cd /mnt/c/QtHomeWork/NewAOE
emcmake cmake -S . -B build/sokol-smoke-web \
  -DAOE_BUILD_WEB_SMOKE_DEMO=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DNEWAOE_SMOKE_ASSET_DIR=/mnt/c/QtHomeWork/NewAOE/smoke
cmake --build build/sokol-smoke-web --parallel 4
python3 Server/render/game/engine/WebSmokeDemo/serve.py build/sokol-smoke-web/bin
```

Open `http://127.0.0.1:8126/SokolSmokeDemo.html`. `Space` pauses or resumes
the trail and `R` restarts the particle instance at its current transform.

