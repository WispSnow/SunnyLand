#include "jump_state.h"
#include "fall_state.h"
#include "climb_state.h"
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/audio_component.h"
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace game::component::state {

void JumpState::enter() {
    playAnimation("jump");  // 播放跳跃动画
    auto physics_component = player_component_->getPhysicsComponent();
    physics_component->velocity_.y = -player_component_->getJumpVelocity();     // 向上跳跃
    
    if (auto* audio_component = player_component_->getAudioComponent(); audio_component) {
        audio_component->playSound("jump");  // 播放跳跃音效
    }
    spdlog::debug("PlayerComponent 进入 JumpState，设置初始垂直速度为: {}", physics_component->velocity_.y);
}

void JumpState::exit() {

}

std::unique_ptr<PlayerState> JumpState::update(float, engine::core::Context&)
{
    // 限制最大速度(水平方向)
    auto physics_component = player_component_->getPhysicsComponent();
    auto max_speed = player_component_->getMaxSpeed();
    physics_component->velocity_.x = glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

    // 如果速度为正，切换到 FallState
    if (physics_component->velocity_.y >= 0.0f) {
        return std::make_unique<FallState>(player_component_);
    }

    return nullptr;
}

std::unique_ptr<PlayerState> JumpState::moveLeft() {
    auto physics_component = player_component_->getPhysicsComponent();
    auto sprite_component = player_component_->getSpriteComponent();
    // 如果当前速度是向右的，则先减速到0 (增强操控手感)
    if (physics_component->velocity_.x > 0.0f) physics_component->velocity_.x = 0.0f;
    physics_component->addForce({-player_component_->getMoveForce(), 0.0f});
    sprite_component->setFlipped(true);

    return nullptr;
}

std::unique_ptr<PlayerState> JumpState::moveRight() {
    auto physics_component = player_component_->getPhysicsComponent();
    auto sprite_component = player_component_->getSpriteComponent();
    // 如果当前速度是向左的，则先减速到0 (增强操控手感)
    if (physics_component->velocity_.x < 0.0f) physics_component->velocity_.x = 0.0f;
    physics_component->addForce({player_component_->getMoveForce(), 0.0f});
    sprite_component->setFlipped(false);

    return nullptr;
}

std::unique_ptr<PlayerState> JumpState::climbUp() {
    auto physics_component = player_component_->getPhysicsComponent();
    // 如果与梯子重合，则切换到 ClimbState
    if (physics_component->hasCollidedLadder()){
        return std::make_unique<ClimbState>(player_component_);
    }

    return nullptr;
}

std::unique_ptr<PlayerState> JumpState::climbDown() {
    auto physics_component = player_component_->getPhysicsComponent();
    // 如果与梯子重合，则切换到 ClimbState
    if (physics_component->hasCollidedLadder()){
        return std::make_unique<ClimbState>(player_component_);
    }

    return nullptr;
}

} // namespace game::component::state