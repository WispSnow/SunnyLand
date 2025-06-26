#include "level_loader.h"
#include "../component/parallax_component.h"
#include "../component/transform_component.h"
#include "../component/tilelayer_component.h"
#include "../component/sprite_component.h"
#include "../component/collider_component.h"
#include "../component/physics_component.h"
#include "../component/animation_component.h"
#include "../component/health_component.h"
#include "../component/audio_component.h"
#include "../object/game_object.h"
#include "../object/object_builder.h"
#include "../scene/scene.h"
#include "../core/context.h"
#include "../resource/resource_manager.h"
#include "../render/sprite.h"
#include "../render/animation.h"
#include "../utils/math.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <glm/vec2.hpp>
#include <filesystem>

namespace engine::scene {

bool LevelLoader::loadLevel(std::string_view level_path, Scene& scene) {
    // 1. 加载 JSON 文件
    auto path = std::filesystem::path(level_path);
    std::ifstream file(path);
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
            auto tileset_path = resolvePath(tileset_json["source"].get<std::string>(), map_path_);  // 支持隐式转换，可以省略.get<T>()方法，
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
    std::string image_path = layer_json.value("image", "");     // json.value()返回的是一个临时对象，需要赋值才能保存，
                                                                // 不能用std::string_view
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
    std::string layer_name = layer_json.value("name", "Unnamed");
    
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
    std::string layer_name = layer_json.value("name", "Unnamed");
    // 创建游戏对象
    auto game_object = std::make_unique<engine::object::GameObject>(layer_name);
    // 添加Tilelayer组件
    game_object->addComponent<engine::component::TileLayerComponent>(tile_size_, map_size_, std::move(tiles));
    // 添加到场景中
    scene.addGameObject(std::move(game_object));
    spdlog::info("加载瓦片图层: '{}' 完成", layer_name);
}

void LevelLoader::loadObjectLayer(const nlohmann::json& layer_json, Scene& scene)
{
    if (!layer_json.contains("objects") || !layer_json["objects"].is_array()) {
        spdlog::error("对象图层 '{}' 缺少 'objects' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }
    // 创建对象生成器
    engine::object::ObjectBuilder object_builder(*this, scene.getContext());
    // 获取对象数据
    const auto& objects = layer_json["objects"];
    // 遍历对象数据
    for (const auto& object : objects) {
        // 获取对象gid
        auto gid = object.value("gid", 0);
        if (gid == 0) {     // 如果gid为0 (即不存在)，则代表自己绘制的形状
            // 配置生成器，并调用build，针对自定义形状
            object_builder.configure(&object)->build();

        } else {        // 如果gid存在，则按照图片解析流程
            // 配置生成器，针对图片对象
            auto tile_json = getTileJsonByGid(gid);
            auto tile_info = getTileInfoByGid(gid);
            if (!tile_json || !tile_json->is_object()) {
                spdlog::warn("对象图层 '{}' 中的对象缺少有效的 'gid' 或瓦片信息。", layer_json.value("name", "Unnamed"));
                continue;
            }
            // 配置生成器，并调用build，针对图片对象
            object_builder.configure(&object, &tile_json.value(), tile_info)->build();
        }
        // 获取 GameObject
        auto game_object = object_builder.getGameObject();
        // 添加到场景中
        if (game_object) {
            spdlog::info("加载对象: '{}' 完成", game_object->getName());
            scene.addGameObject(std::move(game_object));
        }
    }
}

void LevelLoader::addAnimation(const nlohmann::json& anim_json, engine::component::AnimationComponent *ac, const glm::vec2& sprite_size)
{
    // 检查 anim_json 必须是一个对象，并且 ac 不能为 nullptr
    if (!anim_json.is_object() || !ac) {
        spdlog::error("无效的动画 JSON 或 AnimationComponent 指针。");
        return;
    }
    // 遍历动画 JSON 对象中的每个键值对（动画名称 : 动画信息）
    for (const auto& anim : anim_json.items()) {
        std::string_view anim_name = anim.key();
        const auto& anim_info = anim.value();
        if (!anim_info.is_object()) {
            spdlog::warn("动画 '{}' 的信息无效或为空。", anim_name);
            continue;
        }
        // 获取可能存在的动画帧信息
        auto duration_ms = anim_info.value("duration", 100);        // 默认持续时间为100毫秒
        auto duration = static_cast<float>(duration_ms) / 1000.0f;  // 转换为秒
        auto row = anim_info.value("row", 0);                       // 默认行数为0
        // 帧信息（数组）是必须存在的
        if (!anim_info.contains("frames") || !anim_info["frames"].is_array()) {
            spdlog::warn("动画 '{}' 缺少 'frames' 数组。", anim_name);
            continue;
        }
        // 创建一个Animation对象 (默认为循环播放)
        auto animation = std::make_unique<engine::render::Animation>(anim_name);

        // 遍历数组并进行添加帧信息到animation对象
        for (const auto& frame : anim_info["frames"]) {
            if (!frame.is_number_integer()) {
                spdlog::warn("动画 {} 中 frames 数组格式错误！", anim_name);
                continue;;
            }
            auto column = frame.get<int>();
            // 计算源矩形
            SDL_FRect src_rect = { 
                column * sprite_size.x, 
                row * sprite_size.y, 
                sprite_size.x, 
                sprite_size.y 
            };
            // 添加动画帧到 Animation
            animation->addFrame(src_rect, duration);
        }
        // 将 Animation 对象添加到 AnimationComponent 中
        ac->addAnimation(std::move(animation));
    }
}

void LevelLoader::addSound(const nlohmann::json &sound_json, engine::component::AudioComponent *audio_component)
{
    if (!sound_json.is_object() || !audio_component) {
        spdlog::error("无效的音效 JSON 或 AudioComponent 指针。");
        return;
    }
    // 遍历音效 JSON 对象中的每个键值对（音效id : 音效路径）
    for (const auto& sound : sound_json.items()) {
        const std::string& sound_id = sound.key();
        const std::string& sound_path = sound.value();
        if (sound_id.empty() || sound_path.empty() ) {
            spdlog::warn("音效 '{}' 缺少必要信息。", sound_id);
            continue;
        }
        // 添加音效到 AudioComponent
        audio_component->addSound(sound_id, sound_path);
    }
}

std::optional<engine::utils::Rect> LevelLoader::getColliderRect(const nlohmann::json &tile_json)
{
    if (!tile_json.contains("objectgroup")) return std::nullopt;
    auto& objectgroup = tile_json["objectgroup"];
    if (!objectgroup.contains("objects")) return std::nullopt;
    auto& objects = objectgroup["objects"];
    for (const auto& object : objects) {    // 一个图片只支持一个碰撞器。如果有多个，则返回第一个不为空的
        auto rect = engine::utils::Rect(glm::vec2(object.value("x", 0.0f), object.value("y", 0.0f)), 
                                        glm::vec2(object.value("width", 0.0f), object.value("height", 0.0f)));
        if (rect.size.x > 0 && rect.size.y > 0) {
            return rect;
        }
    }
    return std::nullopt;    // 如果没找到碰撞器，则返回空
}

engine::component::TileType LevelLoader::getTileType(const nlohmann::json &tile_json)
{
    if (tile_json.contains("properties")) {
        auto& properties = tile_json["properties"];
        for (auto& property : properties) {
            if (property.contains("name") && property["name"] == "solid") {
                auto is_solid = property.value("value", false);
                return is_solid ? engine::component::TileType::SOLID : engine::component::TileType::NORMAL;
            }
            else if (property.contains("name") && property["name"] == "slope") {
                auto slope_type = property.value("value", "");
                if (slope_type == "0_1") {
                    return engine::component::TileType::SLOPE_0_1;
                } else if (slope_type == "1_0") {
                    return engine::component::TileType::SLOPE_1_0;
                } else if (slope_type == "0_2") {
                    return engine::component::TileType::SLOPE_0_2;
                } else if (slope_type == "2_0") {
                    return engine::component::TileType::SLOPE_2_0;
                } else if (slope_type == "2_1") {
                    return engine::component::TileType::SLOPE_2_1;
                } else if (slope_type == "1_2") {
                    return engine::component::TileType::SLOPE_1_2;
                } else {
                    spdlog::error("未知的斜坡类型: {}", slope_type);
                    return engine::component::TileType::NORMAL;
                }
            }
            else if (property.contains("name") && property["name"] == "unisolid") {
                auto is_unisolid = property.value("value", false);
                return is_unisolid ? engine::component::TileType::UNISOLID : engine::component::TileType::NORMAL;
            }
            else if (property.contains("name") && property["name"] == "hazard") {
                auto is_hazard = property.value("value", false);
                return is_hazard ? engine::component::TileType::HAZARD : engine::component::TileType::NORMAL;
            }
            else if (property.contains("name") && property["name"] == "ladder") {
                auto is_ladder = property.value("value", false);
                return is_ladder ? engine::component::TileType::LADDER : engine::component::TileType::NORMAL;
            }
            // TODO: 可以在这里添加更多的自定义属性处理逻辑
        }
    }
    return engine::component::TileType::NORMAL;
}

engine::component::TileType LevelLoader::getTileTypeById(const nlohmann::json &tileset_json, int local_id)
{
    if (tileset_json.contains("tiles")) {
        auto& tiles = tileset_json["tiles"];
        for (auto& tile : tiles) {
            if (tile.contains("id") && tile["id"] == local_id) {
                return getTileType(tile);
            }
        }
    }
    return engine::component::TileType::NORMAL;
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
    std::string file_path = tileset.value("file_path", "");       // 获取图块集文件路径
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
        auto tile_type = getTileTypeById(tileset, local_id);   // 获取瓦片类型（只有瓦片id，还没找具体瓦片json）
        return engine::component::TileInfo(sprite, tile_type);
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
                auto tile_type = getTileType(tile_json);    // 获取瓦片类型（已经有具体瓦片json了）
                return engine::component::TileInfo(sprite, tile_type);
            }
        }
    }
    // 如果能走到这里，说明查找失败，返回空的瓦片信息
    spdlog::error("图块集 '{}' 中未找到gid为 {} 的瓦片。", tileset_it->first, gid);
    return engine::component::TileInfo();
}

