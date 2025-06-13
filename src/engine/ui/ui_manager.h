#pragma once
#include <memory>
#include <vector>
#include <glm/vec2.hpp>

namespace engine::core {
    class Context;
}
namespace engine::ui {
    class UIElement;
    class UIPanel; // UIPanel 将作为根元素
}

namespace engine::ui {

/**
 * @brief 管理特定场景中的UI元素集合。
 *
 * 负责UI元素的生命周期管理（通过根元素）、渲染调用和输入事件分发。
 * 每个需要UI的场景（如菜单、游戏HUD）应该拥有一个UIManager实例。
 */
class UIManager final {
private:
    std::unique_ptr<UIPanel> root_element_;     ///< @brief 一个UIPanel作为根节点(UI元素)

public:
    UIManager();        ///< @brief 构造函数将创建默认的根节点。
    ~UIManager();


    [[nodiscard]] bool init(const glm::vec2& window_size);  ///< @brief 初始化UI管理器，设置根元素的大小。
    void addElement(std::unique_ptr<UIElement> element);    ///< @brief 添加一个UI元素到根节点的child_容器中。
    UIPanel* getRootElement() const;                        ///< @brief 获取根UIPanel元素的指针。
    void clearElements();                                   ///< @brief 清除所有UI元素，通常用于重置UI状态。

    // --- 核心循环方法 ---
    bool handleInput(engine::core::Context&);               ///< @brief 处理输入事件，如果事件被处理则返回true。
    void update(float delta_time, engine::core::Context&);
    void render(engine::core::Context&);

    // 禁止拷贝和移动构造/赋值
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;
    UIManager(UIManager&&) = delete;
    UIManager& operator=(UIManager&&) = delete;

};

} // namespace engine::ui