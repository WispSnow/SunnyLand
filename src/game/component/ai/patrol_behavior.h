#pragma once
#include "ai_behavior.h"

namespace game::component::ai {

/**
 * @brief AI 行为：在指定范围内左右巡逻。
 *
 * 遇到墙壁或到达巡逻边界时会转身。
 */
class PatrolBehavior final : public AIBehavior {
    friend class game::component::AIComponent;
private:
    float patrol_min_x_ = 0.0f;     ///< @brief 巡逻范围的左边界
    float patrol_max_x_ = 0.0f;     ///< @brief 巡逻范围的右边界
    float move_speed_ = 50.0f;      ///< @brief 移动速度 (像素/秒)
    bool moving_right_ = false;     ///< @brief 当前是否向右移动

public:
    /**
     * @brief 构造函数。
     * @param min_x 巡逻范围的最小 x 坐标。
     * @param max_x 巡逻范围的最大 x 坐标。
     * @param speed 移动速度。
     */
    PatrolBehavior(float min_x, float max_x, float speed = 50.0f);
    ~PatrolBehavior() override = default;

    // 禁止拷贝和移动
    PatrolBehavior(const PatrolBehavior&) = delete;
    PatrolBehavior& operator=(const PatrolBehavior&) = delete;
    PatrolBehavior(PatrolBehavior&&) = delete;
    PatrolBehavior& operator=(PatrolBehavior&&) = delete;

private:
    void enter(AIComponent& ai_component) override;
    void update(float delta_time, AIComponent& ai_component) override;
};

} // namespace game::component::ai