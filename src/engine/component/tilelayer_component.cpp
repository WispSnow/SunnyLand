#include "tilelayer_component.h"
#include "../object/game_object.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include "../render/camera.h"
#include <spdlog/spdlog.h>

namespace engine::component {

TileLayerComponent::TileLayerComponent(glm::ivec2 tile_size, glm::ivec2 map_size, std::vector<TileInfo>&& tiles)
    : tile_size_(tile_size),
      map_size_(map_size),
      tiles_(std::move(tiles))
{
    if (tiles_.size() != static_cast<size_t>(map_size_.x * map_size_.y)) {
        spdlog::error("TileLayerComponent: 地图尺寸与提供的瓦片向量大小不匹配。瓦片数据将被清除。");
        tiles_.clear();
        map_size_ = {0, 0};
    }
    spdlog::trace("TileLayerComponent 构造完成");
}

void TileLayerComponent::init() {
    if (!owner_) {
        spdlog::warn("TileLayerComponent 的 owner_ 未设置。");
    }
    spdlog::trace("TileLayerComponent 初始化完成");
}

void TileLayerComponent::render(engine::core::Context& context) {
    if (tile_size_.x <= 0 || tile_size_.y <= 0) {
        return; // 防止除以零或无效尺寸
    }
    // 遍历所有瓦片
    for (int y = 0; y < map_size_.y; ++y) {
        for (int x = 0; x < map_size_.x; ++x) {
            size_t index = static_cast<size_t>(y) * map_size_.x + x;
            // 检查索引有效性以及瓦片是否需要渲染
            if (index < tiles_.size() && tiles_[index].type != TileType::EMPTY) {
                const auto& tile_info = tiles_[index];
                // 计算该瓦片在世界中的左上角位置 (drawSprite 预期接收左上角坐标)
                glm::vec2 tile_left_top_pos = {
                    offset_.x + static_cast<float>(x) * tile_size_.x,
                    offset_.y + static_cast<float>(y) * tile_size_.y
                };
                // 但如果图片的大小与瓦片的大小不一致，需要调整 y 坐标 (瓦片层的对齐点是左下角)
                if(static_cast<int>(tile_info.sprite.getSourceRect()->h) != tile_size_.y) {
                    tile_left_top_pos.y -= (tile_info.sprite.getSourceRect()->h - static_cast<float>(tile_size_.y));
                }
                // 执行绘制
                context.getRenderer().drawSprite(context.getCamera(), tile_info.sprite, tile_left_top_pos);
            }
        }
    }
}

const TileInfo* TileLayerComponent::getTileInfoAt(glm::ivec2 pos) const {
    if (pos.x < 0 || pos.x >= map_size_.x || pos.y < 0 || pos.y >= map_size_.y) {
        spdlog::warn("TileLayerComponent: 瓦片坐标越界: ({}, {})", pos.x, pos.y);
        return nullptr;
    }
    size_t index = static_cast<size_t>(pos.y * map_size_.x + pos.x);
    // 瓦片索引不能越界
    if (index < tiles_.size()) {
        return &tiles_[index];
    }
    spdlog::warn("TileLayerComponent: 瓦片索引越界: {}", index);
    return nullptr;
}

TileType TileLayerComponent::getTileTypeAt(glm::ivec2 pos) const {
    const TileInfo* info = getTileInfoAt(pos);
    return info ? info->type : TileType::EMPTY;
}

TileType TileLayerComponent::getTileTypeAtWorldPos(const glm::vec2& world_pos) const {
    glm::vec2 relative_pos = world_pos - offset_;

    int tile_x = static_cast<int>(std::floor(relative_pos.x / tile_size_.x));
    int tile_y = static_cast<int>(std::floor(relative_pos.y / tile_size_.y));

    return getTileTypeAt(glm::ivec2{tile_x, tile_y});
}

} // namespace engine::component
