#pragma once
#include "game/Scene.h"
#include "physics/PhysicsSystem.h"
#include "ecs/Entity.h"
#include <cstdint>

// The physics sandbox (Phases 5-6): a walled arena of bouncing crates plus a
// WASD-driven, collidable player that the camera follows.
class ArenaScene : public Scene {
public:
    ArenaScene();

    const char* name() const override { return "Arena sandbox"; }
    void update(float dt, const Input& input) override;
    void render(SpriteBatch& batch, const Texture& tex) override;
    void onGui() override;
    glm::vec2 cameraFocus() const override;

private:
    // Tiny deterministic LCG so the scene is the same every run.
    struct Rng {
        std::uint32_t s = 0x12345678u;
        float next(float lo, float hi) {
            s = s * 1664525u + 1013904223u;
            float t = (s >> 8) / 16777216.0f;   // [0,1)
            return lo + t * (hi - lo);
        }
    };

    void spawnCrates(int count);

    PhysicsSystem m_physics;
    Rng           m_rng;
    Entity        m_player = NULL_ENTITY;
    int           m_crates = 0;
};
