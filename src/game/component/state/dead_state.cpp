#include "dead_state.h"
#include "../player_component.h"
#include "../../../engine/object/game_object.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/collider_component.h"
#include "../../../engine/component/audio_component.h"

namespace game::component::state {

void DeadState::enter() {
    spdlog::debug("玩家进入死亡状态。");
    playAnimation("hurt");  // 播放死亡(受伤)动画

    // 应用击退力（只向上）
    auto physics_component = player_component_->getPhysicsComponent();
    physics_component->velocity_ = glm::vec2(0.0f, -200.0f);  // 向上击退

    // 禁用碰撞(自动掉出屏幕)
    auto collider_component = player_component_->getOwner()->getComponent<engine::component::ColliderComponent>();
    if (collider_component) {
        collider_component->setActive(false);
    }

    if (auto* audio_component = player_component_->getAudioComponent(); audio_component) {
        audio_component->playSound("dead");  // 播放死亡音效
    }
}

void DeadState::exit() {

}

std::unique_ptr<PlayerState> DeadState::handleInput(engine::core::Context&){
    // 死亡状态下不处理输入
    return nullptr;
}

std::unique_ptr<PlayerState> DeadState::update(float, engine::core::Context&){
    // 死亡状态下不更新状态
    return nullptr;
}

}