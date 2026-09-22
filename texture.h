#pragma once
#include "gl.h"
#include <string>

namespace ml {

class Texture {
public:
    Texture() = default;
    ~Texture();

    bool LoadFromFile(const std::string& path, bool flipVertically = true);
    void Bind(int unit = 0) const;
    void Unbind() const;

    GLuint Id()     const { return id_; }
    int    Width()  const { return width_; }
    int    Height() const { return height_; }

private:
    GLuint id_     = 0;
    int    width_  = 0;
    int    height_ = 0;
};

} // namespace ml
