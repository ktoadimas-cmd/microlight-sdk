#include "font.h"
#include <iostream>
#include <vector>
#include <cstdio>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

namespace ml {

Font::~Font() {
    if (atlasTex_) glDeleteTextures(1, &atlasTex_);
}

bool Font::Load(const std::string& ttfPath, float pixelHeight) {
    pixelHeight_ = pixelHeight;

    // Читаем TTF файл
    FILE* f = fopen(ttfPath.c_str(), "rb");
    if (!f) {
        std::cerr << "Font: cannot open " << ttfPath << "\n";
        return false;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<unsigned char> ttf(sz);
    fread(ttf.data(), 1, sz, f);
    fclose(f);

    // Атлас 512x512, шрифт 32px, ASCII 32..126
    const int ATLAS_W = 512;
    const int ATLAS_H = 512;
    const int FIRST   = 32;
    const int COUNT   = 95;

    std::vector<unsigned char> bitmap(ATLAS_W * ATLAS_H, 0);

    stbtt_bakedchar baked[COUNT];
    int res = stbtt_BakeFontBitmap(ttf.data(), 0, pixelHeight,
                                   bitmap.data(), ATLAS_W, ATLAS_H,
                                   FIRST, COUNT, baked);
    if (res <= 0) {
        std::cerr << "Font: stbtt_BakeFontBitmap failed (res=" << res << ")\n";
        return false;
    }

    // Настройки scale для stb (сколько пикселей на em)
    stbtt_fontinfo info;
    stbtt_InitFont(&info, ttf.data(), 0);
    scale_ = stbtt_ScaleForPixelHeight(&info, pixelHeight);

    atlasW_ = ATLAS_W;
    atlasH_ = ATLAS_H;

    // Загружаем атлас в GL (RED-канал)
    glGenTextures(1, &atlasTex_);
    glBindTexture(GL_TEXTURE_2D, atlasTex_);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ATLAS_W, ATLAS_H, 0,
                 GL_RED, GL_UNSIGNED_BYTE, bitmap.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Заполняем glyphs
    for (int i = 0; i < COUNT; ++i) {
        int cp = FIRST + i;
        const stbtt_bakedchar& b = baked[i];
        Glyph g;
        g.u0 = b.x0 / (float)ATLAS_W;
        g.v0 = b.y0 / (float)ATLAS_H;
        g.u1 = b.x1 / (float)ATLAS_W;
        g.v1 = b.y1 / (float)ATLAS_H;
        g.width    = b.x1 - b.x0;
        g.height   = b.y1 - b.y0;
        g.bearingX = b.xoff;
        g.bearingY = b.yoff;
        g.advance  = (int)b.xadvance;
        glyphs_[cp] = g;
    }

    std::cout << "Font loaded: " << ttfPath << " (" << pixelHeight << "px, "
              << COUNT << " glyphs)\n";
    return true;
}

const Glyph* Font::GetGlyph(int codepoint) const {
    auto it = glyphs_.find(codepoint);
    return (it != glyphs_.end()) ? &it->second : nullptr;
}

} // namespace ml
