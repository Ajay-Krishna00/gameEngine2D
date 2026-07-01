#include "core/Window.h"
#include <glad/glad.h>
#include <cstdio>

Window::Window(const std::string& title, int width, int height)
    : m_width(width), m_height(height) {
#ifdef SDL_HINT_WINDOWS_DPI_AWARENESS
    // Without this, Windows display scaling (e.g. 125%) bitmap-stretches the
    // window, so a 1280x720 window can overflow a small laptop screen.
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
#endif
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

void Window::setTitle(const std::string& title) {
    if (m_window) SDL_SetWindowTitle(m_window, title.c_str());
}
