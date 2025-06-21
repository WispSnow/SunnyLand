#pragma once
#include <vector>
#include <memory>
#include <string>
#include <string_view>

namespace engine::core {
    class Context;
}

namespace engine::ui {
    class UIManager;
}

namespace engine::object {
    class GameObject;
}

namespace engine::scene {
    class SceneManager;

/**
 * @brief 场景基类，负责管理场景中的游戏对象和场景生命周期。
 *
 * 包含一组游戏对象，并提供更新、渲染、处理输入和清理的接口。
 * 派生类应实现具体的场景逻辑。
 */
class Scene {
protected:
    std::string scene_name_;                            ///< @brief 场景名称
    engine::core::Context& context_;                    ///< @brief 上下文引用（隐式，构造时传入）
    engine::scene::SceneManager& scene_manager_;        ///< @brief 场景管理器引用（构造时传入）
    std::unique_ptr<engine::ui::UIManager> ui_manager_; ///< @brief UI管理器(初始化时自动创建)
    
    bool is_initialized_ = false;                       ///< @brief 场景是否已初始化(非当前场景很可能未被删除，因此需要初始化标志避免重复初始化)
    std::vector<std::unique_ptr<engine::object::GameObject>> game_objects_;         ///< @brief 场景中的游戏对象
    std::vector<std::unique_ptr<engine::object::GameObject>> pending_additions_;    ///< @brief 待添加的游戏对象（延时添加）

public:
    /**
     * @brief 构造函数。
     *
     * @param name 场景的名称。
     * @param context 场景上下文。
     * @param scene_manager 场景管理器。
     */
    Scene(std::string_view name, engine::core::Context& context, engine::scene::SceneManager& scene_manager);

    virtual ~Scene();           // 1. 基类必须声明虚析构函数才能让派生类析构函数被正确调用。
                                // 2. 析构函数定义必须写在cpp中，不然需要引入GameObject头文件

    // 禁止拷贝和移动构造
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

    // 核心循环方法
    virtual void init();                        ///< @brief 初始化场景。
    virtual void update(float delta_time);      ///< @brief 更新场景。
    virtual void render();                      ///< @brief 渲染场景。
    virtual void handleInput();                 ///< @brief 处理输入。
    virtual void clean();                       ///< @brief 清理场景。

    /// @brief 直接向场景中添加一个游戏对象。（初始化时可用，游戏进行中不安全） （&&表示右值引用，与std::move搭配使用，避免拷贝）
    virtual void addGameObject(std::unique_ptr<engine::object::GameObject>&& game_object);

    /// @brief 安全地添加游戏对象。（添加到pending_additions_中）
    virtual void safeAddGameObject(std::unique_ptr<engine::object::GameObject>&& game_object); 

    /// @brief 直接从场景中移除一个游戏对象。（一般不使用，但保留实现的逻辑）
    virtual void removeGameObject(engine::object::GameObject* game_object_ptr);

    /// @brief 安全地移除游戏对象。（设置need_remove_标记）
    virtual void safeRemoveGameObject(engine::object::GameObject* game_object_ptr);

    /// @brief 获取场景中的游戏对象容器。
    const std::vector<std::unique_ptr<engine::object::GameObject>>& getGameObjects() const { return game_objects_; }

    /// @brief 根据名称查找游戏对象（返回找到的第一个对象）。
    engine::object::GameObject* findGameObjectByName(std::string_view name) const;

    // getters and setters
    void setName(std::string_view name) { scene_name_ = name; }               ///< @brief 设置场景名称
    std::string_view getName() const { return scene_name_; }                  ///< @brief 获取场景名称
    void setInitialized(bool initialized) { is_initialized_ = initialized; }    ///< @brief 设置场景是否已初始化
    bool isInitialized() const { return is_initialized_; }                      ///< @brief 获取场景是否已初始化

    engine::core::Context& getContext() const { return context_; }                  ///< @brief 获取上下文引用
    engine::scene::SceneManager& getSceneManager() const { return scene_manager_; } ///< @brief 获取场景管理器引用
    std::vector<std::unique_ptr<engine::object::GameObject>>& getGameObjects() { return game_objects_; } ///< @brief 获取场景中的游戏对象

protected:
    void processPendingAdditions();     ///< @brief 处理待添加的游戏对象。（每轮更新的最后调用）
};

} // namespace engine::scene