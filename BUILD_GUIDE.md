# engine2d — Complete Build Guide

A from-scratch 2D game engine in **C++17 + SDL2 + OpenGL 3.3**, built on Windows with
**VS Code + MSVC Build Tools + vcpkg + CMake**.

This document is the full plan: every tool, every phase, every milestone. Work it
top to bottom. Each phase ends in something you can run and screenshot.

> **How to use this with Claude:** the scaffolding (Phase 0 + Phase 1) is already
> written for you in this folder. When you finish a phase and want the next one's
> full code, ask: *"generate Phase N for engine2d."* I'll write the files into `src/`
> and update `CMakeLists.txt`.

---

## 0. The mental model (read this once)

A "game engine" is just these layers, each built on the one below:

```
  ┌─────────────────────────────────────────────┐
  │  YOUR GAME  (built using the engine)         │   Phase 7
  ├─────────────────────────────────────────────┤
  │  ECS  — entities, components, systems        │   Phase 4
  │  Physics — collision detection + response    │   Phase 6
  ├─────────────────────────────────────────────┤
  │  Renderer — sprites, batching, camera        │   Phase 2–3
  │  Resources — shaders, textures, asset loader │   Phase 2, 7
  ├─────────────────────────────────────────────┤
  │  Core — window, OpenGL context, game loop,   │   Phase 1
  │         input, time                          │
  ├─────────────────────────────────────────────┤
  │  Platform — SDL2 (OS window/input/audio),    │   (libraries)
  │             OpenGL (GPU), GLAD, GLM, stb      │
  └─────────────────────────────────────────────┘
```

You build **bottom-up**, in thin vertical slices. Never "all of the renderer, then
all of physics." Always "the smallest thing that runs, then make it do more."

---

## 1. Toolchain setup (Phase 0)

You said VS Build Tools is installed. Two things remain: **vcpkg** (gets the
libraries) and **VS Code extensions** (the editor glue).

### 1.1 Install vcpkg

Open **PowerShell** and run:

```powershell
git clone https://github.com/microsoft/vcpkg C:\dev\vcpkg
C:\dev\vcpkg\bootstrap-vcpkg.bat
```

Then set the `VCPKG_ROOT` environment variable permanently (CMake reads it):

```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\dev\vcpkg", "User")
```

**Close and reopen** PowerShell / VS Code so the new variable is picked up.
Verify:

```powershell
echo $env:VCPKG_ROOT      # should print C:\dev\vcpkg
```

> You do **not** run `vcpkg install` by hand. This project uses **manifest mode**:
> the dependency list lives in `vcpkg.json`, and CMake installs them automatically
> the first time you configure. That first configure downloads + compiles SDL2 etc.,
> so it takes a few minutes **once**. After that it's cached.

### 1.2 VS Code extensions

Install these two (Extensions panel, `Ctrl+Shift+X`):

| Extension | Publisher | Purpose |
|---|---|---|
| **C/C++** | Microsoft | IntelliSense + the `cppvsdbg` debugger |
| **CMake Tools** | Microsoft | Configure / build / run / debug |

(Optional: **C/C++ Extension Pack** bundles both plus themes.)

### 1.3 Open the folder

In VS Code: **File → Open Folder →** `D:\CODES_GALLERY\githubGAMES\engine2d`.

CMake Tools will detect `CMakePresets.json` and the `default` preset.

---

## 2. First build & run

1. **Select the preset:** `Ctrl+Shift+P` → **CMake: Select Configure Preset** → **Default**.
2. **Select a kit/compiler if asked:** `Ctrl+Shift+P` → **CMake: Select a Kit** →
   pick the **"Visual Studio Build Tools 2022 - amd64"** entry. (This is what makes
   the MSVC compiler available to Ninja.)
3. **Configure:** `Ctrl+Shift+P` → **CMake: Configure**.
   - *First time only:* vcpkg compiles SDL2/glad/glm/stb. Grab coffee. Watch the
     Output panel; it's working, not frozen.
4. **Build:** press `F7` (or **CMake: Build**).
5. **Run with the debugger:** press `F5`.

✅ **Success looks like:** a 1280×720 window filled cornflower blue, with the
terminal printing your OpenGL version and GPU name. Press `Esc` or close the window
to quit.

If it doesn't work, jump to **§10 Troubleshooting**.

> Once you see the blue window, **you are officially building a game engine.**
> Everything from here is adding capability to a thing that already runs.

---

## 3. Project structure

```
engine2d/
├─ BUILD_GUIDE.md        ← this file
├─ vcpkg.json            ← dependency manifest (auto-installed)
├─ CMakeLists.txt        ← build config; add new .cpp files here as you go
├─ CMakePresets.json     ← wires vcpkg + Ninja
├─ .vscode/              ← editor + debugger config
├─ assets/
│  ├─ shaders/           ← .vert / .frag GLSL files (Phase 2+)
│  └─ textures/          ← .png sprites (Phase 2+)
└─ src/
   ├─ main.cpp           ← entry point (Phase 1 ✓ done)
   ├─ core/              ← Window, Application, Input, Time   (Phase 1.5)
   ├─ renderer/          ← Shader, Texture, SpriteBatch, Camera (Phase 2–3)
   ├─ ecs/               ← Registry, components, systems       (Phase 4)
   └─ physics/           ← collision detection + response      (Phase 6)
```

**Rule:** every time you add a `.cpp` file under `src/`, add it to the
`add_executable(engine2d ...)` list in `CMakeLists.txt`, then re-configure.

