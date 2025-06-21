#pragma once
#include <glm/vec2.hpp>
#include <utility>

namespace engine::physics {

/**
 * @brief 定义不同类型的碰撞器。
 */
enum class ColliderType {
    NONE,
    AABB,
    CIRCLE,
    // 未来可能添加其他类型，如 Capsule, Polygon 等
};

/**
 * @brief 碰撞器的抽象基类。
 * 所有具体的碰撞器都应继承此类。
 */
class Collider {
protected:
    glm::vec2 aabb_size_ = {0.0f, 0.0f};    ///< @brief 覆盖Collider的最小包围盒的尺寸（宽度和高度）。

public:
    virtual ~Collider() = default;
    virtual ColliderType getType() const = 0;   ///< @brief 获取碰撞器的类型。

    void setAABBSize(glm::vec2 size) { aabb_size_ = std::move(size); }  ///< @brief 设置最小包围盒的尺寸（宽度和高度）。
    const glm::vec2& getAABBSize() const { return aabb_size_; }     ///< @brief 获取最小包围盒的尺寸（宽度和高度）。
};

/**
 * @brief 轴对齐包围盒 (Axis-Aligned Bounding Box) 碰撞器。
 */
class AABBCollider final : public Collider {
private:
    glm::vec2 size_ = {0.0f, 0.0f};  ///< @brief 包围盒的尺寸（和aabb_size_相同）。

public:
    /**
     * @brief 构造函数。
     * @param size 包围盒的宽度和高度。
     */
    explicit AABBCollider(glm::vec2 size) : size_(std::move(size)) { setAABBSize(size_); }
    ~AABBCollider() override = default;

    // --- Getters and Setters ---
    ColliderType getType() const override { return ColliderType::AABB; }
    const glm::vec2& getSize() const { return size_; }
    void setSize(glm::vec2 size) { size_ = std::move(size); }
};

/**
 * @brief 圆形碰撞器。
 */
class CircleCollider final : public Collider {
private:
    float radius_ = 0.0f;  ///< @brief 圆的半径。

public:
    /**
     * @brief 构造函数。
     * @param radius 圆的半径。
     */
    explicit CircleCollider(float radius) : radius_(radius) { setAABBSize(glm::vec2(radius * 2.0f, radius * 2.0f)); }
    ~CircleCollider() override = default;

    // --- Getters and Setters ---
    ColliderType getType() const override { return ColliderType::CIRCLE; }
    float getRadius() const { return radius_; }
    void setRadius(float radius) { radius_ = radius; }
};

} // namespace engine::physics