// Created by Jens Kromdijk 03-02-2026

#ifndef AUDIO_H
#define AUDIO_H

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

#include <memory>
#include <vector>

#include "engine_types.hpp"

class AudioHandler : public EngineObject
{
public:
    explicit AudioHandler(EngineObject* parent);
    ~AudioHandler();

    unsigned int loadSound(const char* path);
    unsigned int loadStream(const char* path);

    [[nodiscard]] SoLoud::Soloud& getSoLoud() { return m_SoLoud; }

    [[nodiscard]] const std::vector<std::unique_ptr<SoLoud::Wav>>& getSounds() const { return m_sounds; }
    [[nodiscard]] SoLoud::Wav* getSound(const unsigned int index) { return m_sounds[index].get(); }
    void playSound(const unsigned int index);

    [[nodiscard]] const std::vector<std::unique_ptr<SoLoud::WavStream>>& getStreams() const { return m_streams; }
    [[nodiscard]] SoLoud::WavStream* getStream(const unsigned int index) { return m_streams[index].get(); }
    void playStream(const unsigned int index);

private:
    SoLoud::Soloud m_SoLoud;

    std::vector<std::unique_ptr<SoLoud::Wav>> m_sounds{};
    std::vector<std::unique_ptr<SoLoud::WavStream>> m_streams{};
};

#endif
