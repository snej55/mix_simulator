#include <chrono>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iomanip>
#include <soloud_biquadresonantfilter.h>

#include "core/audio.hpp"
#include "core/bounds.hpp"
#include "core/engine.hpp"
#include "core/fonts.hpp"
#include "core/ibl.hpp"
#include "core/lights.hpp"
#include "core/physics.hpp"
#include "core/renderer.hpp"
#include "core/texture.hpp"
#include "core/ui.hpp"
#include "core/util.hpp"

#include "particles.hpp"
#include "pathfinding.hpp"
#include "constants.hpp"
#include "game.hpp"

#include <memory>

Game::~Game() {}

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

    m_engine.setCameraControlsEnabled(false);
    m_engine.setScreenShader(m_engine.getShader("shockwaves"));

    m_engine.initFontRenderer("data/fonts/grow.ttf", CST::FONT_TEX_SIZE);
    m_assets = std::make_unique<Assets>();
    m_assets->loadAssets(m_engine.getAudioHandler(), &m_engine);

    m_bqrFilter.setParams(SoLoud::BiquadResonantFilter::LOWPASS, 22000.f, 2.f);
    m_engine.getAudioHandler()->getSound(m_assets->m_SFX_metalImpact)->setFilter(0, &m_bqrFilter);
    m_engine.getAudioHandler()->getSound(m_assets->m_SFX_metalImpact2)->setFilter(0, &m_bqrFilter);

    // load level data
    m_scene = std::make_unique<Scene>(&m_engine, m_engine.getJoltInstance());
    m_scene->init("data/maps/4.json");
    m_scene->initPhysicsBodies(m_engine.getJoltInstance());
    // m_scene->initRaw("data/maps/1.json");
    // m_scene->initPhysicsBodies(m_engine.getJoltInstance());

    // load skybox
    m_iblGenerator = std::make_unique<IBLGenerator>(&m_engine);
    m_iblGenerator->init("data/skyboxes/bright.hdr", "data/IBL/bright/output_iem.hdr", "data/IBL/brdf_lut.png",
                         &m_engine);
    m_engine.setLightDirection(m_iblGenerator->getLightDirection());

    m_renderQueue = std::make_unique<RenderQueue>(&m_engine);
    m_renderQueue->initPointLightModel("data/models/point_light.glb");

    m_player = std::make_unique<Player>(glm::vec3{50.0f, 50.f, 50.0f}, m_engine.getModel("table"));
    m_player->setupPhysicsBody(m_engine.getJoltInstance()->getBodyInterface());
    m_scene->addEntity(m_player->getEntity());

    // generate flow field quadtree
    m_flowField = std::make_unique<FlowFieldGenerator>(
        glm::ivec2{std::ceil(m_scene->getLevelExtents().x / CST::FLOW_FIELD_TILE_SIZE),
                   std::ceil(m_scene->getLevelExtents().z / CST::FLOW_FIELD_TILE_SIZE)},
        glm::ivec2{std::ceil(m_scene->getLevelCenter().x / CST::FLOW_FIELD_TILE_SIZE),
                   std::ceil(m_scene->getLevelCenter().z / CST::FLOW_FIELD_TILE_SIZE)},
        3.f);
    m_flowField->init(m_scene.get());

    m_enemyManager = std::make_unique<EnemyManager>(m_player.get(), m_scene.get(), m_flowField.get(),
                                                    m_engine.getJoltInstance()->getBodyInterface(), &m_engine);
    m_engine.getJoltInstance()->getCollisionListener()->addListener(m_enemyManager->getListener());

    m_particles = std::make_unique<ParticleManager>(m_engine.getShader("particles"));
    m_enemyManager->setParticleManager(m_particles.get());

    // m_lensDirt = std::make_unique<LensDirt>(m_engine.getWidth(), m_engine.getHeight());
    TextureN::loadFromFile("data/images/dirt.png", &m_dirtTex);
    TextureN::genNoise(64, 64, &m_noiseTex);

    auto start{std::chrono::high_resolution_clock::now()};
    loadLevel("data/maps/0.json");
    auto end{std::chrono::high_resolution_clock::now()};
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;

    // unsigned int handle{m_engine.getAudioHandler()->playStream(m_assets->m_MUSIC_menu)};
    // m_engine.getAudioHandler()->getSoLoud().setLooping(handle, true);
    return true;
}

void Game::nextLevel()
{
    if (++m_level >= CST::NUM_LEVELS)
    {
        m_win = true;
        return;
    }

    loadLevel(CST::LEVEL_PATHS[m_level]);
}

