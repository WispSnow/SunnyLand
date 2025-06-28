#include "walk_state.h"
#include "jump_state.h"
#include "fall_state.h"
#include "idle_state.h"
#include "climb_state.h"
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/object/game_object.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include <glm/common.hpp>


namespace game::component::state {

void WalkState::enter() {
    playAnimation("walk");  // 播放步行动画
    is_moving_ = true;
}

void WalkState::exit() {

}

std::unique_ptr<PlayerState> WalkState::update(float, engine::core::Context&)
{
    // 限制最大速度
    auto physics_component = player_component_->getPhysicsComponent();
    auto max_speed = player_component_->getMaxSpeed();
    physics_component->velocity_.x = glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

    // 如果没有移动，则切换到 IdleState
    if (!is_moving_) {
        return std::make_unique<IdleState>(player_component_);
    }

    // 如果离地，则切换到 FallState
    if (!player_component_->is_on_ground()) {
        return std::make_unique<FallState>(player_component_);
    }

    is_moving_ = false;         // 循环的最后重置移动标志
    return nullptr;
}

std::unique_ptr<PlayerState> WalkState::moveLeft() {
    auto physics_component = player_component_->getPhysicsComponent();
    auto sprite_component = player_component_->getSpriteComponent();

    if (physics_component->velocity_.x > 0.0f) {
        physics_component->velocity_.x = 0.0f;  // 如果当前速度是向右的，则先减速到0 (增强操控手感)
    }
    // 添加向左的水平力
    physics_component->addForce({-player_component_->getMoveForce(), 0.0f});
    sprite_component->setFlipped(true);         // 向左移动时翻转

    is_moving_ = true;
    return nullptr;
}

std::unique_ptr<PlayerState> WalkState::moveRight() {
    auto physics_component = player_component_->getPhysicsComponent();
    auto sprite_component = player_component_->getSpriteComponent();

    if (physics_component->velocity_.x < 0.0f) {
        physics_component->velocity_.x = 0.0f;  // 如果当前速度是向左的，则先减速到0
    }
    // 添加向右的水平力
    physics_component->addForce({player_component_->getMoveForce(), 0.0f});
    sprite_component->setFlipped(false);        // 向右移动时不翻转 

    is_moving_ = true;
    return nullptr;
}

std::unique_ptr<PlayerState> WalkState::jump() {
    // 直接切换到 JumpState
    return std::make_unique<JumpState>(player_component_);
}

std::unique_ptr<PlayerState> WalkState::climbUp() {
    auto physics_component = player_component_->getPhysicsComponent();

    // 如果与梯子重合，则切换到 ClimbState
    if (physics_component->hasCollidedLadder()) {
        return std::make_unique<ClimbState>(player_component_);
    }

    return nullptr;
}

} // namespace game::component::state