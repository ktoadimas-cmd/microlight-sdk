#include "texture.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace ml {

Texture::~Texture() {
    if (id_) glDeleteTextures(1, &id_);
}

bool Texture::LoadFromFile(const std::string& path, bool flipVertically) {
    stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

    int w = 0, h = 0, channels = 0;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    if (!data) {
        std::cerr << "Texture load failed: " << path
                  << " (" << stbi_failure_reason() << ")\n";
        return false;
    }

    GLenum fmt = GL_RGB;
    if      (channels == 4) fmt = GL_RGBA;
    else if (channels == 1) fmt = GL_RED;

    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    width_  = w;
    height_ = h;
    std::cout << "Texture loaded: " << path << " (" << w << "x" << h
              << ", " << channels << " ch)\n";
    return true;
}

void Texture::Bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id_);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace ml
