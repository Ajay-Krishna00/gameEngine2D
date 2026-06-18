#include "physics/PhysicsSystem.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <cstdint>

namespace {
// Pack two 32-bit cell coordinates into one 64-bit map key.
inline std::int64_t cellKey(int cx, int cy) {
    return (static_cast<std::int64_t>(cx) << 32)
         ^ static_cast<std::int64_t>(static_cast<std::uint32_t>(cy));
}

// Reflect (or kill) the velocity component along the resolution axis.
inline void applyBounce(Velocity* v, const Collider* c, glm::vec2 mtv) {
    if (!v) return;
    if (mtv.x != 0.0f) v->value.x = -v->value.x * c->restitution;
    if (mtv.y != 0.0f) v->value.y = -v->value.y * c->restitution;
}
} // namespace

void PhysicsSystem::resolvePair(Body& a, Body& b) {
    if (a.c->isStatic && b.c->isStatic) return;

    glm::vec2 d       = b.t->position - a.t->position;
    glm::vec2 overlap = (a.c->half + b.c->half) - glm::abs(d);
    if (overlap.x <= 0.0f || overlap.y <= 0.0f) return;   // no penetration

    ++m_resolved;

    // Minimum translation vector: push along the axis of least penetration.
    glm::vec2 mtv{0.0f, 0.0f};
    if (overlap.x < overlap.y) mtv.x = overlap.x * (d.x < 0.0f ? -1.0f : 1.0f);
    else                       mtv.y = overlap.y * (d.y < 0.0f ? -1.0f : 1.0f);

    const bool aStatic = a.c->isStatic;
    const bool bStatic = b.c->isStatic;

    if (aStatic) {                       // only b moves
        b.t->position += mtv;
        applyBounce(b.v, b.c, mtv);
    } else if (bStatic) {                // only a moves
        a.t->position -= mtv;
        applyBounce(a.v, a.c, mtv);
    } else {                             // split the correction
        a.t->position -= mtv * 0.5f;
        b.t->position += mtv * 0.5f;
        applyBounce(a.v, a.c, mtv);
        applyBounce(b.v, b.c, mtv);
    }
}

void PhysicsSystem::step(Registry& reg) {
    m_checks = 0;
    m_resolved = 0;

    // Gather all colliding bodies (stable pointers — no pool mutation below).
    std::vector<Body> bodies;
    reg.view<Transform, Collider>([&](Entity e, Transform& t, Collider& c) {
        bodies.push_back(Body{ &t, &c, reg.get<Velocity>(e) });
    });

    // Broadphase: bucket each body into its center cell.
    std::unordered_map<std::int64_t, std::vector<int>> grid;
    grid.reserve(bodies.size() * 2);
    auto cellOf = [&](const Body& b) {
        return std::pair<int,int>{
            static_cast<int>(std::floor(b.t->position.x / m_cell)),
            static_cast<int>(std::floor(b.t->position.y / m_cell)) };
    };
    for (int i = 0; i < static_cast<int>(bodies.size()); ++i) {
        auto [cx, cy] = cellOf(bodies[i]);
        grid[cellKey(cx, cy)].push_back(i);
    }

    // Narrowphase: test each body against bodies in its 3x3 cell neighborhood.
    for (int i = 0; i < static_cast<int>(bodies.size()); ++i) {
        auto [cx, cy] = cellOf(bodies[i]);
        for (int gy = -1; gy <= 1; ++gy)
        for (int gx = -1; gx <= 1; ++gx) {
            auto it = grid.find(cellKey(cx + gx, cy + gy));
            if (it == grid.end()) continue;
            for (int j : it->second) {
                if (j <= i) continue;     // visit each unordered pair once
                ++m_checks;
                resolvePair(bodies[i], bodies[j]);
            }
        }
    }
}
