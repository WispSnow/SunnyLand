#include "scene_manager.h"
#include "scene.h"
#include "../core/context.h"
#include <spdlog/spdlog.h>

namespace engine::scene {

SceneManager::SceneManager(engine::core::Context& context)
    : context_(context) {
    spdlog::trace("场景管理器已创建。");
}

SceneManager::~SceneManager() {
    spdlog::trace("场景管理器已销毁。");
    close(); // 即使不手动调用 close 也能确保清理
}

Scene* SceneManager::getCurrentScene() const {
    if (scene_stack_.empty()) {
        return nullptr;
    }
    return scene_stack_.back().get(); // 返回栈顶场景的裸指针
}

void SceneManager::update(float delta_time) {
    // 只更新栈顶（当前）场景
    Scene* current_scene = getCurrentScene();
    if (current_scene) {
        current_scene->update(delta_time);
    }
    // 执行可能的切换场景操作
    processPendingActions();
}

void SceneManager::render() {
    // 渲染时需要叠加渲染所有场景，而不只是栈顶
    for (const auto& scene : scene_stack_) {
        if (scene) {
            scene->render();
        }
    }
}

void SceneManager::handleInput() {
    // 只考虑栈顶场景
    Scene* current_scene = getCurrentScene();
    if (current_scene) {
        current_scene->handleInput();
    }
}

void SceneManager::close() {
    spdlog::trace("正在关闭场景管理器并清理场景栈...");
    // 清理栈中所有剩余的场景（从顶到底）
    while (!scene_stack_.empty()) {
        if (scene_stack_.back()) {
            spdlog::debug("正在清理场景 '{}' 。", scene_stack_.back()->getName());
            scene_stack_.back()->clean();
        }
        scene_stack_.pop_back();
    }   
}

void SceneManager::requestPopScene()
{
    pending_action_ = PendingAction::Pop;
}

void SceneManager::requestReplaceScene(std::unique_ptr<Scene>&& scene)
{
    pending_action_ = PendingAction::Replace;
    pending_scene_ = std::move(scene);
}

void SceneManager::requestPushScene(std::unique_ptr<Scene>&& scene)
{
    pending_action_ = PendingAction::Push;
    pending_scene_ = std::move(scene);
}

// --- Private Methods ---

void SceneManager::processPendingActions()
{
    if (pending_action_ == PendingAction::None) {
        return;
    }

    switch (pending_action_) {
        case PendingAction::Pop:
            popScene();
            break;
        case PendingAction::Replace:
            replaceScene(std::move(pending_scene_));
            break;
        case PendingAction::Push:
            pushScene(std::move(pending_scene_));
            break;
        default:
            break;
    }

    pending_action_ = PendingAction::None;
}

void SceneManager::pushScene(std::unique_ptr<Scene>&& scene) {
    if (!scene) {
        spdlog::warn("尝试将空场景压入栈。");
        return;
    }
    spdlog::debug("正在将场景 '{}' 压入栈。", scene->getName());

    // 初始化新场景
    if (!scene->isInitialized()) { // 确保只初始化一次
        scene->init(); 
    }

    // 将新场景移入栈顶
    scene_stack_.push_back(std::move(scene));
}

void SceneManager::popScene() {
    if (scene_stack_.empty()) {
        spdlog::warn("尝试从空场景栈中弹出。");
        return;
    }
    spdlog::debug("正在从栈中弹出场景 '{}' 。", scene_stack_.back()->getName());

    // 清理并移除栈顶场景
    if (scene_stack_.back()) {
        scene_stack_.back()->clean();       // 显式调用清理
    }
    scene_stack_.pop_back();
}

void SceneManager::replaceScene(std::unique_ptr<Scene>&& scene) {
    if (!scene) {
        spdlog::warn("尝试用空场景替换。");
        return;
    }
    spdlog::debug("正在用场景 '{}' 替换场景 '{}' 。", scene->getName(), scene_stack_.back()->getName());

    // 清理并移除场景栈中所有场景
    while (!scene_stack_.empty()) {
        if (scene_stack_.back()) {
            scene_stack_.back()->clean();
        }
        scene_stack_.pop_back();
    }

    // 初始化新场景
    if (!scene->isInitialized()) {
        scene->init();
    }

    // 将新场景压入栈顶
    scene_stack_.push_back(std::move(scene));
}

} // namespace engine::scene