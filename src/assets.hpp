// Created by Jens Kromdijk 11-02-2026

#ifndef ASSETS_H
#define ASSETS_H

#include <filesystem>
#include <algorithm>

#include "core/engine.hpp"
#include "core/model.hpp"
#include "core/audio.hpp"

struct Assets
{
    unsigned int m_SFX_metalImpact;
    unsigned int m_SFX_metalImpact2;
    unsigned int m_SFX_dash;
    unsigned int m_MUSIC_menu;

    void loadFolder(std::vector<Model*>& models, const char* path, const std::string& name, Engine* engine, float start,
                    float end)
    {
        models.clear();
        std::vector<std::filesystem::path> modelPaths{};
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            const std::filesystem::path path{entry.path()};
            const std::string extension{path.extension().string()};
            if (extension == ".glb" || extension == ".gltf")
            {
                modelPaths.emplace_back(path);
            }
        }

        std::sort(modelPaths.begin(), modelPaths.end());

        if (modelPaths.empty())
        {
            return;
        }

        for (std::size_t i{0}; i < modelPaths.size(); ++i)
        {
            const std::string modelName{name + "_" + modelPaths[i].stem().string()};
            if (!engine->modelExists(modelName))
            {
                engine->addModel(modelName, modelPaths[i].string());
            }
            models.emplace_back(engine->getModel(modelName));
        }
    }

    void loadAssets(AudioHandler* audioHandler, Engine* engine)
    {
        m_SFX_metalImpact = audioHandler->loadSound(
            std::string(engine->getBinaryPath().string() + "/data/audio/sfx/metal_impact.ogg").c_str());
        m_SFX_metalImpact2 = audioHandler->loadSound(
            std::string(engine->getBinaryPath().string() + "/data/audio/sfx/metal_impact2.ogg").c_str());
        m_SFX_dash = audioHandler->loadSound(
            std::string(engine->getBinaryPath().string() + "/data/audio/sfx/arrow.ogg").c_str());
        m_MUSIC_menu = audioHandler->loadStream(
            std::string(engine->getBinaryPath().string() + "/data/audio/music/menu.mp3").c_str());
    }
};

#endif
