#pragma once
#include "core/Application.h"
#include "renderer/SpriteBatch.h"
#include "renderer/Camera2D.h"
#include "renderer/Texture.h"
#include "renderer/ResourceManager.h"
#include "ecs/Registry.h"
#include "physics/PhysicsSystem.h"

class Game : public Application {
public:
    Game();
protected:
    void onUpdate(float dt) override;
    void onRender() override;
private:
    void updateTitle();

    Registry        m_reg;
    SpriteBatch     m_batch;      // constructed AFTER Application's Window => GL is ready
    Camera2D        m_camera{1280, 720};
    ResourceManager m_resources;
    Texture*        m_tex = nullptr;
    PhysicsSystem   m_physics;
    Entity          m_player = NULL_ENTITY;
    int             m_titleTimer = 0;
};
