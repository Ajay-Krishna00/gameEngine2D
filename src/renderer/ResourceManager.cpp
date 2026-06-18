#include "renderer/ResourceManager.h"

Texture& ResourceManager::texture(const std::string& path) {
    auto it = m_textures.find(path);
    if (it != m_textures.end()) return *it->second;

    auto tex = std::make_unique<Texture>();
    tex->loadFromFile(path);          // falls back to a white texture on failure
    Texture& ref = *tex;
    m_textures.emplace(path, std::move(tex));
    return ref;
}

Shader& ResourceManager::shader(const std::string& name,
                                const std::string& vertPath, const std::string& fragPath) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) return *it->second;

    auto sh = std::make_unique<Shader>();
    sh->loadFromFiles(vertPath, fragPath);
    Shader& ref = *sh;
    m_shaders.emplace(name, std::move(sh));
    return ref;
}
