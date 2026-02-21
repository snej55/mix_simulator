// Created by Jens Kromdijk 05-01-2025

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <soloud_biquadresonantfilter.h>

#include "constants.hpp"
#include "core/audio.hpp"
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

    glfwSetWindowSizeLimits(m_engine.getWindow()->getWindow(), CST::WINDOW_START_WIDTH, CST::WINDOW_START_HEIGHT,
                            GLFW_DONT_CARE, GLFW_DONT_CARE);

    m_assets = std::make_unique<Assets>();
    m_assets->loadAssets(m_engine.getAudioHandler());

    m_bqrFilter.setParams(SoLoud::BiquadResonantFilter::LOWPASS, 22000.f, 2.f);
    m_engine.getAudioHandler()->getSound(m_assets->m_SFX_metalImpact)->setFilter(0, &m_bqrFilter);

    // load level data
    m_scene = std::make_unique<Scene>(&m_engine);
    m_scene->init("data/maps/0.json");
    m_scene->initPhysicsBodies(m_engine.getJoltInstance());

    // load skybox
    m_iblGenerator = std::make_unique<IBLGenerator>(&m_engine);
    m_iblGenerator->init("data/skyboxes/clouds.hdr", "data/IBL/clouds/output_iem.hdr", "data/IBL/brdf_lut.png",
                         &m_engine);
    Util::printVec3(m_iblGenerator->getLightDirection());
    m_engine.setLightDirection(m_iblGenerator->getLightDirection());

    m_renderQueue = std::make_unique<RenderQueue>(&m_engine);
    m_renderQueue->initPointLightModel("data/models/point_light.glb");

    m_engine.initFontRenderer("data/fonts/Gilda_Display/GildaDisplay-Regular.ttf", CST::FONT_TEX_SIZE);

    m_player = std::make_unique<Player>(glm::vec3{10.0f, 100.f, 10.0f}, m_engine.getModel("table"));
    m_player->setupPhysicsBody(m_engine.getJoltInstance()->getBodyInterface());
    m_scene->addEntity(m_player->getEntity());

    return true;
}

