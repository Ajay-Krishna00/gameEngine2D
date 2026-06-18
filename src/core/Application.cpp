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
        m_input.update();   // snapshot keyboard/mouse for this frame

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
