#pragma once
#include <glm/glm.hpp>
#include "ecs/Registry.h"
#include "core/Input.h"
#include "renderer/SpriteBatch.h"
#include "renderer/Texture.h"

// A self-contained demo scene: owns its entities and simulation, renders
// through the shared batch, and contributes a section to the debug panel.
// Game swaps between scenes at runtime (Phase 8).
class Scene {
public:
    virtual ~Scene() = default;

    virtual const char* name() const = 0;
    virtual void update(float dt, const Input& input) = 0;   // fixed step
    virtual void render(SpriteBatch& batch, const Texture& tex) = 0;
    virtual void onGui() {}                                   // ImGui section

    // Where the camera should look this frame.
    virtual glm::vec2 cameraFocus() const { return {0.0f, 0.0f}; }

    Registry& registry() { return m_reg; }

protected:
    Registry m_reg;
};