bool Game::menu()
{
    m_engine.setupViewport();

    GLFWwindow* windowPtr{m_engine.getWindow()->getWindow()};
    FontManager* fontRenderer{m_engine.getFontRenderer()};
    UIRenderer* uiRenderer{m_engine.getUIRenderer()};

    TextureN::TextureData playButtonTex;
    TextureN::loadFromFile("data/images/ui/playbutton.png", &playButtonTex);

    glDisable(GL_DEPTH_TEST);
    m_engine.setCameraEnabled(false);

    const std::vector<const char*> titleText{"M", "i", "x", " ", "S", "i", "m", "u", "l", "a", "t", "o", "r"};
    const float startTime{m_engine.getTime() + 0.5f};

    float playButtonScale{0.f};
    float playButtonVel{0.0f};
    float targetPBScale{0.0f};

    UI::Button playButton{
        {static_cast<float>(m_engine.getWidth()) * 0.5f - static_cast<float>(playButtonTex.width) * 0.25f,
         static_cast<float>(m_engine.getHeight()) * 0.5f - static_cast<float>(playButtonTex.height) * 0.25f,
         static_cast<float>(playButtonTex.width) * 0.5f, static_cast<float>(playButtonTex.height) * 0.5f}};

    while (!m_engine.getQuit())
    {
        constexpr float typeRate{10.f};
        glBindFramebuffer(GL_FRAMEBUFFER, uiRenderer->getFBO());
        glViewport(0, 0, uiRenderer->getWidth(), uiRenderer->getHeight());
        glDisable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // ---- RENDER FONTS ---- //
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const Shader* fontShader{m_engine.getShader("fonts")};

        std::string titleTextStr{};
        const std::size_t limit{std::min(
            std::size(titleText),
            static_cast<std::size_t>(std::max(0, static_cast<int>((m_engine.getTime() - startTime) * typeRate))))};
        for (std::size_t i{0}; i < limit; ++i)
        {
            titleTextStr += titleText[i];
        }
        fontRenderer->renderText(fontShader, titleTextStr, static_cast<float>(m_engine.getWidth()) * 0.5f - 190.f,
                                 static_cast<float>(m_engine.getHeight()) * 0.7f, 1.0, glm::vec3{1.0f, 1.0f, 1.0f});

        fontRenderer->renderText(fontShader, "A game by @snej55",
                                 std::min(-300.f + (m_engine.getTime() - startTime) * 30.f, 10.f), 10.f, 0.5f,
                                 glm::vec3{1.0f});

        glDisable(GL_BLEND);
        // ---------------------- //

        double cposX, cposY;
        float windowScaleX, windowScaleY;
        glfwGetCursorPos(windowPtr, &cposX, &cposY);
        glfwGetWindowContentScale(windowPtr, &windowScaleX, &windowScaleY);
        cposX *= windowScaleX;
        cposY *= windowScaleY;

        playButton.m_rect = {static_cast<float>(m_engine.getWidth()) * 0.5f -
                                 static_cast<float>(playButtonTex.width) * playButtonScale * 0.25f,
                             static_cast<float>(m_engine.getHeight()) * 0.6f -
                                 static_cast<float>(playButtonTex.height) * playButtonScale * 0.25f,
                             static_cast<float>(playButtonTex.width) * playButtonScale * 0.5f,
                             static_cast<float>(playButtonTex.height) * playButtonScale * 0.5f};
        playButton.update(cposX, cposY);

        targetPBScale = (limit == std::size(titleText)) ? (playButton.m_highlighted ? 0.6f : 0.5f) : 0.0f;
        playButtonVel += (targetPBScale - playButtonScale) * 0.6f * m_engine.getDeltaTime();
        playButtonScale += playButtonVel * 0.5f * m_engine.getDeltaTime();
        playButtonVel += (playButtonVel * 0.8f - playButtonVel) * m_engine.getDeltaTime();

        // ---- RENDER TEXTURES ---- //
        TextureN::renderTexture(m_engine.getShader("texture"), playButtonTex.id,
                                {static_cast<float>(m_engine.getWidth()) * 0.5f,
                                 static_cast<float>(m_engine.getHeight()) * 0.6f, playButtonScale, playButtonScale},
                                &m_engine, playButtonTex.width, playButtonTex.height, true,
                                playButton.m_highlighted ? glm::vec3{0.8f, 0.9f, 1.0f} : glm::vec3{1.0f});

        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ------------------------ //

        if (glfwGetMouseButton(windowPtr, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (playButton.m_highlighted)
                return true;
        }

        m_engine.update(nullptr, nullptr, {}, {}, true);
        glfwSetWindowTitle(m_engine.getWindow()->getWindow(), "Mix Simulator");
    }
    return false;
}

void Game::handleIO()
{
    if (m_engine.getPressed(GLFW_KEY_W) || m_engine.getPressed(GLFW_KEY_UP))
    {
        m_player->getController()->setControl(Controls::UP, true);
    }
    else
    {
        m_player->getController()->setControl(Controls::UP, false);
    }

    if (m_engine.getPressed(GLFW_KEY_S) || m_engine.getPressed(GLFW_KEY_DOWN))
    {
        m_player->getController()->setControl(Controls::DOWN, true);
    }
    else
    {
        m_player->getController()->setControl(Controls::DOWN, false);
    }

    if (m_engine.getPressed(GLFW_KEY_D) || m_engine.getPressed(GLFW_KEY_RIGHT))
    {
        m_player->getController()->setControl(Controls::RIGHT, true);
    }
    else
    {
        m_player->getController()->setControl(Controls::RIGHT, false);
    }

    if (m_engine.getPressed(GLFW_KEY_A) || m_engine.getPressed(GLFW_KEY_LEFT))
    {
        m_player->getController()->setControl(Controls::LEFT, true);
    }
    else
    {
        m_player->getController()->setControl(Controls::LEFT, false);
    }

    if (m_engine.getPressed(GLFW_KEY_SPACE))
    {
        m_player->getController()->setControl(Controls::SPACE, true);
    }
    else
    {
        m_player->getController()->setControl(Controls::SPACE, false);
    }
}

void Game::update()
{
    m_player->update(m_engine.getJoltInstance()->getBodyInterface());

    // update entities
    m_scene->updateEntities(m_engine.getDeltaTime(), m_engine.getJoltInstance());

    std::vector<SceneChunk*> visibleChunks{};
    const Bounds::Frustum camFrustum{m_engine.getCameraFrustum()};
    m_scene->getVisibleChunks(camFrustum, m_engine.getCameraFrustumBV(), visibleChunks);
    for (std::size_t i{0}; i < std::size(visibleChunks); ++i)
    {
        m_renderQueue->addChunk(visibleChunks[i], camFrustum);
    }

    // update sounds
    AudioHandler* audioHandler{m_engine.getAudioHandler()};
    std::vector<std::pair<JPH::RVec3, float>> sounds{m_engine.getJoltInstance()->getSoundListener()->getSoundsQueue()};

    for (std::size_t i{0}; i < sounds.size(); ++i)
    {
        const float velocity{sounds[i].second};
        constexpr float minVel{0.5f};
        constexpr float maxVel{20.f};

        if (velocity < minVel)
            continue;
        const float intensity{glm::clamp((velocity - minVel) / (maxVel - minVel), 0.0f, 1.0f)};

        constexpr float a{1.4142}; // sqrt 10
        const float volume{a * glm::sqrt(intensity)};

        const glm::vec3 worldPosition{sounds[i].first.GetX(), sounds[i].first.GetY(), sounds[i].first.GetZ()};
        const glm::vec3 viewPosition{m_engine.getViewMatrix() * glm::vec4{worldPosition, 1.0}};

        constexpr float maxDist{60.f};
        const float dist{glm::length(viewPosition)};
        const float distFreq{glm::mix(20000.f, 1000.f, glm::clamp(dist / maxDist, 0.0f, 1.0f))};
        const float frequency{distFreq * glm::mix(0.2f, 1.0f, intensity)};

        const unsigned int handle{audioHandler->getSoLoud().play3dClocked(
            static_cast<int>(glfwGetTime()), *audioHandler->getSound(m_assets->m_SFX_metalImpact), viewPosition.x,
            viewPosition.y, viewPosition.z, 0.0f, 0.0f, 0.0f, volume)};

        audioHandler->getSoLoud().setFilterParameter(handle, 0, SoLoud::BiquadResonantFilter::FREQUENCY, frequency);
    }
    m_engine.getJoltInstance()->getSoundListener()->clearSounds();
}

void Game::render()
{
    // render ui framebuffer
    renderUI();

    // render scene
    std::vector<Lights::PointLight*> pointLights{};
    m_scene->getPointLights(pointLights);

    std::vector<std::pair<Model*, glm::mat4>> shadowModels{};
    m_scene->getShadowModels(shadowModels);
    m_engine.update(m_renderQueue.get(), m_iblGenerator.get(), pointLights, shadowModels);
}

void Game::run()
{
    m_engine.setupViewport();
    m_engine.setCameraEnabled(true);
    while (!m_engine.getQuit())
    {
        handleIO();
        update();
        render();

        m_engine.displayFrameTime();
    }
}

void Game::renderUI() const
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

    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Game::gameover()
{
    m_engine.setupViewport();

    GLFWwindow* windowPtr{m_engine.getWindow()->getWindow()};
    FontManager* fontRenderer{m_engine.getFontRenderer()};
    UIRenderer* uiRenderer{m_engine.getUIRenderer()};

    TextureN::TextureData playButtonTex;
    TextureN::loadFromFile("data/images/ui/playbutton.png", &playButtonTex);

    glDisable(GL_DEPTH_TEST);
    m_engine.setCameraEnabled(false);

    const std::vector<const char*> titleText{"G", "a", "m", "e", " ", "o", "v", "e", "r", "!"};
    const float startTime{m_engine.getTime() + 0.5f};

    float playButtonScale{0.f};
    float playButtonVel{0.0f};
    float targetPBScale{0.0f};

    UI::Button playButton{
        {static_cast<float>(m_engine.getWidth()) * 0.5f - static_cast<float>(playButtonTex.width) * 0.25f,
         static_cast<float>(m_engine.getHeight()) * 0.5f - static_cast<float>(playButtonTex.height) * 0.25f,
         static_cast<float>(playButtonTex.width) * 0.5f, static_cast<float>(playButtonTex.height) * 0.5f}};

    while (!m_engine.getQuit())
    {
        constexpr float typeRate{10.f};
        glBindFramebuffer(GL_FRAMEBUFFER, uiRenderer->getFBO());
        glViewport(0, 0, uiRenderer->getWidth(), uiRenderer->getHeight());
        glDisable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // ---- RENDER FONTS ---- //
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const Shader* fontShader{m_engine.getShader("fonts")};

        std::string titleTextStr{};
        const std::size_t limit{std::min(
            std::size(titleText),
            static_cast<std::size_t>(std::max(0, static_cast<int>((m_engine.getTime() - startTime) * typeRate))))};
        for (std::size_t i{0}; i < limit; ++i)
        {
            titleTextStr += titleText[i];
        }
        fontRenderer->renderText(fontShader, titleTextStr, static_cast<float>(m_engine.getWidth()) * 0.5f - 190.f,
                                 static_cast<float>(m_engine.getHeight()) * 0.7f, 1.0, glm::vec3{1.0f, 1.0f, 1.0f});

        fontRenderer->renderText(fontShader, "Play again?",
                                 std::min(-300.f + (m_engine.getTime() - startTime) * 30.f,
                                          static_cast<float>(m_engine.getWidth()) * 0.5f -
                                              fontRenderer->getTextWidth("Play again?", 0.5f) * 0.5f),
                                 60.f, 0.5f, glm::vec3{1.0f});

        fontRenderer->renderText(
            fontShader,
            "Credits: GLFW (windowing library), GLAD (OpenGL bindings), GLM (Matrix operations), JoltPhysics (Physics "
            "engine), SoLoud (Audio library), Assimp (Model loading), STB_Image (Image loading), Freetype2 (Font "
            "rendering), nlohmann json (JSON loading, duh), mikktspace.h (fix TBN matrix tangents). Source code: "
            "https://github.com/snej55/mix_simulator",
            static_cast<float>(m_engine.getWidth()) + 10.f - (m_engine.getTime() - startTime) * 60.f, 10.f, 0.5f,
            glm::vec3{1.0f});

        glDisable(GL_BLEND);
        // ---------------------- //

        double cposX, cposY;
        float windowScaleX, windowScaleY;
        glfwGetCursorPos(windowPtr, &cposX, &cposY);
        glfwGetWindowContentScale(windowPtr, &windowScaleX, &windowScaleY);
        cposX *= windowScaleX;
        cposY *= windowScaleY;

        playButton.m_rect = {static_cast<float>(m_engine.getWidth()) * 0.5f -
                                 static_cast<float>(playButtonTex.width) * playButtonScale * 0.25f,
                             static_cast<float>(m_engine.getHeight()) * 0.6f -
                                 static_cast<float>(playButtonTex.height) * playButtonScale * 0.25f,
                             static_cast<float>(playButtonTex.width) * playButtonScale * 0.5f,
                             static_cast<float>(playButtonTex.height) * playButtonScale * 0.5f};
        playButton.update(cposX, cposY);

        targetPBScale = (limit == std::size(titleText)) ? (playButton.m_highlighted ? 0.6f : 0.5f) : 0.0f;
        playButtonVel += (targetPBScale - playButtonScale) * 0.6f * m_engine.getDeltaTime();
        playButtonScale += playButtonVel * 0.5f * m_engine.getDeltaTime();
        playButtonVel += (playButtonVel * 0.8f - playButtonVel) * m_engine.getDeltaTime();

        // ---- RENDER TEXTURES ---- //
        TextureN::renderTexture(m_engine.getShader("texture"), playButtonTex.id,
                                {static_cast<float>(m_engine.getWidth()) * 0.5f,
                                 static_cast<float>(m_engine.getHeight()) * 0.6f, playButtonScale, playButtonScale},
                                &m_engine, playButtonTex.width, playButtonTex.height, true,
                                playButton.m_highlighted ? glm::vec3{0.8f, 0.9f, 1.0f} : glm::vec3{1.0f});

        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ------------------------ //

        if (glfwGetMouseButton(windowPtr, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (playButton.m_highlighted)
                return true;
        }

        m_engine.update(nullptr, nullptr, {}, {}, true);
        glfwSetWindowTitle(m_engine.getWindow()->getWindow(), "Mix Simulator");
    }
    return false;
}
