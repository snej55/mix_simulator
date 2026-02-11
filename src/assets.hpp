// Created by Jens Kromdijk 11-02-2026

#ifndef ASSETS_H
#define ASSETS_H

#include "core/audio.hpp"

struct Assets
{
    explicit Assets() {}
    ~Assets() = default;

    unsigned int m_SFX_metalImpact;

    void loadAssets(AudioHandler* audioHandler)
    {
        m_SFX_metalImpact = audioHandler->loadSound("data/audio/sfx/metal_impact.ogg");
        std::cout << "Loaded game assets!" << std::endl;
    }
};

#endif
