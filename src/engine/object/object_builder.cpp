#include "object_builder.h"
#include "game_object.h"
#include "../core/context.h"
#include "../component/parallax_component.h"
#include "../component/transform_component.h"
#include "../component/tilelayer_component.h"
#include "../component/sprite_component.h"
#include "../component/collider_component.h"
#include "../component/physics_component.h"
#include "../component/animation_component.h"
#include "../component/health_component.h"
#include "../component/audio_component.h"
#include "../resource/resource_manager.h"
#include "../physics/physics_engine.h"
#include "../render/animation.h"
#include "../scene/level_loader.h"
#include <string>
#include <string_view>
#include <spdlog/spdlog.h>

namespace engine::object {

ObjectBuilder::~ObjectBuilder() = default;

ObjectBuilder::ObjectBuilder(engine::scene::LevelLoader& level_loader, engine::core::Context& context)
    : level_loader_(level_loader), context_(context) {
    spdlog::trace("ObjectBuilder 已创建.");
}

ObjectBuilder *ObjectBuilder::configure(const nlohmann::json* object_json)
{
    reset();  // 重置生成器状态，确保每次配置都是干净的
    if (!object_json) {
        spdlog::error("ObjectBuilder 配置失败：object_json 为空。");
        return this;    // 返回自身，允许链式调用
    }
    // 非矩形对象会有额外标识（目前不支持，直接返回）
    if (object_json->value("point", false)) {             // 如果是点对象
        return this;
    } else if (object_json->value("ellipse", false)) {    // 如果是椭圆对象
        return this;
    } else if (object_json->value("polygon", false)) {    // 如果是多边形对象
        return this;
    } 
    // 没有这些标识则默认是矩形对象，可以继续
    object_json_ = object_json;
    tile_json_ = nullptr;         // 确保 tile_json_ 为空，作为自定义形状对象的标识
    spdlog::trace("ObjectBuilder 配置成功：object_json_ 已设置。");
    return this;
}

ObjectBuilder *ObjectBuilder::configure(const nlohmann::json *object_json, 
                                        const nlohmann::json *tile_json, 
                                        engine::component::TileInfo tile_info)
{
    reset();
    if (!object_json || !tile_json) {
        spdlog::error("ObjectBuilder 配置失败：object_json 或 tile_json 为空。");
        return this;
    }
    object_json_ = object_json;
    tile_json_ = tile_json;
    tile_info_ = std::move(tile_info);
    spdlog::trace("ObjectBuilder 配置成功：object_json_ 和 tile_json_ 已设置。");
    return this;
}

void ObjectBuilder::build()
{
    if (!object_json_) {
        spdlog::error("ObjectBuilder 构建失败：object_json_ 为空。");
        return;  // 如果没有配置，直接返回
    }
    // 按顺序构建各个组件
    buildBase();          // 构建基本信息
    buildTransform();     // 构建变换组件
    buildSprite();        // 构建精灵组件（如果是图片对象）
    buildPhysics();       // 构建物理组件
    buildAnimation();     // 构建动画组件（如果有）
    buildAudio();         // 构建音频组件（如果有）
    buildHealth();        // 构建生命值组件（如果有）
    spdlog::debug("ObjectBuilder 构建完成：{}", name_);
}

std::unique_ptr<GameObject> ObjectBuilder::getGameObject()
{
    return std::move(game_object_);  // 返回构建好的 GameObject，并将其所有权转移出去
}

void ObjectBuilder::reset()
{
    object_json_ = nullptr;  // 重置为 nullptr
    tile_json_ = nullptr;    // 重置为 nullptr
    tile_info_ = {};         // 重置 TileInfo
    game_object_ = nullptr;  // 重置 GameObject
    name_.clear();           // 清空名称
    dst_size_ = glm::vec2(0.0f);  // 重置目标尺寸
    src_size_ = glm::vec2(0.0f);  // 重置源尺寸
}

void ObjectBuilder::buildBase()
{
    // 名称信息从 object_json_ 中获取
    name_ = object_json_->value("name", "");
    
    // 先查找地图中的标签，没有再查找图块集中的标签（前者覆盖后者）
    auto tag = getTileProperty<std::string>(*object_json_, "tag");

    if (!tag && tile_json_) {   // 如果没找到标签且 tile_json_ 存在，则尝试从中获取标签
        tag = getTileProperty<std::string>(*tile_json_, "tag");
        // 如果也没找到标签，但它是危险图块，则自动设置标签为 "hazard"
        if (!tag && tile_info_.type == engine::component::TileType::HAZARD) {
            tag = "hazard";  // 危险图块默认标签
        }
    }

    // 创建游戏对象
    game_object_ = std::make_unique<GameObject>(name_, tag.value_or(""));
}

void ObjectBuilder::buildTransform()
{
    // 位置、尺寸和旋转信息从 object_json_ 中获取
    auto position = glm::vec2(object_json_->value("x", 0.0f), object_json_->value("y", 0.0f));
    dst_size_ = glm::vec2(object_json_->value("width", 0.0f), object_json_->value("height", 0.0f));
    auto rotation = object_json_->value("rotation", 0.0f);
    auto scale = glm::vec2(1.0f);  // 默认缩放为1.0f
    
    // 如果是图片对象，需要额外处理瓦片json中的数据
    if (tile_json_) {
        position = glm::vec2(position.x, position.y - dst_size_.y);  // 图片对象的position需要进行调整(左下角到左上角)
        // --- 计算缩放比例 ---
        auto src_size_opt = tile_info_.sprite.getSourceRect();
        if (src_size_opt) {        // 正常情况下，所有瓦片的Sprite都设置了源矩形，没有就跳过
            src_size_ = glm::vec2(src_size_opt->w, src_size_opt->h);
            scale = dst_size_ / src_size_;    // 更新缩放比例
        }   
    }

    // 添加 TransformComponent
    game_object_->addComponent<engine::component::TransformComponent>(position, scale, rotation);
}

void ObjectBuilder::buildSprite()
{
    // 如果是自定义形状对象，则不需要SpriteComponent
    if (!tile_json_) return; 

    // 确保 tile_info_ 已经被正确配置
    if (tile_info_.sprite.getTextureId().empty()) {
        spdlog::error("对象 '{}' 在 tile_info_ 中的 sprite 没有纹理 ID。", name_);
        return;  // 如果没有纹理ID，则不添加 SpriteComponent
    }

    // 添加 SpriteComponent
    game_object_->addComponent<engine::component::SpriteComponent>(tile_info_.sprite, 
                                                                   context_.getResourceManager());
}

void ObjectBuilder::buildPhysics()
{
    // 如果是自定义形状对象
    if (!tile_json_){
        // 碰撞盒大小与dst_size相同 
        auto collider = std::make_unique<engine::physics::AABBCollider>(dst_size_);
        auto* cc = game_object_->addComponent<engine::component::ColliderComponent>(std::move(collider));
        // 自定义形状通常是trigger类型，除非显式指定 （因此默认为真）
        cc->setTrigger(object_json_->value("trigger", true));
        // 添加物理组件，不受重力影响
        game_object_->addComponent<engine::component::PhysicsComponent>(&context_.getPhysicsEngine(), false);
    }
    // 如果是图片对象
    else {
        // 获取碰信息：如果是SOLID类型，则添加物理组件，且图片源矩形区域就是碰撞盒大小
        if (tile_info_.type == engine::component::TileType::SOLID) {
            auto collider = std::make_unique<engine::physics::AABBCollider>(src_size_);
            game_object_->addComponent<engine::component::ColliderComponent>(std::move(collider));
            // 物理组件不受重力影响
            game_object_->addComponent<engine::component::PhysicsComponent>(&context_.getPhysicsEngine(), false);
            // 设置标签方便物理引擎检索
            game_object_->setTag("solid");
        }
        // 如果非SOLID类型，检查自定义碰撞盒是否存在
        else if (auto rect = getColliderRect(*tile_json_); rect) {  
            // 如果有，添加碰撞组件
            auto collider = std::make_unique<engine::physics::AABBCollider>(rect->size);
            auto* cc = game_object_->addComponent<engine::component::ColliderComponent>(std::move(collider));
            cc->setOffset(rect->position);  // 自定义碰撞盒的坐标是相对于图片坐标，也就是针对Transform的偏移量
            // 和物理组件（默认不受重力影响）
            game_object_->addComponent<engine::component::PhysicsComponent>(&context_.getPhysicsEngine(), false);
        }
        // 尝试获取显式标注的重力信息并设置
        auto gravity = getTileProperty<bool>(*tile_json_, "gravity");
        if (gravity) {
            auto pc = game_object_->getComponent<engine::component::PhysicsComponent>();
            if (pc) {
                pc->setUseGravity(gravity.value());
            } else {
                spdlog::warn("对象 '{}' 在设置重力信息时没有物理组件，请检查地图设置。", name_);
                game_object_->addComponent<engine::component::PhysicsComponent>(&context_.getPhysicsEngine(), gravity.value());
            }
        }
    }
}

void ObjectBuilder::buildAnimation()
{
    if (!tile_json_) return;  // 如果是自定义形状对象，则不需要 AnimationComponent

    auto anim_string = getTileProperty<std::string>(*tile_json_, "animation");
    if (anim_string) {
        // 解析string为JSON对象
        nlohmann::json anim_json;
        try {
            anim_json = nlohmann::json::parse(anim_string.value());
        } catch (const nlohmann::json::parse_error& e) {
            spdlog::error("解析动画 JSON 字符串失败: {}", e.what());
            return;  // 跳过此对象
        }
        // 添加AnimationComponent
        auto* ac = game_object_->addComponent<engine::component::AnimationComponent>();
        // 添加动画到 AnimationComponent
        addAnimation(anim_json, ac, src_size_);
    }
}

void ObjectBuilder::buildHealth()
{
    if (!tile_json_) return;  // 如果是自定义形状对象，则不需要 HealthComponent

    // 获取生命值信息并设置
    auto health = getTileProperty<int>(*tile_json_, "health");
    if (health) {
        // 添加 HealthComponent
        game_object_->addComponent<engine::component::HealthComponent>(health.value());
    }
}

void ObjectBuilder::buildAudio()
{
    if (!tile_json_) return;  // 如果是自定义形状对象，则不需要 AudioComponent
    
    // 获取音效信息并设置
    auto sound_string = getTileProperty<std::string>(*tile_json_, "sound");
    if (sound_string) {
        // 解析string为JSON对象
        nlohmann::json sound_json;
        try {
            sound_json = nlohmann::json::parse(sound_string.value());
        } catch (const nlohmann::json::parse_error& e) {
            spdlog::error("解析音效 JSON 字符串失败: {}", e.what());
            return;  // 跳过此对象
        }
        // 添加AudioComponent
        auto* audio_component = game_object_->addComponent<engine::component::AudioComponent>(&context_.getCamera());
        // 添加音效到 AudioComponent
        addSound(sound_json, audio_component);
    }
}

// --- 代理函数，让子类能获取到LevelLoader的私有方法 ---
template<typename T>
std::optional<T> ObjectBuilder::getTileProperty(const nlohmann::json& tile_json, std::string_view property_name) {
    return level_loader_.getTileProperty<T>(tile_json, property_name);
}

engine::component::TileType ObjectBuilder::getTileType(const nlohmann::json& tile_json) {
    return level_loader_.getTileType(tile_json);
}

std::optional<engine::utils::Rect> ObjectBuilder::getColliderRect(const nlohmann::json& tile_json) {
    return level_loader_.getColliderRect(tile_json);
}

void ObjectBuilder::addAnimation(const nlohmann::json& anim_json, engine::component::AnimationComponent* ac, const glm::vec2& sprite_size) {
    level_loader_.addAnimation(anim_json, ac, sprite_size);
}

void ObjectBuilder::addSound(const nlohmann::json& sound_json, engine::component::AudioComponent* audio_component) {
    level_loader_.addSound(sound_json, audio_component);
}

} // namespace engine::object