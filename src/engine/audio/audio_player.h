#pragma once
#include <string>
#include <string_view>

namespace engine::resource {
    class ResourceManager;
}

struct Mix_Chunk;
struct Mix_Music;

namespace engine::audio {

/**
 * @brief 用于控制音频播放的单例类。
 *
 * 提供播放音效和音乐的方法，使用由 ResourceManager 管理的资源。
 * 必须使用有效的 ResourceManager 实例初始化。
 */
class AudioPlayer final{
private:
    engine::resource::ResourceManager* resource_manager_;   ///< @brief 指向 ResourceManager 的非拥有指针，用于加载和管理音频资源。
    std::string current_music_;         ///< @brief 当前正在播放的音乐路径，用于避免重复播放同一音乐。

public:
    /**
     * @brief 构造函数，使用 ResourceManager 初始化。
     */
    explicit AudioPlayer(engine::resource::ResourceManager* resource_manager);
    ~AudioPlayer();

    // 删除复制/移动操作
    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;
    AudioPlayer(AudioPlayer&&) = delete;
    AudioPlayer& operator=(AudioPlayer&&) = delete;

    // --- 播放控制方法 --- 
    /**
     * @brief 播放音效（chunk）。
     * 如果尚未缓存，则通过 ResourceManager 加载音效。
     * @param sound_path 音效文件的路径。
     * @param channel 要播放的特定通道，或 -1 表示第一个可用通道。默认为 -1。
     * @return 音效正在播放的通道，出错时返回 -1。
     */
    int playSound(std::string_view sound_path, int channel = -1);

    /**
     * @brief 播放背景音乐。如果正在播放，则淡出之前的音乐。
     * 如果尚未缓存，则通过 ResourceManager 加载音乐。
     * @param music_path 音乐文件的路径。
     * @param loops 循环次数（-1 无限循环，0 播放一次，1 播放两次，以此类推）。默认为 -1。
     * @param fade_in_ms 音乐淡入的时间（毫秒）（0 表示不淡入）。默认为 0。
     * @return 成功返回 true，出错返回 false。
     */
    bool playMusic(std::string_view music_path, int loops = -1, int fade_in_ms = 0);

    /**
     * @brief 停止当前正在播放的背景音乐。
     * @param fade_out_ms 淡出时间（毫秒）（0 表示立即停止）。默认为 0。
     */
    void stopMusic(int fade_out_ms = 0);

    /**
     * @brief 暂停当前正在播放的背景音乐。
     */
    void pauseMusic();

    /**
     * @brief 恢复已暂停的背景音乐。
     */
    void resumeMusic();

    /**
     * @brief 设置音效通道的音量。
     * @param volume 音量级别（0.0-1.0）。
     * @param channel 通道号（-1 表示所有通道）。默认为 -1。
     */
    void setSoundVolume(float volume, int channel = -1);

    /**
     * @brief 设置音乐通道的音量。
     * @param volume 音量级别（0.0-1.0）。
     */
    void setMusicVolume(float volume);

    /**
     * @brief 获取当前音乐音量。
     * @return 音量级别（0.0-1.0）。
     */
    float getMusicVolume();

    /**
     * @brief 获取当前音效音量。
     * @param channel 通道号（-1 表示所有通道）。默认为 -1。
     * @return 音量级别（0.0-1.0）。
     */
    float getSoundVolume(int channel = -1);

};

} // namespace engine::audio
