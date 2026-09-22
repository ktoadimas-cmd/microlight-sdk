#include "map.h"
#include "kv.h"
#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <iomanip>

namespace fs = std::filesystem;

namespace ml {

static void ParseFloats(const std::string& s, float* out, int count, const float* defs) {
    for (int i = 0; i < count; ++i) out[i] = defs[i];
    std::string cleaned;
    cleaned.reserve(s.size());
    for (char c : s) {
        if (c == ',' || c == ';' || c == '\t') cleaned += ' ';
        else cleaned += c;
    }
    std::istringstream is(cleaned);
    for (int i = 0; i < count; ++i) if (!(is >> out[i])) break;
}

static glm::vec3 ParseVec3(const std::string& s, glm::vec3 def = {0,0,0}) {
    float d[3] = { def.x, def.y, def.z };
    float v[3];
    ParseFloats(s, v, 3, d);
    return { v[0], v[1], v[2] };
}

static glm::vec4 ParseVec4(const std::string& s, glm::vec4 def = {0,0,0,1}) {
    float d[4] = { def.x, def.y, def.z, def.w };
    float v[4];
    ParseFloats(s, v, 4, d);
    return { v[0], v[1], v[2], v[3] };
}

static glm::vec3 ReadVec3(const KV* parent, const char* key, glm::vec3 def) {
    if (!parent) return def;
    if (const KV* n = parent->Find(key)) return ParseVec3(n->Value(), def);
    std::string kx = std::string(key) + "X";
    std::string ky = std::string(key) + "Y";
    std::string kz = std::string(key) + "Z";
    const KV* nx = parent->Find(kx);
    const KV* ny = parent->Find(ky);
    const KV* nz = parent->Find(kz);
    if (nx || ny || nz) {
        glm::vec3 v = def;
        if (nx) try { v.x = std::stof(nx->Value()); } catch(...) {}
        if (ny) try { v.y = std::stof(ny->Value()); } catch(...) {}
        if (nz) try { v.z = std::stof(nz->Value()); } catch(...) {}
        return v;
    }
    return def;
}

static glm::vec4 ReadVec4(const KV* parent, const char* key, glm::vec4 def) {
    if (!parent) return def;
    if (const KV* n = parent->Find(key)) return ParseVec4(n->Value(), def);
    return def;
}

std::unique_ptr<Map> Map::Load(const std::string& path) {
    if (!fs::exists(path)) {
        std::cerr << "Map: file not found: " << path << "\n";
        return nullptr;
    }

    auto root = KV::ParseFile(path);
    if (!root) return nullptr;

    const KV* mgs = (root->Key() == "MGS") ? root.get() : root->Find("MGS");
    if (!mgs) {
        std::cerr << "Map: no 'MGS' root node in " << path << "\n";
        return nullptr;
    }

    auto m = std::make_unique<Map>();
    m->sourcePath_ = path;
    m->name    = mgs->GetString("Name", fs::path(path).stem().string());
    m->author  = mgs->GetString("Author", "");
    m->skybox  = mgs->GetString("Skybox", "day");

    if (const KV* sky = mgs->Find("SkyColor")) {
        m->skyColor = ParseVec3(sky->Value(), m->skyColor);
    }

    if (const KV* sp = mgs->Find("Spawn")) {
        m->spawn.pos = ReadVec3(sp, "Pos", m->spawn.pos);
        m->spawn.yaw = sp->GetFloat("Yaw", m->spawn.yaw);
    }

    if (const KV* li = mgs->Find("Light")) {
        m->light.dir     = ReadVec3(li, "Dir",     m->light.dir);
        m->light.color   = ReadVec3(li, "Color",   m->light.color);
        m->light.ambient = ReadVec3(li, "Ambient", m->light.ambient);
    }

    if (const KV* ents = mgs->Find("Entities")) {
        for (auto& c : ents->Children()) {
            if (c->Key() == "Brush") {
                MapBrush b;
                b.pos     = ReadVec3(c.get(), "Pos",  b.pos);
                b.size    = ReadVec3(c.get(), "Size", b.size);
                b.texture = c->GetString("Texture", b.texture);
                b.tint    = ReadVec4(c.get(), "Tint", b.tint);
                m->brushes.push_back(b);
            }
            else if (c->Key() == "Model") {
                MapModelEntity e;
                e.file  = c->GetString("File");
                e.pos   = ReadVec3(c.get(), "Pos",   e.pos);
                e.scale = ReadVec3(c.get(), "Scale", e.scale);
                e.spin  = ReadVec3(c.get(), "Spin",  e.spin);
                e.yaw   = ReadVec3(c.get(), "Yaw",   e.yaw);
                if (!e.file.empty()) m->models.push_back(e);
            }
        }
    }

    return m;
}

static std::string Fmt3(const glm::vec3& v, int prec = 4) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(prec) << v.x << " " << v.y << " " << v.z;
    return os.str();
}

static std::string Fmt4(const glm::vec4& v, int prec = 4) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(prec) << v.x << " " << v.y << " " << v.z << " " << v.w;
    return os.str();
}

bool Map::Save(const std::string& path) const {
    std::ofstream f(path);
    if (!f) {
        std::cerr << "Map::Save: cannot open " << path << "\n";
        return false;
    }

    f << "\"MGS\"\n{\n";
    f << "    Name      \"" << name << "\"\n";
    if (!author.empty())
        f << "    Author    \"" << author << "\"\n";
    f << "    SkyColor  \"" << Fmt3(skyColor) << "\"\n\n";

    f << "    Spawn\n    {\n";
    f << "        Pos    \"" << Fmt3(spawn.pos) << "\"\n";
    f << "        Yaw    " << spawn.yaw << "\n";
    f << "    }\n\n";

    f << "    Light\n    {\n";
    f << "        Dir     \"" << Fmt3(light.dir)     << "\"\n";
    f << "        Color   \"" << Fmt3(light.color)   << "\"\n";
    f << "        Ambient \"" << Fmt3(light.ambient) << "\"\n";
    f << "    }\n\n";

    f << "    Entities\n    {\n";
    for (const auto& b : brushes) {
        f << "        Brush\n        {\n";
        f << "            Pos     \"" << Fmt3(b.pos)  << "\"\n";
        f << "            Size    \"" << Fmt3(b.size) << "\"\n";
        f << "            Texture \"" << b.texture    << "\"\n";
        f << "            Tint    \"" << Fmt4(b.tint) << "\"\n";
        f << "        }\n";
    }
    for (const auto& e : models) {
        f << "        Model\n        {\n";
        f << "            File    \"" << e.file << "\"\n";
        f << "            Pos     \"" << Fmt3(e.pos)   << "\"\n";
        f << "            Scale   \"" << Fmt3(e.scale) << "\"\n";
        f << "            Spin    \"" << Fmt3(e.spin)  << "\"\n";
        f << "        }\n";
    }
    f << "    }\n}\n";

    std::cout << "Map saved: " << path
              << " (" << brushes.size() << " brushes, "
              << models.size() << " models)\n";
    return true;
}

void Map::Dump() const {
    std::cout << "=== Map: " << name << " ===\n";
    std::cout << "  spawn:    " << spawn.pos.x << " " << spawn.pos.y << " " << spawn.pos.z << "\n";
    std::cout << "  brushes:  " << brushes.size() << "\n";
    std::cout << "  models:   " << models.size() << "\n";
    std::cout << "================\n";
}

} // namespace ml
