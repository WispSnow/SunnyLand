#pragma once
#include "command.h"

namespace game::component::command {

/**
 * @brief 向左移动的命令。
 */
class LeftCommand : public Command {
public:
    LeftCommand(PlayerComponent& player_component) : Command(player_component) {}
    void execute() override;
};

}   // namespace game::component::command