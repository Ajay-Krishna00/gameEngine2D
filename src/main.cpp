// =============================================================================
//  engine2d — Phase 1: window + OpenGL context + fixed-timestep game loop
//
//  This is the "hello world" of a game engine. If this compiles, runs, and
//  shows a cornflower-blue window you can close, your whole toolchain
//  (compiler + CMake + vcpkg + SDL2 + OpenGL/GLAD) is working end to end.
//
//  Everything else in the engine grows out of this file.
// =============================================================================

#include <glad/glad.h>   // MUST be included before any other OpenGL header
#include <SDL.h>
#include <cstdio>

int main(int /*argc*/, char* /*argv*/[]) {
    // ---- 1. init SDL (video subsystem) ----------------------------------
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // ---- 2. ask for an OpenGL 3.3 Core context --------------------------
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // ---- 3. create the window with an OpenGL surface --------------------
    SDL_Window* window = SDL_CreateWindow(
        "engine2d  —  Phase 1",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // ---- 4. create the GL context and load GL functions via GLAD --------
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        std::printf("Failed to load OpenGL via GLAD\n");
        return 1;
    }
    SDL_GL_SetSwapInterval(1);   // vsync on

    std::printf("OpenGL %s\n", glGetString(GL_VERSION));
    std::printf("GPU     %s\n", glGetString(GL_RENDERER));

    // ---- 5. the fixed-timestep game loop --------------------------------
    // We update the simulation in fixed 1/60s steps (deterministic physics)
    // but render as fast as vsync allows. The "accumulator" pattern below is
    // the standard solution (see "Fix Your Timestep!" by Glenn Fiedler).
    const double FIXED_DT = 1.0 / 60.0;
    const double freq      = static_cast<double>(SDL_GetPerformanceFrequency());
    Uint64       prev      = SDL_GetPerformanceCounter();
    double       accumulator = 0.0;

    bool running = true;
    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        double frameTime = (now - prev) / freq;
        prev = now;
        if (frameTime > 0.25) frameTime = 0.25;   // avoid spiral-of-death after a stall
        accumulator += frameTime;

        // --- handle OS / input events ---
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                glViewport(0, 0, e.window.data1, e.window.data2);
            }
        }

        // --- update simulation in fixed steps ---
        while (accumulator >= FIXED_DT) {
            // update(FIXED_DT);   // (Phase 4+: step ECS systems here)
            accumulator -= FIXED_DT;
        }

        // --- render ---
        glClearColor(0.39f, 0.58f, 0.93f, 1.0f);   // cornflower blue
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
    }

    // ---- 6. shutdown ----------------------------------------------------
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
