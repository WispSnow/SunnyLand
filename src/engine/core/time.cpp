#include "time.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_timer.h>    // 用于 SDL_GetTicksNS()

namespace engine::core {

Time::Time() {
    // 初始化 last_time_ 和 frame_start_time_ 为当前时间，避免第一帧 DeltaTime 过大
    last_time_ = SDL_GetTicksNS();
    frame_start_time_ = last_time_;
    spdlog::trace("Time 初始化。Last time: {}", last_time_);
}

void Time::update() {
    
    frame_start_time_ = SDL_GetTicksNS();   // 记录进入 update 时的时间戳
    auto current_delta_time = static_cast<double>(frame_start_time_ - last_time_) / 1000000000.0;
    if (target_frame_time_ > 0.0){      // 如果设置了目标帧率，则限制帧率；否则delta_time_ = current_delta_time
        limitFrameRate(current_delta_time);
    } else {
        delta_time_ = current_delta_time;
    }

    last_time_ = SDL_GetTicksNS(); // 记录离开 update 时的时间戳
}

void Time::limitFrameRate(float current_delta_time) {
    // 如果当前帧耗费的时间小于目标帧时间，则等待剩余时间
    if (current_delta_time < target_frame_time_) {
        double time_to_wait = target_frame_time_ - current_delta_time;
        Uint64 ns_to_wait = static_cast<Uint64>(time_to_wait * 1000000000.0);
        SDL_DelayNS(ns_to_wait);
        delta_time_ = static_cast<double>(SDL_GetTicksNS() - last_time_) / 1000000000.0;
    }
}

float Time::getDeltaTime() const {
    return delta_time_ * time_scale_;
}

float Time::getUnscaledDeltaTime() const {
    return delta_time_;
}

void Time::setTimeScale(float scale) {
    if (scale < 0.0) {
        spdlog::warn("Time scale 不能为负。Clamping to 0.");
        scale = 0.0; // 防止负时间缩放
    }
    time_scale_ = scale;
}

float Time::getTimeScale() const {
    return time_scale_;
}

void Time::setTargetFps(int fps) {
    if (fps < 0) {
        spdlog::warn("Target FPS 不能为负。Setting to 0 (unlimited).");
        target_fps_ = 0;
    } else {
        target_fps_ = fps;
    }

    if (target_fps_ > 0) {
        target_frame_time_ = 1.0 / static_cast<double>(target_fps_);
        spdlog::info("Target FPS 设置为: {} (Frame time: {:.6f}s)", target_fps_, target_frame_time_);
    } else {
        target_frame_time_ = 0.0;
        spdlog::info("Target FPS 设置为: Unlimited");
    }
}

int Time::getTargetFps() const {
    return target_fps_;
}

} // namespace engine::core 