#include "transform_component.h"
#include "../object/game_object.h"
#include "sprite_component.h" 
#include "collider_component.h"

namespace engine::component { 

void TransformComponent::setScale(glm::vec2 scale)
{
    scale_ = std::move(scale);
    if (owner_) {
        auto sprite_comp = owner_->getComponent<SpriteComponent>();
        if (sprite_comp) {
            sprite_comp->updateOffset();
        }
        auto collider_comp = owner_->getComponent<ColliderComponent>();
        if (collider_comp) {
            collider_comp->updateOffset();
        }
    }
}

} // namespace engine::component 