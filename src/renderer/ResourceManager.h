#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "renderer/Texture.h"
#include "renderer/Shader.h"

// Loads + caches GPU resources by key so duplicates are never re-uploaded.
// Returned references stay valid for the lifetime of the manager.
class ResourceManager {
public:
    // Load (once) and return the texture at `path`. Subsequent calls are cache hits.
    Texture& texture(const std::string& path);

    // Load (once) and return a shader keyed by `name`.
    Shader& shader(const std::string& name,
                   const std::string& vertPath, const std::string& fragPath);

    std::size_t textureCount() const { return m_textures.size(); }

private:
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::unique_ptr<Shader>>  m_shaders;
};
