#include "mesh.h"
#include <iostream>

namespace ml {

Mesh::~Mesh() {
    Shutdown();
}

void Mesh::Shutdown() {
    if (ebo_) glDeleteBuffers(1, &ebo_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    vao_ = vbo_ = ebo_ = 0;
    vertexCount_ = indexCount_ = 0;
}

void Mesh::Upload(const std::vector<VertexPNU>& verts,
                  const std::vector<unsigned int>& indices)
{
    Shutdown();

    vertexCount_ = verts.size();
    indexCount_  = indices.size();

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 verts.size() * sizeof(VertexPNU),
                 verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPNU),
                          (void*)offsetof(VertexPNU, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPNU),
                          (void*)offsetof(VertexPNU, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPNU),
                          (void*)offsetof(VertexPNU, uv));

    glBindVertexArray(0);

    std::cout << "Mesh uploaded: " << vertexCount_ << " verts, "
              << indexCount_ << " indices\n";
}

void Mesh::Draw() const {
    if (!vao_) return;
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, (GLsizei)indexCount_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

} // namespace ml
