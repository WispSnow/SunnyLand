#pragma once
#include <memory>
#include <string>
#include <vector>

// 前置声明
namespace engine::core {
    class Context;
}
namespace engine::scene {
    class Scene;
}

namespace engine::scene {

/**
 * @brief 管理游戏中的场景栈，处理场景切换和生命周期。
 */
class SceneManager final {
private:
    engine::core::Context& context_;                        ///< @brief 引擎上下文引用
    std::vector<std::unique_ptr<Scene>> scene_stack_;       ///< @brief 场景栈

    enum class PendingAction { None, Push, Pop, Replace };  ///< @brief 待处理的动作
    PendingAction pending_action_ = PendingAction::None;    ///< @brief 待处理的动作
    std::unique_ptr<Scene> pending_scene_;                  ///< @brief 待处理场景

public:
    explicit SceneManager(engine::core::Context& context);
    ~SceneManager();

    // 禁止拷贝和移动
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

    // 延时切换场景
    void requestPushScene(std::unique_ptr<Scene>&& scene);      ///< @brief 请求压入一个新场景。
    void requestPopScene();                                     ///< @brief 请求弹出当前场景。
    void requestReplaceScene(std::unique_ptr<Scene>&& scene);   ///< @brief 请求替换当前场景。

    // getters
    Scene* getCurrentScene() const;                                 ///< @brief 获取当前活动场景（栈顶场景）的指针。
    engine::core::Context& getContext() const { return context_; }  ///< @brief 获取引擎上下文引用。

    // 核心循环函数
    void update(float delta_time);
    void render();
    void handleInput();
    void close();

private:
    void processPendingActions();                           ///< @brief 处理挂起的场景操作（每轮更新最后调用）。
    // 直接切换场景
    void pushScene(std::unique_ptr<Scene>&& scene);         ///< @brief 将一个新场景压入栈顶，使其成为活动场景。
    void popScene();                                        ///< @brief 移除栈顶场景。
    void replaceScene(std::unique_ptr<Scene>&& scene);      ///< @brief 清理场景栈所有场景，将此场景设为栈顶场景。
    
};

} // namespace engine::scene