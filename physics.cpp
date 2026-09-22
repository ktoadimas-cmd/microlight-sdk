#include "physics.h"
#include <cmath>
#include <algorithm>

namespace ml {

bool PhysWorld::OverlapsAny(const AABB& box) const {
    for (auto& b : boxes_) {
        if (box.Intersects(b)) return true;
    }
    return false;
}

// AABB персонажа из позиции ног
static AABB CharacterBox(const glm::vec3& feet, float radius, float height) {
    AABB b;
    b.min = { feet.x - radius, feet.y,          feet.z - radius };
    b.max = { feet.x + radius, feet.y + height, feet.z + radius };
    return b;
}

glm::vec3 PhysWorld::MoveCharacter(const glm::vec3& pos,
                                   float radius,
                                   float height,
                                   glm::vec3& velocity,
                                   float dt,
                                   bool& outOnGround) const
{
    outOnGround = false;
    glm::vec3 p = pos;

    // ===== ОСЬ X =====
    {
        float dx = velocity.x * dt;
        if (dx != 0.0f) {
            glm::vec3 tryP = p; tryP.x += dx;
            AABB box = CharacterBox(tryP, radius, height);
            bool hit = false;
            for (auto& b : boxes_) {
                if (box.Intersects(b)) { hit = true; break; }
            }
            if (hit) {
                // Подтянуть к грани: бинарный поиск с 8 итераций
                float lo = 0.0f, hi = dx;
                for (int i = 0; i < 8; ++i) {
                    float mid = (lo + hi) * 0.5f;
                    glm::vec3 t = p; t.x += mid;
                    AABB tb = CharacterBox(t, radius, height);
                    bool th = false;
                    for (auto& b : boxes_) if (tb.Intersects(b)) { th = true; break; }
                    if (th) hi = mid; else lo = mid;
                }
                p.x += lo;
                velocity.x = 0.0f;
            } else {
                p.x = tryP.x;
            }
        }
    }

    // ===== ОСЬ Z =====
    {
        float dz = velocity.z * dt;
        if (dz != 0.0f) {
            glm::vec3 tryP = p; tryP.z += dz;
            AABB box = CharacterBox(tryP, radius, height);
            bool hit = false;
            for (auto& b : boxes_) {
                if (box.Intersects(b)) { hit = true; break; }
            }
            if (hit) {
                float lo = 0.0f, hi = dz;
                for (int i = 0; i < 8; ++i) {
                    float mid = (lo + hi) * 0.5f;
                    glm::vec3 t = p; t.z += mid;
                    AABB tb = CharacterBox(t, radius, height);
                    bool th = false;
                    for (auto& b : boxes_) if (tb.Intersects(b)) { th = true; break; }
                    if (th) hi = mid; else lo = mid;
                }
                p.z += lo;
                velocity.z = 0.0f;
            } else {
                p.z = tryP.z;
            }
        }
    }

    // ===== ОСЬ Y =====
    {
        float dy = velocity.y * dt;
        if (dy != 0.0f) {
            glm::vec3 tryP = p; tryP.y += dy;
            AABB box = CharacterBox(tryP, radius, height);
            bool hit = false;
            for (auto& b : boxes_) {
                if (box.Intersects(b)) { hit = true; break; }
            }
            if (hit) {
                float lo = 0.0f, hi = dy;
                for (int i = 0; i < 8; ++i) {
                    float mid = (lo + hi) * 0.5f;
                    glm::vec3 t = p; t.y += mid;
                    AABB tb = CharacterBox(t, radius, height);
                    bool th = false;
                    for (auto& b : boxes_) if (tb.Intersects(b)) { th = true; break; }
                    if (th) hi = mid; else lo = mid;
                }
                p.y += lo;
                if (dy < 0.0f) outOnGround = true;
                velocity.y = 0.0f;
            } else {
                p.y = tryP.y;
                // Отдельно: проверяем, стоим ли на чём-то (по 1 юниту вниз)
                glm::vec3 probe = p; probe.y -= 0.01f;
                AABB pb = CharacterBox(probe, radius, height);
                for (auto& b : boxes_) {
                    if (pb.Intersects(b)) { outOnGround = true; break; }
                }
            }
        } else {
            // velocity.y == 0 — проверяем, стоим ли на полу
            glm::vec3 probe = p; probe.y -= 0.01f;
            AABB pb = CharacterBox(probe, radius, height);
            for (auto& b : boxes_) {
                if (pb.Intersects(b)) { outOnGround = true; break; }
            }
        }
    }

    return p;
}

} // namespace ml
