#pragma once

#include <string_view>

namespace engine::audio {

/**
 * @brief 音频播放器接口
 * 
 * 定义了音频播放器的基本操作接口。
 * 具体实现由子类提供。
 */
class IAudioPlayer {
public:
    virtual ~IAudioPlayer() = default;

    // --- 播放控制方法 --- 
    /**
    * @brief 播放音效。
    * 
    * @param sound_path 音效文件的路径。
    * @param channel 要播放的特定通道，或 -1 表示第一个可用通道。默认为 -1。
    * @return 音效正在播放的通道，出错时返回 -1。
    */
    virtual int playSound(std::string_view sound_path, int channel = -1) = 0;

    /**
    * @brief 播放背景音乐。如果正在播放，则淡出之前的音乐。
    * 
    * @param music_path 音乐文件的路径。
    * @param loops 循环次数（-1 无限循环，0 播放一次，1 播放两次，以此类推）。默认为 -1。
    * @param fade_in_ms 音乐淡入的时间（毫秒）（0 表示不淡入）。默认为 0。
    * @return 成功返回 true，出错返回 false。
    */
    virtual bool playMusic(std::string_view music_path, int loops = -1, int fade_in_ms = 0) = 0;

    /**
    * @brief 停止当前正在播放的背景音乐。
    * @param fade_out_ms 淡出时间（毫秒）（0 表示立即停止）。默认为 0。
    */
    virtual void stopMusic(int fade_out_ms = 0) = 0;

    /**
    * @brief 暂停当前正在播放的背景音乐。
    */
    virtual void pauseMusic() = 0;

    /**
    * @brief 恢复已暂停的背景音乐。
    */
    virtual void resumeMusic() = 0;

    /**
    * @brief 设置音效通道的音量。
    * @param volume 音量级别（0.0-1.0）。
    * @param channel 通道号（-1 表示所有通道）。默认为 -1。
    */
    virtual void setSoundVolume(float volume, int channel = -1) = 0;

    /**
    * @brief 设置音乐通道的音量。
    * @param volume 音量级别（0.0-1.0）。
    */
    virtual void setMusicVolume(float volume) = 0;

    /**
    * @brief 获取当前音乐音量。
    * @return 音量级别（0.0-1.0）。
    */
    virtual float getMusicVolume() = 0;

    /**
    * @brief 获取当前音效音量。
    * @param channel 通道号（-1 表示所有通道）。默认为 -1。
    * @return 音量级别（0.0-1.0）。
    */
    virtual float getSoundVolume(int channel = -1) = 0;
};

/**
 * @brief 空音频播放器
 * 
 * 实现 IAudioPlayer 接口，但什么都不做。
 * 用于在音频系统未初始化时提供默认实现。
 */
class NullAudioPlayer final : public IAudioPlayer {
public:
    int playSound(std::string_view, int) override { return -1; }
    bool playMusic(std::string_view, int, int) override { return false; }
    void stopMusic(int) override {}
    void pauseMusic() override {}
    void resumeMusic() override {}
    void setSoundVolume(float, int) override {}
    void setMusicVolume(float) override {}
    float getMusicVolume() override { return 0.0f; }
    float getSoundVolume(int) override { return 0.0f; }
};
} // namespace engine::audio