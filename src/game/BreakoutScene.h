#pragma once
#include "game/Scene.h"
#include "ecs/Entity.h"

// Breakout — the "tiny game shipped on the engine" (Phase 8). Entities,
// storage, and rendering go through the ECS + batch renderer; the ball's
// bounce/brick response is game code layered on the same AABB overlap test
// the physics system uses (a paddle needs angled bounces and bricks must
// die on touch, which generic penetration resolution doesn't express).
class BreakoutScene : public Scene {
public:
    BreakoutScene();

    const char* name() const override { return "Breakout"; }
    void update(float dt, const Input& input) override;
    void render(SpriteBatch& batch, const Texture& tex) override;
    void onGui() override;

    int  score() const { return m_score; }
    int  lives() const { return m_lives; }

private:
    enum class State { Ready, Playing, Won, GameOver };

    void spawnBricks();
    void resetBall();          // glue the ball back onto the paddle
    void restart();            // full new game (bricks, score, lives)

    Entity    m_paddle = NULL_ENTITY;
    Entity    m_ball   = NULL_ENTITY;
    glm::vec2 m_ballVel{0.0f, 0.0f};
    State     m_state      = State::Ready;
    int       m_score      = 0;
    int       m_lives      = 3;
    int       m_bricksLeft = 0;
    float     m_ballSpeed  = 430.0f;
};