void Game::loadLevel(const std::string& path)
{
    m_scene->initRaw(path.c_str());
    m_scene->initPhysicsBodies(m_engine.getJoltInstance());

    m_player = std::make_unique<Player>(glm::vec3{50.0f, 50.f, 50.0f}, m_engine.getModel("table"));
    m_player->setupPhysicsBody(m_engine.getJoltInstance()->getBodyInterface());
    m_scene->addEntity(m_player->getEntity());

    m_flowField->setExtents(glm::ivec2{std::ceil(m_scene->getLevelExtents().x / CST::FLOW_FIELD_TILE_SIZE),
                                       std::ceil(m_scene->getLevelExtents().z / CST::FLOW_FIELD_TILE_SIZE)});
    m_flowField->setCenter(glm::ivec2{std::ceil(m_scene->getLevelCenter().x / CST::FLOW_FIELD_TILE_SIZE),
                                      std::ceil(m_scene->getLevelCenter().z / CST::FLOW_FIELD_TILE_SIZE)});
    m_flowField->init(m_scene.get());

    m_enemyManager->setPlayer(m_player.get());
    m_enemyManager->clearEnemies();
    m_enemyManager->getEnemies();

    m_particles = std::make_unique<ParticleManager>(m_engine.getShader("particles"));
    m_enemyManager->setParticleManager(m_particles.get());

    m_iblGenerator = std::make_unique<IBLGenerator>(&m_engine);

    const std::string skyboxPath{"data/skyboxes/" + std::string(CST::IBL_SKIES[m_iblIdx]) + ".hdr"};
    const std::string irradiancePath{"data/IBL/" + std::string(CST::IBL_SKIES[m_iblIdx]) + "/output_iem.hdr"};
    m_iblGenerator->init(skyboxPath.c_str(), irradiancePath.c_str(), "data/IBL/brdf_lut.png", &m_engine);
    m_iblIdx = ++m_iblIdx & 3;
    m_engine.setLightDirection(m_iblGenerator->getLightDirection());
}

