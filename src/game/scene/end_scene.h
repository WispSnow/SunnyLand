#pragma once
#include "../../engine/scene/scene.h"
#include <memory>
#include <string>

// 前置声明
namespace engine::core {
    class Context;
}
namespace engine::scene {
    class SceneManager;
}
namespace game::data {
    class SessionData;
}
namespace engine::ui {
    class UIManager;
    class Label;
    class Button;
}
namespace engine::input {
    class InputManager;
}
namespace engine::render{
    class Renderer;
    class Camera;
}


namespace game::scene {

/**
 * @class EndScene
 * @brief 显示游戏结束（胜利或失败）信息的叠加场景。
 *
 * 提供重新开始或返回主菜单的选项。
 */
class EndScene final : public engine::scene::Scene {
private:
    std::shared_ptr<game::data::SessionData> session_data_;

public:
    /**
     * @brief 构造函数
     * @param context 引擎上下文
     * @param scene_manager 场景管理器
     * @param session_data 指向游戏数据状态的共享指针
     */
    EndScene(engine::core::Context& context,
             engine::scene::SceneManager& scene_manager,
             std::shared_ptr<game::data::SessionData> session_data);

    ~EndScene() override = default;

    // --- 核心循环方法 ---
    void init() override;

    // 禁止拷贝和移动
    EndScene(const EndScene&) = delete;
    EndScene& operator=(const EndScene&) = delete;
    EndScene(EndScene&&) = delete;
    EndScene& operator=(EndScene&&) = delete;

private:
    // 用于创建UI和按钮回调的私有辅助方法
    void createUI();
    void onBackClick();
    void onRestartClick();
};

} // namespace game::scene
