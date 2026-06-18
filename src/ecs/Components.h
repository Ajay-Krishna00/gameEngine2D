#pragma once
#include <glm/glm.hpp>

struct Transform  { glm::vec2 position{0,0}; float rotation = 0.0f; glm::vec2 scale{1,1}; };
struct Velocity   { glm::vec2 value{0,0}; };
struct SpriteComp { glm::vec2 size{32,32};   glm::vec4 color{1,1,1,1}; };

// Tag + tuning for the WASD-driven entity (Phase 5).
struct Player     { float speed = 300.0f; };

// Axis-aligned box collider (Phase 6). restitution: 0 = stop on hit, 1 = elastic.
struct Collider   { glm::vec2 half{16,16}; bool isStatic = false; float restitution = 0.0f; };
