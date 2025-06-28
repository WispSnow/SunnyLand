#include "player_component.h"
#include "state/idle_state.h"
#include "state/hurt_state.h"
#include "state/dead_state.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/health_component.h"
#include "../../engine/component/audio_component.h"
#include "../../engine/object/game_object.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include <utility>
#include <spdlog/spdlog.h>
#include <glm/common.hpp>

namespace game::component {

void PlayerComponent::init() {
    if (!owner_) {
        spdlog::error("PlayerComponent 没有所属游戏对象!");
        return;
    }

    // 获取必要的组件
    transform_component_ = owner_->getComponent<engine::component::TransformComponent>();
    physics_component_ = owner_->getComponent<engine::component::PhysicsComponent>();
    sprite_component_ = owner_->getComponent<engine::component::SpriteComponent>();
    animation_component_ = owner_->getComponent<engine::component::AnimationComponent>();
    health_component_ = owner_->getComponent<engine::component::HealthComponent>();
    audio_component_ = owner_->getComponent<engine::component::AudioComponent>();

    // 检查必要组件是否存在
    if (!transform_component_ || !physics_component_ || !sprite_component_ ||
        !animation_component_ || !health_component_ || !audio_component_) {
        spdlog::error("Player 对象缺少必要组件！");
    }

    // 初始化状态机
    current_state_ = std::make_unique<state::IdleState>(this);
    if (current_state_) {
        setState(std::move(current_state_));
    } else {
        spdlog::error("初始化玩家状态失败（make_unique 返回空指针）！");
    }
    spdlog::debug("PlayerComponent 初始化完成。");
}

bool PlayerComponent::takeDamage(int damage) {
    if (is_dead_ || !health_component_ || damage <= 0) {
        spdlog::warn("玩家已死亡或却少必要组件，并未造成伤害。");
        return false;
    }

    bool success = health_component_->takeDamage(damage);
    if (!success) return false;
    // --- 成功造成伤害了，根据是否存活决定状态切换
    if (health_component_->isAlive()) {
        spdlog::debug("玩家受到了 {} 点伤害，当前生命值: {}/{}。",
                     damage, health_component_->getCurrentHealth(), health_component_->getMaxHealth());
        // 切换到受伤状态
        setState(std::make_unique<state::HurtState>(this));
    } else {
        spdlog::debug("玩家死亡。");
        is_dead_ = true;
        // 切换到死亡状态
        setState(std::make_unique<state::DeadState>(this));
    }
    return true;
}

void PlayerComponent::moveLeft() {
    auto next_state = current_state_->moveLeft();
    if (next_state) setState(std::move(next_state));
}

void PlayerComponent::moveRight() {
    auto next_state = current_state_->moveRight();
    if (next_state) setState(std::move(next_state));
}

void PlayerComponent::jump() {
    auto next_state = current_state_->jump();
    if (next_state) setState(std::move(next_state));
}

void PlayerComponent::climbUp() {
    auto next_state = current_state_->climbUp();
    if (next_state) setState(std::move(next_state));
}

void PlayerComponent::climbDown() {
    auto next_state = current_state_->climbDown();
    if (next_state) setState(std::move(next_state));
}

void PlayerComponent::setState(std::unique_ptr<state::PlayerState> new_state) {
    if (!new_state) {
        spdlog::warn("尝试设置空的玩家状态！");
        return;
    }
    if (current_state_) {
        current_state_->exit();
    }

    current_state_ = std::move(new_state);
    // spdlog::debug("玩家组件正在切换到状态: {}", typeid(*current_state_).name());
    current_state_->enter();

}

bool PlayerComponent::is_on_ground() const
{
    return coyote_timer_ <= coyote_time_ || physics_component_->hasCollidedBelow();
}

void PlayerComponent::handleInput(engine::core::Context& context) {
    if (!current_state_) return;

    // --- 现在在这里负责判断输入并调用对应的方法，不再将handleInput委托给状态类处理 ---
    auto& input_manager = context.getInputManager();
    // 判断左右移动操作
    if (input_manager.isActionDown("move_left")) moveLeft();
    else if (input_manager.isActionDown("move_right")) moveRight();
    // 判断跳跃或上下移动操作（可以和左右操作同时进行）
    if (input_manager.isActionPressed("jump")) jump();
    else if (input_manager.isActionDown("move_up")) climbUp();
    else if (input_manager.isActionDown("move_down")) climbDown();

}

void PlayerComponent::update(float delta_time, engine::core::Context& context) {
    if (!current_state_) return;

    // 一旦离地，开始计时 Coyote Timer
    if (!physics_component_->hasCollidedBelow()) {
        coyote_timer_ += delta_time;
    } else {    // 如果碰撞到地面，重置 Coyote Timer
        coyote_timer_ = 0.0f; 
    }

    // 如果处于无敌状态，则进行闪烁
    if (health_component_->isInvincible()) {
        flash_timer_ += delta_time;         // 闪烁计时器增加
        if (flash_timer_ >= 2 * flash_interval_) {
            flash_timer_ -= 2 * flash_interval_;    // 闪烁计时器在 0～2倍闪烁间隔 中循环
        }
        // 一半时间可见，一半时间不可见。
        if (flash_timer_ < flash_interval_) {
            sprite_component_->setHidden(true);
        } else {
            sprite_component_->setHidden(false);
        }
    }
    // 非无敌状态时确保精灵可见
    else if (sprite_component_->isHidden()) {
        sprite_component_->setHidden(false);
    }

    auto next_state = current_state_->update(delta_time, context);
    if (next_state) {
        setState(std::move(next_state));
    }
}

} // namespace game::component 