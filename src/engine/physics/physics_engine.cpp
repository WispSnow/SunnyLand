#include "physics_engine.h"
#include "collision.h"
#include "../component/physics_component.h"
#include "../component/transform_component.h"
#include "../component/collider_component.h"
#include "../component/tilelayer_component.h"
#include "../object/game_object.h"
#include <set>
#include <spdlog/spdlog.h>
#include <glm/common.hpp>

namespace engine::physics {

void PhysicsEngine::registerComponent(engine::component::PhysicsComponent* component) {
    components_.push_back(component);
    spdlog::trace("物理组件注册完成。");
}

void PhysicsEngine::unregisterComponent(engine::component::PhysicsComponent* component) {
    // 使用 remove-erase 方法安全地移除指针
    auto it = std::remove(components_.begin(), components_.end(), component);
    components_.erase(it, components_.end());
    spdlog::trace("物理组件注销完成。");
}

void PhysicsEngine::registerCollisionLayer(engine::component::TileLayerComponent *layer)
{
    layer->setPhysicsEngine(this); // 设置物理引擎指针
    collision_tile_layers_.push_back(layer);
    spdlog::trace("碰撞瓦片图层注册完成。");
}

void PhysicsEngine::unregisterCollisionLayer(engine::component::TileLayerComponent* layer) {
    auto it = std::remove(collision_tile_layers_.begin(), collision_tile_layers_.end(), layer);
    collision_tile_layers_.erase(it, collision_tile_layers_.end());
    spdlog::trace("碰撞瓦片图层注销完成。");
}

void PhysicsEngine::update(float delta_time) {
    // 每帧开始时先清空碰撞对列表和瓦片触发事件列表
    collision_pairs_.clear();
    tile_trigger_events_.clear();

    // 遍历所有注册的物理组件
    for (auto* pc : components_) {
        if (!pc || !pc->isEnabled()) { // 检查组件是否有效和启用
            continue;
        }

        pc->resetCollisionFlags();  // 重置碰撞标志

        // 应用重力 (如果组件受重力影响)：F = g * m
        if (pc->isUseGravity()) {
            pc->addForce(gravity_ * pc->getMass());
        }
        /* 还可以添加其它力影响，比如风力、摩擦力等，目前不考虑 */
        
        // 更新速度： v += a * dt，其中 a = F / m
        pc->velocity_ += (pc->getForce() / pc->getMass()) * delta_time;
        pc->clearForce(); // 清除当前帧的力

        // 处理瓦片层碰撞（速度和位置的更新移入此函数）
        resolveTileCollisions(pc, delta_time);

        // 应用世界边界
        applyWorldBounds(pc);
    }
    // 处理对象间碰撞
    checkObjectCollisions();

    // 检测瓦片触发事件 (检测前已经处理完位移)
    checkTileTriggers();
}

void PhysicsEngine::checkObjectCollisions()
{
    // 两层循环遍历所有包含物理组件的 GameObject
    for (size_t i = 0; i < components_.size(); ++i) {
        auto* pc_a = components_[i];
        if (!pc_a || !pc_a->isEnabled()) continue;
        auto* obj_a = pc_a->getOwner();
        if (!obj_a) continue;
        auto* cc_a = obj_a->getComponent<engine::component::ColliderComponent>();
        if (!cc_a || !cc_a->isActive()) continue;

        for (size_t j = i + 1; j < components_.size(); ++j) {
            auto* pc_b = components_[j];
            if (!pc_b || !pc_b->isEnabled()) continue;
            auto* obj_b = pc_b->getOwner();
            if (!obj_b) continue;
            auto* cc_b = obj_b->getComponent<engine::component::ColliderComponent>();
            if (!cc_b || !cc_b->isActive()) continue;
            /* --- 通过保护性测试后，正式执行逻辑 --- */

            if (collision::checkCollision(*cc_a, *cc_b)) {
                // 如果是可移动物体与SOLID物体碰撞，则直接处理位置变化，不用记录碰撞对
                if (obj_a->getTag() != "solid" && obj_b->getTag() == "solid") {
                    resolveSolidObjectCollisions(obj_a, obj_b);
                }
                else if (obj_a->getTag() == "solid" && obj_b->getTag() != "solid") {
                    resolveSolidObjectCollisions(obj_b, obj_a);
                }
                else {
                    // 记录碰撞对
                    collision_pairs_.emplace_back(obj_a, obj_b);
                }
            }
        }
    }
}

void PhysicsEngine::resolveTileCollisions(engine::component::PhysicsComponent* pc, float delta_time) {
    // 检查组件是否有效
    auto* obj = pc->getOwner();
    if (!obj) return;
    auto* tc = obj->getComponent<engine::component::TransformComponent>();
    auto* cc = obj->getComponent<engine::component::ColliderComponent>();
    if (!tc || !cc || cc->isTrigger()) return;
    auto world_aabb = cc->getWorldAABB();   // 使用最小包围盒进行碰撞检测（简化）
    auto obj_pos = world_aabb.position;
    auto obj_size = world_aabb.size;
    if (world_aabb.size.x <= 0.0f || world_aabb.size.y <= 0.0f) return;
    // -- 检查结束, 正式开始处理 --
    
    constexpr float tolerance = 1.0f;       // 检查右边缘和下边缘时，需要减1像素，否则会检查到下一行/列的瓦片
    auto ds = pc->velocity_ * delta_time;  // 计算物体在delta_time内的位移
    auto new_obj_pos = obj_pos + ds;        // 计算物体在delta_time后的新位置

    if (!cc->isActive()) {  // 如果碰撞器未激活，直接让物体正常移动，然后返回。
        tc->translate(ds);
        pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
        return;
    }

    // 遍历所有注册的碰撞瓦片层
    for (auto* layer : collision_tile_layers_) {
        if (!layer) continue;
        auto tile_size = layer->getTileSize();
        // 轴分离碰撞检测：先检查X方向是否有碰撞 (y方向使用初始值obj_pos.y)
        if (ds.x > 0.0f) {
            // 检查右侧碰撞，需要分别测试右上和右下角
            auto right_top_x = new_obj_pos.x + obj_size.x;
            auto tile_x = static_cast<int>(floor(right_top_x / tile_size.x));   // 获取x方向瓦片坐标
            // y方向坐标有两个，右上和右下
            auto tile_y = static_cast<int>(floor(obj_pos.y / tile_size.y));
            auto tile_type_top = layer->getTileTypeAt({tile_x, tile_y});        // 右上角瓦片类型
            auto tile_y_bottom = static_cast<int>(floor((obj_pos.y + obj_size.y - tolerance) / tile_size.y));
            auto tile_type_bottom = layer->getTileTypeAt({tile_x, tile_y_bottom});     // 右下角瓦片类型

            if (tile_type_top == engine::component::TileType::SOLID || tile_type_bottom == engine::component::TileType::SOLID) {
                // 撞墙了！速度归零，x方向移动到贴着墙的位置
                new_obj_pos.x = tile_x * layer->getTileSize().x - obj_size.x;
                pc->velocity_.x = 0.0f;
                pc->setCollidedRight(true);
            } else {
                // 检测右下角斜坡瓦片
                auto width_right = new_obj_pos.x + obj_size.x - tile_x * tile_size.x;
                auto height_right = getTileHeightAtWidth(width_right, tile_type_bottom, tile_size);
                if (height_right > 0.0f) {
                    // 如果有碰撞（角点的世界y坐标 > 斜坡地面的世界y坐标）, 就让物体贴着斜坡表面
                    if (new_obj_pos.y > (tile_y_bottom + 1) * layer->getTileSize().y - obj_size.y - height_right) {
                        new_obj_pos.y = (tile_y_bottom + 1) * layer->getTileSize().y - obj_size.y - height_right;
                        pc->setCollidedBelow(true);
                    }
                }
            }
        }
        else if (ds.x < 0.0f) {
            // 检查左侧碰撞，需要分别测试左上和左下角
            auto left_top_x = new_obj_pos.x;
            auto tile_x = static_cast<int>(floor(left_top_x / tile_size.x));    // 获取x方向瓦片坐标
            // y方向坐标有两个，左上和左下
            auto tile_y = static_cast<int>(floor(obj_pos.y / tile_size.y));
            auto tile_type_top = layer->getTileTypeAt({tile_x, tile_y});        // 左上角瓦片类型
            auto tile_y_bottom = static_cast<int>(floor((obj_pos.y + obj_size.y - tolerance) / tile_size.y));
            auto tile_type_bottom = layer->getTileTypeAt({tile_x, tile_y_bottom});     // 左下角瓦片类型

            if (tile_type_top == engine::component::TileType::SOLID || tile_type_bottom == engine::component::TileType::SOLID) {
                // 撞墙了！速度归零，x方向移动到贴着墙的位置
                new_obj_pos.x = (tile_x + 1) * layer->getTileSize().x;
                pc->velocity_.x = 0.0f;
                pc->setCollidedLeft(true);
            } else {
                // 检测左下角斜坡瓦片
                auto width_left = new_obj_pos.x - tile_x * tile_size.x;
                auto height_left = getTileHeightAtWidth(width_left, tile_type_bottom, tile_size);
                if (height_left > 0.0f) {
                    if (new_obj_pos.y > (tile_y_bottom + 1) * layer->getTileSize().y - obj_size.y - height_left) {
                        new_obj_pos.y = (tile_y_bottom + 1) * layer->getTileSize().y - obj_size.y - height_left;
                        pc->setCollidedBelow(true);
                    }
                }
            }
        }
        // 轴分离碰撞检测：再检查Y方向是否有碰撞 (x方向使用初始值obj_pos.x)
        if (ds.y > 0.0f) {
            // 检查底部碰撞，需要分别测试左下和右下角
            auto bottom_left_y = new_obj_pos.y + obj_size.y;
            auto tile_y = static_cast<int>(floor(bottom_left_y / tile_size.y));

            auto tile_x = static_cast<int>(floor(obj_pos.x / tile_size.x));
            auto tile_type_left = layer->getTileTypeAt({tile_x, tile_y});           // 左下角瓦片类型   
            auto tile_x_right = static_cast<int>(floor((obj_pos.x + obj_size.x - tolerance) / tile_size.x));
            auto tile_type_right = layer->getTileTypeAt({tile_x_right, tile_y});     // 右下角瓦片类型

            if (tile_type_left == engine::component::TileType::SOLID || tile_type_right == engine::component::TileType::SOLID ||
                tile_type_left == engine::component::TileType::UNISOLID || tile_type_right == engine::component::TileType::UNISOLID) {
                // 到达地面！速度归零，y方向移动到贴着地面的位置
                new_obj_pos.y = tile_y * layer->getTileSize().y - obj_size.y;
                pc->velocity_.y = 0.0f;
                pc->setCollidedBelow(true);
            // 如果两个角点都位于梯子上，则判断是不是处在梯子顶层
            } else if (tile_type_left == engine::component::TileType::LADDER && tile_type_right == engine::component::TileType::LADDER) {
                auto tile_type_up_l = layer->getTileTypeAt({tile_x, tile_y - 1});       // 检测左角点上方瓦片类型
                auto tile_type_up_r = layer->getTileTypeAt({tile_x_right, tile_y - 1}); // 检测右角点上方瓦片类型
                // 如果上方不是梯子，证明处在梯子顶层
                if (tile_type_up_r != engine::component::TileType::LADDER && tile_type_up_l != engine::component::TileType::LADDER) {
                    // 通过是否使用重力来区分是否处于攀爬状态。
                    if (pc->isUseGravity()) {   // 非攀爬状态
                        pc->setOnTopLadder(true);       // 设置在梯子顶层标志
                        pc->setCollidedBelow(true);     // 设置下方碰撞标志
                        // 让物体贴着梯子顶层位置(与SOLID情况相同)
                        new_obj_pos.y = tile_y * layer->getTileSize().y - obj_size.y;
                        pc->velocity_.y = 0.0f;
                    } else {}    // 攀爬状态，不做任何处理
                }
            } else {
                // 检测斜坡瓦片（下方两个角点都要检测）
                auto width_left = obj_pos.x - tile_x * tile_size.x;
                auto width_right = obj_pos.x + obj_size.x - tile_x_right * tile_size.x;
                auto height_left = getTileHeightAtWidth(width_left, tile_type_left, tile_size);
                auto height_right = getTileHeightAtWidth(width_right, tile_type_right, tile_size);
                auto height = glm::max(height_left, height_right);  // 找到两个角点的最高点进行检测
                if (height > 0.0f) {    // 说明至少有一个角点处于斜坡瓦片
                    if (new_obj_pos.y > (tile_y + 1) * layer->getTileSize().y - obj_size.y - height) {
                        new_obj_pos.y = (tile_y + 1) * layer->getTileSize().y - obj_size.y - height;
                        pc->velocity_.y = 0.0f;     // 只有向下运动时才需要让 y 速度归零
                        pc->setCollidedBelow(true);
                    }
                }
            }
        }
        else if (ds.y < 0.0f) {
            // 检查顶部碰撞，需要分别测试左上和右上角
            auto top_left_y = new_obj_pos.y;
            auto tile_y = static_cast<int>(floor(top_left_y / tile_size.y));

            auto tile_x = static_cast<int>(floor(obj_pos.x / tile_size.x));
            auto tile_type_left = layer->getTileTypeAt({tile_x, tile_y});        // 左上角瓦片类型
            auto tile_x_right = static_cast<int>(floor((obj_pos.x + obj_size.x - tolerance) / tile_size.x));
            auto tile_type_right = layer->getTileTypeAt({tile_x_right, tile_y});     // 右上角瓦片类型

            if (tile_type_left == engine::component::TileType::SOLID || tile_type_right == engine::component::TileType::SOLID) {
                // 撞到天花板！速度归零，y方向移动到贴着天花板的位置
                new_obj_pos.y = (tile_y + 1) * layer->getTileSize().y;
                pc->velocity_.y = 0.0f;
                pc->setCollidedAbove(true);
            }
        }
    }
    // 更新物体位置，并限制最大速度
    tc->translate(new_obj_pos - obj_pos);   // 使用translate方法，避免直接设置位置，因为碰撞盒可能有偏移量
    pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
}

void PhysicsEngine::resolveSolidObjectCollisions(engine::object::GameObject* move_obj, engine::object::GameObject* solid_obj)
{
    // 进入此函数前，已经检查了各个组件的有效性，因此直接进行计算
    auto* move_tc = move_obj->getComponent<engine::component::TransformComponent>();
    auto* move_pc = move_obj->getComponent<engine::component::PhysicsComponent>();
    auto* move_cc = move_obj->getComponent<engine::component::ColliderComponent>();
    auto* solid_cc = solid_obj->getComponent<engine::component::ColliderComponent>();

    // 这里只能获取期望位置，无法获取当前帧初始位置，因此无法进行轴分离碰撞检测
    /* 未来可以进行重构，让这里可以获取初始位置。但是我们展示另外一种处理方法 */
    auto move_aabb = move_cc->getWorldAABB();
    auto solid_aabb = solid_cc->getWorldAABB();

    // --- 使用最小平移向量解决碰撞问题 ---
    auto move_center = move_aabb.position + move_aabb.size / 2.0f;
    auto solid_center = solid_aabb.position + solid_aabb.size / 2.0f;
    // 计算两个包围盒的重叠部分
    auto overlap = glm::vec2(move_aabb.size / 2.0f + solid_aabb.size / 2.0f) - glm::abs(move_center - solid_center);
    if (overlap.x < 0.1f && overlap.y < 0.1f) return;  // 如果重叠部分太小，则认为没有碰撞

    if (overlap.x < overlap.y) {    // 如果重叠部分在x方向上更小，则认为碰撞发生在x方向上（推出x方向平移向量最小）
        if (move_center.x < solid_center.x) {
            // 移动物体在左边，让它贴着右边SOLID物体（相当于向左移出重叠部分），y方向正常移动
            move_tc->translate(glm::vec2(-overlap.x, 0.0f));
            // 如果速度为正(向右移动)，则归零 （if判断不可少，否则可能出现错误吸附）
            if (move_pc->velocity_.x > 0.0f) {
                move_pc->velocity_.x = 0.0f;
                move_pc->setCollidedRight(true);
            }
        }
        else {
            // 移动物体在右边，让它贴着左边SOLID物体（相当于向右移出重叠部分），y方向正常移动
            move_tc->translate(glm::vec2(overlap.x, 0.0f));
            if (move_pc->velocity_.x < 0.0f) {
                move_pc->velocity_.x = 0.0f;
                move_pc->setCollidedLeft(true);
            }
        }
    } else {                        // 重叠部分在y方向上更小，则认为碰撞发生在y方向上（推出y方向平移向量最小）
        if (move_center.y < solid_center.y) {
            // 移动物体在上面，让它贴着下面SOLID物体（相当于向上移出重叠部分），x方向正常移动
            move_tc->translate(glm::vec2(0.0f, -overlap.y));
            if (move_pc->velocity_.y > 0.0f) {
                move_pc->velocity_.y = 0.0f;
                move_pc->setCollidedBelow(true);
            }
        }
        else {
            // 移动物体在下面，让它贴着上面SOLID物体（相当于向下移出重叠部分），x方向正常移动
            move_tc->translate(glm::vec2(0.0f, overlap.y));
            if (move_pc->velocity_.y < 0.0f) {
                move_pc->velocity_.y = 0.0f;
                move_pc->setCollidedAbove(true);
            }
        }
    }
}

float PhysicsEngine::getTileHeightAtWidth(float width, engine::component::TileType type, glm::vec2 tile_size)
{
    auto rel_x = glm::clamp(width / tile_size.x, 0.0f, 1.0f);
    switch (type) {
        case engine::component::TileType::SLOPE_0_1:        // 左0  右1
            return rel_x * tile_size.y;
        case engine::component::TileType::SLOPE_0_2:        // 左0  右1/2
            return rel_x * tile_size.y * 0.5f;
        case engine::component::TileType::SLOPE_2_1:        // 左1/2右1
            return rel_x * tile_size.y * 0.5f + tile_size.y * 0.5f;
        case engine::component::TileType::SLOPE_1_0:        // 左1  右0
            return (1.0f - rel_x) * tile_size.y;
        case engine::component::TileType::SLOPE_2_0:        // 左1/2右0
            return (1.0f - rel_x) * tile_size.y * 0.5f;
        case engine::component::TileType::SLOPE_1_2:        // 左1  右1/2
            return (1.0f - rel_x) * tile_size.y * 0.5f + tile_size.y * 0.5f;
        default:
            return 0.0f;   // 默认返回0，表示没有斜坡
    }
}

void PhysicsEngine::checkTileTriggers()
{
    for (auto* pc : components_) {
        if (!pc || !pc->isEnabled()) continue;  // 检查组件是否有效和启用
        auto* obj = pc->getOwner();
        if (!obj) continue;
        auto* cc = obj->getComponent<engine::component::ColliderComponent>();
        if (!cc || !cc->isActive() || cc->isTrigger()) continue;    // 如果游戏对象本就是触发器，则不需要检查瓦片触发事件

        // 获取物体的世界AABB
        auto world_aabb = cc->getWorldAABB();

        // 使用 set 来跟踪循环遍历中已经触发过的瓦片类型，防止重复添加（例如，玩家同时踩到两个尖刺，只需要受到一次伤害）
        std::set<engine::component::TileType> triggers_set;

        // 遍历所有注册的碰撞瓦片层分别进行检测
        for (auto* layer : collision_tile_layers_) {
            if (!layer) continue;
            auto tile_size = layer->getTileSize();
            constexpr float tolerance = 1.0f;   // 检查右边缘和下边缘时，需要减1像素，否则会检查到下一行/列的瓦片
            // 获取瓦片坐标范围
            auto start_x = static_cast<int>(floor(world_aabb.position.x / tile_size.x));
            auto end_x = static_cast<int>(ceil((world_aabb.position.x + world_aabb.size.x - tolerance) / tile_size.x));
            auto start_y = static_cast<int>(floor(world_aabb.position.y / tile_size.y));
            auto end_y = static_cast<int>(ceil((world_aabb.position.y + world_aabb.size.y - tolerance) / tile_size.y));

            // 遍历瓦片坐标范围进行检测
            for (int x = start_x; x < end_x; ++x) {
                for (int y = start_y; y < end_y; ++y) {
                    auto tile_type = layer->getTileTypeAt({x, y});
                    // 未来可以添加更多触发器类型的瓦片，目前只有 HAZARD 类型
                    if (tile_type == engine::component::TileType::HAZARD) {
                        triggers_set.insert(tile_type);     // 记录触发事件，set 保证每个瓦片类型只记录一次
                    }
                    // 梯子类型不必记录到事件容器，物理引擎自己处理
                    else if (tile_type == engine::component::TileType::LADDER) { 
                        pc->setCollidedLadder(true);
                    }
                }
            }
            // 遍历触发事件集合，添加到 tile_trigger_events_ 中
            for (const auto& type : triggers_set) {
                tile_trigger_events_.emplace_back(obj, type);
                spdlog::trace("tile_trigger_events_中 添加了 GameObject {} 和瓦片触发类型: {}", 
                    obj->getName(), static_cast<int>(type));
            }
        }
    }
}

void PhysicsEngine::applyWorldBounds(engine::component::PhysicsComponent *pc)
{
    if (!pc || !world_bounds_) return;

    // 只限定左、上、右边界，不限定下边界，以碰撞盒作为判断依据
    auto* obj = pc->getOwner();
    auto* cc = obj->getComponent<engine::component::ColliderComponent>();
    auto* tc = obj->getComponent<engine::component::TransformComponent>();
    auto world_aabb = cc->getWorldAABB();
    auto obj_pos = world_aabb.position;
    auto obj_size = world_aabb.size;

    // 检查左边界
    if (obj_pos.x < world_bounds_->position.x) {
        pc->velocity_.x = 0.0f;
        obj_pos.x = world_bounds_->position.x;
        pc->setCollidedLeft(true);
    }
    // 检查上边界
    if (obj_pos.y < world_bounds_->position.y) {
        pc->velocity_.y = 0.0f;
        obj_pos.y = world_bounds_->position.y;
        pc->setCollidedAbove(true);
    }
    // 检查右边界
    if (obj_pos.x + obj_size.x > world_bounds_->position.x + world_bounds_->size.x) {
        pc->velocity_.x = 0.0f;
        obj_pos.x = world_bounds_->position.x + world_bounds_->size.x - obj_size.x;
        pc->setCollidedRight(true);
    }
    // 更新物体位置(使用translate方法，新位置 - 旧位置)
    tc->translate(obj_pos - world_aabb.position);
}

} // namespace engine::physics 