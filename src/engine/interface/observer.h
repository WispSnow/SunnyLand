#pragma once
#include <any>
#include <vector>

namespace engine::interface {
    class Subject;

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
    friend class Subject;
private:
    std::vector<Subject*> subjects_;    ///< @brief 订阅的 Subject 列表

public:
    virtual ~Observer();    ///< @brief 析构函数中，让所有 Subject 移除对自身的引用

    /**
     * @brief 当 Subject 的状态改变时，此方法被调用。
     * @param event 事件类型
     * @param data 事件数据
     */
    virtual void onNotify(const EventType event, const std::any& data) = 0;

private:
    // --- 与 Subject 建立双向联系 --- (解决可能的悬垂指针问题)
    // NOTE: 建立/解除联系必须从Subject发起，因此这里设为private，不会被外部调用
    void addSubject(Subject* subject);      ///< @brief 添加一个 Subject
    void removeSubject(Subject* subject);   ///< @brief 移除一个 Subject
    void clearSubjects();                   ///< @brief 清空所有 Subject
};

} // namespace engine::interface 