#pragma once
// 前置声明核心系统
namespace engine::input {
    class InputManager;
}

namespace engine::render {
    class Renderer;
    class Camera;
}

namespace engine::resource {
    class ResourceManager;
}

namespace engine::core {

/**
 * @brief 持有对核心引擎模块引用的上下文对象。
 *
 * 用于简化依赖注入，传递Context即可获取引擎的各个模块。
 */
class Context final {
private:
    // 使用引用，确保每个模块都有效，使用时不需要检查指针是否为空。
    engine::input::InputManager& input_manager_;            ///< @brief 输入管理器
    engine::render::Renderer& renderer_;                    ///< @brief 渲染器
    engine::render::Camera& camera_;                        ///< @brief 相机
    engine::resource::ResourceManager& resource_manager_;   ///< @brief 资源管理器

public:
    /**
     * @brief 构造函数。
     * @param input_manager 对 InputManager 实例的引用。
     * @param renderer 对 Renderer 实例的引用。
     * @param camera 对 Camera 实例的引用。
     * @param resource_manager 对 ResourceManager 实例的引用。
     */
    Context(engine::input::InputManager& input_manager,
            engine::render::Renderer& renderer,
            engine::render::Camera& camera,
            engine::resource::ResourceManager& resource_manager);

    // 禁止拷贝和移动，Context 对象通常是唯一的或按需创建/传递
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;

    // --- Getters ---
    engine::input::InputManager& getInputManager() const { return input_manager_; }             ///< @brief 获取输入管理器
    engine::render::Renderer& getRenderer() const { return renderer_; }                         ///< @brief 获取渲染器
    engine::render::Camera& getCamera() const { return camera_; }                               ///< @brief 获取相机
    engine::resource::ResourceManager& getResourceManager() const { return resource_manager_; } ///< @brief 获取资源管理器

};

} // namespace engine::core