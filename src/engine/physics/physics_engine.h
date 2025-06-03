#pragma once
#include <vector>
#include <utility>  // for std::pair
#include "glm/vec2.hpp"

namespace engine::component {
    class PhysicsComponent;
    class TileLayerComponent;
}

namespace engine::object {
    class GameObject;
}

namespace engine::physics {

/**
 * @brief 负责管理和模拟物理行为及碰撞检测。
 */
class PhysicsEngine {
private:
    std::vector<engine::component::PhysicsComponent*> components_; ///< @brief 注册的物理组件容器，非拥有指针
    std::vector<engine::component::TileLayerComponent*> collision_tile_layers_; ///< @brief 注册的碰撞瓦片图层容器
    glm::vec2 gravity_ = {0.0f, 980.0f};        ///< @brief 默认重力值 (像素/秒^2, 相当于100像素对应现实1m)
    float max_speed_ = 500.0f;                  ///< @brief 最大速度 (像素/秒)

    /// @brief 存储本帧发生的 GameObject 碰撞对 （每次 update 开始时清空）
    std::vector<std::pair<engine::object::GameObject*, engine::object::GameObject*>> collision_pairs_;

public:
    PhysicsEngine() = default;

    // 禁止拷贝和移动
    PhysicsEngine(const PhysicsEngine&) = delete;
    PhysicsEngine& operator=(const PhysicsEngine&) = delete;
    PhysicsEngine(PhysicsEngine&&) = delete;
    PhysicsEngine& operator=(PhysicsEngine&&) = delete;

    void registerComponent(engine::component::PhysicsComponent* component);     ///< @brief 注册物理组件
    void unregisterComponent(engine::component::PhysicsComponent* component);   ///< @brief 注销物理组件

    // 如果瓦片层需要进行碰撞检测则注册。（不需要则不必注册）
    void registerCollisionLayer(engine::component::TileLayerComponent* layer);  ///< @brief 注册用于碰撞检测的 TileLayerComponent
    void unregisterCollisionLayer(engine::component::TileLayerComponent* layer);///< @brief 注销用于碰撞检测的 TileLayerComponent

    void update(float delta_time);      ///< @brief 核心循环：更新所有注册的物理组件的状态
    void checkObjectCollisions();       ///< @brief 检测并处理对象之间的碰撞，并记录需要游戏逻辑处理的碰撞对。
    /// @brief 检测并处理游戏对象和瓦片层之间的碰撞。
    void resolveTileCollisions(engine::component::PhysicsComponent* pc, float delta_time);

    // 设置器/获取器
    void setGravity(const glm::vec2& gravity) { gravity_ = gravity; }   ///< @brief 设置全局重力加速度
    const glm::vec2& getGravity() const { return gravity_; }            ///< @brief 获取当前的全局重力加速度
    void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }       ///< @brief 设置最大速度
    float getMaxSpeed() const { return max_speed_; }                    ///< @brief 获取当前的最大速度
    /// @brief 获取本帧检测到的所有 GameObject 碰撞对。(此列表在每次 update 开始时清空)
    const std::vector<std::pair<engine::object::GameObject*, engine::object::GameObject*>>& getCollisionPairs() const {
        return collision_pairs_;
    };
};

} // namespace engine::physics