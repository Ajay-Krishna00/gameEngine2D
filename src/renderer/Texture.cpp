#include "renderer/Texture.h"
#define STB_IMAGE_IMPLEMENTATION         // define in exactly ONE .cpp
#include <stb_image.h>
#include <cstdio>

static void setDefaultParams() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

bool Texture::loadFromFile(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);  // OpenGL UV origin is bottom-left
    int channels;
    unsigned char* data = stbi_load(path.c_str(), &m_w, &m_h, &channels, 4);
    if (!data) {
        std::printf("Texture load failed: %s — using white fallback\n", path.c_str());
        createWhite();
        return false;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    setDefaultParams();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_w, m_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return true;
}

void Texture::createWhite() {
    if (m_id) { glDeleteTextures(1, &m_id); m_id = 0; }
    const unsigned char px[4] = { 255, 255, 255, 255 };
    m_w = m_h = 1;
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    setDefaultParams();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
}

Texture::~Texture() { if (m_id) glDeleteTextures(1, &m_id); }

void Texture::bind(unsigned slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_id);
}
