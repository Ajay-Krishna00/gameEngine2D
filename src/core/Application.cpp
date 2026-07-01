#include "core/Application.h"
#include <glad/glad.h>
#include <SDL.h>

Application::Application() : m_window("engine2d", 1280, 720) {}

int Application::run() {
    if (!m_window.good()) return 1;
    m_debugUI.init(m_window.sdl(), m_window.glContext());

    Clock  clock;
    double accumulator = 0.0;

    while (m_running) {
        double frameTime = clock.tick();
        if (frameTime > 0.25) frameTime = 0.25; // avoid spiral-of-death
        accumulator += frameTime;

        // smoothed FPS, recomputed about twice a second
        m_fpsAccum += frameTime;
        ++m_fpsFrames;
        if (m_fpsAccum >= 0.5) {
            m_fps = static_cast<float>(m_fpsFrames / m_fpsAccum);
            m_fpsAccum = 0.0;
            m_fpsFrames = 0;
        }

        pollEvents();

        // Snapshot input only on frames that will run a fixed step. Above
        // 60 FPS many frames run zero steps; snapshotting every frame would
        // burn isPressed()/isReleased() edges before onUpdate() sees them.
        if (accumulator >= FIXED_DT) m_input.update();

        while (accumulator >= FIXED_DT) {
            onUpdate(static_cast<float>(FIXED_DT));
            accumulator -= FIXED_DT;
        }

        glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        onRender();

        m_debugUI.beginFrame();
        onGui();
        m_debugUI.endFrame();   // ImGui draws on top of the scene

        m_window.swapBuffers();
    }
    return 0;
}

void Application::pollEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        m_debugUI.processEvent(e);
        if (e.type == SDL_QUIT) m_running = false;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) m_running = false;
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            m_window.onResize(e.window.data1, e.window.data2);
            glViewport(0, 0, e.window.data1, e.window.data2);
        }
    }
}
