#pragma once
#include <memory>
#include <string>

namespace engine::core {
    class Context;
}

namespace game::component {
    class PlayerComponent;
}

namespace game::component::state {

/**
 * @brief 玩家状态机的抽象基类。
 */
class PlayerState {
    friend class game::component::PlayerComponent;
protected:
    PlayerComponent* player_component_ = nullptr;   ///< @brief 指向拥有此状态的玩家组件

public:
    PlayerState(PlayerComponent* player_component) : player_component_(player_component) {}
    virtual ~PlayerState() = default;

    // 禁止拷贝和移动
    PlayerState(const PlayerState&) = delete;
    PlayerState& operator=(const PlayerState&) = delete;
    PlayerState(PlayerState&&) = delete;
    PlayerState& operator=(PlayerState&&) = delete;

    void playAnimation(const std::string& animation_name);      ///< @brief 播放指定名称的动画，使用 AnimationComponent 的方法

    // --- 不同状态下的操作逻辑，子类需要什么就实现什么 ---
    virtual std::unique_ptr<PlayerState> moveLeft() { return nullptr; }
    virtual std::unique_ptr<PlayerState> moveRight() { return nullptr; }
    virtual std::unique_ptr<PlayerState> jump() { return nullptr; }
    virtual std::unique_ptr<PlayerState> climbUp() { return nullptr; }
    virtual std::unique_ptr<PlayerState> climbDown() { return nullptr; }

protected:
    // 核心状态方法
    virtual void enter() = 0;       ///< @brief 进入
    virtual void exit() = 0;        ///< @brief 离开
    virtual std::unique_ptr<PlayerState> update(float, engine::core::Context&) = 0; ///< @brief 更新
    /* update 返回值为下一个状态，如果不需要切换状态，则返回 nullptr （不再需要 handleInput 方法）*/

};

} // namespace game::component::state