// Created by Jens Kromdijk 11-02-2026

#ifndef ASSETS_H
#define ASSETS_H

#include <filesystem>
#include <atomic>
#include <thread>
#include <algorithm>

#include "core/engine.hpp"
#include "core/model.hpp"
#include "core/audio.hpp"

struct Assets
{
    explicit Assets() {}
    ~Assets()
    {
        if (m_loader.joinable())
            m_loader.join();
    }

    std::atomic<float> m_progress{0.0f};
    std::atomic<bool> m_loaded{false};

    std::thread m_loader{};

    unsigned int m_SFX_metalImpact;
    unsigned int m_MUSIC_menu;

    std::vector<Model*> m_mugShards{};

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
            m_progress = end;
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

            m_progress = start + (static_cast<float>(i + 1) / static_cast<float>(modelPaths.size()) * (end - start));
        }
    }

    void loadAssets(AudioHandler* audioHandler, Engine* engine)
    {
        if (m_loader.joinable())
            m_loader.join();

        m_loaded = false;
        m_progress = 0.0f;

        m_loader = std::thread(
            [this, audioHandler, engine]()
            {
                m_SFX_metalImpact = audioHandler->loadSound("data/audio/sfx/metal_impact.ogg");
                m_progress = 0.1f;

                m_MUSIC_menu = audioHandler->loadStream("data/audio/music/menu.mp3");
                m_progress = 0.2f;

                loadFolder(m_mugShards, "data/models/mug_shards/", "mug_shards", engine, 0.2f, 1.0f);

                m_progress = 1.0f;
                m_loaded = true;
            });
    }
};

#endif
