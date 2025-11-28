#include "observer.h"
#include "subject.h"
#include <algorithm>

namespace engine::interface {

Observer::~Observer() {
    clearSubjects();
}

void Observer::addSubject(Subject* subject) {
    subjects_.push_back(subject);
}

void Observer::removeSubject(Subject* subject) {
    subjects_.erase(std::remove(subjects_.begin(), subjects_.end(), subject), subjects_.end());
}

void Observer::clearSubjects() {
    // 使用while循环，先弹出再移除。避免使用for循环，因为循环中会修改容器大小，导致迭代器失效
    while (!subjects_.empty()) {
        auto* subject = subjects_.back();
        subjects_.pop_back();
        if (subject) {
            subject->removeObserver(this);  // 触发 Subject 解除双向联系
        }
    }
}

} // namespace engine::interface