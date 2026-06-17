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

    void setTitle(const std::string& title);

    SDL_Window* sdl() const { return m_window; }

private:
    SDL_Window*   m_window  = nullptr;
    SDL_GLContext m_context = nullptr;
    int m_width;
    int m_height;
};
