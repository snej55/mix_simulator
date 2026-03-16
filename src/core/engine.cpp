// gl libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>

// json library
#include <JSON/json.hpp>
#include <numeric>

#include "audio.hpp"
#include "bounds.hpp"
#include "camera.hpp"
#include "fonts.hpp"
#include "ibl.hpp"
#include "lights.hpp"
#include "physics.hpp"
#include "postprocessing.hpp"
#include "renderer.hpp"
#include "shadows.hpp"
#include "shapes.hpp"
using json = nlohmann::json;

#include <iostream>
#include <fstream>
#include <sstream>

#include "engine.hpp"
#include "util.hpp"

Engine::Engine() : EngineObject{"Engine"}
{
    // memory manager
    m_arena = new Arena{this};
}

// free components
Engine::~Engine()
{
    // free memory
    delete m_arena;
    // quit glfw
    glfwTerminate();
    std::cout << "ENGINE::FREE: Terminated OpenGL context!" << std::endl;
}

// initialize components
bool Engine::init(const int width, const int height, const char* title)
{
    // initialize opengl context
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    if (!createWindow(width, height, title))
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create window!";
        Util::endError();
        glfwTerminate();
        return false;
    }

    std::cout << "ENGINE::INIT: Successfully initialized GLFW!\n";

    // initialize glad
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to initialize GLAD!";
        Util::endError();
        return false;
    }

    std::cout << "ENGINE::INIT: Successfully initialized GLAD!\n";

    // create view port
    m_window->createViewPort();
    // fix framebuffer scaling issue
    m_window->updateDimensions();

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glDisable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    std::cout << "ENGINE::INIT: Initialized global OpenGL state!\n";

    // ----- create objects ----- //

    // create IOHandler
    if (!createIOHandler())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create IOHandler!";
        Util::endError();
        return false;
    }

    // create clock
    if (!createClock())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create Clock!";
        Util::endError();
        return false;
    }

    // create shader manager
    if (!createShaderManager())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create ShaderManager!";
        Util::endError();
        return false;
    }

    // check shaders
    if (!checkShaders())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to check all shaders!";
        Util::endError();
        return false;
    }
    loadShaders(); // load verified shaders

    // create texture manager
    if (!createTextureManager())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create TextureManager!";
        Util::endError();
        return false;
    }
    m_textureManager->generateBuffers();

    if (!createShapeManager())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create ShapeManager!";
        Util::endError();
        return false;
    }

    if (!createCamera())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create Camera!";
        Util::endError();
        return false;
    }

    if (!createModelManager())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create ModelManager!";
        Util::endError();
        return false;
    }

    if (!createPostProcessor())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create PostProcessor!";
        Util::endError();
        return false;
    }
    m_postProcessor->init(getWidth(), getHeight());
    m_postProcessor->enableBloom(this);

    if (!createDeferredRenderer())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create DeferredRenderer!";
        Util::endError();
        return false;
    }
    m_deferredRenderer->init(getWidth(), getHeight());

    if (!createSSAOGenerator())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create SSAOGenerator!" << std::endl;
        Util::endError();
        return false;
    }
    m_ssaoGenerator->init(getWidth(), getHeight());

    if (!createCSMGenerator())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create CSMGenerator" << std::endl;
        Util::endError();
        return false;
    }

    if (!createJoltInstance())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to initialize physics engine!" << std::endl;
        Util::endError();
        return false;
    }
    m_joltInstance->init();
    std::cout << "ENGINE:INIT: Initialized JoltPhysics!" << std::endl;

    if (!createFontRenderer())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create font renderer!" << std::endl;
        Util::endError();
        return false;
    }

    if (!createUIRenderer())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create UI renderer!" << std::endl;
        Util::endError();
        return false;
    }
    m_uiRenderer->init(getWidth(), getHeight());

    if (!createAudioHandler())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT::ERROR: Failed to create audio handler!" << std::endl;
        Util::endError();
        return false;
    }

    std::cout << "ENGINE::INIT: Successfully created components!\n";
    return true;
}

