#pragma once

#include <AL/al.h>
#include <AL/alc.h>

namespace inexor::vulkan_renderer::audio {

/// RAII wrapper for openal-soft
class AudioTest {
private:
    ALCdevice *m_audio_device{nullptr};
    ALCcontext *m_context{nullptr};
    ALuint m_buffer;
    ALuint m_source;

public:
    AudioTest();
    AudioTest(const AudioTest &) = delete;
    AudioTest(AudioTest &&) noexcept;
    ~AudioTest();

    AudioTest &operator=(const AudioTest &) = delete;
    AudioTest &operator=(AudioTest &&) = delete;
};

} // namespace inexor::vulkan_renderer::audio
