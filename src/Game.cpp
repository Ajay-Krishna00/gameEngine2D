#include "Game.h"
#include "ecs/Components.h"
#include "ecs/Systems.h"

Game::Game() {
    m_tex.loadFromFile("assets/textures/sprite.png");

    // spawn a grid of moving entities
    for (int i = 0; i < 2000; ++i) {
        Entity e = m_reg.create();
        m_reg.add(e, Transform{ { (float)((i % 50) * 34 - 800),
                                  (float)((i / 50) * 34 - 400) }, 0.0f, {1,1} });
        m_reg.add(e, Velocity{ { (float)((i % 7) - 3) * 10.0f, (float)((i % 5) - 2) * 10.0f } });
        m_reg.add(e, SpriteComp{ {28,28}, {1,1,1,1} });
    }

    // the player: a WASD-driven, tinted sprite the camera follows
    m_player = m_reg.create();
    m_reg.add(m_player, Transform{ {0, 0}, 0.0f, {1.5f, 1.5f} });
    m_reg.add(m_player, Velocity{});
    m_reg.add(m_player, SpriteComp{ {40, 40}, {1.0f, 0.85f, 0.2f, 1.0f} });
    m_reg.add(m_player, Player{ 300.0f });
}

void Game::onUpdate(float dt) {
    playerControlSystem(m_reg, input());
    movementSystem(m_reg, dt);

    // camera follows the player
    if (Transform* t = m_reg.get<Transform>(m_player))
        m_camera.position = t->position;
}

void Game::onRender() {
    m_camera.setViewport((float)window().width(), (float)window().height());
    m_batch.begin(m_camera.viewProjection());
    renderSystem(m_reg, m_batch, m_tex);
    m_batch.end();
}
