#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace ml {

// Осевой bounding box
struct AABB {
    glm::vec3 min{0.0f};
    glm::vec3 max{0.0f};

    static AABB FromCenterSize(const glm::vec3& center, const glm::vec3& size) {
        glm::vec3 half = size * 0.5f;
        return { center - half, center + half };
    }
    bool Intersects(const AABB& o) const {
        return min.x < o.max.x && max.x > o.min.x &&
               min.y < o.max.y && max.y > o.min.y &&
               min.z < o.max.z && max.z > o.min.z;
    }
};

// Мир физики: список статических AABB
class PhysWorld {
public:
    void AddBox(const AABB& box) { boxes_.push_back(box); }
    void Clear() { boxes_.clear(); }

    const std::vector<AABB>& Boxes() const { return boxes_; }

    // Проверка пересечения с любым боксом
    bool OverlapsAny(const AABB& box) const;

    // Движение "капсулы" (упрощённо — вертикальный цилиндр как AABB)
    //   pos       — центр ног персонажа (position = feet center)
    //   radius    — радиус (для X/Z)
    //   height    — полная высота
    //   velocity  — вход/выход (после столкновения компоненты обнуляются)
    //   dt        — шаг времени
    //   outOnGround — true если стоим на чём-то
    // Возвращает новую позицию ног.
    glm::vec3 MoveCharacter(const glm::vec3& pos,
                            float radius,
                            float height,
                            glm::vec3& velocity,
                            float dt,
                            bool& outOnGround) const;

private:
    std::vector<AABB> boxes_;
};

} // namespace ml
