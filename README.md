# engine2d

A 2D game engine written from scratch in **C++17**, on top of **SDL2 + OpenGL 3.3**.
No game-engine framework underneath — the window, render pipeline, batch renderer,
camera, ECS, input, and physics are all hand-built. It's a learning / portfolio
project: every subsystem is small enough to read in one sitting and explain out loud.

> **Status:** Phases 1–7 implemented. The demo is a walled arena with a WASD-driven,
> wall-collidable player and ~580 dynamic crates bouncing off the walls and each
> other — all entities, movement, rendering, and collision run through the engine's
> own ECS, batch renderer, and spatial-grid physics. Live stats (FPS, entity count,
> draw calls, quads, collision checks) are shown in the window title bar.

## What's in the box

| Layer | Pieces | Status |
|---|---|---|
| **Core** | `Window` (SDL2 window + GL context), `Application` (fixed-timestep loop + FPS), `Time` (high-res clock), `Input` (keyboard/mouse polling) | ✅ |
| **Renderer** | `Shader`, `Texture` (via stb_image), `Camera2D` (orthographic pan/zoom + screen→world), `SpriteBatch` (one draw call per texture run), `ResourceManager` (cached textures/shaders) | ✅ |
| **ECS** | sparse-set `Registry`, `ComponentPool`, generational entity handles, variadic `view<...>()` iteration | ✅ |
| **Physics** | AABB collision, uniform spatial-grid broadphase (O(n) vs O(n²)), penetration-resolution response, run in the fixed step | ✅ |

### Design highlights

- **Fixed-timestep game loop with an accumulator** — simulation advances in constant
  `1/60s` steps independent of framerate, so updates and physics stay deterministic;
  rendering happens once per frame (`core/Application.cpp`).
- **Batch renderer** — accumulates quads into one big dynamic vertex buffer and submits
  a run of same-textured sprites in a single `glDrawElements` call, flushing only when
  the texture changes or the buffer fills (`renderer/SpriteBatch.cpp`).
- **Sparse-set ECS** — components live in packed, contiguous arrays for cache-friendly
  iteration; an entity-index→dense-position sparse map gives O(1) add/remove (swap-and-pop)
  and lookup; 64-bit handles carry a generation counter to detect stale references
  (`ecs/`).
- **Spatial-grid physics** — colliders are bucketed into a uniform grid by cell, and only
  bodies in the same 3×3 cell neighbourhood are pair-tested, turning broadphase from
  O(n²) into roughly O(n). Penetration is resolved along the minimum-translation axis
  (`physics/PhysicsSystem.cpp`).
- **Orthographic 2D camera** — `view*projection` built with `glm::ortho`, with position,
  zoom, and screen→world unprojection (`renderer/Camera2D.cpp`).

## Controls

| Key | Action |
|---|---|
| `W` `A` `S` `D` / arrows | move the player (camera follows) |
| `Esc` | quit |

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
│  ├─ shaders/           sprite.vert / sprite.frag
│  └─ textures/          sprite.png
└─ src/
   ├─ main.cpp           entry point
   ├─ Game.{h,cpp}       the demo: arena + player + crates wired to the engine
   ├─ core/              Window, Application, Time, Input
   ├─ renderer/          Shader, Texture, Camera2D, SpriteBatch, ResourceManager
   ├─ ecs/               Entity, ComponentPool, Registry, Components, Systems
   └─ physics/           PhysicsSystem (AABB + spatial grid)
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

✅ **Success looks like:** a window showing a grey-walled arena full of coloured crates
bouncing around; drive the yellow player box with WASD and shove crates into the walls.
The title bar reports live FPS / entity / draw-call / collision counts. Press `Esc` to quit.

> **Rule of thumb:** every `.cpp` you add under `src/` must be listed in the
> `add_executable(engine2d ...)` block in `CMakeLists.txt`, then re-configure. Headers
> are pulled in via `#include` and are not listed.

## How it fits together

```
main → Game : Application
                │  fixed 1/60s step:
                │    playerControlSystem(reg, input)   // WASD → Velocity
                │    movementSystem(reg, dt)           // Velocity → Transform
                │    physics.step(reg)                 // AABB + spatial grid resolve
                │  render:
                │    camera.viewProjection()
                │    renderSystem(reg, batch, tex)     // Transform+Sprite → SpriteBatch
                ▼
        Window (SDL2 + OpenGL context)
```

## Build phases

The phased build plan and full reference implementations are in
[`BUILD_GUIDE.md`](BUILD_GUIDE.md). All seven phases are now implemented:

1. ✅ **Phase 1 / 1.5** — window, game loop, refactor into Window/Application/Time
2. ✅ **Phase 2** — shaders, textures, the modern OpenGL pipeline
3. ✅ **Phase 3** — sprite batching + 2D camera
4. ✅ **Phase 4** — sparse-set ECS (entities, components, systems)
5. ✅ **Phase 5** — input + a playable, camera-followed player
6. ✅ **Phase 6** — 2D physics: AABB collision + spatial-grid broadphase + resolution
7. ✅ **Phase 7** — polish: cached resource manager + live in-title performance HUD

### Possible next steps

Dear ImGui debug/editor panel, a JSON scene format (`nlohmann-json`), a texture atlas
for cross-texture batching, text/HUD rendering (`stb_truetype`), and audio (SDL_mixer).
