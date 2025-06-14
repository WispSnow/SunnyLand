#pragma once
#include "ui_state.h"

namespace engine::ui::state {

/**
 * @brief 悬停状态
 *
 * 当鼠标悬停在UI元素上时，会切换到该状态。
 */
class UIHoverState final: public UIState {
    friend class engine::ui::UIInteractive;
public:
    UIHoverState(engine::ui::UIInteractive* owner) : UIState(owner) {}
    ~UIHoverState() override = default;

private:
    void enter() override;
    std::unique_ptr<UIState> handleInput(engine::core::Context& context) override;
};

} // namespace engine::ui::state