bool Game::menu()
{
    m_menuMessages.clear();
    m_engine.setupViewport();

    GLFWwindow* windowPtr{m_engine.getWindow()->getWindow()};
    FontManager* fontRenderer{m_engine.getFontRenderer()};
    UIRenderer* uiRenderer{m_engine.getUIRenderer()};

    TextureN::TextureData playButtonTex;
    TextureN::loadFromFile("data/images/ui/playbutton.png", &playButtonTex);

    glDisable(GL_DEPTH_TEST);
    m_engine.setCameraEnabled(false);

    const std::vector<const char*> titleText{"D", "u", "c", "k", " ", "B", "o", "w", "l", "i", "n", "g"};
    const float startTime{m_engine.getTime() + 0.5f};

    float playButtonScale{0.f};
    float playButtonVel{0.0f};
    float targetPBScale{0.0f};

    UI::Button playButton{
        {static_cast<float>(m_engine.getWidth()) * 0.5f - static_cast<float>(playButtonTex.width) * 0.25f,
         static_cast<float>(m_engine.getHeight()) * 0.5f - static_cast<float>(playButtonTex.height) * 0.25f,
         static_cast<float>(playButtonTex.width) * 0.5f, static_cast<float>(playButtonTex.height) * 0.5f}};

    bool settings{false};
    float renderScale{m_engine.getRenderScale()};
    bool quit{true};
    constexpr std::size_t nHandles{5};
    std::array<void*, nHandles> handler{&settings, &renderScale, &m_engine, &quit, &m_useACES};
    void (*keyCallback)(int, int, int, int, void*){
        [](const int key, const int scancode, const int action, const int mods, void* handler)
        {
            std::array<void*, nHandles>* handlerPtr{static_cast<std::array<void*, nHandles>*>(handler)};
            if (key == GLFW_KEY_S && action == GLFW_PRESS)
            {
                *static_cast<bool*>((*handlerPtr)[0]) = !(*static_cast<bool*>((*handlerPtr)[0]));
            }
            else if (key == GLFW_KEY_J && action == GLFW_PRESS && *static_cast<bool*>((*handlerPtr)[0]))
            {
                std::size_t rScale{static_cast<std::size_t>((*static_cast<float*>((*handlerPtr)[1]) - 0.2f) * 10.f)};
                rScale = ++rScale & 7;
                *static_cast<float*>((*handlerPtr)[1]) = static_cast<float>(rScale) * 0.1f + 0.2f;
                static_cast<Engine*>((*handlerPtr)[2])->setRenderScale(*static_cast<float*>((*handlerPtr)[1]) + 0.1f);
            }
            else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
            {
                if (*static_cast<bool*>((*handlerPtr)[0]))
                {
                    *static_cast<bool*>((*handlerPtr)[0]) = false;
                }
                else
                {
                    *static_cast<bool*>((*handlerPtr)[3]) = false;
                }
            }
            else if (key == GLFW_KEY_G && action == GLFW_PRESS && *static_cast<bool*>((*handlerPtr)[0]))
            {
                static_cast<Engine*>((*handlerPtr)[2])
                    ->setSSAOEnabled(!static_cast<Engine*>((*handlerPtr)[2])->getSSAOEnabled());
            }
            else if (key == GLFW_KEY_H && action == GLFW_PRESS && *static_cast<bool*>((*handlerPtr)[0]))
            {
                static_cast<Engine*>((*handlerPtr)[2])->toggleScreenShakeEnabled();
            }
            else if (key == GLFW_KEY_T && action == GLFW_PRESS && *static_cast<bool*>((*handlerPtr)[0]))
            {
                *static_cast<bool*>((*handlerPtr)[4]) = !(*static_cast<bool*>((*handlerPtr)[4]));
            }
        }};
    m_engine.getWindow()->setKeyCallback(keyCallback, &handler);

    float settingsTarget{0.0f};
    float settingsY{0.0f};

    float messageTimer{0.0f};
    while (!m_engine.getQuit() && quit)
    {
        messageTimer += m_engine.getDeltaTime();
        if (messageTimer > 180.f)
        {
            const float minX{10.f};
            const float maxX{static_cast<float>(m_engine.getWidth()) - 10.f -
                             fontRenderer->getTextWidth(std::string(CST::MENU_MESSAGES[m_menuIdx]), 0.5f)};
            m_menuMessages.push_back(Message{m_menuIdx, minX + Util::random() * (maxX - minX), -10.f});
            messageTimer = 0.0f;
            m_menuIdx = ++m_menuIdx % CST::MENU_MESSAGES.size();
        }
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

        for (std::size_t i{0}; i < m_menuMessages.size(); ++i)
        {
            Message& msg{m_menuMessages[i]};
            msg.m_y += m_engine.getDeltaTime() * 0.5f;
            msg.m_size = std::clamp(msg.m_y * 0.1f / static_cast<float>(m_engine.getHeight()), 0.0f, 1.0f);
            if (msg.m_y - 40.f > static_cast<float>(m_engine.getHeight()))
            {
                std::swap(m_menuMessages[i], m_menuMessages[m_menuMessages.size() - 1]);
                m_menuMessages.pop_back();
            }
            else
            {
                fontRenderer->renderText(fontShader, CST::MENU_MESSAGES[msg.m_ID], msg.m_x, msg.m_y, 0.5f,
                                         {msg.m_size, msg.m_size, msg.m_size});
            }
        }

        std::string titleTextStr{};
        const std::size_t limit{std::min(
            titleText.size(),
            static_cast<std::size_t>(std::max(0, static_cast<int>((m_engine.getTime() - startTime) * typeRate))))};
        for (std::size_t i{0}; i < limit; ++i)
        {
            titleTextStr += titleText[i];
        }
        const float titleSize{std::min(1.0f,
                                       std::max(0.f, (m_engine.getTime() - startTime - 2.2f) * typeRate) /
                                               static_cast<float>(titleText.size()) +
                                           0.2f)};
        fontRenderer->renderText(
            fontShader, titleTextStr,
            static_cast<float>(m_engine.getWidth()) * 0.5f - fontRenderer->getTextWidth(titleTextStr, titleSize) * 0.5f,
            static_cast<float>(m_engine.getHeight()) * 0.7f, titleSize, glm::vec3{1.0f, 1.0f, 1.0f});

        fontRenderer->renderText(fontShader, "A game by @snej55",
                                 std::min(-300.f + (m_engine.getTime() - startTime) * 30.f, 10.f), 10.f, 0.5f,
                                 glm::vec3{1.0f});
        fontRenderer->renderText(
            fontShader, "Press [s] for settings",
            static_cast<float>(m_engine.getWidth()) - fontRenderer->getTextWidth("Press [s] for settings", 0.5) - 10.f,
            std::min(10.f, -100.f + (m_engine.getTime() - startTime) * 30.f), 0.5f, {1.f, 1.f, 1.f});

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

        targetPBScale = (limit == titleText.size()) ? (playButton.m_highlighted ? 0.6f : 0.5f) : 0.0f;
        playButtonVel += (targetPBScale - playButtonScale) * 0.6f * m_engine.getDeltaTime();
        playButtonScale += playButtonVel * 0.5f * m_engine.getDeltaTime();
        playButtonVel += (playButtonVel * 0.8f - playButtonVel) * m_engine.getDeltaTime();

        // ---- RENDER TEXTURES ---- //
        TextureN::renderTexture(m_engine.getShader("texture"), playButtonTex.id,
                                {static_cast<float>(m_engine.getWidth()) * 0.5f,
                                 static_cast<float>(m_engine.getHeight()) * 0.6f, playButtonScale, playButtonScale},
                                &m_engine, playButtonTex.width, playButtonTex.height, true,
                                playButton.m_highlighted ? glm::vec3{0.8f, 0.9f, 1.0f} : glm::vec3{1.0f});

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        settingsTarget = settings ? static_cast<float>(m_engine.getHeight()) : 0.0f;
        settingsY += (settingsTarget - settingsY) * 0.1f * m_engine.getDeltaTime();
        m_engine.drawScreenRect(
            {0.0f, static_cast<float>(m_engine.getHeight()), static_cast<float>(m_engine.getWidth()), settingsY},
            {1, 1, 1});

        std::stringstream ss{};
        ss << "Render Scale: " << std::setprecision(1) << m_engine.getRenderScale() << "     ([j] to cycle)";
        fontRenderer->renderText(fontShader, ss.str(),
                                 static_cast<float>(m_engine.getWidth()) * 0.5f -
                                     fontRenderer->getTextWidth(ss.str(), 0.5f) * 0.5f,
                                 static_cast<float>(m_engine.getHeight()) * 1.7f - settingsY, 0.5f, {1.f, 1.f, 1.f});
        ss.str(std::string());
        ss << "SSAO: " << (m_engine.getSSAOEnabled() ? "Enabled" : "Disabled") << "     ([g] to "
           << (m_engine.getSSAOEnabled() ? "Disable" : "Enable") << ")";
        fontRenderer->renderText(
            fontShader, ss.str(),
            static_cast<float>(m_engine.getWidth()) * 0.5f - fontRenderer->getTextWidth(ss.str(), 0.5f) * 0.5f,
            static_cast<float>(m_engine.getHeight()) * 1.7f - settingsY - 60.f, 0.5f, {1.f, 1.f, 1.f});
        ss.str(std::string());
        ss << "Screenshake: " << (m_engine.getScreenShakeEnabled() ? "Enabled" : "Disabled") << "     ([h] to "
           << (m_engine.getScreenShakeEnabled() ? "Disable" : "Enable") << ")";
        fontRenderer->renderText(
            fontShader, ss.str(),
            static_cast<float>(m_engine.getWidth()) * 0.5f - fontRenderer->getTextWidth(ss.str(), 0.5f) * 0.5f,
            static_cast<float>(m_engine.getHeight()) * 1.7f - settingsY - 120.f, 0.5f, {1.f, 1.f, 1.f});
        ss.str(std::string());
        ss << "Tone mapping: " << (m_useACES ? "ACES" : "Khronos PBR Neutral") << "     ([t] to toggle)";
        fontRenderer->renderText(
            fontShader, ss.str(),
            static_cast<float>(m_engine.getWidth()) * 0.5f - fontRenderer->getTextWidth(ss.str(), 0.5f) * 0.5f,
            static_cast<float>(m_engine.getHeight()) * 1.7f - settingsY - 180.f, 0.5f, {1.f, 1.f, 1.f});
        glDisable(GL_BLEND);

        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ------------------------ //

        if (glfwGetMouseButton(windowPtr, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (playButton.m_highlighted && !settings)
            {
                quit = false;
            }
        }

        m_engine.update(nullptr, nullptr, {}, {}, true);
        glfwSetWindowTitle(m_engine.getWindow()->getWindow(), "Mix Simulator");
    }

    if (quit)
    {
        return false;
    }
    else
    {
        m_engine.getWindow()->setKeyCallback(nullptr, nullptr);
        return true;
    }
}

void Game::handleIO()
{
    PlayerController* controller{m_player->getController()};
    controller->setControl(Controls::UP, m_engine.getPressed(GLFW_KEY_W) || m_engine.getPressed(GLFW_KEY_UP));
    controller->setControl(Controls::DOWN, m_engine.getPressed(GLFW_KEY_S) || m_engine.getPressed(GLFW_KEY_DOWN));
    controller->setControl(Controls::LEFT, m_engine.getPressed(GLFW_KEY_A) || m_engine.getPressed(GLFW_KEY_LEFT));
    controller->setControl(Controls::RIGHT, m_engine.getPressed(GLFW_KEY_D) || m_engine.getPressed(GLFW_KEY_RIGHT));
    controller->setControl(Controls::SPACE, m_engine.getPressed(GLFW_KEY_SPACE));
    controller->setDashedPressed(m_engine.getPressed(GLFW_KEY_E));

    if (m_engine.getPressed(GLFW_KEY_T))
    {
        m_shockwaveTime = m_engine.getTime();
    }
}

void Game::update()
{
    m_engine.getCamera()->followPlayer(m_player->getEntity()->getPosition(),
                                       m_player->getEntity()->getPhysicsBody()->getBodyID(), m_engine.getJoltInstance(),
                                       m_engine.getDeltaTime());
    m_player->update(m_engine.getJoltInstance()->getBodyInterface(), m_engine.getCamera());

    m_flowField->setPlayerPos(m_player->get2DPos());
    m_flowField->calculateFlowField(true);

    bool shockwave{false};
    glm::vec3 shockwaveCenter{0.0f};
    m_enemyManager->update(m_engine.getDeltaTime(), m_particles.get(), shockwaveCenter, shockwave);

    if (shockwave)
    {
        const glm::vec4 vp{0.0f, 0.0f, static_cast<float>(m_engine.getWidth()),
                           static_cast<float>(m_engine.getHeight())};
        glm::vec3 screenPos{
            glm::project(shockwaveCenter, m_engine.getViewMatrix(), m_engine.getProjectionMatrix(), vp)};
        m_shockwaveCenter = glm::vec2{screenPos.x / static_cast<float>(m_engine.getWidth()),
                                      screenPos.y / static_cast<float>(m_engine.getHeight())};
        m_shockwaveTime = m_engine.getTime();

        m_pointLightCenter = shockwaveCenter;
        m_pointLightRadius = 1000.0f;
    }

    bool dash{false};
    m_player->getController()->update(m_engine.getDeltaTime(), &m_engine, dash);
    AudioHandler* audioHandler{m_engine.getAudioHandler()};
    if (dash)
    {
        audioHandler->getSoLoud().play3d(*audioHandler->getSound(m_assets->m_SFX_dash), m_player->get2DPos().x,
                                         m_player->getEntity()->getGlobalMidpoint().y, m_player->get2DPos().y, 0.0f,
                                         0.0f, 0.0f, 1.0f);
    }
    if (m_player->getController()->getDashing())
    {
        for (std::size_t i{0}; i < 2.f; ++i)
        {
            m_particles->addParticle(
                m_player->getEntity()->getGlobalMidpoint(),
                glm::vec3{Util::random() - 0.5f, Util::random() - 0.5f, Util::random() - 0.5f} * 0.1f, Util::random());
        }
    }
    m_particles->update(m_engine.getDeltaTime());
    // m_lensDirt->update(m_engine.getDeltaTime());

    // update entities
    m_scene->updateEntities(m_engine.getDeltaTime(), m_engine.getJoltInstance());

    std::vector<SceneChunk*> visibleChunks{};
    const Bounds::Frustum camFrustum{m_engine.getCameraFrustum()};
    m_scene->getVisibleChunks(camFrustum, m_engine.getCameraFrustumBV(), visibleChunks);
    for (std::size_t i{0}; i < visibleChunks.size(); ++i)
    {
        m_renderQueue->addChunk(visibleChunks[i], camFrustum);
    }

    // update sounds
    std::vector<std::pair<JPH::RVec3, float>> sounds{m_engine.getJoltInstance()->getSoundListener()->getSoundsQueue()};

    constexpr std::size_t maxSounds{4};
    std::size_t soundsPlayed{0};

    const glm::vec3 camPos{m_engine.getCamera()->getPosition()};
    const bool finite{Util::finite3D(camPos)};

    for (std::size_t i{0}; i < sounds.size(); ++i)
    {
        if (soundsPlayed >= maxSounds)
            break;

        const float velocity{sounds[i].second};
        constexpr float minVel{0.5f};
        constexpr float maxVel{20.f};

        if (velocity < minVel || !std::isfinite(velocity))
        {
            continue;
        }

        const glm::vec3 worldPosition{sounds[i].first.GetX(), sounds[i].first.GetY(), sounds[i].first.GetZ()};
        if (!Util::finite3D(worldPosition))
        {
            continue;
        }

        const float intensity{glm::clamp((velocity - minVel) / (maxVel - minVel), 0.0f, 1.0f)};
        constexpr float a{1.4142}; // sqrt 10
        const float volume{a * glm::sqrt(intensity)};
        if (!std::isfinite(volume))
        {
            continue;
        }

        float frequency{12000.f};
        if (Util::finite3D(camPos))
        {
            constexpr float maxDist{60.f};
            const float dist{glm::length(worldPosition - camPos)};
            if (std::isfinite(dist))
            {
                const float distFreq{glm::mix(20000.f, 1000.f, glm::clamp(dist / maxDist, 0.0f, 1.0f))};
                const float f{distFreq * glm::mix(0.2f, 1.0f, intensity)};
                if (std::isfinite(f))
                {
                    frequency = f;
                }
            }
        }

        const unsigned int handle{audioHandler->getSoLoud().play3dClocked(
            m_engine.getTime(),
            *audioHandler->getSound(Util::random() < 0.5f ? m_assets->m_SFX_metalImpact : m_assets->m_SFX_metalImpact2),
            worldPosition.x, worldPosition.y, worldPosition.z, 0.0f, 0.0f, 0.0f, volume)};

        audioHandler->getSoLoud().setFilterParameter(handle, 0, SoLoud::BiquadResonantFilter::FREQUENCY, frequency);
        ++soundsPlayed;
    }

    audioHandler->getSoLoud().set3dListenerAt(camPos.x, camPos.y, camPos.z);
    audioHandler->getSoLoud().update3dAudio();
    // m_engine.getJoltInstance()->getSoundListener()->clearSounds();
}

void Game::render()
{
    // render ui framebuffer
    renderUI();

    m_engine.setupViewport();

    // m_lensDirt->renderDirt(m_engine.getShader("dirt"), 0);

    Shader* shockwaves{m_engine.getShader("shockwaves")};
    shockwaves->use();
    shockwaves->setVec2("center", {0.5f, 0.5f});
    shockwaves->setFloat("time", m_engine.getTime() - m_shockwaveTime);
    shockwaves->setFloat("scrWidth", static_cast<float>(m_engine.getWidth()));
    shockwaves->setFloat("scrHeight", static_cast<float>(m_engine.getHeight()));
    shockwaves->setInt("useACES", m_useACES ? 1 : 0);

    const glm::vec3 lightPos{m_engine.getCameraPosition() + (m_engine.getLightDirection() * 500.f)};
    const glm::vec4 clip{m_engine.getProjectionMatrix() * m_engine.getViewMatrix() * glm::vec4{lightPos, 1.0f}};
    const glm::vec3 ndc{glm::vec3{clip} / clip.w};

    const glm::vec2 fLightPos{glm::vec2{ndc.x, ndc.y} * 0.5f + 0.5f};
    shockwaves->setVec2("lightPos", fLightPos);
    shockwaves->setInt("noiseTex", 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_noiseTex.id);
    shockwaves->setInt("gPositionE", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_engine.getDeferredRenderer()->getPositionEBuffer());
    shockwaves->setFloat("engineTime", m_engine.getTime() * 10.f);
    shockwaves->setInt("showFlare", (clip.w <= 0.01f) ? 0 : 1);
    shockwaves->setInt("useDirtMask", 1);
    shockwaves->setInt("dirtMask", 6);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_dirtTex.id);

    // render scene
    std::vector<Lights::PointLight*> pointLights{};
    m_scene->getPointLights(pointLights);
    m_pointLightRadius = std::max(0.0f, m_pointLightRadius - m_engine.getDeltaTime() * 100.f);
    Lights::PointLight pointLight{m_pointLightCenter, {1.f, 0.7f, 0.5f}, m_pointLightRadius};
    if (m_pointLightRadius > 0.0f)
    {
        pointLights.emplace_back(&pointLight);
    }

    std::vector<std::pair<Model*, glm::mat4>> shadowModels{};
    m_scene->getShadowModels(shadowModels);

    m_particles->getShader()->use();
    m_particles->getShader()->setMat4("projection", m_engine.getProjectionMatrix());
    m_particles->getShader()->setMat4("view", m_engine.getViewMatrix());
    void (*particleCallback)(void* handler){[](void* handler)
                                            {
                                                static_cast<ParticleManager*>(handler)->getShader()->use();
                                                static_cast<ParticleManager*>(handler)->render();
                                            }};
    m_engine.update(m_renderQueue.get(), m_iblGenerator.get(), pointLights, shadowModels, false,
                    RenderQueue::fdDrawCallback{particleCallback, m_particles.get()});
}

