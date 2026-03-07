// Created by Jens Kromdijk 05-01-2025

#ifndef GAME_H
#define GAME_H

#include <soloud_biquadresonantfilter.h>
#include <memory>

#include "core/engine.hpp"
#include "core/ibl.hpp"
#include "core/renderer.hpp"
#include "core/scene.hpp"

#include "player.hpp"
#include "assets.hpp"
#include "pathfinding.hpp"

class Game final
{
public:
    Game() = default;
    ~Game();

    bool init();

    bool menu();

    bool gameover();

    void handleIO();
    void update();
    void render();

    void run();

    void renderUI() const;

private:
    // core components
    Engine m_engine{};

    std::unique_ptr<Scene> m_scene{nullptr};
    std::unique_ptr<IBLGenerator> m_iblGenerator{nullptr};
    std::unique_ptr<RenderQueue> m_renderQueue{nullptr};
    std::unique_ptr<FlowFieldGenerator> m_flowField{nullptr};

    std::unique_ptr<Player> m_player{nullptr};
    std::unique_ptr<Assets> m_assets{nullptr};
    SoLoud::BiquadResonantFilter m_bqrFilter;

    std::vector<Entity*> m_enemies{};

    void getEnemies();
    void updateEnemies();
};

#endif
