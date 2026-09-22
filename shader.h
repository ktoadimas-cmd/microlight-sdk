#pragma once
#include "gl.h"
#include <glm/glm.hpp>
#include <string>

namespace ml {

class Shader {
public:
    Shader() = default;
    ~Shader();

    bool LoadFromFiles(const std::string& vertPath, const std::string& fragPath);
    void Use() const;

    void SetMat4 (const char* name, const glm::mat4& m) const;
    void SetVec4 (const char* name, const glm::vec4& v) const;
    void SetVec3 (const char* name, const glm::vec3& v) const;
    void SetFloat(const char* name, float v) const;
    void SetInt  (const char* name, int v) const;

    GLuint Id() const { return program_; }

private:
    static GLuint Compile(GLenum type, const std::string& src, const std::string& label);
    GLuint program_ = 0;
};

} // namespace ml
