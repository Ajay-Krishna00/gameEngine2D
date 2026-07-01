#include "game/BreakoutScene.h"
#include "ecs/Components.h"
#include "ecs/Systems.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <vector>

namespace {
// Play field (world units), centered on the origin, y-up. Bottom edge is open.
constexpr float HW = 440.0f;                 // field half-width
constexpr float HH = 320.0f;                 // field half-height
constexpr float WALL = 16.0f;                // wall thickness (half)

constexpr glm::vec2 PADDLE_HALF{60.0f, 9.0f};
constexpr float     PADDLE_Y     = -HH + 30.0f;
constexpr float     PADDLE_SPEED = 520.0f;
constexpr glm::vec2 BALL_HALF{7.0f, 7.0f};

constexpr int   BRICK_COLS = 12, BRICK_ROWS = 6;
constexpr glm::vec2 BRICK_HALF{32.0f, 12.0f};
constexpr float BRICK_GAP = 6.0f;

// Game-side components: the engine's ECS is generic, so a game can define
// its own component types and the Registry just makes pools for them.
struct Brick  { int points = 10; };

// AABB overlap: returns penetration depths (positive on both axes = hit).
inline glm::vec2 overlapOf(glm::vec2 posA, glm::vec2 halfA,
                           glm::vec2 posB, glm::vec2 halfB) {
    return (halfA + halfB) - glm::abs(posB - posA);
}

// Row colors, top to bottom.
constexpr glm::vec4 ROW_COLORS[BRICK_ROWS] = {
    {0.92f, 0.30f, 0.32f, 1.0f},   // red
    {0.95f, 0.55f, 0.25f, 1.0f},   // orange
    {0.95f, 0.80f, 0.30f, 1.0f},   // yellow
    {0.45f, 0.80f, 0.40f, 1.0f},   // green
    {0.35f, 0.65f, 0.90f, 1.0f},   // blue
    {0.65f, 0.45f, 0.90f, 1.0f},   // purple
};
} // namespace

BreakoutScene::BreakoutScene() {
    auto spawnWall = [&](glm::vec2 pos, glm::vec2 half) {
        Entity w = m_reg.create();
        m_reg.add(w, Transform{ pos, 0.0f, {1, 1} });
        m_reg.add(w, SpriteComp{ half * 2.0f, {0.30f, 0.32f, 0.38f, 1.0f} });
    };
    spawnWall({ 0,  HH + WALL },       { HW + 2 * WALL, WALL });   // top
    spawnWall({ -HW - WALL, -WALL },   { WALL, HH + 2 * WALL });   // left
    spawnWall({  HW + WALL, -WALL },   { WALL, HH + 2 * WALL });   // right

    m_paddle = m_reg.create();
    m_reg.add(m_paddle, Transform{ {0.0f, PADDLE_Y}, 0.0f, {1, 1} });
    m_reg.add(m_paddle, SpriteComp{ PADDLE_HALF * 2.0f, {0.90f, 0.90f, 0.95f, 1.0f} });

    m_ball = m_reg.create();
    m_reg.add(m_ball, Transform{ {0.0f, PADDLE_Y}, 0.0f, {1, 1} });
    m_reg.add(m_ball, SpriteComp{ BALL_HALF * 2.0f, {1.0f, 0.85f, 0.2f, 1.0f} });

    spawnBricks();
    resetBall();
}

void BreakoutScene::spawnBricks() {
    const float pitchX = BRICK_HALF.x * 2.0f + BRICK_GAP;
    const float pitchY = BRICK_HALF.y * 2.0f + BRICK_GAP;
    const float x0 = -(BRICK_COLS - 1) * pitchX * 0.5f;
    const float yTop = HH - 50.0f;
    for (int r = 0; r < BRICK_ROWS; ++r) {
        for (int c = 0; c < BRICK_COLS; ++c) {
            Entity b = m_reg.create();
            m_reg.add(b, Transform{ { x0 + c * pitchX, yTop - r * pitchY }, 0.0f, {1, 1} });
            m_reg.add(b, SpriteComp{ BRICK_HALF * 2.0f, ROW_COLORS[r] });
            m_reg.add(b, Brick{ (BRICK_ROWS - r) * 10 });   // top rows score more
        }
    }
    m_bricksLeft = BRICK_COLS * BRICK_ROWS;
}

void BreakoutScene::resetBall() {
    Transform* ball   = m_reg.get<Transform>(m_ball);
    Transform* paddle = m_reg.get<Transform>(m_paddle);
    ball->position = paddle->position + glm::vec2{0.0f, PADDLE_HALF.y + BALL_HALF.y + 1.0f};
    m_ballVel = {0.0f, 0.0f};
    m_state = State::Ready;
}

void BreakoutScene::restart() {
    std::vector<Entity> doomed;
    m_reg.view<Brick>([&](Entity e, Brick&) { doomed.push_back(e); });
    for (Entity e : doomed) m_reg.destroy(e);
    spawnBricks();
    m_score = 0;
    m_lives = 3;
    resetBall();
}

