#include "shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace ml {

static std::string ReadFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open shader: " + path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

GLuint Shader::Compile(GLenum type, const std::string& src, const std::string& label) {
    GLuint id = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(id, 1, &c, nullptr);
    glCompileShader(id);

    GLint ok = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len > 0 ? len : 1);
        glGetShaderInfoLog(id, len, nullptr, log.data());
        std::cerr << "[" << label << "] compile error:\n" << log.data() << "\n";
        glDeleteShader(id);
        return 0;
    }
    return id;
}

Shader::~Shader() {
    if (program_) glDeleteProgram(program_);
}

bool Shader::LoadFromFiles(const std::string& vertPath, const std::string& fragPath) {
    std::string vsSrc = ReadFile(vertPath);
    std::string fsSrc = ReadFile(fragPath);

    GLuint vs = Compile(GL_VERTEX_SHADER,   vsSrc, vertPath);
    GLuint fs = Compile(GL_FRAGMENT_SHADER, fsSrc, fragPath);
    if (!vs || !fs) return false;

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);

    GLint ok = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len > 0 ? len : 1);
        glGetProgramInfoLog(program_, len, nullptr, log.data());
        std::cerr << "link error:\n" << log.data() << "\n";
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
}

void Shader::Use() const { glUseProgram(program_); }

void Shader::SetMat4 (const char* n, const glm::mat4& m) const { glUniformMatrix4fv(glGetUniformLocation(program_, n), 1, GL_FALSE, glm::value_ptr(m)); }
void Shader::SetVec4 (const char* n, const glm::vec4& v) const { glUniform4fv(glGetUniformLocation(program_, n), 1, glm::value_ptr(v)); }
void Shader::SetVec3 (const char* n, const glm::vec3& v) const { glUniform3fv(glGetUniformLocation(program_, n), 1, glm::value_ptr(v)); }
void Shader::SetFloat(const char* n, float v)              const { glUniform1f(glGetUniformLocation(program_, n), v); }
void Shader::SetInt  (const char* n, int v)                const { glUniform1i(glGetUniformLocation(program_, n), v); }

} // namespace ml
