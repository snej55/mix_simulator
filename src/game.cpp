// Created by Jens Kromdijk 05-01-2025

#include "constants.hpp"
#include "core/bounds.hpp"
#include "core/ibl.hpp"
#include "core/lights.hpp"
#include "core/renderer.hpp"

#include "game.hpp"

Game::~Game() = default;

bool Game::init()
{
    if (!m_engine.init(CST::WINDOW_START_WIDTH, CST::WINDOW_START_HEIGHT, "OpenGL Window"))
    {
        std::cout << "Failed to initialize engine!" << std::endl;
        return false;
    }
    std::cout << "Initialized engine!" << std::endl;

    m_engine.setCameraEnabled(true);

    // load level data
    m_scene = std::make_unique<Scene>(&m_engine);
    m_scene->init("data/maps/0.json");
    m_scene->initPhysicsBodies(m_engine.getJoltInstance());

    // load skybox
    m_iblGenerator = std::make_unique<IBLGenerator>(&m_engine);
    m_iblGenerator->init("data/skyboxes/clouds.hdr", "data/IBL/clouds/output_iem.hdr", "data/IBL/brdf_lut.png",
                         &m_engine);

    m_renderQueue = std::make_unique<RenderQueue>(&m_engine);
    m_renderQueue->initPointLightModel("data/models/point_light.glb");

    m_engine.initFontRenderer("data/fonts/pixel_operator/PixelOperator-Bold.ttf", 16);

    return true;
}

void Game::run()
{
    m_engine.setupViewport();
    while (!m_engine.getQuit())
    {
        // update entities
        m_scene->updateEntities(m_engine.getDeltaTime(), m_engine.getJoltInstance());

        std::vector<SceneChunk*> visibleChunks{};
        const Bounds::Frustum camFrustum{m_engine.getCameraFrustum()};
        m_scene->getVisibleChunks(camFrustum, m_engine.getCameraFrustumBV(), visibleChunks);
        for (std::size_t i{0}; i < std::size(visibleChunks); ++i)
        {
            m_renderQueue->addChunk(visibleChunks[i], camFrustum);
        }

        // render frame
        std::vector<Lights::PointLight*> pointLights{};
        m_scene->getPointLights(pointLights);

        m_engine.update(m_renderQueue.get(), m_iblGenerator.get(), pointLights);
        m_engine.displayFrameTime();
    }
}
