#pragma once
#include "../component/tilelayer_component.h"
#include "../utils/math.h"
#include <string>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <glm/vec2.hpp>
#include <optional>

namespace engine::core {
    class Context;
}

namespace engine::component {
class AnimationComponent;
class AudioComponent;
}

namespace engine::scene {
    class LevelLoader;
}

namespace engine::object {
    class GameObject;

/**
 * @brief 游戏对象生成器，用于从 JSON 配置创建 GameObject。
 */
class ObjectBuilder {
protected:
    engine::scene::LevelLoader& level_loader_;      ///< @brief 用于访问 LevelLoader 的私有方法
    engine::core::Context& context_;
    std::unique_ptr<GameObject> game_object_;

    // --- 解析游戏对象所需要的关键信息 ---
    const nlohmann::json* object_json_ = nullptr;
    const nlohmann::json* tile_json_ = nullptr;
    engine::component::TileInfo tile_info_;

    // --- 保存会多次用到的变量，避免重复解析 ---
    std::string name_;
    glm::vec2 dst_size_;
    glm::vec2 src_size_;

public:
    /**
     * @brief 构造函数，传入 LevelLoader 和 Context 引用。
     */
    explicit ObjectBuilder(engine::scene::LevelLoader& level_loader, engine::core::Context& context);
    virtual ~ObjectBuilder();       ///< @brief 虚析构函数确保子类可以正确析构

    // --- 三个关键方法：配置、构建、返回 ---
    ObjectBuilder* configure(const nlohmann::json* object_json);    ///< @brief 配置生成器，针对自定义形状
    ObjectBuilder* configure(const nlohmann::json* object_json,     ///< @brief 配置生成器重载，针对图片对象
                             const nlohmann::json* tile_json,
                             engine::component::TileInfo);

    virtual void build();                     ///< @brief 构建GameObject (拓展时可以重载此方法)

    std::unique_ptr<GameObject> getGameObject();        ///< @brief 返回构建好的 GameObject

protected:
    void reset();       ///< @brief 重置生成器状态，每次configure的时候先调用

    // --- 代理函数，让子类能获取到LevelLoader的私有方法 ---
    template<typename T>
    std::optional<T> getTileProperty(const nlohmann::json& tile_json, std::string_view property_name);
    engine::component::TileType getTileType(const nlohmann::json& tile_json);
    std::optional<engine::utils::Rect> getColliderRect(const nlohmann::json& tile_json);
    void addAnimation(const nlohmann::json& anim_json, engine::component::AnimationComponent* ac, const glm::vec2& sprite_size);
    void addSound(const nlohmann::json& sound_json, engine::component::AudioComponent* audio_component);


    // --- 将原先在LevelLoader中的逻辑拆分成私有方法 ---
    void buildBase();
    void buildTransform();
    void buildSprite();
    void buildPhysics();
    void buildAnimation();
    void buildAudio();
    void buildHealth();
    // ... 未来可新增组件的构建方法
};

} // namespace engine::object