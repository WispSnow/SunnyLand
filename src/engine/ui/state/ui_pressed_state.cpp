#include "ui_pressed_state.h"
#include "ui_normal_state.h"
#include "ui_hover_state.h"
#include "../ui_interactive.h"
#include "../../input/input_manager.h"
#include "../../core/context.h"
#include <spdlog/spdlog.h>

namespace engine::ui::state {

void UIPressedState::enter()
{
    owner_->setSprite("pressed");
    owner_->playSound("pressed");
    spdlog::debug("切换到按下状态");
}

std::unique_ptr<UIState> UIPressedState::handleInput(engine::core::Context& context)
{
    auto& input_manager = context.getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (input_manager.isActionReleased("MouseLeftClick")) {
        if (!owner_->isPointInside(mouse_pos)) {        // 松开鼠标时，如果不在UI元素内，则切换到正常状态
            return std::make_unique<engine::ui::state::UINormalState>(owner_);
        } else {                                        // 松开鼠标时，如果还在UI元素内，则触发点击事件
            owner_->clicked();
            return std::make_unique<engine::ui::state::UIHoverState>(owner_);
        }
    }

    return nullptr;
}

} // namespace engine::ui::state