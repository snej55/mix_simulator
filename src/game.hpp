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
#include "enemy.hpp"
#include "particles.hpp"

class Game final
{
public:
    Game() = default;
    ~Game();

    bool init();

    bool loading();

    bool menu();

    bool gameover();

    void handleIO();
    void update();
    void render();

    void run();

    void renderUI();

private:
    // core components
    Engine m_engine{};

    float m_complete{0.0f};
    float m_renderComplete{0.0f};
    float m_scorePos{180.0f};

    std::unique_ptr<Scene> m_scene{nullptr};
    std::unique_ptr<IBLGenerator> m_iblGenerator{nullptr};
    std::unique_ptr<RenderQueue> m_renderQueue{nullptr};
    std::unique_ptr<FlowFieldGenerator> m_flowField{nullptr};

    std::unique_ptr<Player> m_player{nullptr};
    std::unique_ptr<Assets> m_assets{nullptr};
    SoLoud::BiquadResonantFilter m_bqrFilter;

    std::unique_ptr<EnemyManager> m_enemyManager{nullptr};
    std::unique_ptr<ParticleManager> m_particles{nullptr};
};

#endif