// update components
void Engine::update(RenderQueue* renderQueue, IBLGenerator* ibl, const std::vector<Lights::PointLight*>& pointLights,
                    const std::vector<std::pair<Model*, glm::mat4>>& shadowModels, const bool menu,
                    RenderQueue::fdDrawCallback callback)
{
    assert((renderQueue != nullptr) == (ibl != nullptr));

    resizePPChain();

    // ----- Handle IO ----- //
    // check for esc
    // swap buffers
    m_iohandler->update();
    m_window->setQuit(m_iohandler->getQuit());

    // update camera
    if (m_cameraEnabled && m_cameraControls)
    {
        if (getPressed(GLFW_KEY_W))
        {
            m_camera->processInput(CameraN::CameraMotion::FORWARD, getDeltaTime());
        }
        if (getPressed(GLFW_KEY_S))
        {
            m_camera->processInput(CameraN::CameraMotion::BACKWARD, getDeltaTime());
        }
        if (getPressed(GLFW_KEY_A))
        {
            m_camera->processInput(CameraN::CameraMotion::LEFT, getDeltaTime());
        }
        if (getPressed(GLFW_KEY_D))
        {
            m_camera->processInput(CameraN::CameraMotion::RIGHT, getDeltaTime());
        }
    }
    m_screenShake = std::max(0.0f, m_screenShake - getDeltaTime());
    m_camera->setRoll(Util::random() * m_screenShake - m_screenShake * 0.5f);

    if (m_iohandler->getPressed(GLFW_KEY_T))
    {
        m_ssaoEnabled = false;
    }
    if (m_iohandler->getPressed(GLFW_KEY_E))
    {
        m_ssaoEnabled = true;
    }

    // ----- Update game state ----- //
    if (!menu)
    {
        // update player movement and other stuff here
        m_joltInstance->update(m_clock->getDeltaTime());
    }

    // ----- Render Frame ----- //
    if (renderQueue != nullptr)
    {
        // m_csmGenerator->updateLSMatrices(getViewMatrix(), m_lightDirection, m_camera->getZoom(),
        //                                  static_cast<float>(getWidth()) / static_cast<float>(getHeight()));
        if (shadowModels.size() > 0)
        {
            renderQueue->renderShadows(this, shadowModels);
        }
        renderQueue->renderFrame(getShader("gBuffer"), m_deferredRenderer, getShader("texturePBR"), m_postProcessor,
                                 this, ibl, getCameraPosition(), pointLights, callback);
        renderQueue->update();
    }
    else
    {
        enablePostProcessing();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_CULL_FACE);

        disablePostProcessing();
        renderPostProcessing();
    }

    // ----- Finish up ----- //
    // update delta time
    m_clock->update();
    m_window->tick();
}

// ------ Window ------ //

// create window object
bool Engine::createWindow(const int width, const int height, const char* title)
{
    // check if window already exists
    if (m_window != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_WINDOW::ERROR: Window already exists at `" << m_window << "`";
        Util::endError();
        return false;
    }
    // allocate memory for window
    m_window = new Window{this};
    // add window to arena
    m_arena->addObject(m_window);
    // initialize window
    return m_window->init(width, height, title);
}

void Engine::setFBOCallback(const cFBOCallback callback, void* args)
{
    m_customFramebufferCallback = callback;
    m_cFBO_Handler = args;
}

// clear gl buffers
void Engine::clear() const { m_window->clear(); }

// enable wireframe rendering
void Engine::enableWireframe() const { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }

