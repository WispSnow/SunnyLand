#pragma once
#include <memory>

// 前向声明, 减少头文件的依赖，增加编译速度
struct SDL_Window;
struct SDL_Renderer;

namespace engine::resource {
class ResourceManager;
}

namespace engine::render {
class Renderer;
class Camera;
}

namespace engine::input {
class InputManager;
}

namespace engine::physics {
class PhysicsEngine;
}

namespace engine::scene {
class SceneManager;
}

namespace engine::core {        // 命名空间的最佳实践：与文件路径一致
class Time;
class Config;
class Context;

/**
 * @brief 主游戏应用程序类，初始化SDL，管理游戏循环。
 */
class GameApp final {   // final 表示该类不能被继承
private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* sdl_renderer_ = nullptr;
    bool is_running_ = false;

    // 引擎组件
    std::unique_ptr<engine::core::Time> time_;
    std::unique_ptr<engine::resource::ResourceManager> resource_manager_;
    std::unique_ptr<engine::render::Renderer> renderer_;
    std::unique_ptr<engine::render::Camera> camera_;
    std::unique_ptr<engine::core::Config> config_;
    std::unique_ptr<engine::input::InputManager> input_manager_;
    std::unique_ptr<engine::core::Context> context_;
    std::unique_ptr<engine::scene::SceneManager> scene_manager_;
    std::unique_ptr<engine::physics::PhysicsEngine> physics_engine_;

public:
    GameApp();
    ~GameApp();

    /**
     * @brief 运行游戏应用程序，其中会调用init()，然后进入主循环，离开循环后自动调用close()。
     */
    void run();

    // 禁止拷贝和移动
    GameApp(const GameApp&) = delete;
    GameApp& operator=(const GameApp&) = delete;
    GameApp(GameApp&&) = delete;
    GameApp& operator=(GameApp&&) = delete;

private:
    [[nodiscard]] bool init();      // nodiscard 表示该函数返回值不应该被忽略
    void handleEvents();
    void update(float delta_time);
    void render();
    void close();

    // 各模块的初始化/创建函数，在init()中调用
    [[nodiscard]] bool initConfig();
    [[nodiscard]] bool initSDL();
    [[nodiscard]] bool initTime();
    [[nodiscard]] bool initResourceManager();
    [[nodiscard]] bool initRenderer();
    [[nodiscard]] bool initCamera();
    [[nodiscard]] bool initInputManager();
    [[nodiscard]] bool initPhysicsEngine();
    [[nodiscard]] bool initContext();
    [[nodiscard]] bool initSceneManager();
};

} // namespace engine::core

