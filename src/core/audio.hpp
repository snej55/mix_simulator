// Created by Jens Kromdijk 03-02-2026

#ifndef AUDIO_H
#define AUDIO_H

#include <soloud.h>

#include "engine_types.hpp"

class AudioHandler : public EngineObject
{
public:
    explicit AudioHandler(EngineObject* parent);
    ~AudioHandler();

    [[nodiscard]] SoLoud::Soloud* getSoLoud() const { return m_SoLoud; }

private:
    SoLoud::Soloud* m_SoLoud{nullptr};
};

#endif