void Engine::disableWireframe() const { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

void Engine::displayFrameTime()
{
    m_deltaTimes.push_back(getDeltaTime());
    if (m_deltaTimes.size() > 5)
    {
        // remove first element
        m_deltaTimes.erase(m_deltaTimes.begin());
    }

    const float sum{std::accumulate(m_deltaTimes.begin(), m_deltaTimes.end(), 0.0f)};
    const float avgFrameTime{sum / static_cast<float>(m_deltaTimes.size())};

    std::stringstream ss{};
    ss << "Frame time: " << static_cast<int>(avgFrameTime * 1000.f / 60.f) << "ms";
    m_window->setTitle(ss.str().c_str());
}

void Engine::setupViewport() const { glViewport(0, 0, getWidth(), getHeight()); }

void Engine::resize(const int width, const int height)
{
    if (width <= 0 || height <= 0)
        return;

    m_tempRes.x = width;
    m_tempRes.y = height;
    m_resize = true;
}

void Engine::resizePPChain()
{
    if (!m_resize)
        return;

    updateDeferredRenderer(getRenderWidth(), getRenderHeight());
    updateSSAOGenerator(getRenderWidth(), getRenderHeight());
    updatePostProcessor(getRenderWidth(), getRenderHeight());

    m_uiRenderer->generate(m_tempRes.x, m_tempRes.y);
    if (m_fontRenderer != nullptr && m_fontRenderer->getLoaded())
    {
        m_fontRenderer->updateProjection(static_cast<float>(m_tempRes.x), static_cast<float>(m_tempRes.y));
    }

    if (getFBOCallback() != nullptr)
    {
        getFBOCallback()(getFBOCallbackArgs(), m_tempRes.x, m_tempRes.y);
    }

    glViewport(0, 0, m_tempRes.x, m_tempRes.y);
    m_resize = false;
}

void Engine::setRenderScale(const float scale)
{
    m_renderScale = std::clamp(scale, 0.2f, 1.0f);
    if (m_window != nullptr)
    {
        m_tempRes = {getWidth(), getHeight()};
        m_resize = true;
    }
}

// ------ IOHandler ------ //

// create iohandler for keyboard input
bool Engine::createIOHandler()
{
    if (m_iohandler != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_IOHANDLER::ERROR: IOHandler already exists at `" << m_iohandler << "`";
        Util::endError();
        return false;
    }
    if (m_window == nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_IOHANDLER::ERROR: Window is required to be created before IOHandler!";
        Util::endError();
        return false;
    }
    // allocate memory for iohandler
    m_iohandler = new IOHandler{this, m_window->getWindow()};
    // add iohandler to arena
    m_arena->addObject(m_iohandler);
    return true; // success!
}

bool Engine::getQuit() const { return m_window->getShouldClose(); }

bool Engine::getPressed(const int key) const { return m_iohandler->getPressed(key); }


// ------ Clock ------ //

// create clock
bool Engine::createClock()
{
    if (m_clock != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_CLOCK::ERROR: Clock already exists at `" << m_iohandler << "`";
        Util::endError();
        return false;
    }
    // allocate memory for clock
    m_clock = new Clock{this};
    // add clock to arena
    m_arena->addObject(m_clock);
    return true;
}

// get delta time from clock
float Engine::getDeltaTime() const { return m_clock->getDeltaTime(); }

// get time from clock in seconds
float Engine::getTime() const { return m_clock->getTime(); }

// ------ Shader Manager ------ //
bool Engine::createShaderManager()
{
    if (m_shaderManager != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_SHADER_MANAGER::ERROR: Shader manager already exists at `" << m_shaderManager
                  << "`";
        Util::endError();
        return false;
    }
    // allocate memory for shader manager
    m_shaderManager = new ShaderManager{this};
    // add shader manager to arena
    m_arena->addObject(m_shaderManager);
    return true;
}

void Engine::addShader(const std::string& name, const char* fragPath, const char* vertPath, const char* geomPath) const
{
    m_shaderManager->addShader(name, fragPath, vertPath, m_arena, geomPath);
}

Shader* Engine::getShader(const std::string& name) const { return m_shaderManager->getShader(name); }

void Engine::useShader(const std::string& name) const { m_shaderManager->useShader(name); }

bool Engine::shaderExists(const std::string& name) const { return m_shaderManager->shaderExists(name); }

// check builtin shaders
bool Engine::checkShaders()
{
    // check if config file exists
    if (!Util::fileExists("shaders/shaders.json"))
    {
        Util::beginError();
        std::cout << "ENGINE::CHECK_SHADERS::ERROR: Could not find `shaders.json` at `shaders/shaders.json";
        Util::endError();
        return false;
    }

    // load json
    std::ifstream file{"shaders/shaders.json"};
    json data = json::parse(file); // NOTE: brace initialization doesn't work

    // check builtin shaders
    for (const auto& shader : data["builtin"])
    {
        std::string name{shader["name"]};
        std::string vertPath{"shaders/builtin/" + std::string(shader["shader"]["vert"])};
        std::string fragPath{"shaders/builtin/" + std::string(shader["shader"]["frag"])};
        bool geomExists{false};
        std::string geomPath;
        if (shader["shader"].contains("geom"))
        {
            geomExists = true;
            geomPath = "shaders/builtin/" + std::string(shader["shader"]["geom"]);
        }

        // check if vertex shader exists
        if (!Util::fileExists(vertPath))
        {
            Util::beginError();
            std::cout << "ENGINE::CHECK_SHADERS::ERROR: Could not find vertex shader for *" << name << "* at: `"
                      << vertPath << "`!";
            Util::endError();
            file.close();
            return false;
        }

        // same for fragment shader
        if (!Util::fileExists(fragPath))
        {
            Util::beginError();
            std::cout << "ENGINE::CHECK_SHADERS::ERROR: Could not find fragment shader for *" << name << "* at: `"
                      << fragPath << "`!";
            Util::endError();
            file.close();
            return false;
        }

        if (geomExists)
        {
            if (!Util::fileExists(geomPath))
            {
                Util::beginError();
                std::cout << "ENGINE::CHECK_SHADERS::ERROR: Could not find geometry shader for *" << name << "* at: `"
                          << geomPath << "`!";
                Util::endError();
                file.close();
                return false;
            }
        }
        std::cout << "Found builtin shader *" << name << "* at {vert: " << vertPath << ", frag: " << fragPath
                  << ", geom: " << (geomExists ? geomPath : "none") << "}\n";
    }

    // repeat for custom
    for (const auto& shader : data["custom"])
    {
        std::string name{shader["name"]};
        std::string vertPath{"shaders/" + std::string(shader["shader"]["vert"])};
        std::string fragPath{"shaders/" + std::string(shader["shader"]["frag"])};
        bool geomExists{false};
        std::string geomPath;
        if (shader["shader"].contains("geom"))
        {
            geomExists = true;
            geomPath = "shaders/" + std::string(shader["shader"]["geom"]);
        }

        // check if vertex shader exists
        if (!Util::fileExists(vertPath))
        {
            Util::beginError();
            std::cout << "ENGINE::CHECK_SHADERS::ERROR: Could not find vertex shader for *" << name << "* at: `"
                      << vertPath << "`!";
            Util::endError();
            file.close();
            return false;
        }

        // same for fragment shader
        if (!Util::fileExists(fragPath))
        {
            Util::beginError();
            std::cout << "ENGINE::CHECK_SHADERS::ERROR: Could not find fragment shader for *" << name << "* at: `"
                      << fragPath << "`!";
            Util::endError();
            file.close();
            return false;
        }
        if (geomExists)
        {
            if (!Util::fileExists(geomPath))
            {
                Util::beginError();
                std::cout << "ENGINE::CHECK_SHADERS::ERROR: Could not find geometry shader for *" << name << "* at: `"
                          << geomPath << "`!";
                Util::endError();
                file.close();
                return false;
            }
        }

        std::cout << "Found custom shader *" << name << "* at {vert: " << vertPath << ", frag: " << fragPath
                  << ", geom: " << (geomExists ? geomPath : "none") << "}\n";
    }

    // close fstream
    file.close();
    // set flag
    m_checkedShaders = true;
    return true;
}

// load shaders from shaders.json
void Engine::loadShaders()
{
    if (!m_checkedShaders)
    {
        Util::beginError();
        std::cout << "ENGINE::LOAD_SHADERS::ERROR: Cannot load shaders: shader files have not been verified!";
        Util::endError();
        return;
    }

    std::ifstream file{"shaders/shaders.json"};
    json data = json::parse(file);
    // load builtin shaders
    for (const auto& shader : data["builtin"])
    {
        std::string name{shader["name"]};
        std::string vertPath{"shaders/builtin/" + std::string(shader["shader"]["vert"])};
        std::string fragPath{"shaders/builtin/" + std::string(shader["shader"]["frag"])};
        bool geomExists{false};
        std::string geomPath;
        if (shader["shader"].contains("geom"))
        {
            geomExists = true;
            geomPath = "shaders/builtin/" + std::string(shader["shader"]["geom"]);
        }
        addShader(name, fragPath.c_str(), vertPath.c_str(), geomExists ? geomPath.c_str() : nullptr);
    }
    // repeat for custom shaders
    for (const auto& shader : data["custom"])
    {
        std::string name{shader["name"]};
        std::string vertPath{"shaders/" + std::string(shader["shader"]["vert"])};
        std::string fragPath{"shaders/" + std::string(shader["shader"]["frag"])};
        bool geomExists{false};
        std::string geomPath;
        if (shader["shader"].contains("geom"))
        {
            geomExists = true;
            geomPath = "shaders/" + std::string(shader["shader"]["geom"]);
        }
        addShader(name, fragPath.c_str(), vertPath.c_str(), geomExists ? geomPath.c_str() : nullptr);
    }

    m_loadedShaders = true;
}

void Engine::setCameraUniforms(const Shader* shader) const
{
    shader->setMat4("view", getViewMatrix());
    shader->setMat4("projection", getProjectionMatrix());
}

void Engine::setBool(const std::string& name, const bool value, const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setBool(name, value);
    }
}

