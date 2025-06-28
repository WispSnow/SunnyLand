#include "climb_state.h"
#include "jump_state.h"
#include "idle_state.h"
#include "fall_state.h"
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/animation_component.h"
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace game::component::state {

void ClimbState::enter() {
    spdlog::debug("进入攀爬状态");
    playAnimation("climb");
    if (auto* physics = player_component_->getPhysicsComponent(); physics) {
        physics->setUseGravity(false); // 禁用重力
    }
}

void ClimbState::exit() {
    spdlog::debug("退出攀爬状态");

    if (auto* physics = player_component_->getPhysicsComponent(); physics) {
        physics->setUseGravity(true); // 重新启用重力
    }
}

std::unique_ptr<PlayerState> ClimbState::update(float, engine::core::Context&) {
    auto physics_component = player_component_->getPhysicsComponent();
    auto animation_component = player_component_->getAnimationComponent();

    // 根据是否正在移动，决定是否播放动画
    is_moving_ ? animation_component->resumeAnimation() : animation_component->stopAnimation();

    // 如果着地，则切换到 IdleState
    if (physics_component->hasCollidedBelow()) {
        return std::make_unique<IdleState>(player_component_);
    }
    // 如果离开梯子区域，则切换到 FallState（能走到这里 说明非着地状态）
    if (!physics_component->hasCollidedLadder()) {
        return std::make_unique<FallState>(player_component_);
    }

    is_moving_ = false;         // 循环的最后重置移动标志
    physics_component->velocity_ = glm::vec2(0.0f, 0.0f);  // 速度归零(没有操控就静止不动)
    return nullptr;
}

std::unique_ptr<PlayerState> ClimbState::moveLeft() {
    auto physics_component = player_component_->getPhysicsComponent();

    // 水平向左移动
    physics_component->velocity_.x = -player_component_->getClimbSpeed();
    is_moving_ = true;
    return nullptr;
}

std::unique_ptr<PlayerState> ClimbState::moveRight() {
    auto physics_component = player_component_->getPhysicsComponent();
    // 水平向右移动
    physics_component->velocity_.x = player_component_->getClimbSpeed();
    is_moving_ = true;
    return nullptr;
}

std::unique_ptr<PlayerState> ClimbState::jump() {
    // 直接切换到 JumpState
    return std::make_unique<JumpState>(player_component_);
}

std::unique_ptr<PlayerState> ClimbState::climbUp() {
    auto physics_component = player_component_->getPhysicsComponent();

    // 向上移动
    physics_component->velocity_.y = -player_component_->getClimbSpeed();
    is_moving_ = true;
    return nullptr;
}

std::unique_ptr<PlayerState> ClimbState::climbDown() {
    auto physics_component = player_component_->getPhysicsComponent();

    // 向下移动
    physics_component->velocity_.y = player_component_->getClimbSpeed();
    is_moving_ = true;
    return nullptr;
}

} // namespace game::component::state