#pragma once
#include <SDL3/SDL_rect.h>
#include <memory>
#include <vector>
#include "../utils/math.h"

namespace engine::core {
    class Context;
}

namespace engine::ui {

/**
 * @brief 所有UI元素的基类
 *
 * 定义了位置、大小、可见性、状态等通用属性。
 * 管理子元素的层次结构。
 * 提供事件处理、更新和渲染的虚方法。
 */
class UIElement {
protected:
    glm::vec2 position_;                                    ///< @brief 相对于父元素的局部位置
    glm::vec2 size_;                                        ///< @brief 元素大小
    bool visible_ = true;                                   ///< @brief 元素当前是否可见
    bool need_remove_ = false;                              ///< @brief 是否需要移除(延迟删除)

    UIElement* parent_ = nullptr;                           ///< @brief 指向父节点的非拥有指针
    std::vector<std::unique_ptr<UIElement>> children_;      ///< @brief 子元素列表(容器)

public:
    /**
     * @brief 构造UIElement
     * @param position 初始局部位置
     * @param size 初始大小
     */
    explicit UIElement(glm::vec2 position = {0.0f, 0.0f}, glm::vec2 size = {0.0f, 0.0f});

    /**
     * @brief 虚析构函数，确保派生类正确清理
     */
    virtual ~UIElement() = default;

    // --- 核心虚循环方法 --- (没有使用init和clean，注意构造函数和析构函数的使用)
    virtual bool handleInput(engine::core::Context& context);
    virtual void update(float delta_time, engine::core::Context& context);
    virtual void render(engine::core::Context& context);

    // --- 层次结构管理 ---
    void addChild(std::unique_ptr<UIElement> child);                ///< @brief 添加子元素
    std::unique_ptr<UIElement> removeChild(UIElement* child_ptr);   ///< @brief 将指定子元素从列表中移除，并返回其智能指针
    void removeAllChildren();                                       ///< @brief 移除所有子元素

    // --- Getters and Setters ---
    const glm::vec2& getSize() const { return size_; }              ///< @brief 获取元素大小
    const glm::vec2& getPosition() const { return position_; }      ///< @brief 获取元素位置(相对于父节点)
    bool isVisible() const { return visible_; }                     ///< @brief 检查元素是否可见
    bool isNeedRemove() const { return need_remove_; }              ///< @brief 检查元素是否需要移除
    UIElement* getParent() const { return parent_; }                ///< @brief 获取父元素
    const std::vector<std::unique_ptr<UIElement>>& getChildren() const { return children_; } ///< @brief 获取子元素列表

    void setSize(glm::vec2 size) { size_ = std::move(size); }           ///< @brief 设置元素大小
    void setVisible(bool visible) { visible_ = visible; }           ///< @brief 设置元素的可见性
    void setParent(UIElement* parent) { parent_ = parent; }         ///< @brief 设置父节点
    void setPosition(glm::vec2 position) { position_ = std::move(position); }   ///< @brief 设置元素位置(相对于父节点)
    void setNeedRemove(bool need_remove) { need_remove_ = need_remove; }    ///< @brief 设置元素是否需要移除

    // --- 辅助方法 ---
    engine::utils::Rect getBounds() const;                          ///< @brief 获取(计算)元素的边界(屏幕坐标)
    glm::vec2 getScreenPosition() const;                            ///< @brief 获取(计算)元素在屏幕上位置
    bool isPointInside(const glm::vec2& point) const;               ///< @brief 检查给定点是否在元素的边界内

    // --- 禁用拷贝和移动语义 ---
    UIElement(const UIElement&) = delete;
    UIElement& operator=(const UIElement&) = delete;
    UIElement(UIElement&&) = delete;
    UIElement& operator=(UIElement&&) = delete;
};

} // namespace engine::ui