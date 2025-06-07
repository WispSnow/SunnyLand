#include "player_component.h"
#include "state/idle_state.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/object/game_object.h"
#include "../../engine/input/input_manager.h"
#include <utility>
#include <typeinfo>
#include <spdlog/spdlog.h>

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

    // 检查必要组件是否存在
    if (!transform_component_ || !physics_component_ || !sprite_component_ || !animation_component_) {
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

void PlayerComponent::setState(std::unique_ptr<state::PlayerState> new_state) {
    if (!new_state) {
        spdlog::warn("尝试设置空的玩家状态！");
        return;
    }
    if (current_state_) {
        current_state_->exit();
    }

    current_state_ = std::move(new_state);
    spdlog::debug("玩家组件正在切换到状态: {}", typeid(*current_state_).name());
    current_state_->enter();

}

void PlayerComponent::handleInput(engine::core::Context& context) {
    if (!current_state_) return;

    auto next_state = current_state_->handleInput(context);
    if (next_state) {
        setState(std::move(next_state));
    }
}

void PlayerComponent::update(float delta_time, engine::core::Context& context) {
    if (!current_state_) return;

    auto next_state = current_state_->update(delta_time, context);
    if (next_state) {
        setState(std::move(next_state));
    }
}

} // namespace game::component 