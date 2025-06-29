#pragma once
#include "command.h"

namespace game::component::command {

/**
 * @brief 向右移动的命令。
 */
class RightCommand : public Command {
public:
    RightCommand(PlayerComponent& player_component) : Command(player_component) {}
    void execute() override;
};

}   // namespace game::component::command