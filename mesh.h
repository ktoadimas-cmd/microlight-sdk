#pragma once
#include "../gl.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace ml {

struct VertexPNU {   // Position, Normal, UV
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

class Mesh {
public:
    Mesh() = default;
    ~Mesh();

    // Загружает массив вершин + индексы в GPU
    void Upload(const std::vector<VertexPNU>& verts,
                const std::vector<unsigned int>& indices);

    void Draw() const;

    void Shutdown();

    bool   IsValid()     const { return vao_ != 0; }
    size_t VertexCount() const { return vertexCount_; }
    size_t IndexCount()  const { return indexCount_; }

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    size_t vertexCount_ = 0;
    size_t indexCount_  = 0;
};

} // namespace ml
