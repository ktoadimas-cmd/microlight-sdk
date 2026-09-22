#include "ui_renderer.h"
#include <iostream>

namespace ml {

bool UIRenderer::Init() {
    shader_ = std::make_unique<Shader>();
    if (!shader_->LoadFromFiles("shaders/ui.vert", "shaders/ui.frag")) {
        std::cerr << "UI shader load failed\n";
        return false;
    }

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(UIVertex) * 4, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, color));

    glBindVertexArray(0);
    return true;
}

void UIRenderer::Shutdown() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    vbo_ = vao_ = 0;
}

void UIRenderer::Begin(int screenW, int screenH) {
    screenW_ = screenW;
    screenH_ = screenH;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void UIRenderer::End() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void UIRenderer::SubmitQuad(const glm::vec2& p0, const glm::vec2& p1,
                            const glm::vec2& uv0, const glm::vec2& uv1,
                            const glm::vec4& color, bool useTex, const Texture* tex)
{
    UIVertex verts[4] = {
        { {p0.x, p0.y}, {uv0.x, uv0.y}, color },
        { {p1.x, p0.y}, {uv1.x, uv0.y}, color },
        { {p1.x, p1.y}, {uv1.x, uv1.y}, color },
        { {p0.x, p1.y}, {uv0.x, uv1.y}, color },
    };

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    shader_->Use();
    shader_->SetInt("uUseTex", useTex ? 1 : 0);
    shader_->SetInt("uIsFont", 0);
    if (useTex && tex) {
        tex->Bind(0);
        shader_->SetInt("uTex", 0);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void UIRenderer::SubmitQuadRaw(GLuint texID, const glm::vec2& p0, const glm::vec2& p1,
                               const glm::vec2& uv0, const glm::vec2& uv1,
                               const glm::vec4& color)
{
    UIVertex verts[4] = {
        { {p0.x, p0.y}, {uv0.x, uv0.y}, color },
        { {p1.x, p0.y}, {uv1.x, uv0.y}, color },
        { {p1.x, p1.y}, {uv1.x, uv1.y}, color },
        { {p0.x, p1.y}, {uv0.x, uv1.y}, color },
    };

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    shader_->Use();
    shader_->SetInt("uUseTex", 1);
    shader_->SetInt("uIsFont", 1);   // в шейдере R-канал трактуется как alpha
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    shader_->SetInt("uTex", 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void UIRenderer::DrawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
    SubmitQuad(pos, pos + size, {0,0}, {1,1}, color, false, nullptr);
}

void UIRenderer::DrawTexturedRect(const glm::vec2& pos, const glm::vec2& size,
                                  const Texture& tex, const glm::vec4& tint) {
    SubmitQuad(pos, pos + size, {0,1}, {1,0}, tint, true, &tex);
}

void UIRenderer::DrawOutlinedRect(const glm::vec2& pos, const glm::vec2& size,
                                  const glm::vec4& fill, const glm::vec4& border,
                                  float borderSize) {
    DrawRect(pos, size, fill);
    glm::vec2 p0 = pos;
    glm::vec2 p1 = pos + size;
    DrawRect({ p0.x, p0.y },                { size.x, borderSize }, border);
    DrawRect({ p0.x, p1.y - borderSize },   { size.x, borderSize }, border);
    DrawRect({ p0.x, p0.y },                { borderSize, size.y }, border);
    DrawRect({ p1.x - borderSize, p0.y },   { borderSize, size.y }, border);
}

float UIRenderer::MeasureText(Font& font, const std::string& text, float scale) const {
    float w = 0.0f;
    for (char ch : text) {
        const Glyph* g = font.GetGlyph((int)(unsigned char)ch);
        if (!g) continue;
        w += g->advance * scale;
    }
    return w;
}

void UIRenderer::DrawText(Font& font, const std::string& text,
                          const glm::vec2& posPx, float scale,
                          const glm::vec4& color,
                          int screenW, int screenH,
                          float anchorX)
{
    screenW_ = screenW;
    screenH_ = screenH;

    float textW = MeasureText(font, text, scale);
    float startX = posPx.x - textW * anchorX;
    float penX = startX;
    float baselineY = posPx.y;

    GLuint atlas = font.AtlasTexture();

    for (char ch : text) {
        const Glyph* g = font.GetGlyph((int)(unsigned char)ch);
        if (!g) continue;

        // Позиция глифа в пикселях
        float x0 = penX + g->bearingX * scale;
        float y0 = baselineY - g->bearingY * scale;
        float x1 = x0 + g->width * scale;
        float y1 = y0 + g->height * scale;

        // Пиксели -> NDC
        float nx0 = (x0 / (float)screenW) * 2.0f - 1.0f;
        float nx1 = (x1 / (float)screenW) * 2.0f - 1.0f;
        float ny0 = 1.0f - (y1 / (float)screenH) * 2.0f;  // Y вниз
        float ny1 = 1.0f - (y0 / (float)screenH) * 2.0f;

        SubmitQuadRaw(atlas,
                      { nx0, ny0 }, { nx1, ny1 },
                      { g->u0, g->v1 }, { g->u1, g->v0 },
                      color);

        penX += g->advance * scale;
    }
}

} // namespace ml
