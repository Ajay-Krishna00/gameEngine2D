#pragma once
#include <glad/glad.h>
#include <string>

class Texture {
public:
    Texture() = default;
    ~Texture();
    bool loadFromFile(const std::string& path);
    // Create a solid 1x1 white texture (handy fallback / untextured quads).
    void createWhite();
    void bind(unsigned slot = 0) const;

    int    width()  const { return m_w; }
    int    height() const { return m_h; }
    GLuint id()     const { return m_id; }

private:
    GLuint m_id = 0;
    int m_w = 0, m_h = 0;
};
