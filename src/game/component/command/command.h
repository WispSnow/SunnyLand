#pragma once

namespace game::component {
    class PlayerComponent;
}

namespace game::component::command {

/**
 * @brief 命令模式中的命令抽象基类。用于控制玩家移动、跳跃、攀爬等行为。
 */
class Command {
protected:
    PlayerComponent& player_component_;   ///< @brief 所操控的玩家组件的引用
public:
    Command(PlayerComponent& player_component) : player_component_(player_component) {}
    virtual ~Command() = default;

    virtual void execute() = 0;   ///< @brief 执行命令
};

}   // namespace game::component::command