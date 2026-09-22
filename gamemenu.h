#pragma once
#include <string>
#include <vector>
#include <memory>

namespace ml {

struct MenuButton {
    std::string label;
    std::string action;
    bool        disabled = false;
};

struct GameMenu {
    std::string background;
    std::string title;
    std::string subtitle;

    float buttonWidth   = 0.4f;
    float buttonHeight  = 0.08f;
    float buttonSpacing = 0.11f;

    std::vector<MenuButton> buttons;

    static std::unique_ptr<GameMenu> Load(const std::string& path);
    void Dump() const;
};

} // namespace ml
