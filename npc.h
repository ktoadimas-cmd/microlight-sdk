#pragma once
#include "physics.h"
#include <glm/glm.hpp>

namespace ml {

class Npc {
public:
    glm::vec3 position = {0, 0, 0};   // ноги
    glm::vec3 velocity = {0, 0, 0};
    float radius   = 0.35f;
    float height   = 1.80f;
    float speed    = 2.5f;
    float stopDist = 1.5f;
    float gravity  = 20.0f;
    bool  onGround = false;

    void Init(const glm::vec3& feetPos) {
        position = feetPos;
        velocity = glm::vec3(0.0f);
    }

    void Update(float dt, const glm::vec3& targetFeet, const PhysWorld& world);
};

} // namespace ml