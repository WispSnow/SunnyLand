#include "left_command.h"
#include "../player_component.h"

namespace game::component::command {

void LeftCommand::execute() {
    player_component_.moveLeft();
}

}   // namespace game::component::command