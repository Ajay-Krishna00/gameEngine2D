#include "renderer/Camera2D.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Camera2D::viewProjection() const {
    float halfW = (m_vw * 0.5f) / zoom;
    float halfH = (m_vh * 0.5f) / zoom;
    // orthographic box centered on the camera; y-up
    return glm::ortho(position.x - halfW, position.x + halfW,
                      position.y - halfH, position.y + halfH,
                      -1.0f, 1.0f);
}

glm::vec2 Camera2D::screenToWorld(glm::vec2 screen) const {
    // screen: pixels, top-left origin, y-down. world: y-up, centered on camera.
    return {
        position.x + (screen.x - m_vw * 0.5f) / zoom,
        position.y - (screen.y - m_vh * 0.5f) / zoom
    };
}
