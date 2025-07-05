#pragma once

#include "iaudio_player.h"

namespace engine::audio {

/**
 * @brief 音频定位器
 * 
 * 提供音频播放器的静态访问。
 * 使用 provide 方法设置实际的音频播放器实现。
 */
class AudioLocator final {
private:
    static IAudioPlayer* service_;              // 用于存储实际的音频播放器
    static NullAudioPlayer null_service_;       // 备用空服务，用于在未提供实际服务时提供默认实现
    /* Service Locator 模式的最佳实践：始终提供一个可用的服务实现, 避免检查空指针 */

public:
    // 删除构造函数、复制操作，防止实例化
    AudioLocator() = delete;
    AudioLocator(const AudioLocator&) = delete;
    AudioLocator& operator=(const AudioLocator&) = delete;
    AudioLocator(AudioLocator&&) = delete;
    AudioLocator& operator=(AudioLocator&&) = delete;

    // @brief 获取音频播放器
    static IAudioPlayer& get() { return *service_; }

    // @brief 提供音频播放器
    static void provide(IAudioPlayer* service) {
        if (service == nullptr) {
            // 如果传入空指针，我们就提供一个“空服务”
            service_ = &null_service_;
        } else {
            service_ = service;
        }
    }
};

} // namespace engine::audio