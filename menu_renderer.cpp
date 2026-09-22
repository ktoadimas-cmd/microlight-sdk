#include "menu_renderer.h"
#include <iostream>
#include <cmath>

namespace ml {

bool MenuRenderer::Init(const GameMenu& menu, const std::string& bgPath) {
    title_    = menu.title;
    subtitle_ = menu.subtitle;

    buttonW_       = menu.buttonWidth;
    buttonH_       = menu.buttonHeight;
    buttonSpacing_ = menu.buttonSpacing;
    buttons_       = menu.buttons;

    if (!bgPath.empty()) {
        background_ = std::make_unique<Texture>();
        hasBackground_ = background_->LoadFromFile(bgPath, false);
        if (!hasBackground_) {
            std::cerr << "MenuRenderer: cannot load background " << bgPath << "\n";
        }
    }

    std::cout << "MenuRenderer ready: " << buttons_.size() << " buttons\n";
    return true;
}

void MenuRenderer::Shutdown() {
    background_.reset();
}

MenuRenderer::ButtonRect MenuRenderer::ComputeButtonRect(int index, int total, int, int) const {
    float x = -buttonW_ * 0.5f;
    float y = -0.3f + (total - 1 - index) * buttonSpacing_;
    return { {x, y}, {buttonW_, buttonH_} };
}

static glm::vec2 PixelToNDC(float px, float py, int screenW, int screenH) {
    float x = (px / (float)screenW) * 2.0f - 1.0f;
    float y = 1.0f - (py / (float)screenH) * 2.0f;
    return { x, y };
}

int MenuRenderer::HitTest(float mouseX, float mouseY, int screenW, int screenH) const {
    glm::vec2 ndc = PixelToNDC(mouseX, mouseY, screenW, screenH);
    int total = (int)buttons_.size();
    for (int i = 0; i < total; ++i) {
        if (buttons_[i].disabled) continue;
        auto r = ComputeButtonRect(i, total, screenW, screenH);
        if (ndc.x >= r.pos.x && ndc.x <= r.pos.x + r.size.x &&
            ndc.y >= r.pos.y && ndc.y <= r.pos.y + r.size.y) {
            return i;
        }
    }
    return -1;
}

MenuAction MenuRenderer::HandleClick(float mouseX, float mouseY, int screenW, int screenH) {
    int i = HitTest(mouseX, mouseY, screenW, screenH);
    if (i < 0) return MenuAction::None;
    const auto& a = buttons_[i].action;
    if (a == "startgame") return MenuAction::StartGame;
    if (a == "load")      return MenuAction::Load;
    if (a == "options")   return MenuAction::Options;
    if (a == "quit")      return MenuAction::Quit;
    return MenuAction::None;
}

void MenuRenderer::Render(UIRenderer& ui, int screenW, int screenH, float mouseX, float mouseY) {
    if (hasBackground_ && background_) {
        ui.DrawTexturedRect({ -1, -1 }, { 2, 2 }, *background_, {1,1,1,1});
    } else {
        ui.DrawRect({ -1, -1 }, { 2, 2 }, { 0.05f, 0.06f, 0.10f, 1.0f });
    }

    ui.DrawRect({ -1, 0.55f }, { 2, 0.45f }, { 0, 0, 0, 0.35f });

    if (font_) {
        float cx = screenW * 0.5f;
        ui.DrawText(*font_, title_,
                    { cx, screenH * 0.10f }, 1.8f,
                    { 1.0f, 0.97f, 0.85f, 1.0f },
                    screenW, screenH, 0.5f);
        ui.DrawText(*font_, subtitle_,
                    { cx, screenH * 0.17f }, 1.0f,
                    { 0.7f, 0.8f, 0.95f, 1.0f },
                    screenW, screenH, 0.5f);
    }

    int hovered = HitTest(mouseX, mouseY, screenW, screenH);
    int total = (int)buttons_.size();

    for (int i = 0; i < total; ++i) {
        auto r = ComputeButtonRect(i, total, screenW, screenH);
        bool isHovered = (i == hovered);
        bool disabled = buttons_[i].disabled;

        glm::vec4 fill, border;
        if (disabled) {
            fill   = { 0.10f, 0.10f, 0.12f, 0.75f };
            border = { 0.25f, 0.25f, 0.28f, 0.90f };
        } else if (isHovered) {
            fill   = { 0.35f, 0.50f, 0.85f, 0.85f };
            border = { 0.90f, 0.95f, 1.00f, 1.00f };
        } else {
            fill   = { 0.15f, 0.18f, 0.25f, 0.75f };
            border = { 0.50f, 0.60f, 0.80f, 0.95f };
        }

        ui.DrawOutlinedRect(r.pos, r.size, fill, border, 0.004f);

        if (font_) {
            float px = (r.pos.x + r.size.x * 0.5f + 1.0f) * 0.5f * screenW;
            float py = (1.0f - (r.pos.y + r.size.y * 0.5f)) * 0.5f * screenH - 6.0f;

            glm::vec4 textCol = disabled
                ? glm::vec4(0.45f, 0.45f, 0.50f, 1.0f)
                : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

            ui.DrawText(*font_, buttons_[i].label,
                        { px, py }, 0.75f,
                        textCol, screenW, screenH, 0.5f);
        }
    }
}

} // namespace ml
