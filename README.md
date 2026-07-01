# engine2d

A 2D game engine written from scratch in **C++17**, on top of **SDL2 + OpenGL 3.3**.
No game-engine framework underneath — the window, render pipeline, batch renderer,
camera, ECS, input, physics, and debug UI are all hand-built. It's a learning /
portfolio project: every subsystem is small enough to read in one sitting and
explain out loud.

> **Status: complete (Phases 1–8).** The engine ships with two switchable demo
> scenes and a live Dear ImGui debug panel. The **Arena sandbox** is a walled
> arena with a WASD-driven, wall-collidable player and hundreds of dynamic crates
> bouncing off the walls and each other. **Breakout** is a small but complete
> game — paddle, angled ball bounces, 72 bricks, score, lives, win/lose states —
> built entirely on the engine's ECS, batch renderer, and input system.

## What's in the box

| Layer | Pieces | Status |
|---|---|---|
| **Core** | `Window` (SDL2 window + GL context, DPI-aware), `Application` (fixed-timestep loop + FPS), `Time` (high-res clock), `Input` (keyboard/mouse polling with edge detection), `DebugUI` (Dear ImGui integration) | ✅ |
| **Renderer** | `Shader`, `Texture` (via stb_image), `Camera2D` (orthographic pan/zoom + screen→world), `SpriteBatch` (one draw call per texture run), `ResourceManager` (cached textures/shaders) | ✅ |
| **ECS** | sparse-set `Registry`, `ComponentPool`, generational entity handles, variadic `view<...>()` iteration | ✅ |
| **Physics** | AABB collision, uniform spatial-grid broadphase (O(n) vs O(n²)), penetration-resolution response, run in the fixed step | ✅ |
| **Game layer** | `Scene` abstraction with runtime switching, `ArenaScene` (physics sandbox), `BreakoutScene` (a playable game shipped on the engine) | ✅ |

### Design highlights

- **Fixed-timestep game loop with an accumulator** — simulation advances in constant
  `1/60s` steps independent of framerate, so updates and physics stay deterministic;
  rendering happens once per frame. Input is snapshotted only on frames that run a
  fixed step, so `isPressed()` edges are never dropped at high framerates
  (`core/Application.cpp`).
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
- **ImGui debug panel** — live FPS/entity/draw-call/collision stats, a scene switcher,
  camera zoom, crate spawning, and Breakout tuning, drawn on top of the scene each
  frame; game input is suppressed while ImGui owns the keyboard (`core/DebugUI.cpp`).
- **Games as scenes** — `Scene` owns its own `Registry` and simulation; the engine hosts
  whichever scene is active and swaps them at runtime (`game/Scene.h`). Breakout's
  bespoke responses (angled paddle bounces, bricks dying on touch) are game code built
  on the same AABB overlap test the physics system uses — engine mechanism, game policy.

## Controls

| Key | Action |
|---|---|
| `1` / `2` | switch scene (Arena sandbox / Breakout) |
| `F1` | show/hide the debug panel |
| `W` `A` `S` `D` / arrows | Arena: move the player (camera follows) |
| `A` `D` / arrows | Breakout: move the paddle |
| `Space` | Breakout: launch ball / restart after win or game over |
| `Esc` | quit |

## Tech stack

- **C++17**, **CMake** (≥ 3.21) + **Ninja**
- **SDL2** — OS window, input, GL context
- **OpenGL 3.3 core** + **GLAD** (loader)
- **GLM** — vector/matrix math
- **stb_image** — texture loading
- **Dear ImGui** — debug/editor panel (core + GL3 backend via vcpkg; the SDL2
  backend files are vendored in `third_party/imgui/` because the vcpkg port
  dropped its `sdl2-binding` feature)
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
├─ third_party/
│  └─ imgui/             vendored Dear ImGui SDL2 backend
└─ src/
   ├─ main.cpp           entry point
   ├─ Game.{h,cpp}       hosts the scenes + the debug panel
   ├─ core/              Window, Application, Time, Input, DebugUI
   ├─ renderer/          Shader, Texture, Camera2D, SpriteBatch, ResourceManager
   ├─ ecs/               Entity, ComponentPool, Registry, Components, Systems
   ├─ physics/           PhysicsSystem (AABB + spatial grid)
   └─ game/              Scene, ArenaScene, BreakoutScene
```

## Building & running

You need **vcpkg** (with `VCPKG_ROOT` set) and an MSVC toolchain (VS Build Tools).
The full step-by-step setup — installing vcpkg, VS Code extensions, selecting the
preset, and troubleshooting — lives in [`BUILD_GUIDE.md`](BUILD_GUIDE.md).

Quick version, from a *Developer PowerShell for VS*:

```powershell
# first configure compiles SDL2/glad/glm/stb/imgui via vcpkg — a few minutes, once
cmake --preset default
cmake --build build
.\build\engine2d.exe
```

✅ **Success looks like:** a window opens on the crate-filled arena with the debug
panel in the corner; drive the yellow player box with WASD. Press `2` and you're in
Breakout — `A`/`D` to move the paddle, `Space` to launch. The panel and title bar
report live FPS / entity / draw-call stats, and the panel's *Spawn 100 crates*
button lets you stress the physics until the framerate begs for mercy.

> **Rule of thumb:** every `.cpp` you add under `src/` must be listed in the
> `add_executable(engine2d ...)` block in `CMakeLists.txt`, then re-configure. Headers
> are pulled in via `#include` and are not listed.

## How it fits together

```
main → Game : Application
                │  fixed 1/60s step:
                │    scene->update(dt, input)          // ECS systems + physics
                │      Arena:    WASD → Velocity → Transform → grid physics
                │      Breakout: paddle/ball/brick logic on AABB overlaps
                │  render:
                │    camera.viewProjection()
                │    scene->render(batch, tex)         // Transform+Sprite → SpriteBatch
                │  gui:
                │    ImGui panel (stats, scene switch, tuning)
                ▼
        Window (SDL2 + OpenGL context)
```

## Build phases

The phased build plan and full reference implementations are in
[`BUILD_GUIDE.md`](BUILD_GUIDE.md). All phases are implemented:

1. ✅ **Phase 1 / 1.5** — window, game loop, refactor into Window/Application/Time
2. ✅ **Phase 2** — shaders, textures, the modern OpenGL pipeline
3. ✅ **Phase 3** — sprite batching + 2D camera
4. ✅ **Phase 4** — sparse-set ECS (entities, components, systems)
5. ✅ **Phase 5** — input + a playable, camera-followed player
6. ✅ **Phase 6** — 2D physics: AABB collision + spatial-grid broadphase + resolution
7. ✅ **Phase 7** — polish: cached resource manager + live performance stats
8. ✅ **Phase 8** — Dear ImGui debug panel + Breakout, a real game shipped on the engine

### Possible next steps

A JSON scene format (`nlohmann-json`), a texture atlas for cross-texture batching,
text/HUD rendering (`stb_truetype`), and audio (SDL_mixer).
