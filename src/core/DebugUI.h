#pragma once
#include <SDL.h>

// Thin wrapper around Dear ImGui's SDL2 + OpenGL3 backends (Phase 8).
// Owns init/shutdown and the per-frame begin/end; the game draws its panels
// between beginFrame() and endFrame() via Application::onGui().
class DebugUI {
public:
    DebugUI() = default;
    ~DebugUI();
    DebugUI(const DebugUI&) = delete;
    DebugUI& operator=(const DebugUI&) = delete;

    bool init(SDL_Window* window, SDL_GLContext context);
    void processEvent(const SDL_Event& e);   // forward every SDL event
    void beginFrame();                       // start a new ImGui frame
    void endFrame();                         // render ImGui on top of the scene

    // True while ImGui owns the mouse/keyboard (cursor over a panel, text
    // field focused) — the game should ignore input then.
    bool wantsMouse() const;
    bool wantsKeyboard() const;

private:
    bool m_ready = false;
};
