#pragma once
#include "core/Application.h"
#include "renderer/SpriteBatch.h"
#include "renderer/Camera2D.h"
#include "renderer/Texture.h"
#include "ecs/Registry.h"
#include "physics/PhysicsSystem.h"

class Game : public Application {
public:
    Game();
protected:
    void onUpdate(float dt) override;
    void onRender() override;
private:
    Registry      m_reg;
    SpriteBatch   m_batch;        // constructed AFTER Application's Window => GL is ready
    Camera2D      m_camera{1280, 720};
    Texture       m_tex;
    PhysicsSystem m_physics;
    Entity        m_player = NULL_ENTITY;
};
