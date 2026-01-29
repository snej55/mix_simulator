// Created by Jens Kromdijk 05-01-2025

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "constants.hpp"
#include "core/bounds.hpp"
#include "core/engine.hpp"
#include "core/fonts.hpp"
#include "core/ibl.hpp"
#include "core/lights.hpp"
#include "core/renderer.hpp"
#include "core/texture.hpp"
#include "core/ui.hpp"

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

    m_engine.initFontRenderer("data/fonts/Gilda_Display/GildaDisplay-Regular.ttf", CST::FONT_TEX_SIZE);

    return true;
}

bool Game::menu()
{
    m_engine.setupViewport();

    GLFWwindow* windowPtr{m_engine.getWindow()->getWindow()};
    FontManager* fontRenderer{m_engine.getFontRenderer()};
    UIRenderer* uiRenderer{m_engine.getUIRenderer()};

    int width;
    int height;
    int numChannels;
    bool success;
    unsigned int playButtonTex{
        TextureN::loadFromFile("data/images/ui/playbutton.png", &width, &height, &numChannels, &success)};
    if (!success)
        return false;

    glDisable(GL_DEPTH_TEST);
    while (!m_engine.getQuit())
    {
        glBindFramebuffer(GL_FRAMEBUFFER, uiRenderer->getFBO());
        glViewport(0, 0, uiRenderer->getWidth(), uiRenderer->getHeight());
        glDisable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        fontRenderer->renderText(m_engine.getShader("fonts"), "Mix Simulator",
                                 static_cast<float>(m_engine.getWidth()) * 0.5f - 190.f,
                                 static_cast<float>(m_engine.getHeight()) * 0.75f, 1.0, glm::vec3{1.0f, 1.0f, 1.0f});

        glDisable(GL_BLEND);

        TextureN::renderTexture(m_engine.getShader("texture"), playButtonTex,
                                {static_cast<float>(m_engine.getWidth()) * 0.5f,
                                 static_cast<float>(m_engine.getHeight()) * 0.5f, 0.5f, 0.5f},
                                &m_engine, width, height);

        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_engine.update();
        m_engine.displayFrameTime();
    }
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
        renderUI();

        std::vector<Lights::PointLight*> pointLights{};
        m_scene->getPointLights(pointLights);

        m_engine.update(m_renderQueue.get(), m_iblGenerator.get(), pointLights);
        m_engine.displayFrameTime();
    }
}

void Game::renderUI()
{
    const UIRenderer* uiRenderer{m_engine.getUIRenderer()};
    FontManager* fontRenderer{m_engine.getFontRenderer()};

    glBindFramebuffer(GL_FRAMEBUFFER, uiRenderer->getFBO());
    glViewport(0, 0, uiRenderer->getWidth(), uiRenderer->getHeight());
    glDisable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    fontRenderer->renderText(m_engine.getShader("fonts"), "Hello+World!", 10.f, 10.f, 1.0, glm::vec3{1.0f, 1.0f, 1.0f});

    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