void Engine::setInt(const std::string& name, const int value, const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setInt(name, value);
    }
}

void Engine::setFloat(const std::string& name, const float value, const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setFloat(name, value);
    }
}

// vectors
void Engine::setVec2(const std::string& name, const glm::vec2& value, const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setVec2(name, value);
    }
}

void Engine::setVec2(const std::string& name, const float x, const float y, const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setVec2(name, x, y);
    }
}

void Engine::setVec3(const std::string& name, const glm::vec3& value, const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setVec3(name, value);
    }
}

void Engine::setVec3(const std::string& name, const float x, const float y, const float z,
                     const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setVec3(name, x, y, z);
    }
}

void Engine::setVec4(const std::string& name, const glm::vec4& value, const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setVec4(name, value);
    }
}

void Engine::setVec4(const std::string& name, const float x, const float y, const float z, const float w,
                     const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setVec4(name, x, y, z, w);
    }
}

// matrices
void Engine::setMat2(const std::string& name, const glm::mat2& value, const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setMat2(name, value);
    }
}

void Engine::setMat3(const std::string& name, const glm::mat3& value, const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setMat3(name, value);
    }
}

void Engine::setMat4(const std::string& name, const glm::mat4& value, const std::string& shaderName) const
{
    const Shader* shader{getShader(shaderName)};
    if (shader != nullptr)
    {
        shader->setMat4(name, value);
    }
}


