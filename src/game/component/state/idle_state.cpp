#include "idle_state.h"
#include "fall_state.h"
#include "jump_state.h"
#include "walk_state.h"
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../../../engine/component/physics_component.h"
#include <spdlog/spdlog.h>

namespace game::component::state {

void IdleState::enter() {
    playAnimation("idle");  // 播放待机动画
}

void IdleState::exit() {

}

std::unique_ptr<PlayerState> IdleState::handleInput(engine::core::Context& context)
{
    auto input_manager = context.getInputManager();
    // 如果按下了左右移动键，则切换到 WalkState
    if (input_manager.isActionDown("move_left") || input_manager.isActionDown("move_right")) {
        return std::make_unique<WalkState>(player_component_);
    }

    // 如果按下“jump”则切换到 JumpState
    if (input_manager.isActionPressed("jump")) {
        return std::make_unique<JumpState>(player_component_);
    }
    return nullptr;
}

std::unique_ptr<PlayerState> IdleState::update(float, engine::core::Context&)
{
    // 应用摩擦力(水平方向)
    auto physics_component = player_component_->getPhysicsComponent();
    auto friction_factor = player_component_->getFrictionFactor();
    physics_component->velocity_.x *= friction_factor;

    // 如果下方没有碰撞，则切换到 FallState
    if (!physics_component->hasCollidedBelow()) {
        return std::make_unique<FallState>(player_component_);
    }
    return nullptr;
}

} // namespace game::component::state