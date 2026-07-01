#include "game/ArenaScene.h"
#include "ecs/Components.h"
#include "ecs/Systems.h"
#include <imgui.h>
#include <vector>

namespace {
// Arena (world units). Walls form a closed box centered on the origin.
constexpr float AW = 900.0f;   // half-width
constexpr float AH = 560.0f;   // half-height
constexpr float WT = 30.0f;    // wall thickness (half)
} // namespace

ArenaScene::ArenaScene() {
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

    spawnCrates(578);   // a field of dynamic crates bouncing off everything

    // The player: WASD-driven, collides with everything, camera follows it.
    m_player = m_reg.create();
    m_reg.add(m_player, Transform{ {0, 0}, 0.0f, {1, 1} });
    m_reg.add(m_player, Velocity{});
    m_reg.add(m_player, SpriteComp{ {40, 40}, {1.0f, 0.85f, 0.2f, 1.0f} });
    m_reg.add(m_player, Player{ 320.0f });
    m_reg.add(m_player, Collider{ {20, 20}, /*isStatic*/ false, /*restitution*/ 0.0f });
}

void ArenaScene::spawnCrates(int count) {
    for (int i = 0; i < count; ++i) {
        Entity e = m_reg.create();
        m_reg.add(e, Transform{ { m_rng.next(-AW + 40, AW - 40),
                                  m_rng.next(-AH + 40, AH - 40) }, 0.0f, {1, 1} });
        m_reg.add(e, Velocity{ { m_rng.next(-120, 120), m_rng.next(-120, 120) } });
        m_reg.add(e, SpriteComp{ {28, 28},
                   { m_rng.next(0.4f, 1.0f), m_rng.next(0.4f, 1.0f), m_rng.next(0.5f, 1.0f), 1.0f } });
        m_reg.add(e, Collider{ {14, 14}, /*isStatic*/ false, /*restitution*/ 0.95f });
    }
    m_crates += count;
}

void ArenaScene::update(float dt, const Input& input) {
    playerControlSystem(m_reg, input);
    movementSystem(m_reg, dt);     // integrate positions
    m_physics.step(m_reg);         // detect + resolve collisions (fixed step)
}

void ArenaScene::render(SpriteBatch& batch, const Texture& tex) {
    renderSystem(m_reg, batch, tex);
}

glm::vec2 ArenaScene::cameraFocus() const {
    // const_cast: Registry::get is non-const only; the lookup doesn't mutate.
    if (Transform* t = const_cast<Registry&>(m_reg).get<Transform>(m_player))
        return t->position;
    return {0.0f, 0.0f};
}

void ArenaScene::onGui() {
    ImGui::Text("crates: %d", m_crates);
    ImGui::Text("collision checks: %d   resolved: %d",
                m_physics.narrowChecks(), m_physics.resolved());
    if (ImGui::Button("Spawn 100 crates")) spawnCrates(100);
    ImGui::SameLine();
    if (ImGui::Button("Clear crates")) {
        // Crates = dynamic colliders that aren't the player. Collect first:
        // destroying inside view() would mutate the array being walked.
        std::vector<Entity> doomed;
        m_reg.view<Collider>([&](Entity e, Collider& c) {
            if (!c.isStatic && e != m_player) doomed.push_back(e);
        });
        for (Entity e : doomed) m_reg.destroy(e);
        m_crates = 0;
    }
}
