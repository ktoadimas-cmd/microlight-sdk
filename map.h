#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

namespace ml {

struct MapBrush {
    glm::vec3 pos      = {0,0,0};
    glm::vec3 size     = {1,1,1};
    std::string texture = "test.png";
    glm::vec4 tint     = {1,1,1,1};
};

struct MapModelEntity {
    std::string file;
    glm::vec3   pos   = {0,0,0};
    glm::vec3   scale = {1,1,1};
    glm::vec3   spin  = {0,0,0};
    glm::vec3   yaw   = {0,0,0};
};

struct MapLight {
    glm::vec3 dir     = {-0.5f, -1.0f, -0.3f};
    glm::vec3 color   = {1.0f, 0.97f, 0.9f};
    glm::vec3 ambient = {0.25f, 0.27f, 0.35f};
};

struct MapSpawn {
    glm::vec3 pos = {0, 0, 5};
    float     yaw = -90.0f;
};

class Map {
public:
    std::string name   = "Untitled";
    std::string author;
    glm::vec3   skyColor = {0.05f, 0.06f, 0.09f};
    std::string skybox   = "day";

    MapSpawn spawn;
    MapLight light;

    std::vector<MapBrush>        brushes;
    std::vector<MapModelEntity>  models;

    static std::unique_ptr<Map> Load(const std::string& path);
    bool Save(const std::string& path) const;   // <-- НОВОЕ
    void Dump() const;

    const std::string& SourcePath() const { return sourcePath_; }
    void SetSourcePath(const std::string& p) { sourcePath_ = p; }

private:
    std::string sourcePath_;
};

} // namespace ml
