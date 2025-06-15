#pragma once
#include "../../engine/scene/scene.h"
#include "../../engine/ui/ui_image.h"
#include <memory>

// Forward declarations
namespace engine::core {
    class Context;
}
namespace engine::render {
    class Renderer;
    class Camera;
}
namespace engine::input {
    class InputManager;
}
namespace engine::scene {
    class SceneManager;
}

namespace game::scene {

/**
 * @brief 显示帮助信息的场景。
 *
 * 显示一张帮助图片，按鼠标左键退出。
 */
class HelpsScene final : public engine::scene::Scene {
public:
    /**
     * @brief 构造函数。
     * @param context 引擎上下文引用。
     * @param scene_manager 场景管理器引用。
     */
    HelpsScene(engine::core::Context& context, engine::scene::SceneManager& scene_manager);

    ~HelpsScene() override = default;

    // --- 核心方法 ---
    void init() override;
    void handleInput() override;

     // 禁止拷贝和移动
    HelpsScene(const HelpsScene&) = delete;
    HelpsScene& operator=(const HelpsScene&) = delete;
    HelpsScene(HelpsScene&&) = delete;
    HelpsScene& operator=(HelpsScene&&) = delete;
};

} // namespace game::scene