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
    // 获取对象数据
    const auto& objects = layer_json["objects"];
    // 遍历对象数据
    for (const auto& object : objects) {
        // 获取对象gid
        auto gid = object.value("gid", 0);
        if (gid == 0) {     // 如果gid为0 (即不存在)，则代表自己绘制的形状
            // 非矩形对象会有额外标识（目前不考虑）
            if (object.value("point", false)) {             // 如果是点对象
                continue;       // TODO: 点对象的处理方式
            } else if (object.value("ellipse", false)) {    // 如果是椭圆对象
                continue;       // TODO: 椭圆对象的处理方式
            } else if (object.value("polygon", false)) {    // 如果是多边形对象
                continue;       // TODO: 多边形对象的处理方式
            } 
            // 没有这些标识则默认是矩形对象
            else {  
                // --- 创建游戏对象并添加TransfromComponent ---
                std::string object_name = object.value("name", "Unnamed");
                auto game_object = std::make_unique<engine::object::GameObject>(object_name);
                    // 获取Transform相关信息 （自定义形状的坐标针对左上角）
                auto position = glm::vec2(object.value("x", 0.0f), object.value("y", 0.0f));
                auto dst_size = glm::vec2(object.value("width", 0.0f), object.value("height", 0.0f));
                auto rotation = object.value("rotation", 0.0f);
                    // 添加TransformComponent，缩放为设定为1.0f
                game_object->addComponent<engine::component::TransformComponent>(position, glm::vec2(1.0f), rotation);

                // --- 添加碰撞组件和物理组件 ---
                    // 碰撞盒大小与dst_size相同 
                auto collider = std::make_unique<engine::physics::AABBCollider>(dst_size);
                auto* cc = game_object->addComponent<engine::component::ColliderComponent>(std::move(collider));
                    // 自定义形状通常是trigger类型，除非显示指定 （因此默认为真）
                cc->setTrigger(object.value("trigger", true));
                    // 添加物理组件，不受重力影响
                game_object->addComponent<engine::component::PhysicsComponent>(&scene.getContext().getPhysicsEngine(), false);
                
                // 获取标签信息并设置
                if (auto tag = getTileProperty<std::string>(object, "tag"); tag) {  // 如果有标签
                    game_object->setTag(tag.value());
                }
                // 添加到场景
                scene.addGameObject(std::move(game_object));
                spdlog::info("加载对象: '{}' 完成 (类型: 自定义形状)", object_name);
            }
        } else {        // 如果gid存在，则按照图片解析流程
            // --- 根据gid获取必要信息，每个gid对应一个游戏对象 ---
            auto tile_info = getTileInfoByGid(gid);
            if (tile_info.sprite.getTextureId().empty()) {
                spdlog::error("gid为 {} 的瓦片没有图像纹理。", gid);
                continue;
            }
            // 获取Transform相关信息
            auto position = glm::vec2(object.value("x", 0.0f), object.value("y", 0.0f));
            auto dst_size = glm::vec2(object.value("width", 0.0f), object.value("height", 0.0f));
            position = glm::vec2(position.x, position.y - dst_size.y);  // 实际position需要进行调整(左下角到左上角)

            auto rotation = object.value("rotation", 0.0f);
            auto src_size_opt = tile_info.sprite.getSourceRect();
            if (!src_size_opt) {        // 正常情况下，所有瓦片的Sprite都设置了源矩形，没有代表某处出错
                spdlog::error("gid为 {} 的瓦片没有源矩形。", gid);
                continue;
            }
            auto src_size = glm::vec2(src_size_opt->w, src_size_opt->h);    // 成员变量除了 value().w 外，也可以这样获取
            auto scale = dst_size / src_size;

            // 获取对象名称
            std::string object_name = object.value("name", "Unnamed");

            // 创建游戏对象并添加组件
            auto game_object = std::make_unique<engine::object::GameObject>(object_name);
            game_object->addComponent<engine::component::TransformComponent>(position, scale, rotation);
            game_object->addComponent<engine::component::SpriteComponent>(std::move(tile_info.sprite), scene.getContext().getResourceManager());

            // 获取瓦片json信息      1. 必然存在，因为getTileInfoByGid(gid)函数已经顺利执行
                                // 2. 这里再获取json，实际上检索了两次，未来可以优化
            auto tile_json_opt = getTileJsonByGid(gid);
            if (!tile_json_opt) {
                spdlog::error("gid为 {} 的瓦片没有对应的 JSON 数据。", gid);
                continue;
            }
            auto& tile_json = tile_json_opt.value();

            // 获取碰信息：如果是SOLID类型，则添加物理组件，且图片源矩形区域就是碰撞盒大小
            if (tile_info.type == engine::component::TileType::SOLID) {
                auto collider = std::make_unique<engine::physics::AABBCollider>(src_size);
                game_object->addComponent<engine::component::ColliderComponent>(std::move(collider));
                // 物理组件不受重力影响
                game_object->addComponent<engine::component::PhysicsComponent>(&scene.getContext().getPhysicsEngine(), false);
                // 设置标签方便物理引擎检索
                game_object->setTag("solid");
            }
            // 如果非SOLID类型，检查自定义碰撞盒是否存在
            else if (auto rect = getColliderRect(tile_json); rect) {  
                // 如果有，添加碰撞组件
                auto collider = std::make_unique<engine::physics::AABBCollider>(rect->size);
                auto* cc = game_object->addComponent<engine::component::ColliderComponent>(std::move(collider));
                cc->setOffset(rect->position);  // 自定义碰撞盒的坐标是相对于图片坐标，也就是针对Transform的偏移量
                // 和物理组件（默认不受重力影响）
                game_object->addComponent<engine::component::PhysicsComponent>(&scene.getContext().getPhysicsEngine(), false);
            }

            // 获取标签信息并设置
            auto tag = getTileProperty<std::string>(tile_json, "tag");
            if (tag) {
                game_object->setTag(tag.value());
            }
            // 如果是危险瓦片，且没有手动设置标签，则自动设置标签为 "hazard"
            else if (tile_info.type == engine::component::TileType::HAZARD) {
                game_object->setTag("hazard");
            }

            // 获取重力信息并设置
            auto gravity = getTileProperty<bool>(tile_json, "gravity");
            if (gravity) {
                auto pc = game_object->getComponent<engine::component::PhysicsComponent>();
                if (pc) {
                    pc->setUseGravity(gravity.value());
                } else {
                    spdlog::warn("对象 '{}' 在设置重力信息时没有物理组件，请检查地图设置。", object_name);
                    game_object->addComponent<engine::component::PhysicsComponent>(&scene.getContext().getPhysicsEngine(), gravity.value());
                }
            }

            // 获取动画信息并设置
            auto anim_string = getTileProperty<std::string>(tile_json, "animation");
            if (anim_string) {
                // 解析string为JSON对象
                nlohmann::json anim_json;
                try {
                    anim_json = nlohmann::json::parse(anim_string.value());
                } catch (const nlohmann::json::parse_error& e) {
                    spdlog::error("解析动画 JSON 字符串失败: {}", e.what());
                    continue;  // 跳过此对象
                }
                // 添加AnimationComponent
                auto* ac = game_object->addComponent<engine::component::AnimationComponent>();
                // 添加动画到 AnimationComponent
                addAnimation(anim_json, ac, src_size);
            }

            // 获取音效信息并设置
            auto sound_string = getTileProperty<std::string>(tile_json, "sound");
            if (sound_string) {
                // 解析string为JSON对象
                nlohmann::json sound_json;
                try {
                    sound_json = nlohmann::json::parse(sound_string.value());
                } catch (const nlohmann::json::parse_error& e) {
                    spdlog::error("解析音效 JSON 字符串失败: {}", e.what());
                    continue;  // 跳过此对象
                }
                // 添加AudioComponent
                auto* audio_component = game_object->addComponent<engine::component::AudioComponent>(&scene.getContext().getAudioPlayer(),
                                                                                                     &scene.getContext().getCamera());
                // 添加音效到 AudioComponent
                addSound(sound_json, audio_component);
            }

            // 获取生命值信息并设置
            auto health = getTileProperty<int>(tile_json, "health");
            if (health) {
                // 添加 HealthComponent
                game_object->addComponent<engine::component::HealthComponent>(health.value());
            }

            // 添加到场景中
            scene.addGameObject(std::move(game_object));
            spdlog::info("加载对象: '{}' 完成", object_name);
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