#pragma once
#include "../../engine/component/component.h"

namespace engine::component {

/**
 * @brief 管理 GameObject 的生命值，处理伤害、治疗，并提供无敌帧功能。
 */
class HealthComponent final : public engine::component::Component {
    friend class engine::object::GameObject;
private:
    int max_health_ = 1;                    ///< @brief 最大生命值
    int current_health_ = 1;                ///< @brief 当前生命值
    bool is_invincible_ = false;            ///< @brief 是否处于无敌状态
    float invincibility_duration_ = 2.0f;   ///< @brief 受伤后无敌的总时长（秒）
    float invincibility_timer_ = 0.0f;      ///< @brief 无敌时间计时器（秒）

public:
    /**
     * @brief 构造函数
     * @param max_health 最大生命值，默认为 1
     * @param invincibility_duration 无敌状态持续时间，默认为 2.0 秒
     */
    explicit HealthComponent(int max_health = 1, float invincibility_duration = 2.0f);
    ~HealthComponent() override = default;

    // 禁止拷贝和移动
    HealthComponent(const HealthComponent&) = delete;
    HealthComponent& operator=(const HealthComponent&) = delete;
    HealthComponent(HealthComponent&&) = delete;
    HealthComponent& operator=(HealthComponent&&) = delete;

    /**
     * @brief 对 GameObject 施加伤害。
     *        如果当前处于无敌状态，则伤害无效。
     *        如果成功造成伤害且设置了无敌时长，则会触发无敌帧。
     * @param damage_amount 造成的伤害量（应为正数）。
     * @return bool 如果成功造成伤害，则返回 true，否则返回 false。
     */
    bool takeDamage(int damage_amount);
    int heal(int heal_amount);             ///< @brief 治疗 GameObject，增加当前生命值（不超过最大生命值）,返回治疗后生命值

    // --- Getters and Setters ---
    bool isAlive() const { return current_health_ > 0; }        ///< @brief 检查 GameObject 是否存活（当前生命值大于 0）。
    bool isInvincible() const { return is_invincible_; }        ///< @brief 检查 GameObject 是否处于无敌状态。
    int getCurrentHealth() const { return current_health_; }    ///< @brief 获取当前生命值。
    int getMaxHealth() const { return max_health_; }            ///< @brief 获取最大生命值。

    void setCurrentHealth(int current_health);                  ///< @brief 设置当前生命值 (确保不超过最大生命值)。
    void setMaxHealth(int max_health);                          ///< @brief 设置最大生命值 (确保不小于 1)。
    void setInvincible(float duration);                         ///< @brief 设置 GameObject 进入无敌状态，持续时间为 duration 秒。
    void setInvincibilityDuration(float duration) { invincibility_duration_ = duration; } ///< @brief 设置无敌状态持续时间。

protected:
    // 核心循环函数
    void update(float, engine::core::Context&) override;
};

} // namespace engine::component
