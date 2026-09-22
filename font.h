#pragma once
#include "../gl.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace ml {

struct Glyph {
    float u0, v0, u1, v1;    // UV в атласе
    int   width, height;      // размер в пикселях
    int   bearingX, bearingY; // смещение baseline
    int   advance;            // сдвиг курсора
};

class Font {
public:
    Font() = default;
    ~Font();

    bool Load(const std::string& ttfPath, float pixelHeight = 32.0f);

    // Возвращает глиф по codepoint (ASCII)
    const Glyph* GetGlyph(int codepoint) const;

    GLuint AtlasTexture() const { return atlasTex_; }
    float  PixelHeight()  const { return pixelHeight_; }
    float  Scale()        const { return scale_; }

private:
    GLuint atlasTex_ = 0;
    int    atlasW_   = 0;
    int    atlasH_   = 0;
    float  pixelHeight_ = 32.0f;
    float  scale_       = 1.0f;
    std::unordered_map<int, Glyph> glyphs_;
};

} // namespace ml
