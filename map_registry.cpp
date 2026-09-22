#include "map_registry.h"
#include "map.h"
#include <filesystem>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

namespace ml {

std::vector<std::string> MapRegistry::Scan(const std::string& dir) {
    std::vector<std::string> out;
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        std::cerr << "MapRegistry: not a directory: " << dir << "\n";
        return out;
    }
    for (auto& e : fs::directory_iterator(dir)) {
        if (!e.is_regular_file()) continue;
        if (e.path().extension() == ".mgs") {
            out.push_back(e.path().generic_string());
        }
    }
    std::sort(out.begin(), out.end());
    return out;
}

std::vector<MapEntry> MapRegistry::ScanDetailed(const std::string& dir) {
    std::vector<MapEntry> out;

    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        std::cerr << "MapRegistry: not a directory: " << dir << "\n";
        return out;
    }

    std::vector<fs::path> paths;
    for (auto& e : fs::directory_iterator(dir)) {
        if (!e.is_regular_file()) continue;
        if (e.path().extension() == ".mgs") {
            paths.push_back(e.path());
        }
    }
    std::sort(paths.begin(), paths.end());

    for (auto& p : paths) {
        MapEntry entry;
        entry.path = p.generic_string();

        auto m = Map::Load(entry.path);
        if (m && !m->name.empty()) {
            entry.name = m->name;
        } else {
            entry.name = p.stem().string();
        }
        out.push_back(entry);
    }

    std::cout << "MapRegistry: found " << out.size() << " map(s) in " << dir << "\n";
    for (auto& e : out) std::cout << "  [" << e.name << "] " << e.path << "\n";
    return out;
}

} // namespace ml
