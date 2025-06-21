#include "animation_component.h"
#include "sprite_component.h"
#include "../object/game_object.h"
#include "../render/animation.h"
#include <spdlog/spdlog.h>

namespace engine::component {

AnimationComponent::~AnimationComponent() = default;

void AnimationComponent::init() {
    if (!owner_) {
        spdlog::error("AnimationComponent 没有所有者 GameObject！");
        return;
    }
    sprite_component_ = owner_->getComponent<SpriteComponent>();
    if (!sprite_component_) {
        spdlog::error("GameObject '{}' 的 AnimationComponent 需要 SpriteComponent，但未找到。", owner_->getName());
        return;
    }
}

void AnimationComponent::update(float delta_time, engine::core::Context&) {
    // 如果没有正在播放的动画，或者没有当前动画，或者没有精灵组件，或者当前动画没有帧，则直接返回
    if (!is_playing_ || !current_animation_ || !sprite_component_ || current_animation_->isEmpty()) {
        spdlog::trace("AnimationComponent 更新时没有正在播放的动画或精灵组件为空。");
        return;
    }

    // 推进计时器
    animation_timer_ += delta_time;

    // 根据时间获取当前帧
    const auto& current_frame = current_animation_->getFrame(animation_timer_);

    // 更新精灵组件的源矩形 (使用 SpriteComponent 的新方法)
    sprite_component_->setSourceRect(current_frame.source_rect);

    // 检查非循环动画是否已结束
    if (!current_animation_->isLooping() && animation_timer_ >= current_animation_->getTotalDuration()) {
        is_playing_ = false;
        animation_timer_ = current_animation_->getTotalDuration(); // 将时间限制在结束点
        if (is_one_shot_removal_) {     // 如果 is_one_shot_removal_ 为 true，则删除整个 GameObject
            owner_->setNeedRemove(true);
        }
    }
}

void AnimationComponent::addAnimation(std::unique_ptr<engine::render::Animation> animation) {
    if (!animation) return;
    std::string_view name = animation->getName();    // 获取名称
    animations_[std::string(name)] = std::move(animation);
    spdlog::debug("已将动画 '{}' 添加到 GameObject '{}'", name, owner_ ? owner_->getName() : "未知");
}

void AnimationComponent::playAnimation(std::string_view name) {
    auto it = animations_.find(std::string(name));
    if (it == animations_.end() || !it->second) {
        spdlog::warn("未找到 GameObject '{}' 的动画 '{}'", name, owner_ ? owner_->getName() : "未知");
        return;
    }

    // 如果已经在播放相同的动画，不重新开始（注释这一段则重新开始播放）
    if (current_animation_ == it->second.get() && is_playing_) {
        return;
    }

    current_animation_ = it->second.get();
    animation_timer_ = 0.0f;
    is_playing_ = true;

    // 立即将精灵更新到第一帧
    if (sprite_component_ && !current_animation_->isEmpty()) {
        const auto& first_frame = current_animation_->getFrame(0.0f);
        sprite_component_->setSourceRect(first_frame.source_rect);
        spdlog::debug("GameObject '{}' 播放动画 '{}'", owner_ ? owner_->getName() : "未知", name);
    }
}

std::string_view AnimationComponent::getCurrentAnimationName() const {
    if (current_animation_) {
        return current_animation_->getName();
    }
    return std::string_view();      // 返回一个空的string_view
}

 bool AnimationComponent::isAnimationFinished() const {
    // 如果没有当前动画(说明从未调用过playAnimation)，或者当前动画是循环的，则返回 false
    if (!current_animation_ || current_animation_->isLooping()) {
        return false;
    }
    return animation_timer_ >= current_animation_->getTotalDuration();
 }

} // namespace engine::component 