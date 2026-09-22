#pragma once
#include <string>
#include <vector>

namespace ml {

struct MapEntry {
    std::string path;   // "game/maps/test.mgs"
    std::string name;   // из поля Name внутри файла
};

class MapRegistry {
public:
    static std::vector<std::string> Scan(const std::string& dir);
    static std::vector<MapEntry> ScanDetailed(const std::string& dir);
};

} // namespace ml