// ------ Texture Manager ------ //
bool Engine::createTextureManager()
{
    if (m_textureManager != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_TEXTURE_MANAGER::ERROR: Texture manager already exists at `" << m_textureManager
                  << "`";
        Util::endError();
        return false;
    }
    // allocate memory for texture manager
    m_textureManager = new TextureManager{this};
    // add texture manager to arena
    m_arena->addObject(m_textureManager);
    return true;
}

// load new texture
void Engine::addTexture(const std::string& name, const char* path) const
{
    m_textureManager->addTexture(path, name.c_str(), m_arena);
}

Texture* Engine::getTexture(const std::string& name) const { return m_textureManager->getTexture(name); }

void Engine::activateTexture(const std::string& name, const int slot) const
{
    m_textureManager->activateTexture(name, slot);
}

bool Engine::textureExists(const std::string& name) const { return m_textureManager->textureExists(name); }

void Engine::drawTexture(const std::string& name, const FRect& destination) const
{
    const Texture* tex{m_textureManager->getTexture(name)};
    if (!tex)
    {
        Util::beginError();
        std::cout << "ENGINE::DRAW_TEXTURE::ERROR: Cannot find texture `" << name << "`!\n";
        Util::endError();
        return;
    }

    // get shader from shader manager
    const Shader* textureShader{m_shaderManager->getShader("texture")};
    if (!textureShader)
    {
        Util::beginError();
        std::cout << "ENGINE::DRAW_TEXTURE::ERROR: Could not find shader *texture*!";
        Util::endError();
        return;
    }

    glm::mat4 model{1.0f};
    model = glm::translate(model, glm::vec3(destination.x, destination.y, 0.0f));
    model = glm::scale(model, glm::vec3(destination.w, destination.h, 1.0f));

    tex->activate(0);

    textureShader->use();
    textureShader->setMat4("model", model);
    textureShader->setInt("tex", 0);

    glBindVertexArray(m_textureManager->getVAO());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Engine::drawTexture(const unsigned int texID, const FRect& destination) const
{
    // get shader from shader manager
    const Shader* textureShader{m_shaderManager->getShader("texture")};
    if (!textureShader)
    {
        Util::beginError();
        std::cout << "ENGINE::DRAW_TEXTURE::ERROR: Could not find shader *texture*!";
        Util::endError();
        return;
    }
    textureShader->use();

    glm::mat4 model{1.0f};
    model = glm::translate(model, glm::vec3(destination.x, destination.y, 0.0f));
    model = glm::scale(model, glm::vec3{destination.w, destination.h, 1.0f});
    textureShader->setMat4("model", model);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    textureShader->setInt("tex", 0);

    glBindVertexArray(m_textureManager->getVAO());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

// ------ Shape Manager ------ //
bool Engine::createShapeManager()
{
    if (m_shapeManager != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_SHAPE_MANAGER::ERROR: Shape manager already exists at `" << m_shapeManager << "`";
        Util::endError();
        return false;
    }
    // create new shape manager
    m_shapeManager = new ShapeManager{this};
    // add manager to arena
    m_arena->addObject(m_shapeManager);
    return true;
}

void Engine::drawRect(const FRect& rect, const Color& color) const
{
    m_shapeManager->drawRect<float>(rect, color, m_shaderManager);
}

void Engine::drawScreenRect(const FRect& rect, const Color& color) const
{
    drawRect({rect.x / static_cast<float>(getWidth()) * 2.0f - 1.0f,
              rect.y / static_cast<float>(getHeight()) * 2.0f - 1.0f, rect.w / static_cast<float>(getWidth()) * 2.0f,
              rect.h / static_cast<float>(getHeight()) * 2.0f},
             color);
}

// lerp color rgb
Color Engine::lerpColor(const Color& a, const Color& b, const float amount) const
{
    return Color{static_cast<int>(Util::lerp(static_cast<float>(a.r), static_cast<float>(b.r), amount)),
                 static_cast<int>(Util::lerp(static_cast<float>(a.g), static_cast<float>(b.g), amount)),
                 static_cast<int>(Util::lerp(static_cast<float>(a.b), static_cast<float>(b.b), amount)), 255};
}

// lerp color rgba
Color Engine::lerpColorAlpha(const Color& a, const Color& b, const float amount) const
{
    return Color{
        static_cast<int>(Util::lerp(static_cast<float>(a.r), static_cast<float>(b.r), amount)),
        static_cast<int>(Util::lerp(static_cast<float>(a.g), static_cast<float>(b.g), amount)),
        static_cast<int>(Util::lerp(static_cast<float>(a.b), static_cast<float>(b.b), amount)),
        static_cast<int>(Util::lerp(static_cast<float>(a.a), static_cast<float>(b.a), amount)),
    };
}

// ------ Camera ------ //

bool Engine::createCamera()
{
    if (m_camera != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_CAMERA::ERROR: Camera already exists at `" << m_camera << "`";
        Util::endError();
        return false;
    }
    // create new camera
    m_camera = new Camera{this};
    m_arena->addObject(m_camera);
    return true;
}

Bounds::Frustum Engine::getCameraFrustum() const
{
    return Bounds::createFrustum(m_camera, static_cast<float>(getWidth()) / static_cast<float>(getHeight()),
                                 glm::radians(m_camera->getZoom()), CAMERA_Z_NEAR, CAMERA_Z_FAR);
}

Bounds::AABB Engine::getCameraFrustumBV() const
{
    return Bounds::getFrustumBV(getCameraFrustum(), m_camera, CAMERA_Z_FAR, glm::radians(m_camera->getZoom()),
                                static_cast<float>(getWidth()) / static_cast<float>(getHeight()));
}

glm::mat4 Engine::getViewMatrix() const { return m_camera->getViewMatrix(); }

glm::mat4 Engine::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(m_camera->getZoom()),
                            static_cast<float>(getWidth()) / static_cast<float>(getHeight()), CAMERA_Z_NEAR,
                            CAMERA_Z_FAR);
}

