#pragma once
#include <string>
#include <nlohmann/json_fwd.hpp>

namespace engine::scene {
class Scene;

class LevelLoader final {
    std::string map_path_;      ///< @brief 地图路径（拼接路径时需要）
public:
    LevelLoader() = default;

    /**
     * @brief 加载关卡数据到指定的 Scene 对象中。
     * @param map_path Tiled JSON 地图文件的路径。
     * @param scene 要加载数据的目标 Scene 对象。
     * @return bool 是否加载成功。
     */
    bool loadLevel(const std::string& map_path, Scene& scene);

private:
    void loadImageLayer(const nlohmann::json& layer_json, Scene& scene);    ///< @brief 加载图片图层
    void loadTileLayer(const nlohmann::json& layer_json, Scene& scene);     ///< @brief 加载瓦片图层
    void loadObjectLayer(const nlohmann::json& layer_json, Scene& scene);   ///< @brief 加载对象图层

    /**
     * @brief 解析图片路径，合并地图路径和相对路径。例如：
     * 1. 地图路径："assets/maps/level1.tmj"
     * 2. 相对路径："../textures/Layers/back.png"
     * 3. 最终路径："assets/textures/Layers/back.png"
     * @param image_path （图片）相对路径
     * @return std::string 解析后的完整路径。
     */
    std::string resolvePath(std::string image_path);
};

} // namespace engine::scene