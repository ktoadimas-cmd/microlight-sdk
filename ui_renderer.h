#pragma once
#include "gl.h"
#include "font.h"
#include "../shader.h"
#include "../texture.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace ml {

struct UIVertex {
    glm::vec2 pos;
    glm::vec2 uv;
    glm::vec4 color;
};

class UIRenderer {
public:
    bool Init();
    void Shutdown();

    void Begin(int screenW, int screenH);
    void End();

    void DrawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
    void DrawTexturedRect(const glm::vec2& pos, const glm::vec2& size,
                          const Texture& tex, const glm::vec4& tint = {1,1,1,1});
    void DrawOutlinedRect(const glm::vec2& pos, const glm::vec2& size,
                          const glm::vec4& fill, const glm::vec4& border,
                          float borderSize = 0.004f);

    // Рисует текст шрифтом. posPx — позиция в пикселях (левый-верх глифа),
    // scale — множитель размера, anchor — 0..1 (0.5 = центр по X)
    void DrawText(Font& font, const std::string& text,
                  const glm::vec2& posPx, float scale,
                  const glm::vec4& color,
                  int screenW, int screenH,
                  float anchorX = 0.0f);

    // Возвращает ширину текста в пикселях
    float MeasureText(Font& font, const std::string& text, float scale) const;

private:
    void SubmitQuad(const glm::vec2& p0, const glm::vec2& p1,
                    const glm::vec2& uv0, const glm::vec2& uv1,
                    const glm::vec4& color, bool useTex, const Texture* tex);
    void SubmitQuadRaw(GLuint texID, const glm::vec2& p0, const glm::vec2& p1,
                       const glm::vec2& uv0, const glm::vec2& uv1,
                       const glm::vec4& color);

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    std::unique_ptr<Shader> shader_;
    int screenW_ = 1280;
    int screenH_ = 720;
};

} // namespace ml
