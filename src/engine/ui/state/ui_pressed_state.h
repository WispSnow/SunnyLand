#pragma once
#include "ui_state.h"

namespace engine::ui::state {

/**
 * @brief 按下状态
 *
 * 当鼠标按下UI元素时，会切换到该状态。
 */
class UIPressedState final: public UIState {
    friend class engine::ui::UIInteractive;
public:
    UIPressedState(engine::ui::UIInteractive* owner) : UIState(owner) {}
    ~UIPressedState() override = default;

private:
    void enter() override;
    std::unique_ptr<UIState> handleInput(engine::core::Context& context) override;
};

} // namespace engine::ui::state