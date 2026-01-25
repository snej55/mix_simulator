// Created by Jens Kromdijk 05-01-2025

#ifndef GAME_H
#define GAME_H

#include <memory>

#include "core/engine.hpp"
#include "core/ibl.hpp"
#include "core/renderer.hpp"
#include "core/scene.hpp"

class Game final
{
public:
    Game() = default;
    ~Game();

    bool init();

    void run();

    void renderUI();

private:
    // core components
    Engine m_engine{};

    std::unique_ptr<Scene> m_scene{nullptr};
    std::unique_ptr<IBLGenerator> m_iblGenerator{nullptr};
    std::unique_ptr<RenderQueue> m_renderQueue{nullptr};
};

#endif
