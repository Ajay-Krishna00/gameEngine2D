#pragma once
#include "ecs/Registry.h"
#include "ecs/Components.h"
#include "core/Input.h"
#include "renderer/SpriteBatch.h"
#include "renderer/Texture.h"

// Drive any Player entity from WASD / arrow keys (Phase 5).
inline void playerControlSystem(Registry& reg, const Input& input) {
    reg.view<Player, Velocity>([&](Entity, Player& p, Velocity& v) {
        glm::vec2 dir{0, 0};
        if (input.isDown(SDL_SCANCODE_A) || input.isDown(SDL_SCANCODE_LEFT))  dir.x -= 1;
        if (input.isDown(SDL_SCANCODE_D) || input.isDown(SDL_SCANCODE_RIGHT)) dir.x += 1;
        if (input.isDown(SDL_SCANCODE_W) || input.isDown(SDL_SCANCODE_UP))    dir.y += 1; // y-up
        if (input.isDown(SDL_SCANCODE_S) || input.isDown(SDL_SCANCODE_DOWN))  dir.y -= 1;
        if (dir.x != 0 || dir.y != 0) dir = glm::normalize(dir);
        v.value = dir * p.speed;
    });
}

inline void movementSystem(Registry& reg, float dt) {
    reg.view<Transform, Velocity>([&](Entity, Transform& t, Velocity& v) {
        t.position += v.value * dt;
    });
}

inline void renderSystem(Registry& reg, SpriteBatch& batch, const Texture& tex) {
    reg.view<Transform, SpriteComp>([&](Entity, Transform& t, SpriteComp& s) {
        batch.draw(tex, t.position, s.size * t.scale, t.rotation, s.color);
    });
}
