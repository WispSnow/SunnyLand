#include "animation.h"
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace engine::render {

Animation::Animation(std::string_view name, bool loop)
    : name_(name), loop_(loop) {}

void Animation::addFrame(SDL_FRect source_rect, float duration) {
    if (duration <= 0.0f) {
        spdlog::warn("尝试向动画 '{}' 添加无效持续时间的帧", name_);
        return;
    }
    frames_.push_back({source_rect, duration});
    total_duration_ += duration;
}

const AnimationFrame& Animation::getFrame(float time) const {
    if (frames_.empty()) {
        spdlog::error("动画 '{}' 没有帧，无法获取帧", name_);
        return frames_.back();      // 返回最后一帧（空的）
    }

    float current_time = time;

    if (loop_ && total_duration_ > 0.0f) {
        // 对循环动画使用模运算获取有效时间
        current_time = glm::mod(time, total_duration_);
    } else {
        // 对于非循环动画，如果时间超过总时长，则停留在最后一帧
        if (current_time >= total_duration_) {
            return frames_.back();
        }
    }

    // 遍历帧以找到正确的帧
    float accumulated_time = 0.0f;
    for (const auto& frame : frames_) {
        accumulated_time += frame.duration;
        if (current_time < accumulated_time) {
            return frame;
        }
    }

    // 理论上在不应到达这里，但为了安全起见，返回最后一帧
    spdlog::warn("动画 '{}' 在获取帧信息时出现错误。", name_);
    return frames_.back();
}

} // namespace engine::render 