#pragma once
#include "core/Window.h"
#include "core/Time.h"
#include "core/Input.h"

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
    Input&  input()  { return m_input; }
    float   fps() const { return m_fps; }

private:
    Window m_window;
    Input  m_input;
    bool   m_running = true;
    const double FIXED_DT = 1.0 / 60.0;

    // smoothed frames-per-second, refreshed ~twice a second
    float  m_fps        = 0.0f;
    double m_fpsAccum   = 0.0;
    int    m_fpsFrames  = 0;

    void pollEvents();
};
