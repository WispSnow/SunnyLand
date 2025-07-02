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
    for (auto* subject : subjects_) {
        if (subject) {
            subject->removeObserver(this);  // 让所有 Subject 移除对自身的引用
            subject = nullptr;
        }
    }
    subjects_.clear();
}

} // namespace engine::interface