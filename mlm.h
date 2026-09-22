#pragma once
#include "mesh.h"
#include <string>
#include <vector>

namespace ml {

// Формат MLM (Microlight Model) — текстовый:
//
//   MLM 1.0
//   name ak47
//   scale 1.0
//   vertex 0.0 0.0 0.0  0.0 0.0 1.0  0.0 0.0
//   face 0 1 2
//   ...
//
// Вершины: pos.x pos.y pos.z  normal.x normal.y normal.z  uv.x uv.y
// Грани: face i j k

class MLMLoader {
public:
    struct Data {
        std::string name;
        std::vector<VertexPNU>         vertices;
        std::vector<unsigned int>      indices;
    };

    // Загрузка .mlm файла
    static bool Load(const std::string& path, Data& out);

    // Сохранение .mlm файла
    static bool Save(const std::string& path, const Data& data);
};

} // namespace ml
