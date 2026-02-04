// Created by Jens Kromdijk 03-02-2026

#include "audio.hpp"
#include "soloud_wavstream.h"
#include "util.hpp"

#include <iostream>

AudioHandler::AudioHandler(EngineObject* parent) : EngineObject{"AudioHandler", parent}
{
    m_SoLoud.init();
    std::cout << "Initialized SoLoud Audio!" << std::endl;
}

AudioHandler::~AudioHandler() { m_SoLoud.deinit(); }

unsigned int AudioHandler::loadSound(const char* path)
{
    const unsigned int index{static_cast<unsigned int>(m_sounds.size())};
    m_sounds.push_back(std::unique_ptr<SoLoud::Wav>{});

    const unsigned int status{m_sounds[index]->load(path)};
    if (status != 0)
    {
        Util::beginError();
        std::cout << "AUDIO_HANDLER::LOAD_SOUND::ERROR: Failed to load Wav from `" << path
                  << "`. Error code: " << status;
        Util::endError();
        return -1;
    }
    return index;
}

unsigned int AudioHandler::loadStream(const char* path)
{
    const unsigned int index{static_cast<unsigned int>(m_streams.size())};
    m_streams.push_back(std::unique_ptr<SoLoud::WavStream>{});

    const unsigned int status{m_streams[index]->load(path)};
    if (status != 0)
    {
        Util::beginError();
        std::cout << "AUDIO_HANDLER::LOAD_STREAM::ERROR: Failed to load WavStream from `" << path
                  << "`. Error code: " << status;
        Util::endError();
        return -1;
    }
    return index;
}
