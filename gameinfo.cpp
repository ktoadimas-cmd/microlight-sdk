#include "gameinfo.h"
#include "kv.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace ml {

static std::string ResolvePathToken(const std::string& in, const std::string& gameDir) {
    std::string out = in;
    const std::string token = "|gameinfo_path|";
    size_t p;
    while ((p = out.find(token)) != std::string::npos) {
        out.replace(p, token.size(), gameDir + "/");
    }

    std::string cleaned;
    for (size_t i = 0; i < out.size(); ++i) {
        if (out[i] == '/' && !cleaned.empty() && cleaned.back() == '/') continue;
        cleaned += out[i];
    }
    out = cleaned;

    std::replace(out.begin(), out.end(), '\\', '/');

    if (out.size() >= 2 && out.substr(out.size() - 2) == "/.") {
        out = out.substr(0, out.size() - 2);
    }
    if (!out.empty() && out.back() == '/') {
        out.pop_back();
    }
    return out;
}

std::unique_ptr<GameInfo> GameInfo::Load(const std::string& path) {
    std::string giPath = path;
    if (giPath.empty()) {
        const char* candidates[] = {
            "gameinfo.txt",
            "game/gameinfo.txt",
            "./gameinfo.txt",
            "../gameinfo.txt"
        };
        for (auto c : candidates) {
            if (fs::exists(c)) { giPath = c; break; }
        }
    }
    if (giPath.empty() || !fs::exists(giPath)) {
        std::cerr << "GameInfo: gameinfo.txt not found\n";
        return nullptr;
    }
    std::cout << "GameInfo: loading " << fs::absolute(giPath).string() << "\n";

    auto root = KV::ParseFile(giPath);
    if (!root) return nullptr;

    // Корень может быть "GameInfo" — используем его напрямую
    const KV* gi = (root->Key() == "GameInfo") ? root.get() : root->Find("GameInfo");
    if (!gi) {
        std::cerr << "GameInfo: no 'GameInfo' node\n";
        return nullptr;
    }

    auto info = std::make_unique<GameInfo>();
    info->gameInfoPath = fs::absolute(giPath).string();
    info->gameDir      = fs::absolute(fs::path(giPath).parent_path()).string();
    std::replace(info->gameDir.begin(), info->gameDir.end(), '\\', '/');

    info->game       = gi->GetString("game");
    info->title      = gi->GetString("title");
    info->type       = gi->GetString("type", "singleplayer");
    info->version    = gi->GetString("version", "1.0");
    info->startMap   = gi->GetString("StartMap");

    const KV* fsNode = gi->Find("FileSystem");
    if (fsNode) {
        info->steamAppId = fsNode->GetInt("SteamAppId", 0);
        const KV* sp = fsNode->Find("SearchPaths");
        if (sp) {
            for (auto& c : sp->Children()) {
                SearchPath p;
                p.type = c->Key();
                p.path = ResolvePathToken(c->Value(), info->gameDir);
                info->searchPaths.push_back(p);
            }
        }
    }

    return info;
}

void GameInfo::Dump() const {
    std::cout << "=== GameInfo ===\n";
    std::cout << "  game:       " << game      << "\n";
    std::cout << "  title:      " << title     << "\n";
    std::cout << "  type:       " << type      << "\n";
    std::cout << "  version:    " << version   << "\n";
    std::cout << "  startMap:   " << startMap  << "\n";
    std::cout << "  steamAppId: " << steamAppId << "\n";
    std::cout << "  gameDir:    " << gameDir   << "\n";
    std::cout << "  SearchPaths: " << searchPaths.size() << "\n";
    for (auto& p : searchPaths) {
        std::cout << "    [" << p.type << "] " << p.path << "\n";
    }
    std::cout << "================\n";
}

} // namespace ml
