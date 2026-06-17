#include "renderer/Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <cstdio>

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}

GLuint Shader::compile(GLenum type, const std::string& src) {
    GLuint s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[1024]; glGetShaderInfoLog(s, 1024, nullptr, log);
               std::printf("Shader compile error:\n%s\n", log); }
    return s;
}

bool Shader::loadFromFiles(const std::string& vertPath, const std::string& fragPath) {
    GLuint vs = compile(GL_VERTEX_SHADER,   readFile(vertPath));
    GLuint fs = compile(GL_FRAGMENT_SHADER, readFile(fragPath));
    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);
    GLint ok; glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (!ok) { char log[1024]; glGetProgramInfoLog(m_program, 1024, nullptr, log);
               std::printf("Shader link error:\n%s\n", log); }
    glDeleteShader(vs); glDeleteShader(fs);
    return ok != 0;
}

Shader::~Shader() { if (m_program) glDeleteProgram(m_program); }

GLint Shader::location(const std::string& name) {
    return glGetUniformLocation(m_program, name.c_str());
}
void Shader::setInt (const std::string& n, int v)              { glUniform1i(location(n), v); }
void Shader::setVec4(const std::string& n, const glm::vec4& v) { glUniform4fv(location(n), 1, glm::value_ptr(v)); }
void Shader::setMat4(const std::string& n, const glm::mat4& m) { glUniformMatrix4fv(location(n), 1, GL_FALSE, glm::value_ptr(m)); }
