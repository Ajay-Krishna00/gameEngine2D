#pragma once
#include "ecs/Registry.h"
#include "ecs/Components.h"
#include "renderer/SpriteBatch.h"
#include "renderer/Texture.h"

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
