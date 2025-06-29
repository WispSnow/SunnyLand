#pragma once
#include "command.h"

namespace game::component::command {

/**
 * @brief 跳跃的命令。
 */
class JumpCommand : public Command {
public:
    JumpCommand(PlayerComponent& player_component) : Command(player_component) {}
    void execute() override;
};

}   // namespace game::component::command