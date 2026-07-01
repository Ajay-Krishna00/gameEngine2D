#include "Game.h"
#include "game/ArenaScene.h"
#include "game/BreakoutScene.h"
#include <imgui.h>
#include <cstdio>

namespace {
constexpr const char* SCENE_NAMES[] = { "Arena sandbox", "Breakout" };
constexpr int SCENE_COUNT = 2;
}

Game::Game() {
    m_tex = &m_resources.texture("assets/textures/sprite.png");
    setScene(0);
}

void Game::setScene(int idx) {
    m_sceneIdx = idx;
    m_scene = (idx == 0) ? std::unique_ptr<Scene>(std::make_unique<ArenaScene>())
                         : std::unique_ptr<Scene>(std::make_unique<BreakoutScene>());
    m_camera.position = {0.0f, 0.0f};
    m_camera.zoom = 1.0f;
}

void Game::onUpdate(float dt) {
    // While ImGui owns the keyboard (typing in the panel), feed the scene a
    // blank input snapshot so the player/paddle doesn't react.
    static const Input s_noInput;
    const bool captured = debugUI().wantsKeyboard();
    const Input& in = captured ? s_noInput : input();

    if (!captured) {
        if (input().isPressed(SDL_SCANCODE_1)) setScene(0);
        if (input().isPressed(SDL_SCANCODE_2)) setScene(1);
        if (input().isPressed(SDL_SCANCODE_F1)) m_showPanel = !m_showPanel;
    }

    m_scene->update(dt, in);
    m_camera.position = m_scene->cameraFocus();
}

void Game::onRender() {
    m_camera.setViewport((float)window().width(), (float)window().height());
    m_batch.begin(m_camera.viewProjection());
    m_scene->render(m_batch, *m_tex);
    m_batch.end();

    updateTitle();
}

void Game::onGui() {
    if (!m_showPanel) return;

    ImGui::SetNextWindowPos({10, 10}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({330, 0}, ImGuiCond_FirstUseEver);
    ImGui::Begin("engine2d debug (F1 to hide)");

    ImGui::Text("%.0f FPS (%.2f ms)", fps(), fps() > 0.0f ? 1000.0f / fps() : 0.0f);
    ImGui::Text("entities: %zu   draw calls: %d   quads: %d",
                m_scene->registry().aliveCount(), m_batch.drawCalls(), m_batch.quadsDrawn());

    int idx = m_sceneIdx;
    if (ImGui::Combo("scene (1/2)", &idx, SCENE_NAMES, SCENE_COUNT) && idx != m_sceneIdx)
        setScene(idx);
    ImGui::SliderFloat("camera zoom", &m_camera.zoom, 0.25f, 3.0f, "%.2f");

    ImGui::Separator();
    m_scene->onGui();

    ImGui::End();
}

void Game::updateTitle() {
    // Throttle: refresh the title ~4x/second, not every frame.
    if (++m_titleTimer < 15) return;
    m_titleTimer = 0;

    char buf[256];
    std::snprintf(buf, sizeof(buf),
        "engine2d - %s  |  %.0f FPS  |  entities %zu  |  draw calls %d  |  quads %d",
        m_scene->name(), fps(), m_scene->registry().aliveCount(),
        m_batch.drawCalls(), m_batch.quadsDrawn());
    window().setTitle(buf);
}
