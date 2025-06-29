#pragma once
#include "command.h"

namespace game::component::command {

/**
 * @brief 向上移动的命令。
 */
class UpCommand : public Command {
public:
    UpCommand(PlayerComponent& player_component) : Command(player_component) {}
    void execute() override;
};

}   // namespace game::component::command