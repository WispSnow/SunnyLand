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
    // 获取文本渲染尺寸
    size_ = text_renderer_.getTextSize(text_, font_id_, font_size_);
    spdlog::trace("UILabel 构造完成");
}

void UILabel::render(engine::core::Context& context) {
    if (!visible_ || text_.empty()) return;

    text_renderer_.drawUIText(text_, font_id_, font_size_, getScreenPosition(), text_fcolor_);

    // 渲染子元素（调用基类方法）
    UIElement::render(context);
}

void UILabel::setText(std::string_view text)
{
    text_ = text;
    size_ = text_renderer_.getTextSize(text_, font_id_, font_size_);
}

void UILabel::setFontId(std::string_view font_id)
{
    font_id_ = font_id;
    size_ = text_renderer_.getTextSize(text_, font_id_, font_size_);
}

void UILabel::setFontSize(int font_size)
{
    font_size_ = font_size;
    size_ = text_renderer_.getTextSize(text_, font_id_, font_size_);
}

void UILabel::setTextFColor(engine::utils::FColor text_fcolor)
{
    text_fcolor_ = std::move(text_fcolor);
    /* 颜色变化不影响尺寸 */
}

} // namespace engine::ui