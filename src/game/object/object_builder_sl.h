#pragma once
#include "../../engine/object/object_builder.h"

namespace game::object {

/**
 * @brief 针对具体游戏的对象生成器，继承自 engine::object::ObjectBuilder。
 *        添加游戏中的玩家、敌人、物品等对象的相关组件。
 */
class ObjectBuilderSL final : public engine::object::ObjectBuilder {
public:
    ObjectBuilderSL(engine::scene::LevelLoader& level_loader, engine::core::Context& context);
    ~ObjectBuilderSL() = default;

    void build() override;      ///< @brief 构建游戏对象，重载父类方法以添加具体游戏逻辑
private:
    // --- 针对具体游戏新添加的步骤 ---
    void buildPlayer();
    void buildEnemy();
    void buildItem();
};

}   // namespace game::object