glm::vec3 Engine::getCameraPosition() const { return m_camera->getPosition(); }

glm::mat4 Engine::getNormalMatrix(const glm::mat4& model) const { return glm::transpose(glm::inverse(model)); }

void Engine::setCameraEnabled(const bool value)
{
    m_cameraEnabled = value;
    glfwSetInputMode(m_window->getWindow(), GLFW_CURSOR, value ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

// ------ Models ------ //

bool Engine::createModelManager()
{
    if (m_modelManager != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_MODEL_MANAGER::ERROR: Model manager already exists at `" << m_modelManager << "`";
        Util::endError();
        return false;
    }

    m_modelManager = new ModelManager{this};
    m_arena->addObject(m_modelManager);
    return true;
}

void Engine::addModel(const std::string& name, const std::string& path) const
{
    m_modelManager->addModel(name, path, m_arena);
}

Model* Engine::getModel(const std::string& name) const { return m_modelManager->getModel(name); }

void Engine::renderModel(const std::string& name, const Shader* shader) const
{
    m_modelManager->renderModel(shader, name);
}


Model* Engine::getModelByPath(const std::string& path) const { return m_modelManager->getModelByPath(path); }

void Engine::renderModelForward(const std::string& modelName, const std::string& shaderName,
                                const glm::mat4& modelTransform, const IBLGenerator* ibl)
{
    Model* model{getModel(modelName)};
    const Shader* shader{getShader(shaderName)};

    assert(model != nullptr);
    assert(shader != nullptr);

    renderModelForward(model, shader, modelTransform, ibl);
}

void Engine::renderModelForward(const std::string& modelName, const Shader* shader, const glm::mat4& modelTransform,
                                const IBLGenerator* ibl)
{
    Model* model{getModel(modelName)};
    assert(model != nullptr);

    renderModelForward(model, shader, modelTransform, ibl);
}

void Engine::renderModelForward(Model* model, const Shader* shader, const glm::mat4& modelTransform,
                                const IBLGenerator* ibl)
{
    assert(model != nullptr);
    assert(shader != nullptr);

    shader->use();
    shader->setVec3("viewPos", getCameraPosition());
    shader->setMat4("model", modelTransform);
    shader->setMat4("view", getViewMatrix());
    shader->setMat4("projection", getProjectionMatrix());
    shader->setMat3("normalMat", getNormalMatrix(modelTransform));

    if (ibl != nullptr)
    {
        shader->setInt("irradianceMap", 10);
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getIrradianceMap());
        shader->setInt("prefilterMap", 11);
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getPrefilterMap());
        shader->setInt("brdfLUT", 12);
        glActiveTexture(GL_TEXTURE12);
        glBindTexture(GL_TEXTURE_2D, ibl->getBRDFLutMap());
    }

    model->renderForward(shader, getCameraPosition(), modelTransform);
}


bool Engine::modelExists(const std::string& name) const { return m_modelManager->modelExists(name); }

// ------ Post Processor ------ //

bool Engine::createPostProcessor()
{
    if (m_postProcessor != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_POST_PROCESSOR::ERROR: Post processor already exists at `" << m_postProcessor
                  << "`";
        Util::endError();
        return false;
    }

    m_postProcessor = new PostProcessor{this};
    m_arena->addObject(m_postProcessor);
    return true;
}

void Engine::enablePostProcessing() const { m_postProcessor->enable(); }

void Engine::disablePostProcessing() const { m_postProcessor->disable(); }

