#pragma once
#include "player_state.h"

namespace game::component::state {

class IdleState final : public PlayerState {
    friend class game::component::PlayerComponent;
public:
    IdleState(PlayerComponent* player_component) : PlayerState(player_component) {}
    ~IdleState() override = default;

private:
    void enter() override;
    void exit() override;
    std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context&) override;

    std::unique_ptr<PlayerState> moveLeft() override;
    std::unique_ptr<PlayerState> moveRight() override;
    std::unique_ptr<PlayerState> jump() override;
    std::unique_ptr<PlayerState> climbUp() override;
    std::unique_ptr<PlayerState> climbDown() override;
};

} // namespace game::component::state
