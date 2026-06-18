#pragma once
#include "ecs/Registry.h"
#include "ecs/Components.h"

// 2D collision: AABB detection with a uniform spatial-grid broadphase (O(n)
// instead of O(n^2)) and penetration-resolution response. Run inside the
// fixed-update step (that's why the loop uses a fixed timestep).
class PhysicsSystem {
public:
    explicit PhysicsSystem(float cellSize = 128.0f) : m_cell(cellSize) {}

    void step(Registry& reg);

    int  narrowChecks()  const { return m_checks; }   // pairs AABB-tested this step
    int  resolved()      const { return m_resolved; } // overlaps resolved this step

private:
    struct Body { Transform* t; Collider* c; Velocity* v; };
    void resolvePair(Body& a, Body& b);

    float m_cell;
    int   m_checks   = 0;
    int   m_resolved = 0;
};