---

## 4. Phase 1 — Window + game loop  ✅ (already written)

**File:** `src/main.cpp` (done).

**What it teaches / what to understand before moving on** (interviewers ask this):

- **SDL2** creates the OS window and an **OpenGL context** (the GPU connection).
- **GLAD** loads the actual OpenGL function pointers at runtime (`gladLoadGLLoader`).
- **Fixed-timestep loop with an accumulator:** simulation advances in constant
  `1/60s` steps regardless of framerate, so physics is deterministic and stable.
  Rendering happens once per frame. Read Glenn Fiedler's *"Fix Your Timestep!"* —
  this pattern is the spine of the engine.
- **Double buffering + vsync:** you draw to a back buffer, then `SDL_GL_SwapWindow`
  presents it; vsync caps you to the monitor refresh and prevents tearing.

### Phase 1.5 — refactor into classes (do this before Phase 2)

Move the guts of `main.cpp` into reusable objects. This is what makes it an *engine*
and not a *program*. Target shape:

- `core/Window.h/.cpp` — owns the `SDL_Window*` + GL context; `swap()`, `size()`.
- `core/Application.h/.cpp` — owns the loop; virtual `onUpdate(dt)` / `onRender()`
  so a *game* subclasses it. (Classic engine pattern.)
- `core/Time.h` — delta time + a fixed-step helper.

`main.cpp` shrinks to:

```cpp
#include "core/Application.h"
int main(int, char**) { Application app; return app.run(); }
```

> Ask me to **"generate Phase 1.5 for engine2d"** and I'll write these classes.

---

## 5. Phase 2 — Draw a textured sprite

**Goal:** one image (a quad with a texture) on screen. This is the single biggest
conceptual jump in the whole project, because it forces the modern OpenGL pipeline.

**New files:** `renderer/Shader.{h,cpp}`, `renderer/Texture.{h,cpp}`,
`assets/shaders/sprite.vert`, `assets/shaders/sprite.frag`, a test `.png` in
`assets/textures/`.

**Concepts you must internalize:**

1. **VBO / VAO / EBO** — you upload vertex data (positions + texture coords) for a
   quad to GPU buffers and describe its layout once in a Vertex Array Object.
2. **Shaders** — a **vertex shader** transforms each vertex; a **fragment shader**
   colors each pixel by sampling the texture. You compile + link them into a program.
3. **Textures** — load pixels with `stb_image`, upload with `glTexImage2D`, sample
   in the fragment shader.
4. **The MVP matrix** — `projection * view * model` (use **GLM**). For 2D you use an
   **orthographic** projection. `model` places/scales/rotates the quad.

**Minimal sprite shaders** (write these into `assets/shaders/`):

```glsl
// sprite.vert
#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;
uniform mat4 uMVP;
out vec2 vUV;
void main() { vUV = aUV; gl_Position = uMVP * vec4(aPos, 0.0, 1.0); }
```

```glsl
// sprite.frag
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTex;
uniform vec4 uColor;
void main() { FragColor = texture(uTex, vUV) * uColor; }
```

✅ **Milestone:** a sprite drawn at a position you control. *Huge.*

> Ask me to **"generate Phase 2 for engine2d"** for the full `Shader`/`Texture`
> classes and the draw code.

---

## 6. Phase 3 — Sprite batching + 2D camera  ⭐ (first resume bullet)

Drawing one sprite per `glDrawElements` call is slow. A **batch renderer**
accumulates many sprites into one big vertex buffer and submits them in **one draw
call**. This is real engine engineering and the first thing worth quantifying.

**Design:**
- `renderer/SpriteBatch` with `begin()` → `draw(texture, transform, color, uv)` →
  `end()/flush()`.
- Internally: a CPU-side vertex array you append 4 vertices (a quad) to per sprite;
  flush when full or on `end()`; bind texture(s); one draw call.
- **Texture batching:** group by texture, or use a texture atlas, or bind multiple
  texture units and pass a per-vertex texture index (advanced).
