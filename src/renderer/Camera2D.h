#pragma once
#include <glm/glm.hpp>

class Camera2D {
public:
    Camera2D(float vw, float vh) : m_vw(vw), m_vh(vh) {}
    void setViewport(float w, float h) { m_vw = w; m_vh = h; }

    glm::vec2 position{0.0f, 0.0f};
    float     zoom = 1.0f;

    glm::mat4 viewProjection() const;

    // Map a screen-space pixel (origin top-left) to a world-space point.
    glm::vec2 screenToWorld(glm::vec2 screen) const;

private:
    float m_vw, m_vh;
};