void Engine::renderPostProcessing() const
{
    m_postProcessor->renderHDR(getShader("bloomSS"));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, getWidth(), getHeight());

    unsigned int uiTEX{m_uiRenderer->getTEX()};
    m_postProcessor->renderFinal(getShader("FXAA"), &uiTEX);
}

void Engine::updatePostProcessor(const int width, const int height)
{
    m_postProcessor->generate(width, height, static_cast<void*>(this));
}

// ------ Deferred Renderer ----- //

bool Engine::createDeferredRenderer()
{
    if (m_deferredRenderer != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_DEFERRED_RENDERER::ERROR: Deferred renderer already exists at `"
                  << m_deferredRenderer << "`";
        Util::endError();
        return false;
    }

    m_deferredRenderer = new DeferredRenderer{this};
    m_arena->addObject(m_deferredRenderer);
    return true;
}

void Engine::updateDeferredRenderer(const int width, const int height) { m_deferredRenderer->init(width, height); }

void Engine::renderGBuffer(const IBLGenerator* ibl, const std::vector<Lights::PointLight*>& pointLights)
{
    useShader("deferredShading");

    // ----- lights ----- //
    setInt("activeLights", pointLights.size(), "deferredShading");
    for (std::size_t i{0}; i < std::min(pointLights.size(), Lights::MAX_POINT_LIGHTS); ++i)
    {
        const Lights::PointLight* light{pointLights[i]};
        setVec3("lights[" + std::to_string(i) + "].position", light->m_position, "deferredShading");
        setVec3("lights[" + std::to_string(i) + "].color", light->m_color, "deferredShading");
        setFloat("lights[" + std::to_string(i) + "].radius", light->m_radius, "deferredShading");
    }

    setVec3("viewPos", getCameraPosition(), "deferredShading");
    setMat4("view", getViewMatrix(), "deferredShading");
    setInt("gPositionE", 0, "deferredShading");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_deferredRenderer->getPositionEBuffer());
    setInt("gAlbedo", 1, "deferredShading");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_deferredRenderer->getColorBuffer());
    setInt("gNormalE", 2, "deferredShading");
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_deferredRenderer->getNormalEBuffer());
    setInt("gARME", 3, "deferredShading");
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_deferredRenderer->getARMEBuffer());
    setInt("ssaoEnabled", m_ssaoEnabled ? 1 : 0, "deferredShading");
    if (m_ssaoEnabled)
    {
        setInt("ssao", 4, "deferredShading");
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, m_ssaoGenerator->getSSAO_ColorBufferBlur());
    }
    setInt("gGeomNormal", 5, "deferredShading");
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_deferredRenderer->getGeomNormal());
    setInt("irradianceMap", 10, "deferredShading");
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getIrradianceMap());
    setInt("prefilterMap", 11, "deferredShading");
    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getPrefilterMap());
    setInt("brdfLUT", 12, "deferredShading");
    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, ibl->getBRDFLutMap());

    std::vector<glm::mat4> lightSpaceMatrices{m_csmGenerator->getLightSpaceMatrices(
        m_camera->getZoom(), static_cast<float>(getWidth()) / static_cast<float>(getHeight()), getViewMatrix(),
        m_lightDirection)};
    for (std::size_t i{0}; i < lightSpaceMatrices.size(); ++i)
    {
        setMat4("lightSpaceMatrices[" + std::to_string(i) + "]", lightSpaceMatrices[i], "deferredShading");
    }
    setVec3("lightDir", m_lightDirection, "deferredShading");
    setInt("shadowMap", 13, "deferredShading");
    glActiveTexture(GL_TEXTURE13);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_csmGenerator->getDepthMaps());

    for (std::size_t i{0}; i < Shadows::shadowCascadeLevels.size(); ++i)
    {
        setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", Shadows::shadowCascadeLevels[i],
                 "deferredShading");
    }

    setInt("cascadeCount", Shadows::shadowCascadeLevels.size(), "deferredShading");
    setInt("csmEnabled", 1, "deferredShading");

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    m_deferredRenderer->renderQuad();
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

// ------ SSAO ------- //
bool Engine::createSSAOGenerator()
{
    if (m_ssaoGenerator != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_SSAO_GENERATOR::ERROR: SSAOGenerator already exists at `" << m_ssaoGenerator
                  << "`";
        Util::endError();
        return false;
    }

    m_ssaoGenerator = new SSAOGenerator{m_postProcessor};
    m_arena->addObject(m_ssaoGenerator);
    return true;
}

void Engine::updateSSAOGenerator(const int width, const int height)
{
    if (width <= 0 || height <= 0)
        return;

    if (m_ssaoGenerator == nullptr)
    {
        if (!createSSAOGenerator())
            return;
    }

    m_ssaoGenerator->init(width, height);
}

