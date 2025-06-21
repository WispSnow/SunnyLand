#include "ui_element.h"
#include "../core/context.h"
#include <algorithm>
#include <utility>
#include <spdlog/spdlog.h>

namespace engine::ui {

UIElement::UIElement(glm::vec2 position, glm::vec2 size)
    : position_(std::move(position)), size_(std::move(size)) {
}   

bool UIElement::handleInput(engine::core::Context& context) {
    // 如果元素不可见，直接返回 false
    if (!visible_) return false; 

    // 遍历所有子节点，并删除标记了移除的元素
    for (auto it = children_.begin(); it != children_.end();) {
        if (*it && !(*it)->isNeedRemove()) {
            if ((*it)->handleInput(context)) return true;
            ++it;
        } else {
            it = children_.erase(it);
        }
    }
    // 事件未被消耗，返回假
    return false;
}

void UIElement::update(float delta_time, engine::core::Context& context) {
    if (!visible_) return;

    // 遍历所有子节点，并删除标记了移除的元素
    for (auto it = children_.begin(); it != children_.end();) {
        if (*it && !(*it)->isNeedRemove()) {
            (*it)->update(delta_time, context);
            ++it;
        } else {
            it = children_.erase(it);
        }
    }
}

void UIElement::render(engine::core::Context& context) {
    if (!visible_) return;

    // 渲染子元素
    for (const auto& child : children_) {
        if (child) child->render(context);
    }
}

void UIElement::addChild(std::unique_ptr<UIElement> child) {
    if (child) {
        child->setParent(this); // 设置父指针
        children_.push_back(std::move(child));
    }
}

std::unique_ptr<UIElement> UIElement::removeChild(UIElement* child_ptr) {
    // 使用 std::remove_if 和 lambda 表达式自定义比较的方式移除
    auto it = std::find_if(children_.begin(), children_.end(),
                           [child_ptr](const std::unique_ptr<UIElement>& p) { 
                                return p.get() == child_ptr; 
                           });

    if (it != children_.end()) {
        std::unique_ptr<UIElement> removed_child = std::move(*it);
        children_.erase(it);
        removed_child->setParent(nullptr);      // 清除父指针
        return removed_child;                   // 返回被移除的子元素（可以挂载到别处）
    }
    return nullptr; // 未找到子元素
}

void UIElement::removeAllChildren() {
    for (auto& child : children_) {
        child->setParent(nullptr); // 清除父指针
    }
    children_.clear();
}

glm::vec2 UIElement::getScreenPosition() const {
    if (parent_) {
        return parent_->getScreenPosition() + position_;
    }
    return position_; // 根元素的位置已经是相对屏幕的绝对位置
}

engine::utils::Rect UIElement::getBounds() const {
    auto abs_pos = getScreenPosition();
    return engine::utils::Rect(abs_pos, size_);
}

bool UIElement::isPointInside(const glm::vec2& point) const {
    auto bounds = getBounds();
    return (point.x >= bounds.position.x && point.x < (bounds.position.x + bounds.size.x) &&
            point.y >= bounds.position.y && point.y < (bounds.position.y + bounds.size.y));
}

} // namespace engine::ui 