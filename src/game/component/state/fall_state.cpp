#include "fall_state.h"
#include "idle_state.h"
#include "walk_state.h"
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include <glm/common.hpp>

namespace game::component::state {

void FallState::enter() {
    playAnimation("fall");  // 播放下落动画
}

void FallState::exit() {

}

std::unique_ptr<PlayerState> FallState::handleInput(engine::core::Context& context)
{
    auto input_manager = context.getInputManager();
    auto physics_component = player_component_->getPhysicsComponent();
    auto sprite_component = player_component_->getSpriteComponent();

    // 下落状态下可以左右移动
    if (input_manager.isActionDown("move_left")) {
        if (physics_component->velocity_.x > 0.0f) physics_component->velocity_.x = 0.0f;
        physics_component->addForce({-player_component_->getMoveForce(), 0.0f});
        sprite_component->setFlipped(true);
    } else if (input_manager.isActionDown("move_right")) {
        if (physics_component->velocity_.x < 0.0f) physics_component->velocity_.x = 0.0f;
        physics_component->addForce({player_component_->getMoveForce(), 0.0f});
        sprite_component->setFlipped(false);
    }
    return nullptr;
}

std::unique_ptr<PlayerState> FallState::update(float, engine::core::Context&)
{
    // 限制最大速度(水平方向)
    auto physics_component = player_component_->getPhysicsComponent();
    auto max_speed = player_component_->getMaxSpeed();
    physics_component->velocity_.x = glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

    // 如果下方有碰撞，则根据水平速度来决定 切换到 IdleState 或 WalkState
    if (physics_component->hasCollidedBelow()) {
        if (glm::abs(physics_component->velocity_.x) < 1.0f) {
            return std::make_unique<IdleState>(player_component_);
        } else {
            return std::make_unique<WalkState>(player_component_);
        }
    }
    return nullptr;
}

} // namespace game::component::state