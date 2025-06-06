#pragma once
#include "../../engine/component/component.h"
#include "state/player_state.h"
#include <memory>

namespace engine::input {
    class InputManager;
}
namespace engine::component {
    class TransformComponent;
    class PhysicsComponent;
    class SpriteComponent;
}

namespace game::component::state {
    class PlayerState;
}

namespace game::component {

/**
 * @brief 处理玩家输入、状态和控制 GameObject 移动的组件。
 *        使用状态模式管理 Idle, Walk, Jump, Fall 等状态。
 */
class PlayerComponent final : public engine::component::Component {
    friend class engine::object::GameObject;
private:
    engine::component::TransformComponent* transform_component_ = nullptr; // 指向 TransformComponent 的非拥有指针
    engine::component::SpriteComponent* sprite_component_ = nullptr;
    engine::component::PhysicsComponent* physics_component_ = nullptr;

    std::unique_ptr<state::PlayerState> current_state_;
    bool is_dead_ = false;

    // --- 移动相关参数
    float move_force_ = 200.0f;         ///< @brief 水平移动力
    float max_speed_ = 120.0f;          ///< @brief 最大移动速度 (像素/秒)
    float friction_factor_ = 0.85f;     ///< @brief 摩擦系数 (Idle时缓冲效果，每帧乘以此系数)
    float jump_force_ = 350.0f;         ///< @brief 跳跃力 (按下"jump"键给的瞬间向上的力)

public:
    PlayerComponent() = default;
    ~PlayerComponent() override = default;

    // 禁止拷贝和移动
    PlayerComponent(const PlayerComponent&) = delete;
    PlayerComponent& operator=(const PlayerComponent&) = delete;
    PlayerComponent(PlayerComponent&&) = delete;
    PlayerComponent& operator=(PlayerComponent&&) = delete;

    // setters and getters
    engine::component::TransformComponent* getTransformComponent() const { return transform_component_; }
    engine::component::SpriteComponent* getSpriteComponent() const { return sprite_component_; }
    engine::component::PhysicsComponent* getPhysicsComponent() const { return physics_component_; }

    void setIsDead(bool is_dead) { is_dead_ = is_dead; }                ///< @brief 设置玩家是否死亡
    bool isDead() const { return is_dead_; }                            ///< @brief 获取玩家是否死亡    
    void setMoveForce(float move_force) { move_force_ = move_force; }   ///< @brief 设置水平移动力
    float getMoveForce() const { return move_force_; }                  ///< @brief 获取水平移动力  
    void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }       ///< @brief 设置最大移动速度
    float getMaxSpeed() const { return max_speed_; }                    ///< @brief 获取最大移动速度
    void setFrictionFactor(float friction_factor) { friction_factor_ = friction_factor; }   ///< @brief 设置摩擦系数
    float getFrictionFactor() const { return friction_factor_; }        ///< @brief 获取摩擦系数
    void setJumpForce(float jump_force) { jump_force_ = jump_force; }   ///< @brief 设置跳跃力
    float getJumpForce() const { return jump_force_; }

    void setState(std::unique_ptr<state::PlayerState> new_state);       ///< @brief 切换玩家状态
    
private:
    // 核心循环函数
    void init() override;
    void handleInput(engine::core::Context& context) override;
    void update(float delta_time, engine::core::Context& context) override;
    
};

} // namespace game::component