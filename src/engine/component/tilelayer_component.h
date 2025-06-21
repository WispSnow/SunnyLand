#pragma once
#include "../render/sprite.h"
#include "component.h"
#include <vector>
#include <glm/vec2.hpp>

namespace engine::render {
class Sprite;
}

namespace engine::core {
class Context;
}

namespace engine::physics {
class PhysicsEngine;
}

namespace engine::component {
/**
 * @brief 定义瓦片的类型，用于游戏逻辑（例如碰撞）。
 */
enum class TileType {
    EMPTY,      ///< @brief 空白瓦片
    NORMAL,     ///< @brief 普通瓦片
    SOLID,      ///< @brief 静止可碰撞瓦片
    UNISOLID,   ///< @brief 单向静止可碰撞瓦片
    SLOPE_0_1,  ///< @brief 斜坡瓦片，高度:左0  右1
    SLOPE_1_0,  ///< @brief 斜坡瓦片，高度:左1  右0
    SLOPE_0_2,  ///< @brief 斜坡瓦片，高度:左0  右1/2
    SLOPE_2_1,  ///< @brief 斜坡瓦片，高度:左1/2右1
    SLOPE_1_2,  ///< @brief 斜坡瓦片，高度:左1  右1/2
    SLOPE_2_0,  ///< @brief 斜坡瓦片，高度:左1/2右0
    HAZARD,     ///< @brief 危险瓦片（例如火焰、尖刺等）
    LADDER,     ///< @brief 梯子瓦片
    // 未来补充其它类型
};

/**
 * @brief 包含单个瓦片的渲染和逻辑信息。
 */
struct TileInfo {
    render::Sprite sprite;      ///< @brief 瓦片的视觉表示
    TileType type;              ///< @brief 瓦片的逻辑类型
    TileInfo(render::Sprite s = render::Sprite(), TileType t = TileType::EMPTY) : sprite(std::move(s)), type(t) {}
};

/**
 * @brief 管理和渲染瓦片地图层。
 *
 * 存储瓦片地图的布局、每个瓦片的精灵信息和类型。
 * 负责在渲染阶段绘制可见的瓦片。
 */
class TileLayerComponent final : public Component {
    friend class engine::object::GameObject;
private:
    glm::ivec2 tile_size_;              ///< @brief 单个瓦片尺寸（像素）
    glm::ivec2 map_size_;               ///< @brief 地图尺寸（瓦片数）
    std::vector<TileInfo> tiles_;       ///< @brief 存储所有瓦片信息 (按"行主序"存储, index = y * map_width_ + x)
    glm::vec2 offset_ = {0.0f, 0.0f};   ///< @brief 瓦片层在世界中的偏移量 (瓦片层通常不需要缩放及旋转，因此不引入Transform组件)
                                                 // offset_ 最好也保持默认的0，以免增加不必要的复杂性
    bool is_hidden_ = false;            ///< @brief 是否隐藏（不渲染）
    engine::physics::PhysicsEngine* physics_engine_ = nullptr;   ///< @brief 物理引擎的指针， clean()函数中可能需要反注册

public:
    TileLayerComponent() = default;

    /**
     * @brief 构造函数
     * @param tile_size 单个瓦片尺寸（像素）
     * @param map_size 地图尺寸（瓦片数）
     * @param tiles 初始化瓦片数据的容器 (会被移动)
     */
    TileLayerComponent(glm::ivec2 tile_size, glm::ivec2 map_size, std::vector<TileInfo>&& tiles);

    /**
     * @brief 根据瓦片坐标获取瓦片信息
     * @param pos 瓦片坐标 (0 <= x < map_size_.x, 0 <= y < map_size_.y)
     * @return const TileInfo* 指向瓦片信息的指针，如果坐标无效则返回 nullptr
     */
    const TileInfo* getTileInfoAt(glm::ivec2 pos) const;

    /**
     * @brief 根据瓦片坐标获取瓦片类型
     * @param pos 瓦片坐标 (0 <= x < map_size_.x, 0 <= y < map_size_.y)
     * @return TileType 瓦片类型，如果坐标无效则返回 TileType::EMPTY
     */
    TileType getTileTypeAt(glm::ivec2 pos) const;

     /**
      * @brief 根据世界坐标获取瓦片类型
      * @param world_pos 世界坐标
      * @return TileType 瓦片类型，如果坐标无效或对应空瓦片则返回 TileType::EMPTY
      */
    TileType getTileTypeAtWorldPos(const glm::vec2& world_pos) const;

    // getters and setters
    glm::ivec2 getTileSize() const { return tile_size_; }               ///< @brief 获取单个瓦片尺寸
    glm::ivec2 getMapSize() const { return map_size_; }                 ///< @brief 获取地图尺寸
    glm::vec2 getWorldSize() const {                                    ///< @brief 获取地图世界尺寸
        return glm::vec2(map_size_.x * tile_size_.x, map_size_.y * tile_size_.y); 
    }
    const std::vector<TileInfo>& getTiles() const { return tiles_; }    ///< @brief 获取瓦片容器
    const glm::vec2& getOffset() const { return offset_; }              ///< @brief 获取瓦片层的偏移量
    bool isHidden() const { return is_hidden_; }                        ///< @brief 获取是否隐藏（不渲染）

    void setOffset(glm::vec2 offset) { offset_ = std::move(offset); }       ///< @brief 设置瓦片层的偏移量
    void setHidden(bool hidden) { is_hidden_ = hidden; }                ///< @brief 设置是否隐藏（不渲染）
    void setPhysicsEngine(engine::physics::PhysicsEngine* physics_engine) {physics_engine_ = physics_engine; }

protected:
    // 核心循环方法
    void init() override;
    void update(float, engine::core::Context&) override {}
    void render(engine::core::Context& context) override;
    void clean() override;
};

} // namespace engine::component
