#include "skybox.h"
#include "texture.h"
#include <iostream>

#include <stb/stb_image.h>

namespace ml {

// === Р РЃР ВµР в„–Р Т‘Р ВµРЎР‚РЎвЂ№ Р Р†РЎРѓРЎвЂљРЎР‚Р С•Р ВµР Р…Р Р…РЎвЂ№Р Вµ ===
static const char* kVert = R"(
#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 uView;
uniform mat4 uProj;

out vec3 vTexCoord;

void main() {
    vTexCoord = aPos;
    gl_Position = uProj * uView * vec4(aPos, 1.0);
}
)";

static const char* kFrag = R"(
#version 330 core

in vec3 vTexCoord;

uniform samplerCube uCubemap;
uniform vec3        uTint;

out vec4 FragColor;

void main() {
    FragColor = vec4(texture(uCubemap, vTexCoord).rgb * uTint, 1.0);
}
)";

// === Р С™Р С•Р СР С—Р С‘Р В»РЎРЏРЎвЂ Р С‘РЎРЏ РЎв‚¬Р ВµР в„–Р Т‘Р ВµРЎР‚Р В° ===
static GLuint CompileShader(GLenum type, const char* src) {
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    GLint ok = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(id, 1024, nullptr, log);
        std::cerr << "Skybox shader error: " << log << "\n";
        glDeleteShader(id);
        return 0;
    }
    return id;
}

// === Р вЂ™Р ВµРЎР‚РЎв‚¬Р С‘Р Р…РЎвЂ№ Р С”РЎС“Р В±Р В° (36 Р Р†Р ВµРЎР‚РЎв‚¬Р С‘Р Р…) ===
static const float kCubeVerts[] = {
    // +X (right)
     1, -1,  1,   1, -1, -1,   1,  1, -1,
     1, -1,  1,   1,  1, -1,   1,  1,  1,
    // -X (left)
    -1, -1, -1,  -1, -1,  1,  -1,  1,  1,
    -1, -1, -1,  -1,  1,  1,  -1,  1, -1,
    // +Y (top)
    -1,  1, -1,   1,  1, -1,   1,  1,  1,
    -1,  1, -1,   1,  1,  1,  -1,  1,  1,
    // -Y (bottom)
    -1, -1,  1,   1, -1,  1,   1, -1, -1,
    -1, -1,  1,   1, -1, -1,  -1, -1, -1,
    // +Z (front)
    -1, -1,  1,   1, -1,  1,   1,  1,  1,
    -1, -1,  1,   1,  1,  1,  -1,  1,  1,
    // -Z (back)
     1, -1, -1,  -1, -1, -1,  -1,  1, -1,
     1, -1, -1,  -1,  1, -1,   1,  1, -1,
};

Skybox::~Skybox() {
    Shutdown();
}

bool Skybox::LoadFromFiles(
    const std::string& right,
    const std::string& left,
    const std::string& top,
    const std::string& bottom,
    const std::string& front,
    const std::string& back)
{
    // === 1. Р вЂ”Р В°Р С–РЎР‚РЎС“Р В¶Р В°Р ВµР С 6 PNG Р Р† cubemap ===
    glGenTextures(1, &cubemap_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_);

    // Р СџР С•РЎР‚РЎРЏР Т‘Р С•Р С” GL: +X, -X, +Y, -Y, +Z, -Z
    const char* faces[6] = {
        right.c_str(),
        left.c_str(),
        top.c_str(),
        bottom.c_str(),
        front.c_str(),
        back.c_str()
    };

    stbi_set_flip_vertically_on_load(0);

    for (int i = 0; i < 6; ++i) {
        int w, h, ch;
        unsigned char* data = stbi_load(faces[i], &w, &h, &ch, 0);
        if (!data) {
            std::cerr << "Skybox: cannot load " << faces[i] << "\n";
            return false;
        }

        GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, fmt,
                     w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);

        std::cout << "Skybox face loaded: " << faces[i]
                  << " (" << w << "x" << h << ")\n";
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // === 2. Р РЃР ВµР в„–Р Т‘Р ВµРЎР‚ ===
    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVert);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFrag);
    if (!vs || !fs) return false;

    shader_ = glCreateProgram();
    glAttachShader(shader_, vs);
    glAttachShader(shader_, fs);
    glLinkProgram(shader_);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // === 3. VAO/VBO ===
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVerts), kCubeVerts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    std::cout << "Skybox: loaded successfully\n";
    ready_ = true;
    return true;
}

void Skybox::Render(const glm::mat4& view, const glm::mat4& proj) {
    if (!ready_) return;

    // === Depth off, РЎвЂЎРЎвЂљР С•Р В±РЎвЂ№ РЎРѓР С”Р В°Р в„–Р В±Р С•Р С”РЎРѓ Р В±РЎвЂ№Р В» Р вЂ”Р С’ Р СР С‘РЎР‚Р С•Р С ===
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    glUseProgram(shader_);

    // Р Р€Р В±Р С‘РЎР‚Р В°Р ВµР С translation Р С‘Р В· view Р СР В°РЎвЂљРЎР‚Р С‘РЎвЂ РЎвЂ№ (РЎРѓР С”Р В°Р в„–Р В±Р С•Р С”РЎРѓ Р Р†Р С•Р С”РЎР‚РЎС“Р С– Р С”Р В°Р СР ВµРЎР‚РЎвЂ№)
    glm::mat4 skyView = glm::mat4(glm::mat3(view));

    GLint locView = glGetUniformLocation(shader_, "uView");
    GLint locProj = glGetUniformLocation(shader_, "uProj");
    GLint locCube = glGetUniformLocation(shader_, "uCubemap");

    glUniformMatrix4fv(locView, 1, GL_FALSE, &skyView[0][0]);
    glUniformMatrix4fv(locProj, 1, GL_FALSE, &proj[0][0]);
    glUniform1i(locCube, 0);

    GLint locTint = glGetUniformLocation(shader_, "uTint");
    if (locTint >= 0) glUniform3f(locTint, tint_.x, tint_.y, tint_.z);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // === Р вЂ™Р С•РЎРѓРЎРѓРЎвЂљР В°Р Р…Р В°Р Р†Р В»Р С‘Р Р†Р В°Р ВµР С ===
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glUseProgram(0);
}

void Skybox::Shutdown() {
    if (cubemap_) glDeleteTextures(1, &cubemap_);
    if (vbo_)     glDeleteBuffers(1, &vbo_);
    if (vao_)     glDeleteVertexArrays(1, &vao_);
    if (shader_)  glDeleteProgram(shader_);
    cubemap_ = vbo_ = vao_ = shader_ = 0;
    ready_ = false;
}

} // namespace ml