void BreakoutScene::update(float dt, const Input& input) {
    Transform& paddle = *m_reg.get<Transform>(m_paddle);
    Transform& ball   = *m_reg.get<Transform>(m_ball);

    // Paddle: A/D or arrows, clamped inside the field.
    float dir = 0.0f;
    if (input.isDown(SDL_SCANCODE_A) || input.isDown(SDL_SCANCODE_LEFT))  dir -= 1.0f;
    if (input.isDown(SDL_SCANCODE_D) || input.isDown(SDL_SCANCODE_RIGHT)) dir += 1.0f;
    paddle.position.x = glm::clamp(paddle.position.x + dir * PADDLE_SPEED * dt,
                                   -HW + PADDLE_HALF.x, HW - PADDLE_HALF.x);

    if (m_state == State::Won || m_state == State::GameOver) {
        if (input.isPressed(SDL_SCANCODE_SPACE)) restart();
        return;
    }

    if (m_state == State::Ready) {
        // Ball rides the paddle until launch.
        ball.position = paddle.position + glm::vec2{0.0f, PADDLE_HALF.y + BALL_HALF.y + 1.0f};
        if (input.isPressed(SDL_SCANCODE_SPACE)) {
            m_ballVel = glm::normalize(glm::vec2{dir * 0.4f + 0.2f, 1.0f}) * m_ballSpeed;
            m_state = State::Playing;
        }
        return;
    }

    // --- State::Playing ---
    ball.position += m_ballVel * dt;

    // Side + top walls: reflect and clamp back inside.
    if (ball.position.x < -HW + BALL_HALF.x) { ball.position.x = -HW + BALL_HALF.x; m_ballVel.x =  glm::abs(m_ballVel.x); }
    if (ball.position.x >  HW - BALL_HALF.x) { ball.position.x =  HW - BALL_HALF.x; m_ballVel.x = -glm::abs(m_ballVel.x); }
    if (ball.position.y >  HH - BALL_HALF.y) { ball.position.y =  HH - BALL_HALF.y; m_ballVel.y = -glm::abs(m_ballVel.y); }

    // Paddle: only when the ball is falling. Where it lands on the paddle
    // steers the bounce (classic Breakout "english").
    if (m_ballVel.y < 0.0f) {
        glm::vec2 pen = overlapOf(ball.position, BALL_HALF, paddle.position, PADDLE_HALF);
        if (pen.x > 0.0f && pen.y > 0.0f) {
            ball.position.y = paddle.position.y + PADDLE_HALF.y + BALL_HALF.y;
            float hit = glm::clamp((ball.position.x - paddle.position.x) / PADDLE_HALF.x,
                                   -1.0f, 1.0f);
            m_ballVel = glm::normalize(glm::vec2{hit * 0.85f, 1.0f}) * m_ballSpeed;
        }
    }

    // Bricks: reflect off the first overlapping brick along the axis of least
    // penetration, then destroy it. Collect inside view(), destroy after —
    // destroying mid-iteration would mutate the pool being walked.
    Entity    hitBrick = NULL_ENTITY;
    int       hitPoints = 0;
    glm::vec2 hitPen{0.0f, 0.0f};
    glm::vec2 hitPos{0.0f, 0.0f};
    m_reg.view<Transform, Brick>([&](Entity e, Transform& t, Brick& b) {
        if (hitBrick != NULL_ENTITY) return;
        glm::vec2 pen = overlapOf(ball.position, BALL_HALF, t.position, BRICK_HALF);
        if (pen.x > 0.0f && pen.y > 0.0f) {
            hitBrick = e; hitPoints = b.points; hitPen = pen; hitPos = t.position;
        }
    });
    if (hitBrick != NULL_ENTITY) {
        if (hitPen.x < hitPen.y) {
            m_ballVel.x = (ball.position.x < hitPos.x) ? -glm::abs(m_ballVel.x)
                                                       :  glm::abs(m_ballVel.x);
        } else {
            m_ballVel.y = (ball.position.y < hitPos.y) ? -glm::abs(m_ballVel.y)
                                                       :  glm::abs(m_ballVel.y);
        }
        m_reg.destroy(hitBrick);
        m_score += hitPoints;
        if (--m_bricksLeft == 0) m_state = State::Won;
    }

    // Fell past the paddle: lose a life.
    if (ball.position.y < -HH - 60.0f) {
        if (--m_lives <= 0) m_state = State::GameOver;
        else                resetBall();
    }
}

void BreakoutScene::render(SpriteBatch& batch, const Texture& tex) {
    renderSystem(m_reg, batch, tex);
}

void BreakoutScene::onGui() {
    ImGui::Text("score: %d   lives: %d   bricks left: %d", m_score, m_lives, m_bricksLeft);
    switch (m_state) {
        case State::Ready:    ImGui::TextColored({1.0f, 0.85f, 0.2f, 1.0f}, "SPACE to launch"); break;
        case State::Playing:  break;
        case State::Won:      ImGui::TextColored({0.4f, 1.0f, 0.4f, 1.0f}, "YOU WIN! SPACE to play again"); break;
        case State::GameOver: ImGui::TextColored({1.0f, 0.4f, 0.4f, 1.0f}, "GAME OVER - SPACE to retry"); break;
    }
    if (ImGui::SliderFloat("ball speed", &m_ballSpeed, 200.0f, 800.0f, "%.0f")
        && m_state == State::Playing) {
        m_ballVel = glm::normalize(m_ballVel) * m_ballSpeed;
    }
    if (ImGui::Button("Restart game")) restart();
}
