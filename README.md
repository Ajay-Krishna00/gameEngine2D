# engine2d

A 2D game engine written from scratch in **C++17**, on top of **SDL2 + OpenGL 3.3**.
No game-engine framework underneath — the window, render pipeline, batch renderer,
camera, and (in later phases) ECS and physics are all hand-built. It's a learning /
portfolio project: every subsystem is small enough to read in one sitting and explain
out loud.

> **Status:** Phases 1–3 are implemented (window + game loop, the OpenGL pipeline,
> sprite batching + 2D camera). Phases 4–7 (ECS, input, physics, polish) are designed
> and fully specced in [`BUILD_GUIDE.md`](BUILD_GUIDE.md) but not yet wired in.

## What's in the box

| Layer | Pieces | Status |
|---|---|---|
| **Core** | `Window` (SDL2 window + GL context), `Application` (fixed-timestep loop), `Time` (high-res clock) | ✅ |
| **Renderer** | `Shader`, `Texture` (via stb_image), `Camera2D` (orthographic pan/zoom), `SpriteBatch` (one draw call per texture run) | ✅ |
| **ECS** | sparse-set `Registry`, `ComponentPool`, generational entity handles | 📐 specced |
| **Physics** | AABB collision + spatial-grid broadphase, fixed-step response | 📐 specced |

### Design highlights

- **Fixed-timestep game loop with an accumulator** — simulation advances in constant
  `1/60s` steps independent of framerate, so updates stay deterministic and stable;
  rendering happens once per frame (see `core/Application.cpp`).
- **Batch renderer** — accumulates quads into one big dynamic vertex buffer and submits
  a run of same-textured sprites in a single `glDrawElements` call, flushing only when
  the texture changes or the buffer fills (`renderer/SpriteBatch.cpp`).
- **Orthographic 2D camera** — `view*projection` built with `glm::ortho`, supporting
  position + zoom (`renderer/Camera2D.cpp`).

## Tech stack

- **C++17**, **CMake** (≥ 3.21) + **Ninja**
- **SDL2** — OS window, input, GL context
- **OpenGL 3.3 core** + **GLAD** (loader)
- **GLM** — vector/matrix math
- **stb_image** — texture loading
- Dependencies are pulled automatically by **vcpkg** in manifest mode (`vcpkg.json`).

## Project layout

```
engine2d/
├─ CMakeLists.txt        build config (add new .cpp files here)
├─ CMakePresets.json     wires vcpkg + Ninja
├─ vcpkg.json            dependency manifest (auto-installed)
├─ assets/
│  └─ shaders/           sprite.vert / sprite.frag
└─ src/
   ├─ main.cpp           entry point
   ├─ core/              Window, Application, Time
   └─ renderer/          Shader, Texture, Camera2D, SpriteBatch
```

## Building & running

You need **vcpkg** (with `VCPKG_ROOT` set) and an MSVC toolchain (VS Build Tools).
The full step-by-step setup — installing vcpkg, VS Code extensions, selecting the
preset, and troubleshooting — lives in [`BUILD_GUIDE.md`](BUILD_GUIDE.md).

Quick version, from a *Developer PowerShell for VS*:

```powershell
# first configure compiles SDL2/glad/glm/stb via vcpkg — a few minutes, once
cmake --preset default
cmake --build build
.\build\engine2d.exe
```

✅ **Success looks like:** a 1280×720 window filled cornflower blue, with the terminal
printing your OpenGL version and GPU. Press `Esc` or close the window to quit.

> **Rule of thumb:** every `.cpp` you add under `src/` must be listed in the
> `add_executable(engine2d ...)` block in `CMakeLists.txt`, then re-configure. Headers
> are pulled in via `#include` and are not listed.

## Roadmap

The phased build plan — with full reference implementations for the remaining phases —
is in [`BUILD_GUIDE.md`](BUILD_GUIDE.md):

1. ✅ **Phase 1 / 1.5** — window, game loop, refactor into Window/Application/Time
2. ✅ **Phase 2** — shaders, textures, the modern OpenGL pipeline
3. ✅ **Phase 3** — sprite batching + 2D camera
4. 📐 **Phase 4** — sparse-set ECS (entities, components, systems)
5. 📐 **Phase 5** — input + a playable, controllable sprite
6. 📐 **Phase 6** — 2D physics: AABB collision + spatial-grid broadphase
7. 📐 **Phase 7** — polish: Dear ImGui debug panel, a demo game, asset manager
