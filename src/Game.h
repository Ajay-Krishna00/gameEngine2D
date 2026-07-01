#pragma once
#include "core/Application.h"
#include "renderer/SpriteBatch.h"
#include "renderer/Camera2D.h"
#include "renderer/Texture.h"
#include "renderer/ResourceManager.h"
#include "game/Scene.h"
#include <memory>

// The demo app: hosts the scenes (arena sandbox, Breakout) and the ImGui
// debug panel, and forwards the engine loop into the active scene.
class Game : public Application {
public:
    Game();
protected:
    void onUpdate(float dt) override;
    void onRender() override;
    void onGui() override;
private:
    void setScene(int idx);
    void updateTitle();

    SpriteBatch     m_batch;      // constructed AFTER Application's Window => GL is ready
    Camera2D        m_camera{1280, 720};
    ResourceManager m_resources;
    Texture*        m_tex = nullptr;

    std::unique_ptr<Scene> m_scene;
    int  m_sceneIdx   = 0;
    bool m_showPanel  = true;     // F1 toggles
    int  m_titleTimer = 0;
};
