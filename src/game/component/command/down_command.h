#pragma once
#include "command.h"

namespace game::component::command {

/**
 * @brief 向下移动的命令。
 */
class DownCommand : public Command {
public:
    DownCommand(PlayerComponent& player_component) : Command(player_component) {}
    void execute() override;
};

}   // namespace game::component::command