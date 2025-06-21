#pragma once
#include "ui_interactive.h"
#include <functional>
#include <utility>

namespace engine::ui {

/**
 * @brief 按钮UI元素
 * 
 * 继承自UIInteractive，用于创建可交互的按钮。
 * 支持三种状态：正常、悬停、按下。
 * 支持回调函数，当按钮被点击时调用。
 */
class UIButton final: public UIInteractive {
private:
    std::function<void()> callback_;        ///< @brief 可自定义的函数（函数包装器）

public:
    /**
     * @brief 构造函数
     * @param normal_sprite_id 正常状态的精灵ID
     * @param hover_sprite_id 悬停状态的精灵ID
     * @param pressed_sprite_id 按下状态的精灵ID
     * @param position 位置
     * @param size 大小
     * @param callback 回调函数
     */
    UIButton(engine::core::Context& context,
             std::string_view normal_sprite_id,
             std::string_view hover_sprite_id,
             std::string_view pressed_sprite_id,
             glm::vec2 position = {0.0f, 0.0f}, 
             glm::vec2 size = {0.0f, 0.0f}, 
             std::function<void()> callback = nullptr);
    ~UIButton() override = default;

    void clicked() override;     ///< @brief 重写基类方法，当按钮被点击时调用回调函数

    void setCallback(std::function<void()> callback) { callback_ = std::move(callback); }   ///< @brief 设置点击回调函数
    std::function<void()> getCallback() const { return callback_; }                         ///< @brief 获取点击回调函数

};

} // namespace engine::ui