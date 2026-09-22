#include "npc.h"

namespace ml {

void Npc::Update(float dt, const glm::vec3& targetFeet, const PhysWorld& world) {
    glm::vec3 toTarget = targetFeet - position;
    toTarget.y = 0.0f;
    float dist = glm::length(toTarget);

    glm::vec3 wishDir(0.0f);
    if (dist > stopDist) {
        wishDir = toTarget / dist;
    }

    velocity.x = wishDir.x * speed;
    velocity.z = wishDir.z * speed;
    velocity.y -= gravity * dt;

    position = world.MoveCharacter(position, radius, height,
                                   velocity, dt, onGround);

    if (position.y < 0.0f) {
        position.y = 0.0f;
        velocity.y = 0.0f;
        onGround = true;
    }
}

} // namespace ml