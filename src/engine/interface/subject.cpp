#include "subject.h"
#include "observer.h"
#include <algorithm>
#include <spdlog/spdlog.h>

namespace engine::interface {

Subject::~Subject() {
    clearObservers();
}

void Subject::addObserver(Observer* observer) {
    if (!observer) {
        spdlog::error("尝试添加空观察者");
        return;
    }
    if (std::find(observers_.begin(), observers_.end(), observer) != observers_.end()) {
        spdlog::warn("观察者已存在");
        return;
    }
    observers_.push_back(observer);
    observer->addSubject(this); // 建立双向联系
}

void Subject::removeObserver(Observer* observer) {
    // 使用 find 和 erase 来解除双向联系
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        observers_.erase(it);
        observer->removeSubject(this); // 解除双向联系
    }
}

void Subject::clearObservers() {
    // 清空之前，先解除Observers对自身的联系
    for (auto* observer : observers_) {
        if (observer) {
            observer->removeSubject(this);
            observer = nullptr;
        }
    }
    observers_.clear();
}

void Subject::notifyObservers(const EventType& event, const std::any& data) {
    for (auto* observer : observers_) {
        if (observer) {
            observer->onNotify(event, data);
        }
    }
}

} // namespace engine::interface