#pragma once
#include "component.h"
#include "glm/vec2.hpp"

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
    void setVelocity(const glm::vec2& velocity) { velocity_ = velocity; }       ///< @brief 设置速度
    const glm::vec2& getVelocity() const { return velocity_; }                  ///< @brief 获取当前速度
    TransformComponent* getTransform() const { return transform_; }             ///< @brief 获取TransformComponent指针

private:
    // 核心循环方法
    void init() override;
    void update(float, engine::core::Context&) override {}
    void clean() override;
};

} // namespace engine::component