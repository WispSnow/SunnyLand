#pragma once
#include <vector>
#include <any>

namespace engine::interface {
    class Observer;
    enum class EventType;

/**
 * @brief Subject 类，用于实现观察者模式。
 * 
 * Subject 维护一个观察者列表，并在其状态改变时通知它们。
 * 具体的 Subject 类应该继承自这个类。
 */
class Subject {
private:
    /// @brief 观察者列表，由外部管理生命周期 （要特别注意可能的悬垂指针问题）
    std::vector<Observer*> observers_;  

public:
    virtual ~Subject();     ///< @brief 析构函数中，让所有 Observer 移除对自身的引用

    /**
     * @brief 添加一个观察者。
     * @param observer 指向要添加的观察者的指针。观察者的生命周期由外部管理。
     */
    void addObserver(Observer* observer);

    /**
     * @brief 移除一个观察者。
     * @param observer 指向要移除的观察者的指针。
     */
    void removeObserver(Observer* observer);

    /// @brief 清空所有观察者
    void clearObservers();

protected:
    /**
     * @brief 通知所有注册的观察者。
     * 
     * 具体的 Subject 在其状态改变时应调用此方法。
     * @param event 事件类型
     * @param data 事件数据
     */
    void notifyObservers(const EventType& event, const std::any& data);
};

} // namespace engine::interface 