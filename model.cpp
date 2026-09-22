#include "model.h"
#include "mlm.h"
#include "shader.h"
#include <iostream>
#include <filesystem>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>

namespace fs = std::filesystem;

namespace ml {

bool Model::LoadOBJ(const std::string& path, const std::string& texDir) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string baseDir = texDir;
    if (baseDir.empty()) {
        baseDir = fs::path(path).parent_path().string();
    }
    if (baseDir.empty()) baseDir = ".";

    bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials,
                               &warn, &err,
                               path.c_str(),
                               baseDir.c_str(),
                               true,   // triangulate
                               true);  // default_vcols_fallback

    if (!warn.empty()) std::cerr << "OBJ warn: " << warn << "\n";
    if (!err.empty())  std::cerr << "OBJ err:  " << err  << "\n";
    if (!ok) {
        std::cerr << "Model: failed to load " << path << "\n";
        return false;
    }

    std::cout << "OBJ loaded: " << path
              << " shapes=" << shapes.size()
              << " materials=" << materials.size() << "\n";

    // РљСЌС€ РјР°С‚РµСЂРёР°Р»РѕРІ -> С‚РµРєСЃС‚СѓСЂ (С‡С‚РѕР±С‹ РЅРµ РіСЂСѓР·РёС‚СЊ РґРІР°Р¶РґС‹)
    std::vector<std::shared_ptr<Texture>> matTextures(materials.size());

    for (size_t mi = 0; mi < materials.size(); ++mi) {
        const auto& m = materials[mi];
        if (m.diffuse_texname.empty()) continue;
        auto tex = std::make_shared<Texture>();
        std::string texPath = baseDir + "/" + m.diffuse_texname;
        if (tex->LoadFromFile(texPath)) {
            matTextures[mi] = tex;
        } else {
            std::cerr << "Model: texture not found: " << texPath << "\n";
        }
    }

    for (const auto& shape : shapes) {
        // Р“СЂСѓРїРїРёСЂСѓРµРј С‚СЂРµСѓРіРѕР»СЊРЅРёРєРё РїРѕ material_id
        std::vector<VertexPNU> verts;
        std::vector<unsigned int> indices;

        size_t indexOffset = 0;
        int currentMat = -1;
        std::vector<VertexPNU> chunkVerts;
        std::vector<unsigned int> chunkIndices;

        auto flushChunk = [&]() {
            if (chunkIndices.empty()) return;
            SubMesh sm;
            sm.mesh.Upload(chunkVerts, chunkIndices);
            if (currentMat >= 0 && currentMat < (int)materials.size()) {
                sm.albedo = matTextures[currentMat];
                const auto& m = materials[currentMat];
                sm.tint = { m.diffuse[0], m.diffuse[1], m.diffuse[2], 1.0f };
                if (sm.tint.r == 0 && sm.tint.g == 0 && sm.tint.b == 0) {
                    sm.tint = {1,1,1,1};
                }
            }
            subs_.push_back(std::move(sm));
            chunkVerts.clear();
            chunkIndices.clear();
        };

        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int matId = shape.mesh.material_ids[f];
            if (matId != currentMat) {
                flushChunk();
                currentMat = matId;
            }

            int fv = shape.mesh.num_face_vertices[f]; // РґРѕР»Р¶РЅРѕ Р±С‹С‚СЊ 3 (triangulate)
            for (int v = 0; v < fv; ++v) {
                auto idx = shape.mesh.indices[indexOffset + v];
                VertexPNU vert;
                if (idx.vertex_index >= 0) {
                    vert.pos.x = attrib.vertices[3 * idx.vertex_index + 0];
                    vert.pos.y = attrib.vertices[3 * idx.vertex_index + 1];
                    vert.pos.z = attrib.vertices[3 * idx.vertex_index + 2];
                }
                if (idx.normal_index >= 0) {
                    vert.normal.x = attrib.normals[3 * idx.normal_index + 0];
                    vert.normal.y = attrib.normals[3 * idx.normal_index + 1];
                    vert.normal.z = attrib.normals[3 * idx.normal_index + 2];
                } else {
                    vert.normal = {0, 1, 0};
                }
                if (idx.texcoord_index >= 0) {
                    vert.uv.x = attrib.texcoords[2 * idx.texcoord_index + 0];
                    vert.uv.y = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];
                }
                chunkVerts.push_back(vert);
                chunkIndices.push_back((unsigned int)chunkIndices.size());
            }
            indexOffset += fv;
        }

        flushChunk();
    }

    std::cout << "Model: " << subs_.size() << " submeshes\n";
    return !subs_.empty();
}

void Model::Draw(Shader& shader) const {
    for (const auto& sm : subs_) {
        if (!sm.mesh.IsValid()) continue;
        if (sm.albedo) {
            sm.albedo->Bind(0);
            shader.SetInt("uTex", 0);
            shader.SetInt("uUseTex", 1);
        } else {
            shader.SetInt("uUseTex", 0);
        }
        shader.SetVec4("uTint", sm.tint);
        sm.mesh.Draw();
    }
}

bool Model::LoadMLM(const std::string& path) {
    MLMLoader::Data data;
    if (!MLMLoader::Load(path, data)) return false;

    SubMesh sm;
    sm.mesh.Upload(data.vertices, data.indices);
    sm.tint = {1, 1, 1, 1};
    subs_.push_back(std::move(sm));

    std::cout << "Model: " << subs_.size() << " submeshes (from MLM)\n";
    return true;
}

} // namespace ml