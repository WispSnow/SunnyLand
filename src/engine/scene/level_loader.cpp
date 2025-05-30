#include "level_loader.h"
#include "../component/parallax_component.h"
#include "../component/transform_component.h"
#include "../component/tilelayer_component.h"
#include "../object/game_object.h"
#include "../scene/scene.h"
#include "../core/context.h"
#include "../render/sprite.h"
#include "../utils/math.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <glm/vec2.hpp>
#include <filesystem>

namespace engine::scene {

bool LevelLoader::loadLevel(const std::string& level_path, Scene& scene) {
    // 1. 加载 JSON 文件
    std::ifstream file(level_path);
    if (!file.is_open()) {
        spdlog::error("无法打开关卡文件: {}", level_path);
        return false;
    }

    // 2. 解析 JSON 数据
    nlohmann::json json_data;
    try {
        file >> json_data;
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::error("解析 JSON 数据失败: {}", e.what());
        return false;
    }

    // 3. 获取基本地图信息 (名称、地图尺寸、瓦片尺寸)
    map_path_ = level_path;
    map_size_ = glm::ivec2(json_data.value("width", 0), json_data.value("height", 0));
    tile_size_ = glm::ivec2(json_data.value("tilewidth", 0), json_data.value("tileheight", 0));

    // 4. 加载 tileset 数据
    if (json_data.contains("tilesets") && json_data["tilesets"].is_array()) {
        for (const auto& tileset_json : json_data["tilesets"]) {
            if (!tileset_json.contains("source") || !tileset_json["source"].is_string() ||
                !tileset_json.contains("firstgid") || !tileset_json["firstgid"].is_number_integer()) {
                spdlog::error("tilesets 对象中缺少有效 'source' 或 'firstgid' 字段。");
                continue;
            }
            auto tileset_path = resolvePath(tileset_json["source"], map_path_);  // 支持隐式转换，可以省略.get<T>()方法，
            auto first_gid = tileset_json["firstgid"];
            loadTileset(tileset_path, first_gid);
        }
    }

    // 5. 加载图层数据
    if (!json_data.contains("layers") || !json_data["layers"].is_array()) {       // 地图文件中必须有 layers 数组
        spdlog::error("地图文件 '{}' 中缺少或无效的 'layers' 数组。", level_path);
        return false;
    }
    for (const auto& layer_json : json_data["layers"]) {
        // 获取各图层对象中的类型（type）字段
        std::string layer_type = layer_json.value("type", "none");
        if (!layer_json.value("visible", true)) {
            spdlog::info("图层 '{}' 不可见，跳过加载。", layer_json.value("name", "Unnamed"));
            continue;
        }

        // 根据图层类型决定加载方法
        if (layer_type == "imagelayer") {       
            loadImageLayer(layer_json, scene);
        } else if (layer_type == "tilelayer") {
            loadTileLayer(layer_json, scene);
        } else if (layer_type == "objectgroup") {
            loadObjectLayer(layer_json, scene);
        } else {
            spdlog::warn("不支持的图层类型: {}", layer_type);
        }
    }

    spdlog::info("关卡加载完成: {}", level_path);
    return true;
}

void LevelLoader::loadImageLayer(const nlohmann::json& layer_json, Scene& scene) {
    // 获取纹理相对路径 （会自动处理'\/'符号）
    const std::string& image_path = layer_json.value("image", "");
    if (image_path.empty()) {
        spdlog::error("图层 '{}' 缺少 'image' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }
    auto texture_id = resolvePath(image_path, map_path_);

    // 获取图层偏移量（json中没有则代表未设置，给默认值即可）
    const glm::vec2 offset = glm::vec2(layer_json.value("offsetx", 0.0f), layer_json.value("offsety", 0.0f));
    
    // 获取视差因子及重复标志
    const glm::vec2 scroll_factor = glm::vec2(layer_json.value("parallaxx", 1.0f), layer_json.value("parallaxy", 1.0f));
    const glm::bvec2 repeat = glm::bvec2(layer_json.value("repeatx", false), layer_json.value("repeaty", false));
    
    // 获取图层名称
    const std::string& layer_name = layer_json.value("name", "Unnamed");
    
    /*  可用类似方法获取其它各种属性，这里我们暂时用不上 */
    
    // 创建游戏对象
    auto game_object = std::make_unique<engine::object::GameObject>(layer_name);
    // 依次添加Transform，Parallax组件
    game_object->addComponent<engine::component::TransformComponent>(offset);
    game_object->addComponent<engine::component::ParallaxComponent>(texture_id, scroll_factor, repeat);
    // 添加到场景中
    scene.addGameObject(std::move(game_object));
    spdlog::info("加载图层: '{}' 完成", layer_name);
}

void LevelLoader::loadTileLayer(const nlohmann::json& layer_json, Scene& scene)
{
    if (!layer_json.contains("data") || !layer_json["data"].is_array()) {
        spdlog::error("图层 '{}' 缺少 'data' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }
    // 准备 TileInfo Vector (瓦片数量 = 地图宽度 * 地图高度)
    std::vector<engine::component::TileInfo> tiles;
    tiles.reserve(map_size_.x * map_size_.y);

    // 获取图层数据 (瓦片 ID 列表)
    const auto& data = layer_json["data"];

    // 根据gid获取必要信息，并依次填充 TileInfo Vector
    for (const auto& gid : data) {
        tiles.push_back(getTileInfoByGid(gid));
    }

    // 获取图层名称
    const std::string& layer_name = layer_json.value("name", "Unnamed");
    // 创建游戏对象
    auto game_object = std::make_unique<engine::object::GameObject>(layer_name);
    // 添加Tilelayer组件
    game_object->addComponent<engine::component::TileLayerComponent>(tile_size_, map_size_, std::move(tiles));
    // 添加到场景中
    scene.addGameObject(std::move(game_object));
}

void LevelLoader::loadObjectLayer(const nlohmann::json&, Scene&)
{
    // TODO
}

engine::component::TileInfo LevelLoader::getTileInfoByGid(int gid)
{
    if (gid == 0) {
        return engine::component::TileInfo();
    }

    // upper_bound：查找tileset_data_中键大于 gid 的第一个元素，返回迭代器
    auto tileset_it = tileset_data_.upper_bound(gid);
    if (tileset_it == tileset_data_.begin()) {
        spdlog::error("gid为 {} 的瓦片未找到图块集。", gid);
        return engine::component::TileInfo();
    } 
    --tileset_it;  // 前移一个位置，这样就得到不大于gid的最近一个元素（我们需要的）

    const auto& tileset = tileset_it->second;
    auto local_id = gid - tileset_it->first;        // 计算瓦片在图块集中的局部ID
    const std::string file_path = tileset.value("file_path", "");       // 获取图块集文件路径
    if (file_path.empty()) {
        spdlog::error("Tileset 文件 '{}' 缺少 'file_path' 属性。", tileset_it->first);
        return engine::component::TileInfo();
    }
    // 图块集分为两种情况，需要分别考虑
    if (tileset.contains("image")) {    // 这是单一图片的情况
        // 获取图片路径
        auto texture_id = resolvePath(tileset["image"].get<std::string>(), file_path);
        // 计算瓦片在图片网格中的坐标
        auto coordinate_x = local_id % tileset["columns"].get<int>();
        auto coordinate_y = local_id / tileset["columns"].get<int>();
        // 根据坐标确定源矩形
        SDL_FRect texture_rect = {
            static_cast<float>(coordinate_x * tile_size_.x),
            static_cast<float>(coordinate_y * tile_size_.y),
            static_cast<float>(tile_size_.x),
            static_cast<float>(tile_size_.y)
        };
        engine::render::Sprite sprite{texture_id, texture_rect};
        return engine::component::TileInfo(sprite, engine::component::TileType::NORMAL);    // 目前只完成渲染，以后再考虑瓦片类型
    } else {   // 这是多图片的情况
        if (!tileset.contains("tiles")) {   // 没有tiles字段的话不符合数据格式要求，直接返回空的瓦片信息
            spdlog::error("Tileset 文件 '{}' 缺少 'tiles' 属性。", tileset_it->first);
            return engine::component::TileInfo();
        }
        // 遍历tiles数组，根据id查找对应的瓦片
        const auto& tiles_json = tileset["tiles"];
        for (const auto& tile_json : tiles_json) {
            auto tile_id = tile_json.value("id", 0);
            if (tile_id == local_id) {   // 找到对应的瓦片，进行后续操作
                if (!tile_json.contains("image")) {   // 没有image字段的话不符合数据格式要求，直接返回空的瓦片信息
                    spdlog::error("Tileset 文件 '{}' 中瓦片 {} 缺少 'image' 属性。", tileset_it->first, tile_id);
                    return engine::component::TileInfo();
                }
                // --- 接下来根据必要信息创建并返回 TileInfo ---
                // 获取图片路径
                auto texture_id = resolvePath(tile_json["image"].get<std::string>(), file_path);
                // 先确认图片尺寸
                auto image_width = tile_json.value("imagewidth", 0);
                auto image_height = tile_json.value("imageheight", 0);
                // 从json中获取源矩形信息
                SDL_FRect texture_rect = {      // tiled中源矩形信息只有设置了才会有值，没有就是默认值
                    static_cast<float>(tile_json.value("x", 0)),
                    static_cast<float>(tile_json.value("y", 0)),
                    static_cast<float>(tile_json.value("width", image_width)),    // 如果未设置，则使用图片尺寸
                    static_cast<float>(tile_json.value("height", image_height))
                };
                engine::render::Sprite sprite{texture_id, texture_rect};
                return engine::component::TileInfo(sprite, engine::component::TileType::NORMAL);    // 目前只完成渲染，以后再考虑瓦片类型
            }
        }
    }
    // 如果能走到这里，说明查找失败，返回空的瓦片信息
    spdlog::error("图块集 '{}' 中未找到gid为 {} 的瓦片。", tileset_it->first, gid);
    return engine::component::TileInfo();
}

void LevelLoader::loadTileset(const std::string& tileset_path, int first_gid)
{
    std::ifstream tileset_file(tileset_path);
    if (!tileset_file.is_open()) {
        spdlog::error("无法打开 Tileset 文件: {}", tileset_path);
        return;
    }

    nlohmann::json ts_json;
    try {
        tileset_file >> ts_json;
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::error("解析 Tileset JSON 文件 '{}' 失败: {} (at byte {})", tileset_path, e.what(), e.byte);
        return;
    }
    ts_json["file_path"] = tileset_path;    // 将文件路径存储到json中，后续解析图片路径时需要
    tileset_data_[first_gid] = std::move(ts_json);
    spdlog::info("Tileset 文件 '{}' 加载完成，firstgid: {}", tileset_path, first_gid);
}

std::string LevelLoader::resolvePath(const std::string& relative_path, const std::string& file_path)
{
    try {   
    // 获取地图文件的父目录（相对于可执行文件） "assets/maps/level1.tmj" -> "assets/maps"
    auto map_dir = std::filesystem::path(file_path).parent_path();
    // 合并路径（相对于可执行文件）并返回。 /* std::filesystem::canonical：解析路径中的当前目录（.）和上级目录（..）导航符，
                                      /*  得到一个干净的路径 */
    auto final_path = std::filesystem::canonical(map_dir / relative_path);
    return final_path.string();
    } catch (const std::exception& e) {
        spdlog::error("解析路径失败: {}", e.what());
        return relative_path;
    }
}

} // namespace engine::scene