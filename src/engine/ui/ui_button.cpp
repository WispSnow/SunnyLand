#include "ui_button.h"
#include "state/ui_normal_state.h"
#include <spdlog/spdlog.h>

namespace engine::ui {
UIButton::UIButton(engine::core::Context& context,
                   std::string_view normal_sprite_id, 
                   std::string_view hover_sprite_id, 
                   std::string_view pressed_sprite_id, 
                   glm::vec2 position, 
                   glm::vec2 size, 
                   std::function<void()> callback)
    : UIInteractive(context, std::move(position), std::move(size)), callback_(std::move(callback))
{
    addSprite("normal", std::make_unique<engine::render::Sprite>(normal_sprite_id));
    addSprite("hover", std::make_unique<engine::render::Sprite>(hover_sprite_id));
    addSprite("pressed", std::make_unique<engine::render::Sprite>(pressed_sprite_id));

    // 设置默认状态为"normal"
    setState(std::make_unique<engine::ui::state::UINormalState>(this));

    // 设置默认音效
    addSound("hover", "assets/audio/button_hover.wav");
    addSound("pressed", "assets/audio/button_click.wav");
    spdlog::trace("UIButton 构造完成");
}

void UIButton::clicked()
{
    if (callback_) callback_();
}

} // namespace engine::ui