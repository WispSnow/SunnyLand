#pragma once
#include <string>
#include <string_view>
#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>
#include <map>
#include <optional>
#include "../utils/math.h"

namespace engine::component {
class AnimationComponent;
class AudioComponent;
struct TileInfo;
enum class TileType;
}

namespace engine::scene {
class Scene;

/**
 * @brief 负责从 Tiled JSON 文件 (.tmj) 加载关卡数据到 Scene 中。
 */
class LevelLoader final {
    std::string map_path_;      ///< @brief 地图路径（拼接路径时需要）
    glm::ivec2 map_size_;       ///< @brief 地图尺寸(瓦片数量)
    glm::ivec2 tile_size_;      ///< @brief 瓦片尺寸(像素)
    std::map<int, nlohmann::json> tileset_data_;    ///< @brief firstgid -> 瓦片集数据

public:
    LevelLoader() = default;

    /**
     * @brief 加载关卡数据到指定的 Scene 对象中。
     * @param map_path Tiled JSON 地图文件的路径。
     * @param scene 要加载数据的目标 Scene 对象。
     * @return bool 是否加载成功。
     */
    [[nodiscard]] bool loadLevel(std::string_view map_path, Scene& scene);

private:
    void loadImageLayer(const nlohmann::json& layer_json, Scene& scene);    ///< @brief 加载图片图层
    void loadTileLayer(const nlohmann::json& layer_json, Scene& scene);     ///< @brief 加载瓦片图层
    void loadObjectLayer(const nlohmann::json& layer_json, Scene& scene);   ///< @brief 加载对象图层

    /**
     * @brief 添加动画到指定的 AnimationComponent。
     * @param anim_json 动画json数据（自定义）
     * @param ac AnimationComponent 指针（动画添加到此组件）
     * @param sprite_size 每一帧动画的尺寸
     */
    void addAnimation(const nlohmann::json& anim_json, engine::component::AnimationComponent* ac, const glm::vec2& sprite_size);

    /**
     * @brief 添加音效到指定的 AudioComponent。
     * @param sound_json 音效json数据（自定义）
     * @param audio_component AudioComponent 指针（音效添加到此组件）
     */
    void addSound(const nlohmann::json& sound_json, engine::component::AudioComponent* audio_component);

    /**
     * @brief 获取瓦片属性
     * @tparam T 属性类型
     * @param tile_json 瓦片json数据
     * @param property_name 属性名称
     * @return 属性值，如果属性不存在则返回 std::nullopt
     */
    template<typename T>
    std::optional<T> getTileProperty(const nlohmann::json& tile_json, std::string_view property_name) {
        if (!tile_json.contains("properties")) return std::nullopt;
        const auto& properties = tile_json["properties"];
        for (const auto& property : properties) {
            if (property.contains("name") && property["name"] == std::string(property_name)) {
                if (property.contains("value")) {
                    return property["value"].get<T>();
                }
            }
        }
        return std::nullopt;
    }

    /**
     * @brief 获取瓦片碰撞器矩形
     * @param tile_json 瓦片json数据
     * @return 碰撞器矩形，如果碰撞器不存在则返回 std::nullopt
     */
    std::optional<engine::utils::Rect> getColliderRect(const nlohmann::json& tile_json);

    /**
     * @brief 根据瓦片json对象获取瓦片类型
     * @param tile_json 瓦片json数据
     * @return 瓦片类型
     */
    engine::component::TileType getTileType(const nlohmann::json& tile_json);

    /**
     * @brief 根据（单一图片）图块集中的id获取瓦片类型
     * @param tileset_json 图块集json数据
     * @param local_id 图块集中的id
     * @return 瓦片类型
     */
    engine::component::TileType getTileTypeById(const nlohmann::json& tileset_json, int local_id);

    /**
     * @brief 根据全局 ID 获取瓦片信息。
     * @param gid 全局 ID。
     * @return engine::component::TileInfo 瓦片信息。
     */
    engine::component::TileInfo getTileInfoByGid(int gid);

    /**
     * @brief 根据全局 ID 获取瓦片json对象 (用于对象层获取瓦片信息)
     * @param gid 全局 ID
     * @return 瓦片json对象
     */
    std::optional<nlohmann::json> getTileJsonByGid(int gid) const;

    /**
     * @brief 加载 Tiled tileset 文件 (.tsj)。
     * @param tileset_path Tileset 文件路径。
     * @param first_gid 此 tileset 的第一个全局 ID。
     */
    void loadTileset(std::string_view tileset_path, int first_gid);

    /**
     * @brief 解析图片路径，合并地图路径和相对路径。例如：
     * 1. 文件路径："assets/maps/level1.tmj"
     * 2. 相对路径："../textures/Layers/back.png"
     * 3. 最终路径："assets/textures/Layers/back.png"
     * @param relative_path 相对路径（相对于文件）
     * @param file_path 文件路径
     * @return std::string 解析后的完整路径。
     */
    std::string resolvePath(std::string_view relative_path, std::string_view file_path);
};

} // namespace engine::scene