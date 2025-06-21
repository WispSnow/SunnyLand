#pragma once
#include "../utils/math.h"
#include <optional>

namespace engine::component {
    class TransformComponent;
}

namespace engine::render {

/**
 * @brief 相机类负责管理相机位置和视口大小，并提供坐标转换功能。
 * 它还包含限制相机移动范围的边界。
 */
class Camera final {
private:
    glm::vec2 viewport_size_;                                                ///< @brief 视口大小（屏幕大小）
    glm::vec2 position_;                                                     ///< @brief 相机左上角的世界坐标
    std::optional<engine::utils::Rect> limit_bounds_;                        ///< @brief 限制相机的移动范围，空值表示不限制
    float smooth_speed_ = 5.0f;                                              ///< @brief 相机移动的平滑速度
    engine::component::TransformComponent* target_ = nullptr;                ///< @brief 跟随目标变换组件，空值表示不跟随

public:
    /**
     * @brief 构造相机对象
     * @param viewport_size 视口大小
     * @param position 相机位置
     * @param limit_bounds 限制相机的移动范围
     */
    Camera(glm::vec2 viewport_size, 
           glm::vec2 position = glm::vec2(0.0f, 0.0f), 
           std::optional<engine::utils::Rect> limit_bounds = std::nullopt);
    
    void update(float delta_time);                                          ///< @brief 更新相机位置
    void move(const glm::vec2& offset);                                     ///< @brief 移动相机

    glm::vec2 worldToScreen(const glm::vec2& world_pos) const;              ///< @brief 世界坐标转屏幕坐标
    glm::vec2 worldToScreenWithParallax(const glm::vec2& world_pos, const glm::vec2& scroll_factor) const; ///< @brief 世界坐标转屏幕坐标，考虑视差滚动
    glm::vec2 screenToWorld(const glm::vec2& screen_pos) const;             ///< @brief 屏幕坐标转世界坐标

    void setPosition(glm::vec2 position);                                   ///< @brief 设置相机位置
    void setLimitBounds(std::optional<engine::utils::Rect> limit_bounds);   ///< @brief 设置限制相机的移动范围
    void setTarget(engine::component::TransformComponent* target);          ///< @brief 设置跟随目标变换组件

    const glm::vec2& getPosition() const;                                   ///< @brief 获取相机位置
    std::optional<engine::utils::Rect> getLimitBounds() const;              ///< @brief 获取限制相机的移动范围
    glm::vec2 getViewportSize() const;                                      ///< @brief 获取视口大小
    engine::component::TransformComponent* getTarget() const;               ///< @brief 获取跟随目标变换组件

    // 禁用拷贝和移动语义
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

private:
    void clampPosition();                                                   ///< @brief 限制相机位置在边界内
};

} // namespace engine::render