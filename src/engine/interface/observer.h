#pragma once
#include <any>

namespace engine::interface {

/**
 * @brief 事件类型枚举
 * 
 * 定义了所有可能的事件类型，用于通知观察者。
 */
enum class EventType {
    HEALTH_CHANGED,             ///< @brief 生命值改变
    MAX_HEALTH_CHANGED,         ///< @brief 最大生命值改变
    SCORE_CHANGED,              ///< @brief 得分改变
    // 未来可添加更多事件类型
};

/**
 * @brief Observer 接口，用于实现观察者模式。
 * 
 * 观察者订阅一个 Subject，并在 Subject 状态改变时接收通知。
 */
class Observer {
public:
    virtual ~Observer() = default;

    /**
     * @brief 当 Subject 的状态改变时，此方法被调用。
     * @param event 事件类型
     * @param data 事件数据
     */
    virtual void onNotify(const EventType event, const std::any& data) = 0;
};

} // namespace engine::interface 