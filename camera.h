#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ml {

class Camera {
public:
    glm::vec3 position = { 0.0f, 1.7f, 5.0f };
    glm::vec3 front    = { 0.0f, 0.0f, -1.0f };
    glm::vec3 up       = { 0.0f, 1.0f,  0.0f };
    glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };

    float yaw   = -90.0f;
    float pitch =   0.0f;

    float fovY      = 90.0f;
    float aspect    = 16.0f / 9.0f;
    float nearPlane = 0.05f;
    float farPlane  = 500.0f;

    // Player
    float standHeight  = 1.70f;
    float crouchHeight = 1.10f;
    float eyeHeight    = 1.70f;

    float walkSpeed    = 5.0f;
    float crouchSpeed  = 2.5f;
    float accel        = 40.0f;
    float airAccel     = 5.0f;
    float friction     = 8.0f;

    float gravity      = 20.0f;
    float jumpSpeed    = 6.5f;

    // === Bhop ===
    float bhopSpeedMul  = 2.2f;
    float bhopBoost     = 1.08f;
    float bhopMaxSpeed  = 16.0f;
    float bhopDecay     = 3.0f;
    bool  bhopActive    = false;
    float bhopCurrMul   = 1.0f;

    bool  onGround     = false;

    float mouseSens    = 0.10f;

    float collRadius   = 0.35f;
    float collStandH   = 1.80f;
    float collCrouchH  = 1.20f;
    float collHeight   = 1.80f;

    // === Р”Р»СЏ СЂРµРґР°РєС‚РѕСЂР° вЂ” СЃРІРѕР±РѕРґРЅС‹Р№ РїРѕР»С‘С‚ ===
    float movementSpeed = 15.0f;

    void  UpdateVectors();
    void  ProcessMouse(float dx, float dy);

    void  ApplyInput(float dt,
                     const glm::vec3& wishDir,
                     bool jump,
                     bool crouch,
                     bool shift = false,
                     bool forward = false);

    // РЎРІРѕР±РѕРґРЅС‹Р№ РїРѕР»С‘С‚ (РґР»СЏ СЂРµРґР°РєС‚РѕСЂР°)
    void  FlyMove(const glm::vec3& dir, float dt, float speedMul = 1.0f);

    void  SetFeetPosition(const glm::vec3& feet);

    glm::mat4 View()       const;
    glm::mat4 Projection() const;

private:
    void ApplyFriction(float dt);
    void ApplyBhop(float dt, bool shift, bool forward, bool jump);
    void Accelerate(const glm::vec3& wishDir, float wishSpeed, float accel_, float dt);
};

} // namespace ml
