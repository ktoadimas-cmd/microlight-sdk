#include "gamemenu.h"
#include "kv.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace ml {

std::unique_ptr<GameMenu> GameMenu::Load(const std::string& path) {
    if (!fs::exists(path)) {
        std::cerr << "GameMenu: file not found: " << path << "\n";
        return nullptr;
    }

    auto root = KV::ParseFile(path);
    if (!root) return nullptr;

    // Сначала: сам корень может быть "GameMenu"
    const KV* menu = nullptr;
    if (root->Key() == "GameMenu") {
        menu = root.get();
    } else {
        menu = root->Find("GameMenu");
    }
    if (!menu) {
        std::cerr << "GameMenu: no 'GameMenu' node in " << path << "\n";
        return nullptr;
    }

    auto m = std::make_unique<GameMenu>();
    m->background    = menu->GetString("Background");
    m->title         = menu->GetString("Title");
    m->subtitle      = menu->GetString("Subtitle");
    m->buttonWidth   = menu->GetFloat("ButtonWidth",   0.4f);
    m->buttonHeight  = menu->GetFloat("ButtonHeight",  0.08f);
    m->buttonSpacing = menu->GetFloat("ButtonSpacing", 0.11f);

    const KV* buttonsNode = menu->Find("Buttons");
    if (buttonsNode) {
        for (auto& c : buttonsNode->Children()) {
            if (c->Key() != "Button") continue;
            MenuButton b;
            b.label    = c->GetString("Label");
            b.action   = c->GetString("Action");
            b.disabled = c->GetBool("Disabled", false);
            m->buttons.push_back(std::move(b));
        }
    }

    return m;
}

void GameMenu::Dump() const {
    std::cout << "=== GameMenu ===\n";
    std::cout << "  background:    " << background    << "\n";
    std::cout << "  title:         " << title         << "\n";
    std::cout << "  subtitle:      " << subtitle      << "\n";
    std::cout << "  buttonWidth:   " << buttonWidth   << "\n";
    std::cout << "  buttonHeight:  " << buttonHeight  << "\n";
    std::cout << "  buttonSpacing: " << buttonSpacing << "\n";
    std::cout << "  buttons (" << buttons.size() << "):\n";
    for (auto& b : buttons) {
        std::cout << "    [" << (b.disabled ? "X" : " ") << "] "
                  << b.label << "  -> " << b.action << "\n";
    }
    std::cout << "================\n";
}

} // namespace ml
