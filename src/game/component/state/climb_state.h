#pragma once
#include "player_state.h"

namespace game::component::state {

class ClimbState final : public PlayerState {
    friend class game::component::PlayerComponent;

    bool is_moving_ = false;    ///< @brief 记录是否正在移动，用于判断是否需要播放动画

public:
    ClimbState(PlayerComponent* player_component) : PlayerState(player_component) {}
    ~ClimbState() override = default;

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
