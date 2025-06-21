#include "game_object.h"
#include "../render/renderer.h"
#include "../input/input_manager.h" 
#include "../render/camera.h"
#include <spdlog/spdlog.h>

namespace engine::object {
GameObject::GameObject(std::string_view name, std::string_view tag): name_(name), tag_(tag)
{
    spdlog::trace("GameObject created: {} {}", name_, tag_);
}

void GameObject::update(float delta_time, engine::core::Context& context) {
    // 遍历所有组件并调用它们的 update 方法
    for (auto& pair : components_) {
        pair.second->update(delta_time, context);
    }
}

void GameObject::render(engine::core::Context& context) {
    // 遍历所有组件并调用它们的 render 方法
    for (auto& pair : components_) {
        pair.second->render(context);
    }
}

void GameObject::clean() {
    spdlog::trace("Cleaning GameObject...");
    // 遍历所有组件并调用它们的 clean 方法
    for (auto& pair : components_) {
        pair.second->clean();
    }
    components_.clear(); // 清空 map, unique_ptr 会自动释放内存
}

void GameObject::handleInput(engine::core::Context& context) {
    // 遍历所有组件并调用它们的 handleInput 方法
    for (auto& pair : components_) {
        pair.second->handleInput(context);
    }
}


} // namespace engine::object 