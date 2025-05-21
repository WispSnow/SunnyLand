#include "camera.h"
#include "../utils/math.h"
#include <spdlog/spdlog.h>

namespace engine::render {

Camera::Camera(const glm::vec2& viewport_size, const glm::vec2& position, const std::optional<engine::utils::Rect> limit_bounds)
    : viewport_size_(viewport_size), position_(position), limit_bounds_(limit_bounds) {
    spdlog::trace("Camera 初始化成功，位置: {},{}", position_.x, position_.y);
}

void Camera::setPosition(const glm::vec2& position) {
    position_ = position;
    clampPosition();
}

void Camera::update(float /* delta_time */)
{
    // TODO 自动跟随目标
}

void Camera::move(const glm::vec2 &offset)
{
    position_ += offset;
    clampPosition();
}

void Camera::setLimitBounds(const engine::utils::Rect& bounds)
{
    limit_bounds_ = bounds;
    clampPosition(); // 设置边界后，立即应用限制
}

const glm::vec2& Camera::getPosition() const {
    return position_;
}

void Camera::clampPosition()
{
    // 边界检查需要确保相机视图（position 到 position + viewport_size）在 limit_bounds 内
    if (limit_bounds_.has_value() && limit_bounds_->size.x > 0 && limit_bounds_->size.y > 0) {
        // 计算允许的相机位置范围
        glm::vec2 min_cam_pos = limit_bounds_->position;
        glm::vec2 max_cam_pos = limit_bounds_->position + limit_bounds_->size - viewport_size_;

        // 确保 max_cam_pos 不小于 min_cam_pos (视口可能比世界还大)
        max_cam_pos.x = std::max(min_cam_pos.x, max_cam_pos.x);
        max_cam_pos.y = std::max(min_cam_pos.y, max_cam_pos.y);

        position_ = glm::clamp(position_, min_cam_pos, max_cam_pos);
    }
    // 如果 limit_bounds 无效则不进行限制
}

glm::vec2 Camera::worldToScreen(const glm::vec2& world_pos) const {
    // 将世界坐标减去相机左上角位置
    return world_pos - position_;
}

glm::vec2 Camera::worldToScreenWithParallax(const glm::vec2 &world_pos, const glm::vec2 &scroll_factor) const
{
    // 相机位置应用滚动因子
    return world_pos - position_ * scroll_factor;
}

glm::vec2 Camera::screenToWorld(const glm::vec2 &screen_pos) const
{
    // 将屏幕坐标加上相机左上角位置
    return screen_pos + position_;
}

glm::vec2 Camera::getViewportSize() const {
    return viewport_size_;
}

std::optional<engine::utils::Rect> Camera::getLimitBounds() const {
    return limit_bounds_;
}

} // namespace engine::render 