#include "renderer/SpriteBatch.h"
#include "renderer/Texture.h"
#include <cmath>
#include <cstddef>

SpriteBatch::SpriteBatch() {
    m_verts.reserve(MAX_VERTS);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTS * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    // index buffer is static: 6 indices per quad, never changes
    std::vector<GLuint> indices(MAX_INDICES);
    GLuint off = 0;
    for (int i = 0; i < MAX_INDICES; i += 6) {
        indices[i+0] = off + 0; indices[i+1] = off + 1; indices[i+2] = off + 2;
        indices[i+3] = off + 2; indices[i+4] = off + 3; indices[i+5] = off + 0;
        off += 4;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(GLuint),
                 indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glBindVertexArray(0);

    m_shader.loadFromFiles("assets/shaders/sprite.vert", "assets/shaders/sprite.frag");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

SpriteBatch::~SpriteBatch() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void SpriteBatch::begin(const glm::mat4& vp) {
    m_viewProj   = vp;
    m_curTexture = 0;
    m_drawCalls  = 0;
    m_quadsDrawn = 0;
    m_verts.clear();
}

void SpriteBatch::draw(const Texture& tex, glm::vec2 pos, glm::vec2 size,
                       float rot, glm::vec4 color) {
    if ((m_curTexture != 0 && m_curTexture != tex.id()) ||
        m_verts.size() >= static_cast<size_t>(MAX_VERTS)) {
        flush();
    }
    m_curTexture = tex.id();

    glm::vec2 h = size * 0.5f;
    glm::vec2 corners[4] = { {-h.x,-h.y}, { h.x,-h.y}, { h.x, h.y}, {-h.x, h.y} };
    glm::vec2 uv[4]      = { { 0, 0 },    { 1, 0 },    { 1, 1 },    { 0, 1 } };

    float c = std::cos(rot), s = std::sin(rot);
    for (int i = 0; i < 4; ++i) {
        glm::vec2 r = { corners[i].x * c - corners[i].y * s,
                        corners[i].x * s + corners[i].y * c };
        m_verts.push_back({ pos + r, uv[i], color });
    }
}

void SpriteBatch::flush() {
    if (m_verts.empty()) return;
    int quadCount = static_cast<int>(m_verts.size()) / 4;

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_verts.size() * sizeof(Vertex), m_verts.data());

    m_shader.bind();
    m_shader.setMat4("uMVP", m_viewProj);
    m_shader.setInt("uTex", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_curTexture);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, quadCount * 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    ++m_drawCalls;
    m_quadsDrawn += quadCount;
    m_verts.clear();
}

void SpriteBatch::end() { flush(); }
