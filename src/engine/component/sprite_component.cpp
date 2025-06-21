#include "sprite_component.h"
#include "transform_component.h"
#include "../object/game_object.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include "../resource/resource_manager.h"
#include "../render/camera.h"
#include <stdexcept>          // 用于 std::runtime_error
#include <spdlog/spdlog.h>

namespace engine::component {

SpriteComponent::SpriteComponent(
    std::string_view texture_id,
    engine::resource::ResourceManager& resource_manager,
    engine::utils::Alignment alignment,
    std::optional<SDL_FRect> source_rect_opt,
    bool is_flipped
) : resource_manager_(&resource_manager), sprite_(texture_id, std::move(source_rect_opt), is_flipped),
    alignment_(alignment)
{  
    if (!resource_manager_) {
        spdlog::critical("创建 SpriteComponent 时 ResourceManager 为空！，此组件将无效。");
        // 不要在游戏主循环中使用 try...catch / throw，会极大影响性能
    }
    // offset_ 和 sprite_size_ 将在 init 中计算
    spdlog::trace("创建 SpriteComponent，纹理ID: {}", sprite_.getTextureId());
}

SpriteComponent::SpriteComponent(engine::render::Sprite&& sprite, engine::resource::ResourceManager& resource_manager, engine::utils::Alignment alignment)
    : resource_manager_(&resource_manager), sprite_(std::move(sprite)), alignment_(alignment)
{
    if (!resource_manager_) {
        spdlog::critical("创建 SpriteComponent 时 ResourceManager 为空！，此组件将无效。");
        // 不要在游戏主循环中使用 try...catch / throw，会极大影响性能
    }
    // offset_ 和 sprite_size_ 将在 init 中计算
    spdlog::trace("创建 SpriteComponent，纹理ID: {}", sprite_.getTextureId());
}

void SpriteComponent::init() {
    if (!owner_) {
        spdlog::error("SpriteComponent 在初始化前未设置所有者。");
        return;
    }
    transform_ = owner_->getComponent<TransformComponent>();
    if (!transform_) {
        spdlog::warn(
            "GameObject '{}' 上的 SpriteComponent 需要一个 TransformComponent，但未找到。",
            owner_->getName()
        );
        // Sprite没有Transform无法计算偏移和渲染，直接返回
        return;
    }

    // 获取大小及偏移
    updateSpriteSize();
    updateOffset();
}

void SpriteComponent::setAlignment(engine::utils::Alignment anchor) {
    alignment_ = anchor;
    updateOffset();
}

void SpriteComponent::updateOffset() {
    // 如果尺寸无效，偏移为0
    if (sprite_size_.x <= 0 || sprite_size_.y <= 0) {
        offset_ = {0.0f, 0.0f};
        return;
    }
    auto scale = transform_->getScale();
    // 计算精灵左上角相对于 TransformComponent::position_ 的偏移
    switch (alignment_) {
        case engine::utils::Alignment::TOP_LEFT:      offset_ = glm::vec2{0.0f, 0.0f} * scale; break;
        case engine::utils::Alignment::TOP_CENTER:    offset_ = glm::vec2{-sprite_size_.x / 2.0f, 0.0f} * scale; break;
        case engine::utils::Alignment::TOP_RIGHT:     offset_ = glm::vec2{-sprite_size_.x, 0.0f} * scale; break;
        case engine::utils::Alignment::CENTER_LEFT:   offset_ = glm::vec2{0.0f, -sprite_size_.y / 2.0f} * scale; break;
        case engine::utils::Alignment::CENTER:        offset_ = glm::vec2{-sprite_size_.x / 2.0f, -sprite_size_.y / 2.0f} * scale; break;
        case engine::utils::Alignment::CENTER_RIGHT:  offset_ = glm::vec2{-sprite_size_.x, -sprite_size_.y / 2.0f} * scale; break;
        case engine::utils::Alignment::BOTTOM_LEFT:   offset_ = glm::vec2{0.0f, -sprite_size_.y} * scale; break;
        case engine::utils::Alignment::BOTTOM_CENTER: offset_ = glm::vec2{-sprite_size_.x / 2.0f, -sprite_size_.y} * scale; break;
        case engine::utils::Alignment::BOTTOM_RIGHT:  offset_ = glm::vec2{-sprite_size_.x, -sprite_size_.y} * scale; break;
        case engine::utils::Alignment::NONE:
        default:                                      break;
    }
}

void SpriteComponent::render(engine::core::Context& context) {
    if (is_hidden_ || !transform_ || !resource_manager_) {
        return;
    }

    // 获取变换信息（考虑偏移量）
    const glm::vec2& pos = transform_->getPosition() + offset_;
    const glm::vec2& scale = transform_->getScale();
    float rotation_degrees = transform_->getRotation();

    // 执行绘制
    context.getRenderer().drawSprite(context.getCamera(), sprite_, pos, scale, rotation_degrees);
}

void SpriteComponent::setSpriteById(std::string_view texture_id, std::optional<SDL_FRect> source_rect_opt) {
    sprite_.setTextureId(texture_id);
    sprite_.setSourceRect(std::move(source_rect_opt));

    updateSpriteSize();
    updateOffset();
}

void SpriteComponent::setSourceRect(std::optional<SDL_FRect> source_rect_opt) {
    sprite_.setSourceRect(std::move(source_rect_opt));
    updateSpriteSize();
    updateOffset();
}

void SpriteComponent::updateSpriteSize() {
    if (!resource_manager_) {
        spdlog::error("ResourceManager 为空！无法获取纹理尺寸。");
        return;
    }
    if (sprite_.getSourceRect().has_value()) {
        const auto& src_rect = sprite_.getSourceRect().value();
        sprite_size_ = {src_rect.w, src_rect.h};
    } else {
        sprite_size_ = resource_manager_->getTextureSize(sprite_.getTextureId());
    }
}

} // namespace engine::component 