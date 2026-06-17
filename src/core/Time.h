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
