#include "jump_command.h"
#include "../player_component.h"

namespace game::component::command {

void JumpCommand::execute() {
    player_component_.jump();
}

}   // namespace game::component::command