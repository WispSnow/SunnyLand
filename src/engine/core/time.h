#pragma once
#include <SDL3/SDL_stdinc.h>    // 用于 Uint64

namespace engine::core {

/**
 * @brief 管理游戏循环中的时间，计算帧间时间差 (DeltaTime)。
 *
 * 使用 SDL 的高精度性能计数器来确保时间测量的准确性。
 * 提供获取缩放和未缩放 DeltaTime 的方法，以及设置时间缩放因子的能力。
 */
class Time final{
private:
    Uint64 last_time_ = 0;         ///< @brief 上一帧的时间戳 (用于计算 delta)
    Uint64 frame_start_time_ = 0;  ///< @brief 当前帧开始的时间戳 (用于帧率限制)
    double delta_time_ = 0.0;      ///< @brief 未缩放的帧间时间差 (秒)
    double time_scale_ = 1.0;      ///< @brief 时间缩放因子

    // 帧率限制相关
    int target_fps_ = 0;             ///< @brief 目标 FPS (0 表示不限制)
    double target_frame_time_ = 0.0; ///< @brief 目标每帧时间 (秒)

public:
    Time();

    // 简单起见，直接删除拷贝、移动和赋值运算符
    Time(const Time&) = delete;
    Time& operator=(const Time&) = delete;
    Time(Time&&) = delete;
    Time& operator=(Time&&) = delete;

    /**
     * @brief 每帧开始时调用，更新内部时间状态并计算 DeltaTime。
     */
    void update();

    /**
     * @brief 获取经过时间缩放调整后的帧间时间差 (DeltaTime)。
     *
     * @return double 缩放后的 DeltaTime (秒)。
     */
    float getDeltaTime() const;

    /**
     * @brief 获取未经过时间缩放的原始帧间时间差。
     *
     * @return double 未缩放的 DeltaTime (秒)。
     */
    float getUnscaledDeltaTime() const;

    /**
     * @brief 设置时间缩放因子。
     *
     * @param scale 时间缩放值。1.0 为正常速度，< 1.0 为慢动作，> 1.0 为快进。
     *              不允许负值。
     */
    void setTimeScale(float scale);

    /**
     * @brief 获取当前的时间缩放因子。
     *
     * @return float 当前的时间缩放因子。
     */
    float getTimeScale() const;

    /**
     * @brief 设置目标帧率。
     *
     * @param fps 目标每秒帧数。设置为 0 表示不限制帧率。负值将被视为 0。
     */
    void setTargetFps(int fps);

    /**
     * @brief 获取当前设置的目标帧率。
     *
     * @return int 目标 FPS，0 表示不限制。
     */
    int getTargetFps() const;

private:
    /**
     * @brief update 中调用，用于限制帧率。如果设置了 target_fps_ > 0，且当前帧执行时间小于目标帧时间，则会调用 SDL_DelayNS() 来等待剩余时间。
     * 
     * @param current_delta_time 当前帧的执行时间（秒）
     */
    void limitFrameRate(float current_delta_time);
};

} // namespace engine::core