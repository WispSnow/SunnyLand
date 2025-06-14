#include "ui_hover_state.h"
#include "ui_normal_state.h"
#include "ui_pressed_state.h"
#include "../ui_interactive.h"
#include "../../input/input_manager.h"
#include "../../core/context.h"
#include <spdlog/spdlog.h>

namespace engine::ui::state {

void UIHoverState::enter()
{
    owner_->setSprite("hover");
    spdlog::debug("切换到悬停状态");
}

std::unique_ptr<UIState> UIHoverState::handleInput(engine::core::Context& context)
{
    auto& input_manager = context.getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (!owner_->isPointInside(mouse_pos)) {                // 如果鼠标不在UI元素内，则返回正常状态
        return std::make_unique<UINormalState>(owner_);
    }
    if (input_manager.isActionPressed("MouseLeftClick")) {  // 如果鼠标按下，则返回按下状态
        return std::make_unique<UIPressedState>(owner_);
    }
    return nullptr;
}

} // namespace engine::ui::state