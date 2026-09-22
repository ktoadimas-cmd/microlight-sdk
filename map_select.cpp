#include "map_select.h"
#include <iostream>

namespace ml {

void MapSelectMenu::Init(const std::vector<MapEntry>& maps) {
    maps_ = maps;
    buttons_.clear();

    for (size_t i = 0; i < maps_.size(); ++i) {
        Button b;
        b.label = maps_[i].name;
        if (b.label.empty()) b.label = "Untitled";
        b.index = (int)i;
        buttons_.push_back(b);
    }

    Button back;
    back.label = "<< Back";
    back.index = -1;
    buttons_.push_back(back);

    int n = (int)buttons_.size();
    float w = 0.5f;
    float h = 0.075f;
    float gap = 0.02f;

    float totalH = n * h + (n - 1) * gap;
    float startY = -totalH * 0.5f + 0.05f;

    for (int i = 0; i < n; ++i) {
        buttons_[i].pos  = { -w * 0.5f, startY + (n - 1 - i) * (h + gap) };
        buttons_[i].size = { w, h };
    }

    std::cout << "MapSelectMenu: " << buttons_.size() << " buttons\n";
}

static glm::vec2 PixelToNDC(float px, float py, int W, int H) {
    return { (px / (float)W) * 2.0f - 1.0f,
             1.0f - (py / (float)H) * 2.0f };
}

void MapSelectMenu::UpdateHover(float mouseX, float mouseY, int screenW, int screenH) {
    glm::vec2 ndc = PixelToNDC(mouseX, mouseY, screenW, screenH);
    hovered_ = -2;

    for (int i = 0; i < (int)buttons_.size(); ++i) {
        auto& b = buttons_[i];
        if (ndc.x >= b.pos.x && ndc.x <= b.pos.x + b.size.x &&
            ndc.y >= b.pos.y && ndc.y <= b.pos.y + b.size.y) {
            hovered_ = i;
            return;
        }
    }
}

int MapSelectMenu::HandleClick(float mouseX, float mouseY, int screenW, int screenH) {
    UpdateHover(mouseX, mouseY, screenW, screenH);
    if (hovered_ < 0) return -2;
    return buttons_[hovered_].index;
}

void MapSelectMenu::Render(UIRenderer& ui, int screenW, int screenH,
                            float mouseX, float mouseY) {
    ui.DrawRect({ -1, -1 }, { 2, 2 }, { 0.06f, 0.07f, 0.10f, 1.0f });
    ui.DrawRect({ -1, 0.6f }, { 2, 0.4f }, { 0, 0, 0, 0.4f });

    if (font_) {
        ui.DrawText(*font_, "Select Map",
                    { screenW * 0.5f, screenH * 0.12f },
                    1.8f, { 1.0f, 0.97f, 0.85f, 1.0f },
                    screenW, screenH, 0.5f);

        if (maps_.empty()) {
            ui.DrawText(*font_, "No maps found in game/maps/",
                        { screenW * 0.5f, screenH * 0.5f },
                        1.0f, { 0.9f, 0.6f, 0.6f, 1.0f },
                        screenW, screenH, 0.5f);
        }
    }

    UpdateHover(mouseX, mouseY, screenW, screenH);

    for (int i = 0; i < (int)buttons_.size(); ++i) {
        auto& b = buttons_[i];
        bool isHovered = (i == hovered_);
        bool isBack = (b.index == -1);

        glm::vec4 fill, border;
        if (isBack) {
            fill   = isHovered ? glm::vec4(0.5f, 0.3f, 0.3f, 0.9f)
                               : glm::vec4(0.25f, 0.15f, 0.15f, 0.85f);
            border = { 0.8f, 0.5f, 0.5f, 1.0f };
        } else {
            fill   = isHovered ? glm::vec4(0.35f, 0.55f, 0.9f, 0.9f)
                               : glm::vec4(0.15f, 0.18f, 0.25f, 0.85f);
            border = isHovered ? glm::vec4(0.9f, 0.95f, 1.0f, 1.0f)
                               : glm::vec4(0.5f, 0.6f, 0.8f, 0.95f);
        }

        ui.DrawOutlinedRect(b.pos, b.size, fill, border, 0.004f);

        if (font_) {
            float px = (b.pos.x + b.size.x * 0.5f + 1.0f) * 0.5f * screenW;
            float py = (1.0f - (b.pos.y + b.size.y * 0.5f)) * 0.5f * screenH - 6.0f;
            ui.DrawText(*font_, b.label,
                        { px, py }, 0.85f,
                        { 1, 1, 1, 1 }, screenW, screenH, 0.5f);
        }
    }
}

} // namespace ml