std::optional<nlohmann::json> LevelLoader::getTileJsonByGid(int gid) const
{
    // 1. 查找tileset_data_中键小于等于gid的最近元素
    auto tileset_it = tileset_data_.upper_bound(gid);
    if (tileset_it == tileset_data_.begin()) {
        spdlog::error("gid为 {} 的瓦片未找到图块集。", gid);
        return std::nullopt;
    }
    --tileset_it;
    // 2. 获取图块集json对象
    const auto& tileset = tileset_it->second;
    auto local_id = gid - tileset_it->first;        // 计算瓦片在图块集中的局部ID
    if (!tileset.contains("tiles")) {   // 没有tiles字段的话不符合数据格式要求，直接返回空
        spdlog::error("Tileset 文件 '{}' 缺少 'tiles' 属性。", tileset_it->first);
        return std::nullopt;
    }
    // 3. 遍历tiles数组，根据id查找对应的瓦片并返回瓦片json
    const auto& tiles_json = tileset["tiles"];
    for (const auto& tile_json : tiles_json) {
        auto tile_id = tile_json.value("id", 0);
        if (tile_id == local_id) {   // 找到对应的瓦片，返回瓦片json
            return tile_json;
        }
    }
    return std::nullopt;
}

void LevelLoader::loadTileset(std::string_view tileset_path, int first_gid)
{
    auto path = std::filesystem::path(tileset_path);
    std::ifstream tileset_file(path);
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

std::string LevelLoader::resolvePath(std::string_view relative_path, std::string_view file_path)
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
        return std::string(relative_path);
    }
}

} // namespace engine::scene