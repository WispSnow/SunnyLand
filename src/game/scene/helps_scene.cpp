#include "helps_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/scene/scene_manager.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_image.h"
#include <spdlog/spdlog.h>

namespace game::scene {

HelpsScene::HelpsScene(engine::core::Context& context, engine::scene::SceneManager& scene_manager)
    : engine::scene::Scene("HelpsScene", context, scene_manager) {
    spdlog::trace("HelpsScene 创建.");
}

void HelpsScene::init() {
    if (is_initialized_) {
        return;
    }
    auto window_size = glm::vec2(640.0f, 360.0f);

    // 创建帮助图片 UIImage （让它覆盖整个屏幕）
    auto help_image = std::make_unique<engine::ui::UIImage>(
        "assets/textures/UI/instructions.png",
        glm::vec2(0.0f, 0.0f),
        window_size
    );

    ui_manager_->addElement(std::move(help_image));

    Scene::init();
    spdlog::trace("HelpsScene 初始化完成.");
}

void HelpsScene::handleInput() {
    if (!is_initialized_) return;

    // 检测是否按下鼠标左键
    if (context_.getInputManager().isActionPressed("MouseLeftClick")) {
        spdlog::debug("鼠标左键被按下, 退出 HelpsScene.");
        scene_manager_.requestPopScene();
    }
}

} // namespace game::scene