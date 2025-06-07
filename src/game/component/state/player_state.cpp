#include "player_state.h"
#include "../player_component.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/object/game_object.h"
#include <spdlog/spdlog.h>

namespace game::component::state {

void PlayerState::playAnimation(const std::string& animation_name) {
    if (!player_component_) {
        spdlog::error("PlayerState 没有关联的 PlayerComponent，无法播放动画 '{}'", animation_name);
        return;
    }
    
    auto animation_component = player_component_->getAnimationComponent();
    if (!animation_component) {
        spdlog::error("PlayerComponent '{}' 没有 AnimationComponent，无法播放动画 '{}'", 
                      player_component_->getOwner()->getName(), animation_name);
        return;
    }

    animation_component->playAnimation(animation_name);
}

} // namespace game::component::state