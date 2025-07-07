#include "scene.h"
#include "scene_manager.h"
#include "../object/game_object.h"
#include "../core/context.h"
#include "../core/game_state.h"
#include "../physics/physics_engine.h"
#include "../render/camera.h"
#include "../ui/ui_manager.h"
#include <algorithm> // for std::remove_if
#include <spdlog/spdlog.h>

namespace engine::scene {

Scene::Scene(std::string_view name, engine::core::Context& context, engine::scene::SceneManager& scene_manager)
    : scene_name_(name),
      context_(context), 
      scene_manager_(scene_manager), 
      ui_manager_(std::make_unique<engine::ui::UIManager>()),
      is_initialized_(false) {
    spdlog::trace("场景 '{}' 构造完成。", scene_name_);
}

Scene::~Scene() = default;

void Scene::init() {
    is_initialized_ = true;     // 子类应该最后调用父类的 init 方法
    spdlog::trace("场景 '{}' 初始化完成。", scene_name_);
}

void Scene::update(float delta_time) {
    if (!is_initialized_) return;
    
    // 只有游戏进行中，才需要更新物理引擎和相机
    if (context_.getGameState().isPlaying()){
        context_.getPhysicsEngine().update(delta_time);
        context_.getCamera().update(delta_time);
    }

    bool need_remove = false;  // 设定一个标志，用于判断是否需要移除对象

    // 更新所有游戏对象，先略过需要移除的对象
    for (auto& obj : game_objects_) {
        if (obj && !obj->isNeedRemove()) {
            obj->update(delta_time, context_);
        } else {
            need_remove = true;
            if (obj) obj->clean();  // 如果对象需要移除，则先调用clean方法
            else spdlog::warn("尝试更新一个空的游戏对象指针。");
        }
    }

    if (need_remove) {
        // 使用C++20新添加的erase_if删除需要移除的对象，比使用erase - remove_if更简洁
        // NOTE: 用此语句则没有机会调用clean方法，因此要在update中先调用clean方法
        std::erase_if(game_objects_, [](const std::unique_ptr<engine::object::GameObject>& obj) {
            return !obj || obj->isNeedRemove();
        });
    }

    // 更新UI管理器
    ui_manager_->update(delta_time, context_);

    processPendingAdditions();      // 处理待添加（延时添加）的游戏对象
}

void Scene::render() {
     if (!is_initialized_) return;
    // 渲染所有游戏对象
    for (const auto& obj : game_objects_) {
        if (obj) obj->render(context_);
    }

    // 渲染UI管理器
    ui_manager_->render(context_);
}

void Scene::handleInput() {
     if (!is_initialized_) return;

    // 处理UI管理器输入
    if (ui_manager_->handleInput(context_)) return;   // 如果输入事件被UI处理则返回，不再处理游戏对象输入
    
    // 遍历所有游戏对象，略过需要移除的对象
    for (auto& obj : game_objects_) {
        if (obj && !obj->isNeedRemove()) {
            obj->handleInput(context_);
        }
    }
    // 不在这里移除，以免浪费算力。在update中移除
}

void Scene::clean() {
    if (!is_initialized_) return;
    
    for (const auto& obj : game_objects_) {
        if (obj) obj->clean();
    }
    game_objects_.clear();

    is_initialized_ = false;        // 清理完成后，设置场景为未初始化
    spdlog::trace("场景 '{}' 清理完成。", scene_name_);
}

void Scene::addGameObject(std::unique_ptr<engine::object::GameObject>&& game_object) {
    if (game_object) game_objects_.push_back(std::move(game_object));
    else spdlog::warn("尝试向场景 '{}' 添加空游戏对象。", scene_name_);
}

void Scene::safeAddGameObject(std::unique_ptr<engine::object::GameObject>&& game_object)
{
    if (game_object) pending_additions_.push_back(std::move(game_object));
    else spdlog::warn("尝试向场景 '{}' 添加空游戏对象。", scene_name_);
}

void Scene::removeGameObject(engine::object::GameObject* game_object_ptr) {
    if (!game_object_ptr) {
        spdlog::warn("尝试从场景 '{}' 中移除一个空的游戏对象指针。", scene_name_);
        return;
    }

    // erase-remove 移除法不可用，因为智能指针与裸指针无法比较
    // 需要使用 std::remove_if 和 lambda 表达式自定义比较的方式
    auto it = std::remove_if(game_objects_.begin(), game_objects_.end(),
                             [game_object_ptr](const std::unique_ptr<engine::object::GameObject>& p) {
                                 return p.get() == game_object_ptr;    // 比较裸指针是否相等（自定义比较方式）
                             });

    if (it != game_objects_.end()) {
        (*it)->clean();             // 因为传入的是指针，因此只可能有一个元素被移除，不需要遍历it到末尾
        game_objects_.erase(it, game_objects_.end());   // 删除从it到末尾的元素（最后一个元素）
        spdlog::trace("从场景 '{}' 中移除游戏对象。", scene_name_);
    } else {
        spdlog::warn("游戏对象指针未找到在场景 '{}' 中。", scene_name_);
    }
}

void Scene::safeRemoveGameObject(engine::object::GameObject* game_object_ptr)
{
    game_object_ptr->setNeedRemove(true);
}

engine::object::GameObject *Scene::findGameObjectByName(std::string_view name) const
{
    // 找到第一个符合条件的游戏对象就返回
    for (const auto& obj : game_objects_) {
        if (obj && obj->getName() == name) {
            return obj.get();
        }
    }
    return nullptr;
}

void Scene::processPendingAdditions()
{
    // 处理待添加的游戏对象
    for (auto& game_object : pending_additions_) {
        addGameObject(std::move(game_object));
    }
    pending_additions_.clear();
}

} // namespace engine::scene 