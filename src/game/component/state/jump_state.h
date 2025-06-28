#pragma once
#include "player_state.h"

namespace game::component::state {

class JumpState final : public PlayerState {
    friend class game::component::PlayerComponent;
public:
    JumpState(PlayerComponent* player_component) : PlayerState(player_component) {}
    ~JumpState() override = default;

private:
    void enter() override;
    void exit() override;
    std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context&) override;

    std::unique_ptr<PlayerState> moveLeft() override;
    std::unique_ptr<PlayerState> moveRight() override;
    std::unique_ptr<PlayerState> climbUp() override;
    std::unique_ptr<PlayerState> climbDown() override;

};

} // namespace game::component::state