void Game::run()
{
    m_engine.setupViewport();
    m_engine.setCameraEnabled(true);

    m_startTime = std::chrono::high_resolution_clock::now();
    while (!m_engine.getQuit() && !m_win)
    {
        handleIO();
        update();
        render();

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

    float targetPos;
    const float upperTarget{static_cast<float>(m_engine.getHeight()) + 150.f};
    const float lowerTarget{static_cast<float>(m_engine.getHeight()) * 0.85f};
    if (m_complete - m_renderComplete > 0.001f)
    {
        targetPos = lowerTarget;
    }
    else
    {
        targetPos = upperTarget;
    }
    m_scorePos += (targetPos - m_scorePos) * 0.1f * m_engine.getDeltaTime();
    const float cover{(upperTarget - m_scorePos) / (upperTarget - lowerTarget)};

    m_engine.drawScreenRect(
        {0.0f, static_cast<float>(m_engine.getHeight()), static_cast<float>(m_engine.getWidth()), 30.f}, {1, 1, 1});

    std::stringstream scoreText{};
    m_complete =
        static_cast<float>(m_enemyManager->getExploded()) / static_cast<float>(m_enemyManager->getNumEnemies());
    m_renderComplete = std::min(m_renderComplete + 0.003f * m_engine.getDeltaTime(), m_complete);
    m_complete = std::min(1.f, m_complete);
    m_renderComplete = std::min(1.f, m_renderComplete);
    scoreText << "Complete: " << static_cast<int>(m_renderComplete * 100.f) << "%";
    const float scale{0.3f};
    fontRenderer->renderText(m_engine.getShader("fonts"), scoreText.str(),
                             static_cast<float>(m_engine.getWidth()) * 0.5f -
                                 fontRenderer->getTextWidth(scoreText.str(), scale) * 0.5f,
                             static_cast<float>(m_engine.getHeight()) - 24.f, scale, glm::vec3{1.0f, 1.0f, 1.0f});

    scoreText.str("");

    long ms{
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_startTime)
            .count()};
    long seconds{static_cast<long>(std::floor(static_cast<float>(ms) / 1000.f)) % 60};
    long minutes{static_cast<long>(std::floor(static_cast<float>(ms) / 3600.f)) % 60};
    long cs{static_cast<long>(std::floor(static_cast<float>(ms) / 10.f)) % 100};
    scoreText << ((minutes < 10) ? "0" : "") << minutes << ":" << ((seconds < 10) ? "0" : "") << seconds << ":"
              << ((cs < 10) ? "0" : "") << cs;
    fontRenderer->renderText(m_engine.getShader("fonts"), scoreText.str(), 10.f,
                             static_cast<float>(m_engine.getHeight()) - 24.f, scale, glm::vec3{1.0f, 1.0f, 1.0f});
    m_engine.drawScreenRect(
        {0.0f, static_cast<float>(m_engine.getHeight()), static_cast<float>(m_engine.getWidth()), cover * 30.f},
        {1, 1, 1});

    std::string score{std::to_string(static_cast<int>(m_renderComplete * 100.0f)) + "%"};
    // m_engine.drawScreenRect(
    //     {static_cast<float>(m_engine.getWidth()) * 0.5f - fontRenderer->getTextWidth(score, 1.0f) * 0.5f - 2.f,
    //      static_cast<float>(m_engine.getHeight()), fontRenderer->getTextWidth(score, 1.0f) + 4.f,
    //      std::max(0.0f, static_cast<float>(m_engine.getHeight()) - m_scorePos + 30.0f)},
    //     {1, 1, 1});
    fontRenderer->renderText(m_engine.getShader("fonts"), score,
                             static_cast<float>(m_engine.getWidth()) * 0.5f -
                                 fontRenderer->getTextWidth(score, 1.0f) * 0.5f,
                             m_scorePos, 1.0f, {1.0f, 1.0f, 1.0f});

    if (m_renderComplete == 1.f)
    {
        m_fadeDir = 0.02f;
    }
    m_fade = std::clamp(m_fade + m_fadeDir * m_engine.getDeltaTime(), 0.0f, 2.0f);
    if (m_fade == 0.0f && m_fadeDir == -0.02f)
    {
        m_fadeDir = 0.f;
        m_transition = false;
    }
    m_engine.drawScreenRect({0.0f, static_cast<float>(m_engine.getHeight()), static_cast<float>(m_engine.getWidth()),
                             m_fade * static_cast<float>(m_engine.getHeight())},
                            {1, 1, 1});
    std::string levelText{"Level " + std::to_string(m_level + 1)};
    fontRenderer->renderText(m_engine.getShader("fonts"), levelText,
                             static_cast<float>(m_engine.getWidth()) * 0.5f -
                                 fontRenderer->getTextWidth(levelText, 1.f) * 0.5f,
                             static_cast<float>(m_engine.getHeight()) -
                                 (m_fade * 0.6f * static_cast<float>(m_engine.getHeight()) -
                                  static_cast<float>(m_engine.getHeight()) * 0.5f),
                             1.0f, {1.f, 1.f, 1.f});
    if (m_fade == 2.f)
    {
        m_transition = true;
        nextLevel();
        m_fadeDir = -0.02f;
    }

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

    constexpr float typeRate{30.f};
    while (!m_engine.getQuit())
    {
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
            titleText.size(),
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

        targetPBScale = (limit == titleText.size()) ? (playButton.m_highlighted ? 0.6f : 0.5f) : 0.0f;
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

bool Game::win()
{
    m_endTime = std::chrono::high_resolution_clock::now();
    long ms{std::chrono::duration_cast<std::chrono::milliseconds>(m_endTime - m_startTime).count()};
    long seconds{static_cast<long>(std::floor(static_cast<float>(ms) / 1000.f)) % 60};
    long minutes{static_cast<long>(std::floor(static_cast<float>(ms) / 3600.f)) % 60};
    long cs{static_cast<long>(std::floor(static_cast<float>(ms) / 10.f)) % 100};
    std::stringstream timeText{};
    timeText << "Time: " << ((minutes < 10) ? "0" : "") << minutes << ":" << ((seconds < 10) ? "0" : "") << seconds
             << ":" << ((cs < 10) ? "0" : "") << cs;

    m_win = false;
    m_level = 0;
    m_fade = 0.0f;
    m_fadeDir = 0.0f;
    m_transition = false;
    m_iblIdx = 0;
    loadLevel(CST::LEVEL_PATHS[0]);
    m_engine.setupViewport();

    GLFWwindow* windowPtr{m_engine.getWindow()->getWindow()};
    FontManager* fontRenderer{m_engine.getFontRenderer()};
    UIRenderer* uiRenderer{m_engine.getUIRenderer()};

    TextureN::TextureData playButtonTex;
    TextureN::loadFromFile("data/images/ui/playbutton.png", &playButtonTex);

    glDisable(GL_DEPTH_TEST);
    m_engine.setCameraEnabled(false);

    const std::vector<const char*> titleText{"Y", "o", "u", " ", "W", "i", "n", "!", " ", ":", ")"};
    const float startTime{m_engine.getTime() + 0.5f};

    float playButtonScale{0.f};
    float playButtonVel{0.0f};
    float targetPBScale{0.0f};

    UI::Button playButton{
        {static_cast<float>(m_engine.getWidth()) * 0.5f - static_cast<float>(playButtonTex.width) * 0.25f,
         static_cast<float>(m_engine.getHeight()) * 0.5f - static_cast<float>(playButtonTex.height) * 0.25f,
         static_cast<float>(playButtonTex.width) * 0.5f, static_cast<float>(playButtonTex.height) * 0.5f}};

    constexpr float typeRate{30.f};
    while (!m_engine.getQuit())
    {
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
            titleText.size(),
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

        fontRenderer->renderText(fontShader, timeText.str(),
                                 std::min(-300.f + (m_engine.getTime() - startTime) * 30.f,
                                          static_cast<float>(m_engine.getWidth()) * 0.5f -
                                              fontRenderer->getTextWidth(timeText.str(), 0.5f) * 0.5f),
                                 120.f, 0.5f, glm::vec3{1.0f});

        fontRenderer->renderText(
            fontShader,
            "Credits: ambientcg.com (PBR materials), polyhaven.com (HDRI map), Free sfx: kenney.nl, mixkit.co, "
            "cmftStudio (IBL irradiance map), GLFW "
            "(windowing library), GLAD "
            "(OpenGL bindings), GLM (Matrix "
            "operations), JoltPhysics (Physics "
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

        targetPBScale = (limit == titleText.size()) ? (playButton.m_highlighted ? 0.6f : 0.5f) : 0.0f;
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