- `renderer/Camera2D` — builds the orthographic `view*projection`; supports
  position + zoom. (You already understand this from ProceduralRacer's `worldToScreen`.)

✅ **Milestone:** hundreds–thousands of moving sprites at 60 fps in **one draw call**,
with a pan/zoom camera.

📄 **Resume bullet:** *"Batch renderer drawing N sprites per frame in a single draw
call; 2D camera with pan/zoom."* — **measure N and your fps.**

---

## 7. Phase 4 — Entity-Component-System  ⭐⭐ (headline resume bullet)

The architectural centerpiece. **Build your own** — do **not** use EnTT. This is the
single most EA-relevant thing in the project and the part interviewers will dig into.

**What an ECS is:**
- **Entity** = an id (just an integer).
- **Component** = plain data (`Transform { vec2 pos; float rot; vec2 scale; }`,
  `Sprite { TextureRef tex; vec4 color; }`, `Velocity { vec2 v; }`).
- **System** = logic that runs over all entities having a given set of components
  (`MovementSystem` reads Velocity + writes Transform; `RenderSystem` reads
  Transform + Sprite and calls the SpriteBatch).

**Two implementation tiers — pick based on time:**

| Tier | Storage | Pitch |
|---|---|---|
| **A. Sparse-set** (recommended) | one packed array per component type + sparse lookup | cache-friendly, the modern approach, great talking point |
| B. Archetype | entities grouped by component signature | what Unity DOTS/EnTT do; harder, only if you have time |

Start with **sparse-set**. Core pieces: an `Entity` id allocator (with generation
counters to detect stale handles), a `ComponentPool<T>` (sparse-set), and a
`Registry` that ties them together with `create()`, `add<T>()`, `get<T>()`,
`view<A,B>()` iteration.

✅ **Milestone:** movement + rendering both run *through* the ECS. Spawn 10k entities,
watch them update at 60 fps.

📄 **Resume bullet:** *"Designed a cache-friendly sparse-set ECS handling 10k+
entities at 60 fps; data-oriented component storage, generational entity handles."*

> This is the phase to ask me for the most detail — **"generate Phase 4 for
> engine2d"** — and to make sure you can explain *why* sparse sets are cache-friendly.

---

## 8. Phase 5 — Input + a playable thing

**New file:** `core/Input.h/.cpp` — wraps SDL keyboard/mouse into a queryable state
(`Input::isDown(Key::W)`, `Input::mousePos()`), updated once per frame from the SDL
event loop. Feed it into a `PlayerControlSystem`.

✅ **Milestone:** a sprite you drive with WASD. Now it's a *game*, not a demo.

---

## 9. Phase 6 — 2D physics & collision  ⭐ (second resume bullet)

**Scope it to 2D and keep it honest.** Two layers:

1. **Detection:**
   - **AABB vs AABB** (axis-aligned boxes) — start here, it's enough for a platformer.
   - Optional: **circle vs circle**, **AABB vs circle**.
   - **Broadphase** to avoid O(n²): a uniform **spatial grid** (or sweep-and-prune).
     This is a great bullet on its own.
2. **Response:** resolve penetration (push boxes apart along the minimum
   translation), reflect/zero velocity. Optional impulse resolution for bounce.

Add `Collider` components and a `PhysicsSystem` that runs in the fixed-update step
(this is *why* Phase 1 used a fixed timestep).

✅ **Milestone:** characters that collide with walls and each other; nothing tunnels
through at speed (clamp or sub-step).

📄 **Resume bullet:** *"2D physics with AABB collision, spatial-grid broadphase
(O(n) vs O(n²)), and penetration-resolution response, stepped at a fixed 60 Hz."*

---

## 10. Phase 7 — Polish (what makes it portfolio-grade)

Pick from these in priority order; each is a visible win:

1. ✅ **Dear ImGui debug/editor panel** — done (Phase 8): `imgui[opengl3-binding]`
   via vcpkg + the vendored SDL2 backend in `third_party/imgui/`, wrapped in
   `core/DebugUI`. Live panel with fps/entity/draw-call stats, scene switcher,
   camera zoom, crate spawner, and Breakout tuning.
2. ✅ **A tiny demo game built on the engine** — done (Phase 8): Breakout in
   `game/BreakoutScene.cpp` — paddle, angled bounces, bricks, score, lives,
   win/lose — switchable at runtime against the arena sandbox via `game/Scene`.
3. ✅ **Asset/resource manager** — done (Phase 7): load + cache textures/shaders
   by name; don't reload duplicates.
4. **Simple scene format** — load entities from a JSON file (add `nlohmann-json` via
   vcpkg). Shows data-driven design.
5. **Text rendering** — bitmap font or `stb_truetype` for a real score/HUD.
6. **Audio** — SDL_mixer for SFX/music (you already have the SDL dependency).

---

## 11. Suggested schedule (adjust to your placement timeline)

| Week | Deliverable |
|---|---|
| 1 | Phase 0–1 working, Phase 1.5 refactor |
| 2 | Phase 2 (sprite) + Phase 3 (batch + camera) |
| 3 | Phase 4 (ECS) — spend real time here |
| 4 | Phase 5 (input) + Phase 6 (physics) |
| 5 | Phase 7: ImGui panel + a small demo game |
| 6 | README with GIFs, cleanup, resume bullets, mock interview answers |

If crunched: **Phases 1–5 polished beats 1–7 half-done.** A batch renderer + a real
ECS + a controllable, colliding demo is already a strong, non-generic portfolio piece.

---

## 12. Making it count on the resume / GitHub

- **README first impression:** a GIF at the very top (record with ScreenToGif), then
  a one-paragraph "what it is," an architecture diagram (reuse §0), and a
  "what was hard / what I learned" section. Recruiters skim; the GIF sells it.
- **Quantify everything:** entity count, sprites/draw-call, fps, frame time, memory.
  Numbers survive a 10-second resume scan.
- **Name techniques, not genres:** "sparse-set ECS," "batch renderer," "AABB
  broadphase," "orthographic camera," "fixed timestep." These are the keywords an EA
  engineer's eye catches.
- **Be able to explain every subsystem out loud.** EA *will* ask you to walk through
  your ECS and your renderer. If a part is fuzzy, that's the part to rebuild until
  it's yours.
- **Commit history matters:** small, frequent, well-messaged commits show process.
  Run `git init` in this folder early.

---

## 13. Troubleshooting

| Symptom | Fix |
|---|---|
| `VCPKG_ROOT` not found / toolchain error | Set the env var (§1.1), then **fully restart** VS Code so it inherits the variable. |
| First configure is slow / looks frozen | Normal — vcpkg is compiling SDL2 the first time. Watch the Output panel. |
| `Ninja not found` | VS Build Tools includes Ninja+CMake. Launch VS Code from a **"Developer PowerShell for VS"**, or remove `"generator": "Ninja"` from `CMakePresets.json` to use the default. |
| `Could not find SDL2` | Confirm `vcpkg.json` is present and the toolchain file is set; re-run **CMake: Configure**. |
| Black/blank window instead of blue | GL context or GLAD load failed — check the terminal for the printed error; confirm the requested GL 3.3 core profile is supported. |
| IntelliSense red squiggles but it builds fine | Run **CMake: Configure** once so `compile_commands.json` is generated; reload window. |
| Debugger won't start | Ensure the build succeeded and `build/engine2d.exe` exists; the `cppvsdbg` type needs the MSVC toolchain (you have it). |

---

## 14. What to ask Claude for next

- *"generate Phase 1.5 for engine2d"* — refactor into Window/Application/Time.
- *"generate Phase 2 for engine2d"* — Shader + Texture + first sprite.
- *"generate Phase 3 for engine2d"* — SpriteBatch + Camera2D.
- *"generate Phase 4 for engine2d"* — sparse-set ECS (the big one).
- *"explain the tire-grip / ECS / batching math until I can teach it back."*

Build it slice by slice. Each phase, you'll have something that runs — and by the end,
something almost nobody else on the shortlist will have.

---
---

# PART II — Full Implementations

Everything below is real, compilable code. Create each file at the path in its
heading, then update `CMakeLists.txt`'s `add_executable` list (see §II.6) and re-run
**CMake: Configure**. Build (`F7`) and run (`F5`) after each phase.

> Paths are relative to `engine2d/`. Every `.cpp` you add must be listed in
> `CMakeLists.txt`. Headers (`.h`) do **not** need listing.

---

## II.1 — Phase 1.5: Window / Application / Time

This turns the monolithic `main.cpp` into reusable engine objects. A *game* will
subclass `Application` and override `onUpdate` / `onRender`.

### `src/core/Time.h`
```cpp
#pragma once
#include <SDL.h>

// High-resolution clock. tick() returns seconds elapsed since the last tick().
class Clock {
public:
    Clock()
        : m_freq(static_cast<double>(SDL_GetPerformanceFrequency())),
          m_prev(SDL_GetPerformanceCounter()) {}

    double tick() {
        Uint64 now = SDL_GetPerformanceCounter();
        double dt  = (now - m_prev) / m_freq;
        m_prev = now;
        return dt;
    }
private:
    double m_freq;
    Uint64 m_prev;
};
```

### `src/core/Window.h`
```cpp
#pragma once
#include <SDL.h>
#include <string>

class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void swapBuffers();
    bool good() const { return m_window && m_context; }

    int  width()  const { return m_width; }
    int  height() const { return m_height; }
    void onResize(int w, int h) { m_width = w; m_height = h; }

    SDL_Window* sdl() const { return m_window; }

private:
    SDL_Window*   m_window  = nullptr;
    SDL_GLContext m_context = nullptr;
    int m_width;
    int m_height;
};
```

### `src/core/Window.cpp`
```cpp
#include "core/Window.h"
#include <glad/glad.h>
#include <cstdio>

Window::Window(const std::string& title, int width, int height)
    : m_width(width), m_height(height) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    m_window = SDL_CreateWindow(title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_window) { std::printf("CreateWindow: %s\n", SDL_GetError()); return; }

    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context) { std::printf("GL context: %s\n", SDL_GetError()); return; }

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        std::printf("GLAD load failed\n");
        return;
    }
    SDL_GL_SetSwapInterval(1); // vsync
    std::printf("OpenGL %s | GPU %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));
}

Window::~Window() {
    if (m_context) SDL_GL_DeleteContext(m_context);
    if (m_window)  SDL_DestroyWindow(m_window);
}

void Window::swapBuffers() { SDL_GL_SwapWindow(m_window); }
```

### `src/core/Application.h`
```cpp
#pragma once
#include "core/Window.h"
#include "core/Time.h"

// Base engine application: owns the window + the fixed-timestep loop.
// A game subclasses this and overrides onUpdate()/onRender().
class Application {
public:
    Application();
    virtual ~Application() = default;
    int run();

protected:
    virtual void onUpdate(float dt) {}  // fixed-step simulation (physics/ECS)
    virtual void onRender() {}          // draw a frame

    Window& window() { return m_window; }

private:
    Window m_window;
    bool   m_running = true;
    const double FIXED_DT = 1.0 / 60.0;

    void pollEvents();
};
```

### `src/core/Application.cpp`
```cpp
#include "core/Application.h"
#include <glad/glad.h>
#include <SDL.h>

Application::Application() : m_window("engine2d", 1280, 720) {}

int Application::run() {
    if (!m_window.good()) return 1;

    Clock  clock;
    double accumulator = 0.0;

    while (m_running) {
        double frameTime = clock.tick();
        if (frameTime > 0.25) frameTime = 0.25; // avoid spiral-of-death
        accumulator += frameTime;

        pollEvents();

        while (accumulator >= FIXED_DT) {
            onUpdate(static_cast<float>(FIXED_DT));
            accumulator -= FIXED_DT;
        }

        glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        onRender();
        m_window.swapBuffers();
    }
    return 0;
}

void Application::pollEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) m_running = false;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) m_running = false;
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            m_window.onResize(e.window.data1, e.window.data2);
            glViewport(0, 0, e.window.data1, e.window.data2);
        }
    }
}
```

### `src/main.cpp` (replace the Phase 1 version)
```cpp
#include "core/Application.h"
int main(int, char**) {
    Application app;
    return app.run();
}
```

✅ Same blue window, now structured as an engine. **Add to `add_executable`:**
`src/core/Window.cpp`, `src/core/Application.cpp`.

---

## II.2 — Phase 2: Shader + Texture (the OpenGL pipeline)

### `src/renderer/Shader.h`
```cpp
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    Shader() = default;
    ~Shader();
    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath);
    void bind() const { glUseProgram(m_program); }

    void setInt (const std::string& name, int v);
    void setVec4(const std::string& name, const glm::vec4& v);
    void setMat4(const std::string& name, const glm::mat4& m);

private:
    GLuint m_program = 0;
    GLint  location(const std::string& name);
    static GLuint compile(GLenum type, const std::string& src);
};
```

### `src/renderer/Shader.cpp`
```cpp
#include "renderer/Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <cstdio>

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}

GLuint Shader::compile(GLenum type, const std::string& src) {
    GLuint s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[1024]; glGetShaderInfoLog(s, 1024, nullptr, log);
               std::printf("Shader compile error:\n%s\n", log); }
    return s;
}

bool Shader::loadFromFiles(const std::string& vertPath, const std::string& fragPath) {
    GLuint vs = compile(GL_VERTEX_SHADER,   readFile(vertPath));
    GLuint fs = compile(GL_FRAGMENT_SHADER, readFile(fragPath));
    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);
    GLint ok; glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (!ok) { char log[1024]; glGetProgramInfoLog(m_program, 1024, nullptr, log);
               std::printf("Shader link error:\n%s\n", log); }
    glDeleteShader(vs); glDeleteShader(fs);
    return ok != 0;
}

Shader::~Shader() { if (m_program) glDeleteProgram(m_program); }

GLint Shader::location(const std::string& name) {
    return glGetUniformLocation(m_program, name.c_str());
}
void Shader::setInt (const std::string& n, int v)              { glUniform1i(location(n), v); }
void Shader::setVec4(const std::string& n, const glm::vec4& v) { glUniform4fv(location(n), 1, glm::value_ptr(v)); }
void Shader::setMat4(const std::string& n, const glm::mat4& m) { glUniformMatrix4fv(location(n), 1, GL_FALSE, glm::value_ptr(m)); }
```

### `src/renderer/Texture.h`
```cpp
#pragma once
#include <glad/glad.h>
#include <string>

class Texture {
public:
    Texture() = default;
    ~Texture();
    bool loadFromFile(const std::string& path);
    void bind(unsigned slot = 0) const;

    int    width()  const { return m_w; }
    int    height() const { return m_h; }
    GLuint id()     const { return m_id; }

private:
    GLuint m_id = 0;
    int m_w = 0, m_h = 0;
};
```

### `src/renderer/Texture.cpp`
```cpp
#include "renderer/Texture.h"
#define STB_IMAGE_IMPLEMENTATION         // define in exactly ONE .cpp
#include <stb_image.h>
#include <cstdio>

bool Texture::loadFromFile(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);  // OpenGL UV origin is bottom-left
    int channels;
    unsigned char* data = stbi_load(path.c_str(), &m_w, &m_h, &channels, 4);
    if (!data) { std::printf("Texture load failed: %s\n", path.c_str()); return false; }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_w, m_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return true;
}

Texture::~Texture() { if (m_id) glDeleteTextures(1, &m_id); }

void Texture::bind(unsigned slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_id);
}
```

> **Phase 2 is validated by Phase 3's batch.** Rather than write a throwaway
> single-quad renderer here, go straight to the SpriteBatch below — it uses these
> `Shader`/`Texture` classes and is the real, reusable thing. Drop a small PNG at
> `assets/textures/sprite.png` first (any 32×32-ish image works).

**Add to `add_executable`:** `src/renderer/Shader.cpp`, `src/renderer/Texture.cpp`.

---

## II.3 — Phase 3: Camera2D + SpriteBatch ⭐

### `assets/shaders/sprite.vert`
```glsl
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
uniform mat4 uMVP;
out vec2 vUV;
out vec4 vColor;
void main() {
    vUV = aUV;
    vColor = aColor;
    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
}
```

### `assets/shaders/sprite.frag`
```glsl
#version 330 core
in vec2 vUV;
in vec4 vColor;
out vec4 FragColor;
uniform sampler2D uTex;
void main() {
    FragColor = texture(uTex, vUV) * vColor;
}
```

### `src/renderer/Camera2D.h`
```cpp
#pragma once
#include <glm/glm.hpp>

class Camera2D {
public:
    Camera2D(float vw, float vh) : m_vw(vw), m_vh(vh) {}
    void setViewport(float w, float h) { m_vw = w; m_vh = h; }

    glm::vec2 position{0.0f, 0.0f};
    float     zoom = 1.0f;

    glm::mat4 viewProjection() const;

private:
    float m_vw, m_vh;
};
```

### `src/renderer/Camera2D.cpp`
```cpp
#include "renderer/Camera2D.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Camera2D::viewProjection() const {
    float halfW = (m_vw * 0.5f) / zoom;
    float halfH = (m_vh * 0.5f) / zoom;
    // orthographic box centered on the camera; y-up
    return glm::ortho(position.x - halfW, position.x + halfW,
                      position.y - halfH, position.y + halfH,
                      -1.0f, 1.0f);
}
```

### `src/renderer/SpriteBatch.h`
```cpp
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "renderer/Shader.h"

class Texture;

// Accumulates quads and submits them in as few draw calls as possible.
// This version batches runs of the SAME texture into one draw call and
// flushes when the texture changes or the buffer fills.
class SpriteBatch {
public:
    SpriteBatch();
    ~SpriteBatch();

    void begin(const glm::mat4& viewProj);
    void draw(const Texture& tex, glm::vec2 pos, glm::vec2 size,
              float rot = 0.0f, glm::vec4 color = glm::vec4(1.0f));
    void end();

    int drawCalls() const { return m_drawCalls; }

private:
    struct Vertex { glm::vec2 pos; glm::vec2 uv; glm::vec4 color; };
    static constexpr int MAX_QUADS   = 10000;
    static constexpr int MAX_VERTS   = MAX_QUADS * 4;
    static constexpr int MAX_INDICES = MAX_QUADS * 6;

    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
    Shader m_shader;
    std::vector<Vertex> m_verts;
    GLuint    m_curTexture = 0;
    glm::mat4 m_viewProj{1.0f};
    int       m_drawCalls = 0;

    void flush();
};
```

### `src/renderer/SpriteBatch.cpp`
```cpp
#include "renderer/SpriteBatch.h"
#include "renderer/Texture.h"
#include <cmath>
#include <cstddef>

SpriteBatch::SpriteBatch() {
    m_verts.reserve(MAX_VERTS);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTS * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    // index buffer is static: 6 indices per quad, never changes
    std::vector<GLuint> indices(MAX_INDICES);
    GLuint off = 0;
    for (int i = 0; i < MAX_INDICES; i += 6) {
        indices[i+0] = off + 0; indices[i+1] = off + 1; indices[i+2] = off + 2;
        indices[i+3] = off + 2; indices[i+4] = off + 3; indices[i+5] = off + 0;
        off += 4;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(GLuint),
                 indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glBindVertexArray(0);

    m_shader.loadFromFiles("assets/shaders/sprite.vert", "assets/shaders/sprite.frag");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

SpriteBatch::~SpriteBatch() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void SpriteBatch::begin(const glm::mat4& vp) {
    m_viewProj   = vp;
    m_curTexture = 0;
    m_drawCalls  = 0;
    m_verts.clear();
}

void SpriteBatch::draw(const Texture& tex, glm::vec2 pos, glm::vec2 size,
                       float rot, glm::vec4 color) {
    if ((m_curTexture != 0 && m_curTexture != tex.id()) ||
        m_verts.size() >= static_cast<size_t>(MAX_VERTS)) {
        flush();
    }
    m_curTexture = tex.id();

    glm::vec2 h = size * 0.5f;
    glm::vec2 corners[4] = { {-h.x,-h.y}, { h.x,-h.y}, { h.x, h.y}, {-h.x, h.y} };
    glm::vec2 uv[4]      = { { 0, 0 },    { 1, 0 },    { 1, 1 },    { 0, 1 } };

    float c = std::cos(rot), s = std::sin(rot);
    for (int i = 0; i < 4; ++i) {
        glm::vec2 r = { corners[i].x * c - corners[i].y * s,
                        corners[i].x * s + corners[i].y * c };
        m_verts.push_back({ pos + r, uv[i], color });
    }
}

void SpriteBatch::flush() {
    if (m_verts.empty()) return;
    int quadCount = static_cast<int>(m_verts.size()) / 4;

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_verts.size() * sizeof(Vertex), m_verts.data());

    m_shader.bind();
    m_shader.setMat4("uMVP", m_viewProj);
    m_shader.setInt("uTex", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_curTexture);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, quadCount * 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    ++m_drawCalls;
    m_verts.clear();
}

void SpriteBatch::end() { flush(); }
```

✅ Thousands of sprites, batched. **Add to `add_executable`:**
`src/renderer/Camera2D.cpp`, `src/renderer/SpriteBatch.cpp`.

---

## II.4 — Phase 4: Sparse-set ECS ⭐⭐

### `src/ecs/Entity.h`
```cpp
#pragma once
#include <cstdint>

// Entity = 32-bit index + 32-bit generation, packed into 64 bits.
// The generation invalidates stale handles after an index is recycled.
using Entity = std::uint64_t;
constexpr Entity NULL_ENTITY = 0xFFFFFFFFFFFFFFFFull;

inline std::uint32_t entityIndex(Entity e)      { return static_cast<std::uint32_t>(e & 0xFFFFFFFFull); }
inline std::uint32_t entityGeneration(Entity e) { return static_cast<std::uint32_t>(e >> 32); }
inline Entity makeEntity(std::uint32_t idx, std::uint32_t gen) {
    return (static_cast<Entity>(gen) << 32) | static_cast<Entity>(idx);
}
```

### `src/ecs/ComponentPool.h`
```cpp
#pragma once
#include <vector>
#include <cstdint>

// Sparse set: a packed (dense) array of components for cache-friendly iteration,
// plus a sparse array mapping entity-index -> position in the dense array.
// O(1) add / remove (swap-and-pop) / get.
template <typename T>
class ComponentPool {
public:
    bool has(std::uint32_t idx) const {
        return idx < m_sparse.size()
            && m_sparse[idx] != NPOS
            && m_sparse[idx] < m_denseEntities.size()
            && m_denseEntities[m_sparse[idx]] == idx;
    }

    T& add(std::uint32_t idx, const T& value) {
        if (idx >= m_sparse.size()) m_sparse.resize(idx + 1, NPOS);
        if (has(idx)) { m_dense[m_sparse[idx]] = value; return m_dense[m_sparse[idx]]; }
        m_sparse[idx] = static_cast<std::uint32_t>(m_dense.size());
        m_dense.push_back(value);
        m_denseEntities.push_back(idx);
        return m_dense.back();
    }

    void remove(std::uint32_t idx) {
        if (!has(idx)) return;
        std::uint32_t pos     = m_sparse[idx];
        std::uint32_t lastPos = static_cast<std::uint32_t>(m_dense.size()) - 1;
        // swap the removed element with the last, then pop — keeps dense packed
        m_dense[pos]         = m_dense[lastPos];
        m_denseEntities[pos] = m_denseEntities[lastPos];
        m_sparse[m_denseEntities[pos]] = pos;
        m_dense.pop_back();
        m_denseEntities.pop_back();
        m_sparse[idx] = NPOS;
    }

    T* get(std::uint32_t idx) { return has(idx) ? &m_dense[m_sparse[idx]] : nullptr; }

    std::vector<T>&             data()     { return m_dense; }
    std::vector<std::uint32_t>& entities() { return m_denseEntities; }
    std::size_t size() const { return m_dense.size(); }

private:
    static constexpr std::uint32_t NPOS = 0xFFFFFFFFu;
    std::vector<std::uint32_t> m_sparse;         // entity index -> dense position
    std::vector<T>             m_dense;          // packed components
    std::vector<std::uint32_t> m_denseEntities;  // packed owning entity indices
};
```

### `src/ecs/Registry.h`
```cpp
#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include "ecs/Entity.h"
#include "ecs/ComponentPool.h"

class Registry {
public:
    Entity create() {
        std::uint32_t idx;
        if (!m_free.empty()) { idx = m_free.back(); m_free.pop_back(); }
        else { idx = static_cast<std::uint32_t>(m_generations.size()); m_generations.push_back(0); }
        return makeEntity(idx, m_generations[idx]);
    }

    void destroy(Entity e) {
        std::uint32_t idx = entityIndex(e);
        if (idx >= m_generations.size()) return;
        for (auto& p : m_pools) if (p) p->removeIfPresent(idx);
        ++m_generations[idx];          // invalidate existing handles to this index
        m_free.push_back(idx);
    }

    bool valid(Entity e) const {
        std::uint32_t idx = entityIndex(e);
        return idx < m_generations.size() && m_generations[idx] == entityGeneration(e);
    }

    template <typename T> T&   add(Entity e, const T& c) { return pool<T>().add(entityIndex(e), c); }
    template <typename T> T*   get(Entity e)             { return pool<T>().get(entityIndex(e)); }
    template <typename T> bool has(Entity e)             { return pool<T>().has(entityIndex(e)); }
    template <typename T> void remove(Entity e)          { pool<T>().remove(entityIndex(e)); }

    // Call fn(entity, A&, Rest&...) for every entity that has ALL of A, Rest...
    // NOTE: don't add/remove components of the iterated types inside fn — that
    // mutates the dense array you're walking. Modifying VALUES is fine.
    template <typename A, typename... Rest, typename Fn>
    void view(Fn fn) {
        auto& a    = pool<A>();
        auto& ents = a.entities();
        for (std::size_t i = 0; i < ents.size(); ++i) {
            std::uint32_t idx = ents[i];
            if ((pool<Rest>().has(idx) && ...)) {
                fn(makeEntity(idx, m_generations[idx]), a.data()[i], *pool<Rest>().get(idx)...);
            }
        }
    }

private:
    struct IPool { virtual ~IPool() = default; virtual void removeIfPresent(std::uint32_t) = 0; };
    template <typename T>
    struct PoolHolder : IPool {
        ComponentPool<T> pool;
        void removeIfPresent(std::uint32_t idx) override { pool.remove(idx); }
    };

    static std::uint32_t nextTypeId() { static std::uint32_t id = 0; return id++; }
    template <typename T> static std::uint32_t typeId() { static std::uint32_t id = nextTypeId(); return id; }

    template <typename T>
    ComponentPool<T>& pool() {
        std::uint32_t id = typeId<T>();
        if (id >= m_pools.size()) m_pools.resize(id + 1);
        if (!m_pools[id]) m_pools[id] = std::make_unique<PoolHolder<T>>();
        return static_cast<PoolHolder<T>*>(m_pools[id].get())->pool;
    }

    std::vector<std::uint32_t>          m_generations;
    std::vector<std::uint32_t>          m_free;
    std::vector<std::unique_ptr<IPool>> m_pools;
};
```

### `src/ecs/Components.h`
```cpp
#pragma once
#include <glm/glm.hpp>

struct Transform  { glm::vec2 position{0,0}; float rotation = 0.0f; glm::vec2 scale{1,1}; };
struct Velocity   { glm::vec2 value{0,0}; };
struct SpriteComp { glm::vec2 size{32,32};   glm::vec4 color{1,1,1,1}; };
```

### `src/ecs/Systems.h`
```cpp
#pragma once
#include "ecs/Registry.h"
#include "ecs/Components.h"
#include "renderer/SpriteBatch.h"
#include "renderer/Texture.h"

inline void movementSystem(Registry& reg, float dt) {
    reg.view<Transform, Velocity>([&](Entity, Transform& t, Velocity& v) {
        t.position += v.value * dt;
    });
}

inline void renderSystem(Registry& reg, SpriteBatch& batch, const Texture& tex) {
    reg.view<Transform, SpriteComp>([&](Entity, Transform& t, SpriteComp& s) {
        batch.draw(tex, t.position, s.size * t.scale, t.rotation, s.color);
    });
}
```

### `src/Game.h` — tie it all together
```cpp
#pragma once
#include "core/Application.h"
#include "renderer/SpriteBatch.h"
#include "renderer/Camera2D.h"
#include "renderer/Texture.h"
#include "ecs/Registry.h"

class Game : public Application {
public:
    Game();
protected:
    void onUpdate(float dt) override;
    void onRender() override;
private:
    Registry    m_reg;
    SpriteBatch m_batch;          // constructed AFTER Application's Window => GL is ready
    Camera2D    m_camera{1280, 720};
    Texture     m_tex;
};
```

### `src/Game.cpp`
```cpp
#include "Game.h"
#include "ecs/Components.h"
#include "ecs/Systems.h"

Game::Game() {
    m_tex.loadFromFile("assets/textures/sprite.png");

    // spawn a grid of moving entities
    for (int i = 0; i < 2000; ++i) {
        Entity e = m_reg.create();
        m_reg.add(e, Transform{ { (float)((i % 50) * 34 - 800),
                                  (float)((i / 50) * 34 - 400) }, 0.0f, {1,1} });
        m_reg.add(e, Velocity{ { (float)((i % 7) - 3) * 10.0f, (float)((i % 5) - 2) * 10.0f } });
        m_reg.add(e, SpriteComp{ {28,28}, {1,1,1,1} });
    }
}

void Game::onUpdate(float dt) {
    movementSystem(m_reg, dt);
}

void Game::onRender() {
    m_camera.setViewport((float)window().width(), (float)window().height());
    m_batch.begin(m_camera.viewProjection());
    renderSystem(m_reg, m_batch, m_tex);
    m_batch.end();
}
```

### `src/main.cpp` (final form)
```cpp
#include "Game.h"
int main(int, char**) {
    Game game;
    return game.run();
}
```

✅ 2000 entities, moving, rendered through the ECS, batched. **Add to
`add_executable`:** `src/Game.cpp`. (ECS headers need no listing.)

---

## II.5 — "Teach it back" — the math you must own

EA will ask you to explain your own code. Here are the four explanations in plain
language. If you can say these out loud without notes, you're interview-ready.

### (a) Tire-grip decomposition (ProceduralRacer)
Velocity is a vector. Split it into the part **along** the car's heading (forward)
and the part **across** it (sideways/lateral) by projecting onto the heading unit
vector: `forward = (v · heading) * heading`, `lateral = v − forward`. A real tire
resists sideways motion, so each frame you **keep only a fraction** of the lateral
part (`v = forward + lateral * gripKeep`). Low `gripKeep` = grippy (lateral velocity
killed → tight cornering). High `gripKeep` (handbrake, grass) = the back slides →
**drift**. That one decomposition is the entire feel of the car.

### (b) Why a sparse set is cache-friendly (ECS)
Components live in one **contiguous** `m_dense` array with no gaps. Iterating a
system walks that array front-to-back, so the CPU prefetcher streams the next
components into cache before you ask for them — almost zero cache misses. The
`m_sparse` array is just an index map for O(1) random lookup; the hot path
(iteration) never touches it. Compare to storing components on scattered heap
objects (one `new` per entity): every access is a cache miss. Removal uses
**swap-and-pop** — move the last element into the hole and shrink — so the array
stays packed without shifting everything.

### (c) Why batching is one draw call (renderer)
Each `glDrawElements` is a CPU→GPU round trip with fixed overhead; 1000 sprites =
1000 trips = CPU-bound. The batch appends every sprite's 4 vertices into **one** big
CPU array, uploads it **once** (`glBufferSubData`), and issues **one** draw call. The
GPU was never the bottleneck — the per-call overhead was. You only split into
multiple calls when the **texture** changes (the GPU can sample one bound texture per
call). Atlases / multi-texture batching reduce even those splits.

### (d) The orthographic MVP (camera)
`gl_Position = projection * view * model * vertex`. In 2D, `model` places/rotates/
scales a quad in world space (the batch bakes this into the vertices directly).
`view*projection` is an **orthographic** box (`glm::ortho`) — no perspective, so a
sprite is the same size regardless of "depth." Moving the box = panning the camera;
shrinking the box = zooming in. The GPU maps whatever is inside the box to the screen.

---

## II.6 — Final `CMakeLists.txt` source list

By the end of Phase 4 your `add_executable` should read:

```cmake
add_executable(engine2d
    src/main.cpp
    src/Game.cpp
    src/core/Window.cpp
    src/core/Application.cpp
    src/renderer/Shader.cpp
    src/renderer/Texture.cpp
    src/renderer/Camera2D.cpp
    src/renderer/SpriteBatch.cpp
)
```

Re-run **CMake: Configure** after editing this, then `F7` to build. Header-only
files (`Time.h`, all of `ecs/`, `Components.h`, `Systems.h`, `Game.h`) are pulled in
via `#include` and must **not** be listed.

---

## II.7 — Build order recap

1. Phase 1.5 files → update CMake → build → blue window (now class-based).
2. Phase 2 files (Shader, Texture) → build (nothing visible yet).
3. Phase 3 (shaders + Camera2D + SpriteBatch) → build.
4. Phase 4 (ecs/ + Game + new main) → build → **2000 sprites moving**. 🎉

From here, Phases 5–7 (input, physics, ImGui, demo game) extend this same skeleton.
Ask: *"generate Phase 5/6/7 for engine2d"* when you're ready.
```
