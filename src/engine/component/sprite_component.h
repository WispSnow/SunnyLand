#pragma once
#include "../render/sprite.h"
#include "./component.h"
#include "../utils/alignment.h"
#include <string>
#include <string_view>
#include <optional>
#include <SDL3/SDL_rect.h>
#include <glm/vec2.hpp>

namespace engine::core {
    class Context;
}

namespace engine::resource {
    class ResourceManager;
}

namespace engine::component {
class TransformComponent;

/**
 * @brief 管理 GameObject 的视觉表示，通过持有一个 Sprite 对象。
 *
 * 协调 Sprite 数据和渲染逻辑，并与 TransformComponent 交互。
 */
class SpriteComponent final : public engine::component::Component {
    friend class engine::object::GameObject;            // 友元不能继承，必须每个子类单独添加
private:
    engine::resource::ResourceManager* resource_manager_ = nullptr;         ///< @brief 保存资源管理器指针，用于获取纹理大小
    TransformComponent* transform_ = nullptr;                               ///< @brief 缓存 TransformComponent 指针（非必须）

    engine::render::Sprite sprite_;                                         ///< @brief 精灵对象
    engine::utils::Alignment alignment_ = engine::utils::Alignment::NONE;   ///< @brief 对齐方式
    glm::vec2 sprite_size_ = {0.0f, 0.0f};                                  ///< @brief 精灵尺寸
    glm::vec2 offset_ = {0.0f, 0.0f};                                       ///< @brief 偏移量
    bool is_hidden_ = false;                                                ///< @brief 是否隐藏（不渲染）
    
public:
    /**
     * @brief 构造函数
     * @param texture_id 纹理资源的标识符。
     * @param resource_manager 资源管理器指针。
     * @param alignment 初始对齐方式。
     * @param source_rect_opt 可选的源矩形。
     * @param is_flipped 初始翻转状态。
     */
    SpriteComponent(
        std::string_view texture_id,
        engine::resource::ResourceManager& resource_manager,
        engine::utils::Alignment alignment = engine::utils::Alignment::NONE,
        std::optional<SDL_FRect> source_rect_opt = std::nullopt,
        bool is_flipped = false
    );

    /**
     * @brief 构造函数
     * @param sprite 精灵对象。
     * @param resource_manager 资源管理器指针。
     * @param alignment 初始对齐方式。
     */
    SpriteComponent(
        engine::render::Sprite&& sprite,
        engine::resource::ResourceManager& resource_manager,
        engine::utils::Alignment alignment = engine::utils::Alignment::NONE
    );

    ~SpriteComponent() override = default;

    // 禁止拷贝和移动
    SpriteComponent(const SpriteComponent&) = delete;
    SpriteComponent& operator=(const SpriteComponent&) = delete;
    SpriteComponent(SpriteComponent&&) = delete;
    SpriteComponent& operator=(SpriteComponent&&) = delete;

    void updateOffset();           ///< @brief 更新偏移量(根据当前的 alignment_ 和 sprite_size_ 计算 offset_)。

    // Getters
    const engine::render::Sprite& getSprite() const { return sprite_; }         ///< @brief 获取精灵对象
    std::string_view getTextureId() const { return sprite_.getTextureId(); }  ///< @brief 获取纹理ID
    bool isFlipped() const { return sprite_.isFlipped(); }                      ///< @brief 获取是否翻转
    bool isHidden() const { return is_hidden_; }                                ///< @brief 获取是否隐藏
    const glm::vec2& getSpriteSize() const { return sprite_size_; }             ///< @brief 获取精灵尺寸
    const glm::vec2& getOffset() const { return offset_; }                      ///< @brief 获取偏移量
    engine::utils::Alignment getAlignment() const { return alignment_; }        ///< @brief 获取对齐方式

    // Setters
    void setSpriteById(std::string_view texture_id, std::optional<SDL_FRect> source_rect_opt = std::nullopt); ///< @brief 设置精灵对象
    void setFlipped(bool flipped) { sprite_.setFlipped(flipped); }                                             ///< @brief 设置是否翻转
    void setHidden(bool hidden) { is_hidden_ = hidden; }                                                      ///< @brief 设置是否隐藏
    void setSourceRect(std::optional<SDL_FRect> source_rect_opt);                                     ///< @brief 设置源矩形
    void setAlignment(engine::utils::Alignment anchor);                                                     ///< @brief 设置对齐方式

private:
    void updateSpriteSize();        ///< @brief 辅助函数，根据 sprite_ 的 source_rect_ 更新 sprite_size_

    // Component 虚函数覆盖
    void init() override;                                                   ///< @brief 初始化函数需要覆盖
    void update(float, engine::core::Context&) override {}                  ///< @brief 更新函数留空
    void render(engine::core::Context& context) override;                   ///< @brief 渲染函数需要覆盖

};

} // namespace engine::component