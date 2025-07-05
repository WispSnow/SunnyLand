#include "audio_locator.h"

namespace engine::audio {

// 静态成员初始化
NullAudioPlayer AudioLocator::null_service_;
IAudioPlayer* AudioLocator::service_ = &AudioLocator::null_service_;

} // namespace engine::audio