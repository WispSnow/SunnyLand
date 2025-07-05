#pragma once
#include "iaudio_player.h"
#include <memory>
#include <spdlog/spdlog.h>

namespace engine::audio {

/**
 * @brief 用于装饰 IAudioPlayer 的类，记录音频播放的日志。
 *
 * 在播放音频时，记录播放的音频路径和播放状态。
 * 必须使用有效的 IAudioPlayer 实例初始化。
 */
class LogAudioPlayer final : public IAudioPlayer {
private:
    std::unique_ptr<IAudioPlayer> wrapped_;
public:
    explicit LogAudioPlayer(std::unique_ptr<IAudioPlayer> wrapped) : wrapped_(std::move(wrapped)) {}
    ~LogAudioPlayer() = default;

    // --- 给每个方法添加日志，然后调用 wrapped_ 的对应方法 ---
    int playSound(std::string_view sound_path, int channel = -1) {
        spdlog::info("LogAudioPlayer: 播放音频: {}", sound_path);
        return wrapped_->playSound(sound_path, channel);
    }
    bool playMusic(std::string_view music_path, int loops = -1, int fade_in_ms = 0) {
        spdlog::info("LogAudioPlayer: 播放音乐: {}", music_path);
        return wrapped_->playMusic(music_path, loops, fade_in_ms);
    }
    void stopMusic(int fade_out_ms = 0) {
        spdlog::info("LogAudioPlayer: 停止音乐");
        wrapped_->stopMusic(fade_out_ms);
    }
    void pauseMusic() {
        spdlog::info("LogAudioPlayer: 暂停音乐");
        wrapped_->pauseMusic();
    }
    void resumeMusic() {
        spdlog::info("LogAudioPlayer: 恢复音乐");
        wrapped_->resumeMusic();
    }
    void setSoundVolume(float volume, int channel = -1) {
        spdlog::info("LogAudioPlayer: 设置音量: {}", volume);
        wrapped_->setSoundVolume(volume, channel);
    }
    void setMusicVolume(float volume) {
        spdlog::info("LogAudioPlayer: 设置音乐音量: {}", volume);
        wrapped_->setMusicVolume(volume);
    }
    float getMusicVolume() {
        spdlog::info("LogAudioPlayer: 获取音乐音量");
        return wrapped_->getMusicVolume();
    }
    float getSoundVolume(int channel = -1) {
        spdlog::info("LogAudioPlayer: 获取音量");
        return wrapped_->getSoundVolume(channel);
    }
};

} // namespace engine::audio