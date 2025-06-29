#include "down_command.h"
#include "../player_component.h"

namespace game::component::command {

void DownCommand::execute() {
    player_component_.climbDown();
}

}   // namespace game::component::command