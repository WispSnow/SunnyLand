#include "audio_component.h"
#include "transform_component.h"
#include "../object/game_object.h"
#include "../audio/audio_player.h"
#include "../render/camera.h"
#include <spdlog/spdlog.h>

namespace engine::component {

AudioComponent::AudioComponent(engine::audio::AudioPlayer *audio_player, engine::render::Camera *camera)
    : audio_player_(audio_player), camera_(camera)
{
    if (!audio_player_ || !camera_) {
        spdlog::error("AudioComponent 初始化失败: 音频播放器或相机为空");
    }
}

void AudioComponent::init()
{
    if (!owner_) {
        spdlog::error("AudioComponent 没有所有者 GameObject！");
        return;
    }
    transform_ = owner_->getComponent<TransformComponent>();
    if (!transform_) {
        spdlog::warn("AudioComponent 所在的 GameObject 上没有 TransformComponent！，无法进行空间定位");
    }
}

void AudioComponent::playSound(std::string_view sound_id, int channel, bool use_spatial)
{
    // 如果 sound_id 是音效 ID，则在查找在map中查找对应的路径； 没找到的话则把 sound_id 当作路径直接使用
    auto sound_path = sound_id_to_path_.find(std::string(sound_id)) != sound_id_to_path_.end() ? sound_id_to_path_[std::string(sound_id)] : sound_id;

    if (use_spatial && transform_) {    // 使用空间定位
        // TODO: (SDL_Mixer 不支持空间定位，未来更换音频库时可以方便地实现)
                // 这里给一个简单的功能：150像素范围内播放，否则不播放
        auto camera_center = camera_->getPosition() + camera_->getViewportSize() / 2.0f; // 相机中心
        auto object_pos = transform_->getPosition();
        float distance = glm::length(camera_center - object_pos);
        if (distance > 150.0f) {
            spdlog::debug("AudioComponent::playSound: 音效 '{}' 超出范围，不播放。", sound_id);
            return; // 超出范围，不播放
        }
        audio_player_->playSound(sound_path, channel);
    } else {    // 不使用空间定位
        audio_player_->playSound(sound_path, channel);
    }
}

void AudioComponent::addSound(std::string_view sound_id, std::string_view sound_path)
{
    if (sound_id_to_path_.find(std::string(sound_id)) != sound_id_to_path_.end()) {
        spdlog::warn("AudioComponent::addSound: 音效 ID '{}' 已存在，覆盖旧路径。", sound_id);
    }
    sound_id_to_path_[std::string(sound_id)] = sound_path;
    spdlog::debug("AudioComponent::addSound: 添加音效 ID '{}' 路径 '{}'", sound_id, sound_path);
}

} // namespace engine::component