#include "camera.h"
#include <cmath>
#include <algorithm>

namespace ml {

void Camera::UpdateVectors() {
    glm::vec3 f;
    f.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    f.y = std::sin(glm::radians(pitch));
    f.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    front = glm::normalize(f);
}

void Camera::ProcessMouse(float dx, float dy) {
    yaw   += dx * mouseSens;
    pitch -= dy * mouseSens;
    if (pitch >  89.0f) pitch =  89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    UpdateVectors();
}

void Camera::ApplyFriction(float dt) {
    if (!onGround) return;
    float speed = glm::length(glm::vec3(velocity.x, 0.0f, velocity.z));
    if (speed < 0.01f) {
        velocity.x = velocity.z = 0.0f;
        return;
    }
    float drop = speed * friction * dt;
    float newSpeed = std::max(speed - drop, 0.0f);
    float k = newSpeed / speed;
    velocity.x *= k;
    velocity.z *= k;
}

void Camera::Accelerate(const glm::vec3& wishDir, float wishSpeed, float accel_, float dt) {
    float currentSpeed = glm::dot(velocity, wishDir);
    float addSpeed     = wishSpeed - currentSpeed;
    if (addSpeed <= 0.0f) return;
    float accelSpeed = accel_ * dt * wishSpeed;
    if (accelSpeed > addSpeed) accelSpeed = addSpeed;
    velocity += wishDir * accelSpeed;
}

void Camera::ApplyInput(float dt,
                        const glm::vec3& wishDir,
                        bool jump,
                        bool crouch,
                        bool shift,
                        bool forward)
{
    float targetEye = crouch ? crouchHeight : standHeight;
    eyeHeight += (targetEye - eyeHeight) * std::min(dt * 10.0f, 1.0f);
    collHeight = crouch ? collCrouchH : collStandH;

    glm::vec3 horizWish = wishDir;
    horizWish.y = 0.0f;
    float wishLen = glm::length(horizWish);
    if (wishLen > 0.001f) horizWish /= wishLen;

    float speed = crouch ? crouchSpeed : (walkSpeed * bhopCurrMul);
    float a     = onGround ? accel : airAccel;

    ApplyFriction(dt);
    if (wishLen > 0.001f) {
        Accelerate(horizWish, speed, a, dt);
    }

    // === Банихоп ===
    ApplyBhop(dt, shift, forward, jump);

    velocity.y -= gravity * dt;

    if (jump && onGround && !bhopActive) {
        velocity.y = jumpSpeed;
        onGround = false;
    }

    UpdateVectors();
}

// === Банихоп ===
void Camera::ApplyBhop(float dt, bool shift, bool forward, bool jump) {
    bool wantBhop = shift && forward;

    if (wantBhop && onGround && jump) {
        velocity.y = jumpSpeed;
        onGround = false;
        bhopActive = true;
        bhopCurrMul *= bhopBoost;
        if (bhopCurrMul > bhopSpeedMul) bhopCurrMul = bhopSpeedMul;
    } else if (bhopActive && onGround && !wantBhop) {
        bhopCurrMul -= bhopDecay * dt;
        if (bhopCurrMul < 1.0f) {
            bhopCurrMul = 1.0f;
            bhopActive = false;
        }
    }

    // Потолок горизонтальной скорости
    float horizSpeed = glm::length(glm::vec3(velocity.x, 0.0f, velocity.z));
    float maxS = walkSpeed * bhopSpeedMul;
    if (horizSpeed > maxS) {
        float k = maxS / horizSpeed;
        velocity.x *= k;
        velocity.z *= k;
    }
}

// === Свободный полёт для редактора ===
void Camera::FlyMove(const glm::vec3& dir, float dt, float speedMul) {
    position += dir * (movementSpeed * speedMul * dt);
}

void Camera::SetFeetPosition(const glm::vec3& feet) {
    position = feet;
    position.y += eyeHeight;
}

glm::mat4 Camera::View() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::Projection() const {
    return glm::perspective(glm::radians(fovY), aspect, nearPlane, farPlane);
}

} // namespace ml
