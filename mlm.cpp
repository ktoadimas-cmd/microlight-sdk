#include "mlm.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace ml {

bool MLMLoader::Load(const std::string& path, Data& out) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "MLM: cannot open " << path << "\n";
        return false;
    }

    out.vertices.clear();
    out.indices.clear();

    std::string line;
    int lineNum = 0;
    float scale = 1.0f;

    while (std::getline(f, line)) {
        lineNum++;

        // Убираем пробелы в начале/конце
        size_t a = line.find_first_not_of(" \t\r\n");
        if (a == std::string::npos) continue;
        size_t b = line.find_last_not_of(" \t\r\n");
        line = line.substr(a, b - a + 1);

        // Комментарии
        if (line[0] == '#') continue;

        std::istringstream is(line);
        std::string cmd;
        is >> cmd;

        if (cmd == "MLM") {
            std::string version;
            is >> version;
            // ok
        }
        else if (cmd == "name") {
            std::string name;
            is >> name;
            out.name = name;
        }
        else if (cmd == "scale") {
            is >> scale;
        }
        else if (cmd == "vertex" || cmd == "v") {
            VertexPNU v;
            is >> v.pos.x >> v.pos.y >> v.pos.z
               >> v.normal.x >> v.normal.y >> v.normal.z
               >> v.uv.x >> v.uv.y;
            v.pos *= scale;
            out.vertices.push_back(v);
        }
        else if (cmd == "face" || cmd == "f") {
            unsigned int i, j, k;
            is >> i >> j >> k;
            out.indices.push_back(i);
            out.indices.push_back(j);
            out.indices.push_back(k);
        }
        else {
            // неизвестная команда — игнорируем
        }
    }

    std::cout << "MLM loaded: " << path
              << " (" << out.vertices.size() << " verts, "
              << out.indices.size() / 3 << " tris)\n";

    return !out.vertices.empty();
}

bool MLMLoader::Save(const std::string& path, const Data& data) {
    std::ofstream f(path);
    if (!f) {
        std::cerr << "MLM: cannot write " << path << "\n";
        return false;
    }

    f << "MLM 1.0\n";
    f << "name " << data.name << "\n";
    f << "scale 1.0\n";
    f << "\n";

    for (auto& v : data.vertices) {
        f << "vertex "
          << v.pos.x    << " " << v.pos.y    << " " << v.pos.z    << "  "
          << v.normal.x << " " << v.normal.y << " " << v.normal.z << "  "
          << v.uv.x     << " " << v.uv.y     << "\n";
    }

    f << "\n";

    for (size_t i = 0; i + 2 < data.indices.size(); i += 3) {
        f << "face "
          << data.indices[i]   << " "
          << data.indices[i+1] << " "
          << data.indices[i+2] << "\n";
    }

    std::cout << "MLM saved: " << path
              << " (" << data.vertices.size() << " verts)\n";
    return true;
}

} // namespace ml
