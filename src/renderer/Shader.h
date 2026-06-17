#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    Shader() = default;
    ~Shader();
    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath);
    void bind() const { glUseProgram(m_program); }

    void setInt (const std::string& name, int v);
    void setVec4(const std::string& name, const glm::vec4& v);
    void setMat4(const std::string& name, const glm::mat4& m);

private:
    GLuint m_program = 0;
    GLint  location(const std::string& name);
    static GLuint compile(GLenum type, const std::string& src);
};
