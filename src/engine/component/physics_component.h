#pragma once
#include "component.h"
#include "glm/vec2.hpp"
#include <utility>

namespace engine::physics {
    class PhysicsEngine;
}

namespace engine::component {
class TransformComponent;

/**
 * @brief 管理GameObject的物理属性
 *
 * 存储速度、质量、力和重力设置。与PhysicsEngine交互。
 */
class PhysicsComponent final: public Component {
    friend class engine::object::GameObject;
public:
    glm::vec2 velocity_ = {0.0f, 0.0f};             ///< @brief 物体的速度，设为公共成员变量，方便PhysicsEngine访问更新

private:
    engine::physics::PhysicsEngine* physics_engine_ = nullptr;  ///< @brief 指向PhysicsEngine的指针
    TransformComponent* transform_ = nullptr;                   ///< @brief TransformComponent的缓存指针

    glm::vec2 force_ = {0.0f, 0.0f};                            ///< @brief 当前帧受到的力
    float mass_ = 1.0f;                             ///< @brief 物体质量（默认1.0）
    bool use_gravity_ = true;                       ///< @brief 物体是否受重力影响
    bool enabled_ = true;                           ///< @brief 组件是否激活

    // --- 碰撞状态标志 ---
    bool collided_below_ = false;
    bool collided_above_ = false;
    bool collided_left_ = false;
    bool collided_right_ = false;
    bool collided_ladder_ = false;      ///< @brief 是否与梯子发生碰撞
    bool is_on_top_ladder_ = false;     ///< @brief 是否在梯子顶层（梯子上方没有瓦片）

public:
    /**
     * @brief 构造函数  
     * 
     * @param physics_engine 指向PhysicsEngine的指针，不能为nullptr
     * @param use_gravity 物体是否受重力影响，默认true
     * @param mass 物体质量，默认1.0
     */
    PhysicsComponent(engine::physics::PhysicsEngine* physics_engine, bool use_gravity = true, float mass = 1.0f);
    ~PhysicsComponent() override = default;

    // 删除复制/移动操作
    PhysicsComponent(const PhysicsComponent&) = delete;
    PhysicsComponent& operator=(const PhysicsComponent&) = delete;
    PhysicsComponent(PhysicsComponent&&) = delete;
    PhysicsComponent& operator=(PhysicsComponent&&) = delete;

    // PhysicsEngine使用的物理方法
    void addForce(const glm::vec2& force) { if (enabled_) force_ += force; }    ///< @brief 添加力
    void clearForce() { force_ = {0.0f, 0.0f}; }                                ///< @brief 清除力
    const glm::vec2& getForce() const { return force_; }                        ///< @brief 获取当前力
    float getMass() const { return mass_; }                                     ///< @brief 获取质量
    bool isEnabled() const { return enabled_; }                                 ///< @brief 获取组件是否启用
    bool isUseGravity() const { return use_gravity_; }                          ///< @brief 获取组件是否受重力影响

    // 设置器/获取器
    void setEnabled(bool enabled) { enabled_ = enabled; }                       ///< @brief 设置组件是否启用
    void setMass(float mass) { mass_ = (mass >= 0.0f) ? mass : 1.0f; }          ///< @brief 设置质量，质量不能为负
    void setUseGravity(bool use_gravity) { use_gravity_ = use_gravity; }        ///< @brief 设置组件是否受重力影响
    void setVelocity(glm::vec2 velocity) { velocity_ = std::move(velocity); }       ///< @brief 设置速度
    const glm::vec2& getVelocity() const { return velocity_; }                  ///< @brief 获取当前速度
    TransformComponent* getTransform() const { return transform_; }             ///< @brief 获取TransformComponent指针

    // --- 碰撞状态访问与修改 (供 PhysicsEngine 使用) ---
    /** @brief 重置所有碰撞标志 (在物理更新开始时调用) */
    void resetCollisionFlags() {
        collided_below_ = false;
        collided_above_ = false;
        collided_left_ = false;
        collided_right_ = false;
        collided_ladder_ = false;
        is_on_top_ladder_ = false;
    }

    void setCollidedBelow(bool collided) { collided_below_ = collided; }    ///< @brief 设置下方碰撞标志
    void setCollidedAbove(bool collided) { collided_above_ = collided; }    ///< @brief 设置上方碰撞标志
    void setCollidedLeft(bool collided) { collided_left_ = collided; }      ///< @brief 设置左方碰撞标志
    void setCollidedRight(bool collided) { collided_right_ = collided; }    ///< @brief 设置右方碰撞标志
    void setCollidedLadder(bool collided) { collided_ladder_ = collided; }  ///< @brief 设置梯子碰撞标志
    void setOnTopLadder(bool on_top) { is_on_top_ladder_ = on_top; }        ///< @brief 设置是否在梯子顶层

    bool hasCollidedBelow() const { return collided_below_; }       ///< @brief 检查是否与下方发生碰撞
    bool hasCollidedAbove() const { return collided_above_; }       ///< @brief 检查是否与上方发生碰撞
    bool hasCollidedLeft() const { return collided_left_; }         ///< @brief 检查是否与左方发生碰撞
    bool hasCollidedRight() const { return collided_right_; }       ///< @brief 检查是否与右方发生碰撞
    bool hasCollidedLadder() const { return collided_ladder_; }     ///< @brief 检查是否与梯子发生碰撞
    bool isOnTopLadder() const { return is_on_top_ladder_; }        ///< @brief 检查是否在梯子顶层

private:
    // 核心循环方法
    void init() override;
    void update(float, engine::core::Context&) override {}
    void clean() override;
};

} // namespace engine::component