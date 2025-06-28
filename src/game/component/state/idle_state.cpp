#include "idle_state.h"
#include "fall_state.h"
#include "jump_state.h"
#include "walk_state.h"
#include "climb_state.h"
#include "../player_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/transform_component.h"
#include <spdlog/spdlog.h>

namespace game::component::state {

void IdleState::enter() {
    playAnimation("idle");  // 播放待机动画
}

void IdleState::exit() {

}

std::unique_ptr<PlayerState> IdleState::update(float, engine::core::Context&)
{
    // 应用摩擦力(水平方向)
    auto physics_component = player_component_->getPhysicsComponent();
    auto friction_factor = player_component_->getFrictionFactor();
    physics_component->velocity_.x *= friction_factor;

    // 如果离地，则切换到 FallState
    if (!player_component_->is_on_ground()) {
        return std::make_unique<FallState>(player_component_);
    }
    return nullptr;
}

std::unique_ptr<PlayerState> IdleState::moveLeft() {
    // 切换到 WalkState
    return std::make_unique<WalkState>(player_component_);
}

std::unique_ptr<PlayerState> IdleState::moveRight() {
    // 切换到 WalkState
    return std::make_unique<WalkState>(player_component_);
}

std::unique_ptr<PlayerState> IdleState::jump() {
    // 切换到 JumpState
    return std::make_unique<JumpState>(player_component_);
}

std::unique_ptr<PlayerState> IdleState::climbUp() {
    auto physics_component = player_component_->getPhysicsComponent();
    // 如果与梯子重合，则切换到 ClimbState
    if (physics_component->hasCollidedLadder()) {
        return std::make_unique<ClimbState>(player_component_);
    }

    return nullptr;
}

std::unique_ptr<PlayerState> IdleState::climbDown() {
    auto physics_component = player_component_->getPhysicsComponent();
    // 如果已经在梯子顶层，则切换到 ClimbState
    if (physics_component->isOnTopLadder()) {
        // 需要向下移动一点，确保下一帧能与梯子碰撞（否则会切换回FallState）
        player_component_->getTransformComponent()->translate(glm::vec2(0, 2.0f));
        return std::make_unique<ClimbState>(player_component_);
    }

    return nullptr;
}

} // namespace game::component::state