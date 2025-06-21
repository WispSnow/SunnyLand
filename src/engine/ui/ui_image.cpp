#include "ui_image.h"
#include "../render/renderer.h"
#include "../render/sprite.h"
#include "../core/context.h"
#include <spdlog/spdlog.h>

namespace engine::ui {

UIImage::UIImage(std::string_view texture_id,
                 glm::vec2 position,
                 glm::vec2 size,
                 std::optional<SDL_FRect> source_rect,
                 bool is_flipped)
    : UIElement(std::move(position), std::move(size)),
      sprite_(texture_id, std::move(source_rect), is_flipped)
{
    if (texture_id.empty()) {
        spdlog::warn("创建了一个空纹理ID的UIImage。");
    }
    spdlog::trace("UIImage 构造完成");
}

void UIImage::render(engine::core::Context& context) {
    if (!visible_ || sprite_.getTextureId().empty()) {
        return; // 如果不可见或没有分配纹理则不渲染
    }

    // 渲染自身
    auto position = getScreenPosition();
    if (size_.x == 0.0f && size_.y == 0.0f) {   // 如果尺寸为0，则使用纹理的原始尺寸
        context.getRenderer().drawUISprite(sprite_, position);
    } else {
        context.getRenderer().drawUISprite(sprite_, position, size_);
    }

    // 渲染子元素（调用基类方法）
    UIElement::render(context);
}

} // namespace engine::ui 