#include "physics_engine.h"
#include "collision.h"
#include "../component/physics_component.h"
#include "../component/transform_component.h"
#include "../component/collider_component.h"
#include "../component/tilelayer_component.h"
#include "../object/game_object.h"
#include <algorithm>
#include <spdlog/spdlog.h>
#include "glm/common.hpp"

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
    // 每帧开始时先清空碰撞对列表
    collision_pairs_.clear();

    // 遍历所有注册的物理组件
    for (auto* pc : components_) {
        if (!pc || !pc->isEnabled()) { // 检查组件是否有效和启用
            continue;
        }

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
    }
    // 处理对象间碰撞
    checkObjectCollisions();
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
                // TODO: 并不是所有碰撞都需要插入collision_pairs_，未来会添加过滤条件
                // 记录碰撞对
                collision_pairs_.emplace_back(obj_a, obj_b);
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
    if (!tc || !cc || !cc->isActive() || cc->isTrigger()) return;
    auto world_aabb = cc->getWorldAABB();   // 使用最小包围盒进行碰撞检测（简化）
    auto obj_pos = world_aabb.position;
    auto obj_size = world_aabb.size;
    if (world_aabb.size.x <= 0.0f || world_aabb.size.y <= 0.0f) return;
    // -- 检查结束, 正式开始处理 --
    
    auto tolerance = 1.0f;          // 检查右边缘和下边缘时，需要减1像素，否则会检查到下一行/列的瓦片
    auto ds = pc->velocity_ * delta_time;  // 计算物体在delta_time内的位移
    auto new_obj_pos = obj_pos + ds;        // 计算物体在delta_time后的新位置

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

            if (tile_type_left == engine::component::TileType::SOLID || tile_type_right == engine::component::TileType::SOLID) {
                // 到达地面！速度归零，y方向移动到贴着地面的位置
                new_obj_pos.y = tile_y * layer->getTileSize().y - obj_size.y;
                pc->velocity_.y = 0.0f;
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
            }
        }
    }
    // 更新物体位置，并限制最大速度
    tc->setPosition(new_obj_pos);
    pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
}

} // namespace engine::physics 