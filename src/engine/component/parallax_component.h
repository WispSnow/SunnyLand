#pragma once
#include "component.h"
#include "../render/sprite.h"
#include <string>
#include <string_view>
#include <glm/vec2.hpp>

namespace engine::component {
class TransformComponent;

/**
 * @brief 在背景中渲染可滚动纹理的组件，以创建视差效果。
 *
 * 该组件根据相机的位置和滚动因子来移动纹理。
 */
class ParallaxComponent final : public Component {
    friend class engine::object::GameObject;
private:
    TransformComponent* transform_ = nullptr;   ///< @brief 缓存变换组件

    engine::render::Sprite sprite_;             ///< @brief 精灵对象
    glm::vec2 scroll_factor_;                   ///< @brief 滚动速度因子 (0=静止, 1=随相机移动, <1=比相机慢)
    glm::bvec2 repeat_;                         ///< @brief 是否沿着X和Y轴周期性重复
    bool is_hidden_ = false;                    ///< @brief 是否隐藏（不渲染）

public:
    /**
     * @brief 构造函数
     * @param texture_id 背景纹理的资源 ID。
     * @param scroll_factor 控制背景相对于相机移动速度的因子。
     *                      (0, 0) 表示完全静止。
     *                      (1, 1) 表示与相机完全同步移动。
     *                      (0.5, 0.5) 表示以相机一半的速度移动。
     */
    ParallaxComponent(std::string_view texture_id, glm::vec2 scroll_factor, glm::bvec2 repeat);

    // --- 设置器 ---
    void setSprite(const engine::render::Sprite& sprite) { sprite_ = sprite; }  ///< @brief 设置精灵对象
    void setScrollFactor(glm::vec2 factor) { scroll_factor_ = std::move(factor); }  ///< @brief 设置滚动速度因子
    void setRepeat(glm::bvec2 repeat) { repeat_ = std::move(repeat); }              ///< @brief 设置是否重复
    void setHidden(bool hidden) { is_hidden_ = hidden; }                        ///< @brief 设置是否隐藏（不渲染）

    // --- 获取器 ---
    const engine::render::Sprite& getSprite() const { return sprite_; }          ///< @brief 获取精灵对象
    const glm::vec2& getScrollFactor() const { return scroll_factor_; }          ///< @brief 获取滚动速度因子
    const glm::bvec2& getRepeat() const { return repeat_; }                      ///< @brief 获取是否重复
    bool isHidden() const { return is_hidden_; }                                  ///< @brief 获取是否隐藏（不渲染）

protected:
    // 核心循环函数覆盖
    void update(float, engine::core::Context&) override {}     // 必须实现纯虚函数，留空
    void init() override;                                      
    void render(engine::core::Context& context) override;      
};

} // namespace engine::component