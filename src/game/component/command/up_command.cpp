#include "up_command.h"
#include "../player_component.h"

namespace game::component::command {

void UpCommand::execute() {
    player_component_.climbUp();
}

}   // namespace game::component::command