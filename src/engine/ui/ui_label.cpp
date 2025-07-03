#include "ui_label.h"
#include "../core/context.h"
#include "../render/text_renderer.h"
#include <spdlog/spdlog.h>

namespace engine::ui {

UILabel::UILabel(engine::render::TextRenderer& text_renderer,
                 std::string_view text,
                 std::string_view font_id,
                 int font_size,
                 engine::utils::FColor text_color,
                 glm::vec2 position)
    : UIElement(std::move(position)),
      text_renderer_(text_renderer),
      text_(text),
      font_id_(font_id),
      font_size_(font_size),
      text_fcolor_(std::move(text_color)) {
    // 不用在这里获取尺寸，因为需要时再获取
    spdlog::trace("UILabel 构造完成");
}

void UILabel::onNotify(const engine::interface::EventType event, const std::any& data) {
    if (event == engine::interface::EventType::SCORE_CHANGED) {
        if (const int* score = std::any_cast<int>(&data)) {     // 针对std::any数据的常用安全判断方式
            setText("Score: " + std::to_string(*score));
        } else {
            spdlog::warn("UILabel::onNotify: SCORE_CHANGED 事件接收到无效的数据类型。");
        }
    }
}

void UILabel::render(engine::core::Context& context) {
    if (!visible_ || text_.empty()) return;

    text_renderer_.drawUIText(text_, font_id_, font_size_, getScreenPosition(), text_fcolor_, is_dirty_);
    is_dirty_ = false;      // 渲染完成后，脏标志设置为false，下次渲染不需要重新设置TTF_Text

    // 渲染子元素（调用基类方法）
    UIElement::render(context);
}

void UILabel::setText(std::string_view text)
{
    text_ = text;
    is_dirty_ = true;   // 不再立刻更新尺寸，而是设置脏标识为真
}

void UILabel::setFontId(std::string_view font_id)
{
    font_id_ = font_id;
    is_dirty_ = true;
}

void UILabel::setFontSize(int font_size)
{
    font_size_ = font_size;
    is_dirty_ = true;
}

void UILabel::setTextFColor(engine::utils::FColor text_fcolor)
{
    text_fcolor_ = std::move(text_fcolor);
    /* 颜色变化不影响尺寸 */
}

const glm::vec2& UILabel::getSize() {
    // 调用此函数时才尝试获取尺寸
    size_ = text_renderer_.getTextSize(text_, font_id_, font_size_, is_dirty_);
    is_dirty_ = false;
    return size_;
}

} // namespace engine::ui