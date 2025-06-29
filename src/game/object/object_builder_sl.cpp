#include "object_builder_sl.h"
#include "../../engine/object/game_object.h"
#include "../../engine/core/context.h"
#include "../../engine/render/camera.h"
#include "../../engine/render/animation.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/animation_component.h"
#include "../component/ai_component.h"
#include "../component/ai/patrol_behavior.h"
#include "../component/ai/updown_behavior.h"
#include "../component/ai/jump_behavior.h"
#include "../component/player_component.h"
#include <spdlog/spdlog.h>

namespace game::object {

ObjectBuilderSL::ObjectBuilderSL(engine::scene::LevelLoader& level_loader, engine::core::Context& context)
    : engine::object::ObjectBuilder(level_loader, context) {
    spdlog::trace("ObjectBuilderSL 构造完成。");
}

void ObjectBuilderSL::build() {
    ObjectBuilder::build();
    buildPlayer();
    buildEnemy();
    buildItem();
    spdlog::trace("ObjectBuilderSL 构建完成。");
}

void ObjectBuilderSL::buildPlayer() {
    spdlog::trace("ObjectBuilderSL 构建玩家。");
    if (!tile_json_ || game_object_->getTag() != "player") return;
    if (auto* player_component = game_object_->addComponent<game::component::PlayerComponent>(); player_component) {
        if (auto* transform = game_object_->getComponent<engine::component::TransformComponent>(); transform) {
            context_.getCamera().setTarget(transform);  // 相机跟随玩家
        }
    }
}

void ObjectBuilderSL::buildEnemy() {
    spdlog::trace("ObjectBuilderSL 构建敌人。");
    if (!tile_json_ || game_object_->getTag() != "enemy") return;
    if (name_ == "eagle") {
        if (auto* ai_component = game_object_->addComponent<game::component::AIComponent>(); ai_component) {
            auto y_max = game_object_->getComponent<engine::component::TransformComponent>()->getPosition().y;
            auto y_min = y_max - 80.0f;    // 让鹰的飞行范围 (当前位置与上方80像素 的区域)
            ai_component->setBehavior(std::make_unique<game::component::ai::UpDownBehavior>(y_min, y_max));
        }
    } else if (name_ == "frog") {
        if (auto* ai_component = game_object_->addComponent<game::component::AIComponent>(); ai_component) {
            auto x_max = game_object_->getComponent<engine::component::TransformComponent>()->getPosition().x - 10.0f;
            auto x_min = x_max - 90.0f;    // 青蛙跳跃范围（右侧 - 10.0f 是为了增加稳定性）
            ai_component->setBehavior(std::make_unique<game::component::ai::JumpBehavior>(x_min, x_max));
        }
    } else if (name_ == "opossum") {
        if (auto* ai_component = game_object_->addComponent<game::component::AIComponent>(); ai_component) {
            auto x_max = game_object_->getComponent<engine::component::TransformComponent>()->getPosition().x;
            auto x_min = x_max - 200.0f;    // 负鼠巡逻范围
            ai_component->setBehavior(std::make_unique<game::component::ai::PatrolBehavior>(x_min, x_max));
        }
    }
}

void ObjectBuilderSL::buildItem() {
    spdlog::trace("ObjectBuilderSL 构建物品。");
    if (!tile_json_ || game_object_->getTag() != "item") return;
    if (auto* ac = game_object_->addComponent<engine::component::AnimationComponent>(); ac) {
        ac->playAnimation("idle");
    }
}

}   // namespace game::object