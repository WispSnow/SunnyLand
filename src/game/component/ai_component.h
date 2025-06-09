#pragma once
#include "../../engine/component/component.h"
#include "ai/ai_behavior.h"
#include <memory>

namespace game::component::ai { class AIBehavior; }
namespace engine::component {
    class TransformComponent;
    class PhysicsComponent;
    class SpriteComponent;
    class AnimationComponent;
}

namespace game::component {

/**
 * @brief 负责管理 GameObject 的 AI 行为。
 *
 * 使用策略模式，持有一个具体的 AIBehavior 实例来执行实际的 AI 逻辑。
 * 提供对 GameObject 其他关键组件的访问。
 */
class AIComponent final : public engine::component::Component {
    friend class engine::object::GameObject;
private:
    std::unique_ptr<ai::AIBehavior> current_behavior_ = nullptr; ///< @brief 当前 AI 行为策略
    /* 未来可添加一些敌人属性 */

    // --- 缓存组件指针 ---
    engine::component::TransformComponent* transform_component_ = nullptr;
    engine::component::PhysicsComponent* physics_component_ = nullptr;
    engine::component::SpriteComponent* sprite_component_ = nullptr;
    engine::component::AnimationComponent* animation_component_ = nullptr;

public:
    AIComponent() = default;
    ~AIComponent() override = default;

    // 禁止拷贝和移动
    AIComponent(const AIComponent&) = delete;
    AIComponent& operator=(const AIComponent&) = delete;
    AIComponent(AIComponent&&) = delete;
    AIComponent& operator=(AIComponent&&) = delete;

    void setBehavior(std::unique_ptr<ai::AIBehavior> behavior); ///< @brief 设置当前 AI 行为策略
    bool takeDamage(int damage);        ///< @brief 处理伤害逻辑，返回是否造成伤害
    bool isAlive() const;               ///< @brief 检查对象是否存活

    // --- Setters and Getters ---
    engine::component::TransformComponent* getTransformComponent() const { return transform_component_; }
    engine::component::PhysicsComponent* getPhysicsComponent() const { return physics_component_; }
    engine::component::SpriteComponent* getSpriteComponent() const { return sprite_component_; }
    engine::component::AnimationComponent* getAnimationComponent() const { return animation_component_; }

private:
    // 核心循环方法
    void init() override;
    void update(float delta_time, engine::core::Context&) override;
};

} // namespace game::component
