#pragma once
#include <glm/glm.hpp>

struct Transform  { glm::vec2 position{0,0}; float rotation = 0.0f; glm::vec2 scale{1,1}; };
struct Velocity   { glm::vec2 value{0,0}; };
struct SpriteComp { glm::vec2 size{32,32};   glm::vec4 color{1,1,1,1}; };

// Tag + tuning for the WASD-driven entity (Phase 5).
struct Player     { float speed = 300.0f; };
