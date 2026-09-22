#pragma once
#include "gl.h"
#include "shader.h"
#include "texture.h"
#include "core/camera.h"
#include "core/model.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace ml {

struct Vertex3D {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct PointLight {
    glm::vec3 pos;
    glm::vec3 color;
    float     range;
};

class Renderer {
public:
    bool Init();
    void Shutdown();

    void BeginFrame(int w, int h, const Camera& cam, float r, float g, float b);
    void EndFrame();

    // Р СњР В°РЎвЂЎР В°РЎвЂљРЎРЉ Р В·Р В°Р С—Р С‘РЎРѓРЎРЉ shadow map РІР‚вЂќ Р Р†РЎвЂ№Р В·Р Р†Р В°РЎвЂљРЎРЉ Р СџР вЂўР В Р вЂўР вЂќ РЎР‚Р С‘РЎРѓР С•Р Р†Р В°Р Р…Р С‘Р ВµР С
    void BeginShadowPass(const glm::vec3& lightDir);

    void DrawCube(const glm::vec3& pos, float scale, const Texture& tex,
                  const glm::vec4& tint = {1,1,1,1});

    void DrawCubeScaled(const glm::mat4& model, const Texture& tex,
                        const glm::vec4& tint = {1,1,1,1});

    void DrawModel(const Model& model, const glm::mat4& modelMatrix);

    // Р С›РЎР‚РЎС“Р В¶Р С‘Р Вµ Р С•РЎвЂљ Р С—Р ВµРЎР‚Р Р†Р С•Р С–Р С• Р В»Р С‘РЎвЂ Р В° (model space = camera space)
    void BeginViewModel(const Camera& cam);
    void DrawWeapon(const Model& model, const glm::mat4& offset);

    Shader& GetShader() { return *shader_; }
    void SetLights(const std::vector<PointLight>& lights);
    void SetTime(float t) { time_ = t; }

private:
    GLuint cubeVAO_ = 0;
    GLuint cubeVBO_ = 0;
    GLuint cubeEBO_ = 0;

    // Shadow map
    GLuint shadowFBO_ = 0;
    GLuint shadowMap_ = 0;
    int    shadowSize_ = 2048;
    glm::mat4 lightSpace_ = glm::mat4(1.0f);

    std::unique_ptr<Shader> shader_;
    std::unique_ptr<Shader> shadowShader_;

    std::vector<PointLight> lights_;

    // РџРѕСЃС‚-РѕР±СЂР°Р±РѕС‚РєР° (VHS)
    GLuint postFBO_ = 0;
    GLuint postTex_ = 0;
    GLuint postDepth_ = 0;
    GLuint postVAO_ = 0;
    GLuint postVBO_ = 0;
    std::unique_ptr<Shader> postShader_;
    int    postW_ = 640;
    int    postH_ = 360;
    int    screenW_ = 1280;
    int    screenH_ = 720;
    float  time_ = 0.0f;
};

} // namespace ml
