#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "renderer/Shader.h"

class Texture;

// Accumulates quads and submits them in as few draw calls as possible.
// This version batches runs of the SAME texture into one draw call and
// flushes when the texture changes or the buffer fills.
class SpriteBatch {
public:
    SpriteBatch();
    ~SpriteBatch();

    void begin(const glm::mat4& viewProj);
    void draw(const Texture& tex, glm::vec2 pos, glm::vec2 size,
              float rot = 0.0f, glm::vec4 color = glm::vec4(1.0f));
    void end();

    int drawCalls()   const { return m_drawCalls; }
    int quadsDrawn()  const { return m_quadsDrawn; }

private:
    struct Vertex { glm::vec2 pos; glm::vec2 uv; glm::vec4 color; };
    static constexpr int MAX_QUADS   = 10000;
    static constexpr int MAX_VERTS   = MAX_QUADS * 4;
    static constexpr int MAX_INDICES = MAX_QUADS * 6;

    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
    Shader m_shader;
    std::vector<Vertex> m_verts;
    GLuint    m_curTexture = 0;
    glm::mat4 m_viewProj{1.0f};
    int       m_drawCalls = 0;
    int       m_quadsDrawn = 0;

    void flush();
};
