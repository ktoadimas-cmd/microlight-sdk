#pragma once
#include <string>
#include <vector>
#include <memory>

namespace ml {

struct SearchPath {
    std::string type;   // "Game" / "Mod" / "Write"
    std::string path;   // абсолютный путь
};

struct GameInfo {
    std::string game;
    std::string title;
    std::string type;       // singleplayer / multiplayer
    std::string version;
    std::string startMap;
    int         steamAppId = 0;

    std::string              gameInfoPath;   // путь к gameinfo.txt
    std::string              gameDir;        // папка с gameinfo.txt
    std::vector<SearchPath>  searchPaths;

    // Загрузить gameinfo.txt. Если path пустой — ищет в текущей папке и ./game.
    static std::unique_ptr<GameInfo> Load(const std::string& path = "");

    void Dump() const;
};

} // namespace ml
