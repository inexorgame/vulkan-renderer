#include "inexor/vulkan-renderer/audio/setup.hpp"

#include <cmath>
#include <stdexcept>

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::audio {

AudioTest::AudioTest() {
    // Initialize OpenAL
    m_audio_device = alcOpenDevice(nullptr);
    m_context = alcCreateContext(m_audio_device, nullptr);
    alcMakeContextCurrent(m_context);

    // Check if OpenAL initialization succeeded
    if (!m_audio_device || !m_context) {
        throw std::runtime_error("Error: Failed to initialize OpenAL");
    }

    // Generate a buffer and source
    alGenBuffers(1, &m_buffer);
    alGenSources(1, &m_source);

    // Specify the audio data (in this case, a simple tone)
    const int sampleRate = 44100;
    const float frequency = 440.0f;
    const float duration = 3.0f;
    const int bufferSize = sampleRate * duration;

    std::vector<short> sound_data(bufferSize);
    for (int i = 0; i < bufferSize; ++i) {
        sound_data[i] = static_cast<short>(32760 * sin(2.0f * 3.14f * frequency * i / sampleRate));
    }

    // Load the audio data into the buffer
    alBufferData(m_buffer, AL_FORMAT_MONO16, sound_data.data(), bufferSize * sizeof(short), sampleRate);

    // Assign the buffer to the source and set properties
    alSourcei(m_source, AL_BUFFER, m_buffer);
    alSourcef(m_source, AL_GAIN, 1.0f); // Set volume to maximum

    // Play the sound
    alSourcePlay(m_source);

    // Wait for the sound to finish playing
    ALint sourceState;
    do {
        alGetSourcei(m_source, AL_SOURCE_STATE, &sourceState);
    } while (sourceState == AL_PLAYING);
}

AudioTest::~AudioTest() {
    // Clean up
    alDeleteSources(1, &m_source);
    alDeleteBuffers(1, &m_buffer);

    // Clean up OpenAL
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(m_context);
    alcCloseDevice(m_audio_device);
}

} // namespace inexor::vulkan_renderer::audio
