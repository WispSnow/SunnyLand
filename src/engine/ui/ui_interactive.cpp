#include "ui_interactive.h"
#include "state/ui_state.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include "../resource/resource_manager.h"
#include "../audio/audio_player.h"
#include <spdlog/spdlog.h>

namespace engine::ui {

UIInteractive::~UIInteractive() = default;

UIInteractive::UIInteractive(engine::core::Context &context, glm::vec2 position, glm::vec2 size)
    : UIElement(std::move(position), std::move(size)), context_(context)
{
    spdlog::trace("UIInteractive 构造完成");
}

void UIInteractive::setState(std::unique_ptr<engine::ui::state::UIState> state)
{
    if (!state) {
        spdlog::warn("尝试设置空的状态！");
        return;
    }

    state_ = std::move(state);
    state_->enter();
}

void UIInteractive::addSprite(std::string_view name, std::unique_ptr<engine::render::Sprite> sprite)
{
    // 可交互UI元素必须有一个size用于交互检测，因此如果参数列表中没有指定，则用图片大小作为size
    if (size_.x == 0.0f && size_.y == 0.0f) {
        size_ = context_.getResourceManager().getTextureSize(sprite->getTextureId());
    }
    // 添加精灵
    sprites_[std::string(name)] = std::move(sprite);
}

void UIInteractive::setSprite(std::string_view name)
{
    if (sprites_.find(std::string(name)) != sprites_.end()) {
        current_sprite_ = sprites_[std::string(name)].get();
    } else {
        spdlog::warn("Sprite '{}' 未找到", name);
    }
}

void UIInteractive::addSound(std::string_view name, std::string_view path)
{
    sounds_[std::string(name)] = path;
}

void UIInteractive::playSound(std::string_view name)
{
    if (sounds_.find(std::string(name)) != sounds_.end()) {
        context_.getAudioPlayer().playSound(sounds_[std::string(name)]);
    } else {
        spdlog::error("Sound '{}' 未找到", name);
    }
}

bool UIInteractive::handleInput(engine::core::Context &context)
{
    if (UIElement::handleInput(context)) {  
        return true;
    }

    // 先更新子节点，再更新自己（状态）
    if (state_ && interactive_) {
        if (auto next_state = state_->handleInput(context); next_state) {
            setState(std::move(next_state));
            return true;
        }
    }
    return false;
}

void UIInteractive::render(engine::core::Context &context)
{
    if (!visible_ ) return;

    // 先渲染自身
    context.getRenderer().drawUISprite(*current_sprite_, getScreenPosition(), size_);

    // 再渲染子元素（调用基类方法）
    UIElement::render(context);
}

} // namespace engine::ui