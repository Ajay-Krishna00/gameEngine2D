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
