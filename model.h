#pragma once
#include "mesh.h"
#include "texture.h"
#include "../shader.h"
#include <memory>
#include <string>
#include <vector>

namespace ml {

struct SubMesh {
    Mesh mesh;
    std::shared_ptr<Texture> albedo;
    glm::vec4  tint = {1,1,1,1};
};

class Model {
public:
    Model() = default;
    ~Model() = default;

    bool LoadMLM(const std::string& path);
    bool LoadOBJ(const std::string& path,
                 const std::string& texDir = "");

    void Draw(Shader& shader) const;

    const std::vector<SubMesh>& SubMeshes() const { return subs_; }
    bool IsValid() const { return !subs_.empty(); }

private:
    std::vector<SubMesh> subs_;
};

} // namespace ml
