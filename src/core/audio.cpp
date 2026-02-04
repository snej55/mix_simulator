// Created by Jens Kromdijk 03-02-2026

#include "audio.hpp"
#include "soloud.h"

#include <iostream>

AudioHandler::AudioHandler(EngineObject* parent) : EngineObject{"AudioHandler", parent}
{
    m_SoLoud = new SoLoud::Soloud{};
    m_SoLoud->init();
    std::cout << "Initialized SoLoud Audio!" << std::endl;
}

AudioHandler::~AudioHandler()
{
    m_SoLoud->deinit();
    delete m_SoLoud;
}
