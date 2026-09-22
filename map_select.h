#pragma once
#include "ui_renderer.h"
#include "font.h"
#include "../core/map_registry.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace ml {

class MapSelectMenu {
public:
    void Init(const std::vector<MapEntry>& maps);
    void SetFont(Font* f) { font_ = f; }

    int HandleClick(float mouseX, float mouseY, int screenW, int screenH);
    void Render(UIRenderer& ui, int screenW, int screenH,
                float mouseX, float mouseY);
    void UpdateHover(float mouseX, float mouseY, int screenW, int screenH);

private:
    struct Button {
        std::string label;
        glm::vec2   pos  = {0, 0};
        glm::vec2   size = {0, 0};
        int         index = -1;
    };

    std::vector<Button>   buttons_;
    std::vector<MapEntry> maps_;
    Font* font_ = nullptr;
    int   hovered_ = -2;
};

} // namespace ml
