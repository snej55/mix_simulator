// Created by Jens Kromdijk 11-02-2026

#ifndef ASSETS_H
#define ASSETS_H

#include <filesystem>
#include "core/engine.hpp"
#include "core/model.hpp"
#include "core/audio.hpp"

struct Assets
{
    explicit Assets() {}
    ~Assets() = default;

    unsigned int m_SFX_metalImpact;
    unsigned int m_MUSIC_menu;

    std::vector<Model*> m_mugShards{};

    static void loadFolder(std::vector<Model*>& models, const char* path, const std::string& name, Engine* engine)
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

        for (const auto& path : modelPaths)
        {
            const std::string modelName{name + "_" + path.stem().string()};
            if (!engine->modelExists(modelName))
            {
                engine->addModel(modelName, path.string());
            }
            models.emplace_back(engine->getModel(modelName));
        }
    }

    void loadAssets(AudioHandler* audioHandler, Engine* engine)
    {
        m_SFX_metalImpact = audioHandler->loadSound("data/audio/sfx/metal_impact.ogg");
        m_MUSIC_menu = audioHandler->loadStream("data/audio/music/menu.mp3");
        std::cout << "Loaded game assets!" << std::endl;
    }
};

#endif