void Engine::renderSSAO()
{
    if (m_ssaoEnabled)
    {
        m_ssaoGenerator->render(m_deferredRenderer, getShader("ssao"), getProjectionMatrix(), getShader("ssaoBlur"),
                                getViewMatrix());
    }
}

// ------ CSM Generator ------ //
bool Engine::createCSMGenerator()
{
    if (m_csmGenerator != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_CSM_GENERATOR::ERROR: CSMGenerator already exists at `" << m_csmGenerator << "`";
        Util::endError();
        return false;
    }

    m_csmGenerator = new CSMGenerator{this};
    m_arena->addObject(m_csmGenerator);

    // setup light matrices UBO
    glGenBuffers(1, &m_lsMatricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_lsMatricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_lsMatricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return true;
}

void Engine::updateLS_UBO()
{
    std::vector<glm::mat4> lightMatrices{m_csmGenerator->getLightSpaceMatrices(
        m_camera->getZoom(), static_cast<float>(getWidth()) / static_cast<float>(getHeight()), getViewMatrix(),
        m_lightDirection)};
    glBindBuffer(GL_UNIFORM_BUFFER, m_lsMatricesUBO);
    for (std::size_t i{0}; i < lightMatrices.size(); ++i)
    {
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// ------ Jolt Instance ------ //

bool Engine::createJoltInstance()
{
    if (m_joltInstance != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_JOLT_INSTANCE::ERROR: JoltInstance already exists at `" << m_joltInstance << "`";
        Util::endError();
        return false;
    }

    m_joltInstance = new JoltInstance{this};
    m_arena->addObject(m_joltInstance);
    return true;
}

// ----- Font Renderer ----- //

bool Engine::createFontRenderer()
{
    if (m_fontRenderer != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_FONT_RENDERER::ERROR: FontRenderer already exists at `" << m_fontRenderer << "`";
        Util::endError();
        return false;
    }

    m_fontRenderer = new FontManager{this};
    m_fontRenderer->updateProjection(static_cast<float>(getWidth()), static_cast<float>(getHeight()));
    m_arena->addObject(m_fontRenderer);
    return true;
}

void Engine::initFontRenderer(const char* fontPath, int height)
{
    if (m_fontRenderer == nullptr)
    {
        createFontRenderer();
    }

    if (m_fontRenderer->getLoaded())
    {
        Util::beginError();
        std::cout << "ENGINE::INIT_FONT_RENDERER::ERROR: Font renderer has already been initialized!";
        Util::endError();
        return;
    }

    if (!m_fontRenderer->init(fontPath, height))
    {
        Util::beginError();
        std::cout << "ENGINE::INIT_FONT_RENDERER::ERROR: Font renderer init failed!";
        Util::endError();
    }
}

// ------ UI Renderer ----- //

bool Engine::createUIRenderer()
{
    if (m_uiRenderer != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_UI_RENDERER::ERROR: UIRenderer already exists at `" << m_uiRenderer << "`";
        Util::endError();
        return false;
    }

    m_uiRenderer = new UIRenderer{this};
    m_arena->addObject(m_uiRenderer);
    return true;
}

// ------ Audio Handler ------ //

bool Engine::createAudioHandler()
{
    if (m_audioHandler != nullptr)
    {
        Util::beginError();
        std::cout << "ENGINE::CREATE_AUDIO_HANDLER::ERROR: AudioHandler already exists at `" << m_audioHandler << "`";
        Util::endError();
        return false;
    }

    m_audioHandler = new AudioHandler{this};
    m_arena->addObject(m_audioHandler);
    return true;
}

// ------ Arena ------ //

void Engine::addObject(EngineObject*& object) const { m_arena->addObject(object); }

void Engine::removeObject(EngineObject*& object) const
{
    if (object != nullptr)
    {
        m_arena->removeObject(object->getID());
    }
}

void Engine::removeObjectID(const unsigned int id) const { m_arena->removeObject(id); }

// ---- Window callbacks ---- //
void Engine::mouse_callback(const double xPosIn, const double yPosIn)
{
    const float xPos{static_cast<float>(xPosIn)};
    const float yPos{static_cast<float>(yPosIn)};

    if (m_camFirstMouse)
    {
        m_camLastX = xPos;
        m_camLastY = yPos;
        m_camFirstMouse = false;
    }

    const float xOffset{xPos - m_camLastX};
    const float yOffset{m_camLastY - yPos}; // remember to reverse because of reversed coordinates

    m_camLastX = xPos;
    m_camLastY = yPos;

    m_camera->processMouseMovement(xOffset, yOffset);
}

void Engine::scroll_callback(const double yOffset) const { m_camera->processMouseScroll(static_cast<float>(yOffset)); }
