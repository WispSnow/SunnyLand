#include "collision.h"
#include "../component/collider_component.h"
#include "../component/transform_component.h"

namespace engine::physics::collision {

bool checkCollision(const engine::component::ColliderComponent& a, const engine::component::ColliderComponent& b) {
    // 获取两个碰撞盒及对应Transform信息
    auto a_collider = a.getCollider();
    auto b_collider = b.getCollider();
    auto a_transform = a.getTransform();
    auto b_transform = b.getTransform();

    // 先计算最小包围盒是否碰撞，如果没有碰撞，那一定是返回false (不考虑AABB的旋转)
    auto a_size = a_collider->getAABBSize() * a_transform->getScale();
    auto b_size = b_collider->getAABBSize() * b_transform->getScale();
    auto a_pos = a_transform->getPosition() + a.getOffset();
    auto b_pos = b_transform->getPosition() + b.getOffset();
    if (!checkAABBOverlap(a_pos, a_size, b_pos, b_size)) {
        return false;
    }

    // --- 如果最小包围盒有碰撞，再进行更细致的判断 ---
    // AABB vs AABB, 直接返回真
    if (a_collider->getType() == engine::physics::ColliderType::AABB && b_collider->getType() == engine::physics::ColliderType::AABB) {
        return true;
    }
    // Circle vs Circle: 判断两个圆心距离是否小于两个圆的半径之和
    else if (a_collider->getType() == engine::physics::ColliderType::CIRCLE && b_collider->getType() == engine::physics::ColliderType::CIRCLE)
    {
        auto a_center = a_pos + 0.5f * a_size;  // 圆心位置
        auto b_center = b_pos + 0.5f * b_size;
        auto a_radius = 0.5f * a_size.x;        // 圆的半径等于AABB的一半宽度
        auto b_radius = 0.5f * b_size.x;
        return checkCircleOverlap(a_center, a_radius, b_center, b_radius);
    }
    // AABB vs Circle: 判断圆心到AABB的最邻近点是否在圆内
    else if (a_collider->getType() == engine::physics::ColliderType::AABB && b_collider->getType() == engine::physics::ColliderType::CIRCLE)
    {
        auto b_center = b_pos + 0.5f * b_size;
        auto b_radius = 0.5f * b_size.x;
        auto nearest_point = glm::clamp(b_center, a_pos, a_pos + a_size);   // 计算圆心到AABB的最邻近点
        return checkPointInCircle(nearest_point, b_center, b_radius);
    }
    // Circle vs AABB
    else if (a_collider->getType() == engine::physics::ColliderType::CIRCLE && b_collider->getType() == engine::physics::ColliderType::AABB)
    {
        auto a_center = a_pos + 0.5f * a_size;
        auto a_radius = 0.5f * a_size.x;
        auto nearest_point = glm::clamp(a_center, b_pos, b_pos + b_size);   // 计算圆心到AABB的最邻近点
        return checkPointInCircle(nearest_point, a_center, a_radius);
    }
    return false;
}

bool checkCircleOverlap(const glm::vec2& a_center, const float a_radius, const glm::vec2& b_center, const float b_radius)
{
    return (glm::length(a_center - b_center) < a_radius + b_radius);
}

bool checkAABBOverlap(const glm::vec2& a_pos, const glm::vec2& a_size, const glm::vec2& b_pos, const glm::vec2& b_size) {
    // 检查两个AABB是否重叠
    if (a_pos.x + a_size.x <= b_pos.x || a_pos.x >= b_pos.x + b_size.x ||
        a_pos.y + a_size.y <= b_pos.y || a_pos.y >= b_pos.y + b_size.y) {
        return false;
    }
    return true;
}

bool checkRectOverlap(const engine::utils::Rect& a, const engine::utils::Rect& b) {
    return checkAABBOverlap(a.position, a.size, b.position, b.size);
}

bool checkPointInCircle(const glm::vec2 &point, const glm::vec2 &center, const float radius)
{
    return (glm::length(point - center) < radius);
}

} // namespace engine::physics::collision 