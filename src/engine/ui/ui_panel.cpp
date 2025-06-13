#include "ui_panel.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include <SDL3/SDL_pixels.h>
#include <spdlog/spdlog.h>

namespace engine::ui {

UIPanel::UIPanel(const glm::vec2& position, const glm::vec2& size, const std::optional<engine::utils::FColor>& background_color)
    : UIElement(position, size), background_color_(background_color)
{
    spdlog::trace("UIPanel 构造完成。");
}

void UIPanel::render(engine::core::Context& context) {
    if (!visible_) return;

    if (background_color_) {
        context.getRenderer().drawUIFilledRect(getBounds(), background_color_.value());
    }

    UIElement::render(context); // 调用基类渲染方法(绘制子节点)
}

} // namespace engine::ui 