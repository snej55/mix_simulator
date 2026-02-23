// Created by Jens Kromdijk 11-02-2026

#ifndef ASSETS_H
#define ASSETS_H

#include "core/audio.hpp"

struct Assets
{
    explicit Assets() {}
    ~Assets() = default;

    unsigned int m_SFX_metalImpact;
    unsigned int m_MUSIC_menu;

    void loadAssets(AudioHandler* audioHandler)
    {
        m_SFX_metalImpact = audioHandler->loadSound("data/audio/sfx/metal_impact.ogg");
        m_MUSIC_menu = audioHandler->loadStream("data/audio/music/menu.mp3");
        std::cout << "Loaded game assets!" << std::endl;
    }
};

#endif
