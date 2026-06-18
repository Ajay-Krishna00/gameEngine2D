#include "Game.h"
#include "ecs/Components.h"
#include "ecs/Systems.h"
#include <cstdio>

namespace {
// Tiny deterministic LCG so the scene is the same every run (no <random> needed).
struct Rng {
    std::uint32_t s = 0x12345678u;
    float next(float lo, float hi) {
        s = s * 1664525u + 1013904223u;
        float t = (s >> 8) / 16777216.0f;   // [0,1)
        return lo + t * (hi - lo);
    }
};

// Arena (world units). Walls form a closed box centered on the origin.
constexpr float AW = 900.0f;   // half-width
constexpr float AH = 560.0f;   // half-height
constexpr float WT = 30.0f;    // wall thickness (half)
} // namespace

Game::Game() {
    m_tex = &m_resources.texture("assets/textures/sprite.png");
    Rng rng;

    auto spawnWall = [&](glm::vec2 pos, glm::vec2 half) {
        Entity w = m_reg.create();
        m_reg.add(w, Transform{ pos, 0.0f, {1, 1} });
        m_reg.add(w, SpriteComp{ half * 2.0f, {0.30f, 0.32f, 0.38f, 1.0f} });
        m_reg.add(w, Collider{ half, /*isStatic*/ true, 0.0f });
    };
    spawnWall({ 0,  AH + WT }, { AW + WT, WT });   // top
    spawnWall({ 0, -AH - WT }, { AW + WT, WT });   // bottom
    spawnWall({ -AW - WT, 0 }, { WT, AH });        // left
    spawnWall({  AW + WT, 0 }, { WT, AH });        // right

    // A field of dynamic crates that bounce off the walls and each other.
    const int COLS = 34, ROWS = 17;          // 578 colliding bodies
    const float SPACING = 46.0f;
    const float x0 = -(COLS - 1) * SPACING * 0.5f;
    const float y0 = -(ROWS - 1) * SPACING * 0.5f;
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            Entity e = m_reg.create();
            m_reg.add(e, Transform{ { x0 + c * SPACING, y0 + r * SPACING }, 0.0f, {1, 1} });
            m_reg.add(e, Velocity{ { rng.next(-120, 120), rng.next(-120, 120) } });
            m_reg.add(e, SpriteComp{ {28, 28},
                       { rng.next(0.4f, 1.0f), rng.next(0.4f, 1.0f), rng.next(0.5f, 1.0f), 1.0f } });
            m_reg.add(e, Collider{ {14, 14}, /*isStatic*/ false, /*restitution*/ 0.95f });
        }
    }

    // The player: WASD-driven, collides with everything, camera follows it.
    m_player = m_reg.create();
    m_reg.add(m_player, Transform{ {0, 0}, 0.0f, {1, 1} });
    m_reg.add(m_player, Velocity{});
    m_reg.add(m_player, SpriteComp{ {40, 40}, {1.0f, 0.85f, 0.2f, 1.0f} });
    m_reg.add(m_player, Player{ 320.0f });
    m_reg.add(m_player, Collider{ {20, 20}, /*isStatic*/ false, /*restitution*/ 0.0f });
}

void Game::onUpdate(float dt) {
    playerControlSystem(m_reg, input());
    movementSystem(m_reg, dt);     // integrate positions
    m_physics.step(m_reg);         // detect + resolve collisions (fixed step)

    if (Transform* t = m_reg.get<Transform>(m_player))
        m_camera.position = t->position;
}

void Game::onRender() {
    m_camera.setViewport((float)window().width(), (float)window().height());
    m_batch.begin(m_camera.viewProjection());
    renderSystem(m_reg, m_batch, *m_tex);
    m_batch.end();

    updateTitle();
}

void Game::updateTitle() {
    // Throttle: refresh the title ~4x/second, not every frame.
    if (++m_titleTimer < 15) return;
    m_titleTimer = 0;

    char buf[256];
    std::snprintf(buf, sizeof(buf),
        "engine2d  |  %.0f FPS  |  entities %zu  |  draw calls %d  |  quads %d  |  col-checks %d / resolved %d",
        fps(), m_reg.aliveCount(), m_batch.drawCalls(), m_batch.quadsDrawn(),
        m_physics.narrowChecks(), m_physics.resolved());
    window().setTitle(buf);
}
