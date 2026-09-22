#pragma once
#include "ui_renderer.h"
#include "font.h"
#include "../core/gamemenu.h"
#include "../texture.h"
#include <glm/glm.hpp>
#include <memory>

namespace ml {

enum class MenuAction {
    None,
    StartGame,
    Load,
    Options,
    Quit,
};

class MenuRenderer {
public:
    bool Init(const GameMenu& menu, const std::string& bgPath);
    void Shutdown();

    void SetFont(Font* font) { font_ = font; }

    void Render(UIRenderer& ui, int screenW, int screenH, float mouseX, float mouseY);
    MenuAction HandleClick(float mouseX, float mouseY, int screenW, int screenH);

private:
    struct ButtonRect {
        glm::vec2 pos;
        glm::vec2 size;
    };
    ButtonRect ComputeButtonRect(int index, int total, int screenW, int screenH) const;
    int        HitTest(float mouseX, float mouseY, int screenW, int screenH) const;

    std::string              backgroundPath_;
    std::unique_ptr<Texture> background_;
    bool                     hasBackground_ = false;

    std::string title_;
    std::string subtitle_;

    float buttonW_       = 0.4f;
    float buttonH_       = 0.08f;
    float buttonSpacing_ = 0.11f;

    std::vector<MenuButton> buttons_;
    Font*                   font_ = nullptr;
};

} // namespace ml
