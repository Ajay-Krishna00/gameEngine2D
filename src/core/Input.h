#pragma once
#include <SDL.h>
#include <glm/glm.hpp>
#include <cstring>

// Polled input state. Call update() once per frame (the Application does this for
// you) to snapshot the keyboard + mouse. Query with isDown()/isPressed().
class Input {
public:
    void update() {
        std::memcpy(m_prev, m_cur, sizeof(m_cur));
        int n = 0;
        const Uint8* state = SDL_GetKeyboardState(&n);
        if (n > SDL_NUM_SCANCODES) n = SDL_NUM_SCANCODES;
        std::memcpy(m_cur, state, n);

        int mx = 0, my = 0;
        m_mouseButtons = SDL_GetMouseState(&mx, &my);
        m_mouse = { static_cast<float>(mx), static_cast<float>(my) };
    }

    // Held down this frame.
    bool isDown(SDL_Scancode sc) const { return m_cur[sc] != 0; }
    // Went down this frame (edge: up last frame, down now).
    bool isPressed(SDL_Scancode sc) const { return m_cur[sc] != 0 && m_prev[sc] == 0; }
    // Went up this frame.
    bool isReleased(SDL_Scancode sc) const { return m_cur[sc] == 0 && m_prev[sc] != 0; }

    glm::vec2 mousePos() const { return m_mouse; }
    bool mouseDown(int button) const { return (m_mouseButtons & SDL_BUTTON(button)) != 0; }

private:
    Uint8 m_cur[SDL_NUM_SCANCODES]  = {};
    Uint8 m_prev[SDL_NUM_SCANCODES] = {};
    glm::vec2 m_mouse{0, 0};
    Uint32 m_mouseButtons = 0;
};
