#pragma once
#include "gl.h"
#include <glm/glm.hpp>
#include <string>

namespace ml {

class Skybox {
public:
    Skybox() = default;
    ~Skybox();

    // Загрузка 6 PNG (порядок: right, left, top, bottom, front, back)
    bool LoadFromFiles(
        const std::string& right,
        const std::string& left,
        const std::string& top,
        const std::string& bottom,
        const std::string& front,
        const std::string& back
    );

    // Рисуем скайбокс. viewProj = proj * view (без translation)
    void Render(const glm::mat4& view, const glm::mat4& proj);

    // Красный tint (1,1,1 = обычный)
    void SetTint(const glm::vec3& tint) { tint_ = tint; }

    void Shutdown();

    bool IsValid() const { return cubemap_ != 0; }

private:
    GLuint cubemap_ = 0;
    GLuint vao_     = 0;
    GLuint vbo_     = 0;
    GLuint shader_  = 0;
    glm::vec3 tint_ = glm::vec3(1.0f);
    bool   ready_   = false;
};

} // namespace ml
