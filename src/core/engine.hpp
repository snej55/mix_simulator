#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>

#include "arena.hpp"
#include "camera.hpp"
#include "clock.hpp"
#include "engine_types.hpp"
#include "iohandler.hpp"
#include "lights.hpp"
#include "model.hpp"
#include "postprocessing.hpp"
#include "shader.hpp"
#include "shadows.hpp"
#include "shapes.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "ibl.hpp"
#include "bounds.hpp"
#include "physics.hpp"
#include "fonts.hpp"
#include "ui.hpp"
#include "audio.hpp"

class Engine final : public EngineObject
{
public:
    Engine();
    Engine(const Engine&) = default;
    Engine(Engine&&) = delete;
    Engine& operator=(const Engine&) = default;
    Engine& operator=(Engine&&) = delete;

    // free components
    ~Engine() override;

    // initialize components
    bool init(int width, int height, const char* title);
    // update components
    void update(RenderQueue* renderQueue = nullptr, IBLGenerator* ibl = nullptr,
                const std::vector<Lights::PointLight*>& pointLights = {},
                const std::vector<std::pair<Model*, glm::mat4>>& shadowModels = {}, bool menu = false);

    // ------ Window ------ //

    // create window object
    bool createWindow(int width, int height, const char* title);
    // window getters
    [[nodiscard]] Window* getWindow() const { return m_window; }
    [[nodiscard]] int getWidth() const { return m_window->getWidth(); }
    [[nodiscard]] int getHeight() const { return m_window->getHeight(); }

    using cFBOCallback = void (*)(void*, int, int);
    void setFBOCallback(cFBOCallback callback, void* args);
    [[nodiscard]] cFBOCallback getFBOCallback() const { return m_customFramebufferCallback; }
    [[nodiscard]] void* getFBOCallbackArgs() const { return m_cFBO_Handler; }

    // clear screen
    void clear() const;

    // enable wireframe rendering
    void enableWireframe() const;

    void disableWireframe() const;

    void displayFrameTime();

    void setupViewport() const;

    void resize(int width, int height);

    [[nodiscard]] float getRenderScale() const { return m_renderScale; }
    [[nodiscard]] int getRenderWidth() const
    {
        return std::max(1, static_cast<int>(std::floor(static_cast<float>(getWidth()) * m_renderScale)));
    }
    [[nodiscard]] int getRenderHeight() const
    {
        return std::max(1, static_cast<int>(std::floor(static_cast<float>(getHeight()) * m_renderScale)));
    }

    void setRenderScale(float scale);

    // ------ IOHandler ------ //

    // create iohandler for keyboard input
    bool createIOHandler();
    [[nodiscard]] IOHandler* getIOHandler() const { return m_iohandler; }

    // check if ESC has been pressed from IOHandler
    [[nodiscard]] bool getQuit() const;
    [[nodiscard]] bool getPressed(int key) const;

    // ------ Clock ------ //

    // create clock
    bool createClock();
    [[nodiscard]] Clock* getClock() const { return m_clock; }

    // get delta time from clock
    [[nodiscard]] float getDeltaTime() const;
    // get time since start from clock in seconds
    [[nodiscard]] float getTime() const;

    // ------ Shaders ------ //

    // create shader manager
    bool createShaderManager();
    [[nodiscard]] ShaderManager* getShaderManager() const { return m_shaderManager; }

    // shader manager methods
    void addShader(const std::string& name, const char* fragPath, const char* vertPath,
                   const char* geomPath = nullptr) const;
    [[nodiscard]] Shader* getShader(const std::string& name) const;
    void useShader(const std::string& name) const;
    [[nodiscard]] bool shaderExists(const std::string& name) const;

    // verify shaders exist
    [[nodiscard]] bool checkShaders();
    // load shaders from shaders.json
    void loadShaders();

    [[nodiscard]] bool getShadersChecked() const { return m_checkedShaders; }
    [[nodiscard]] bool getShadersLoaded() const { return m_loadedShaders; }

    // shader uniforms
    void setCameraUniforms(const Shader* shader) const;

    void setBool(const std::string& name, bool value, const std::string& shaderName) const;
    void setInt(const std::string& name, int value, const std::string& shaderName) const;
    void setFloat(const std::string& name, float value, const std::string& shaderName) const;

    // vectors
    void setVec2(const std::string& name, const glm::vec2& value, const std::string& shaderName) const;
    void setVec2(const std::string& name, float x, float y, const std::string& shaderName) const;

    void setVec3(const std::string& name, const glm::vec3& value, const std::string& shaderName) const;
    void setVec3(const std::string& name, float x, float y, float z, const std::string& shaderName) const;

    void setVec4(const std::string& name, const glm::vec4& value, const std::string& shaderName) const;
    void setVec4(const std::string& name, float x, float y, float z, float w, const std::string& shaderName) const;

    // matrices
    void setMat2(const std::string& name, const glm::mat2& value, const std::string& shaderName) const;
    void setMat3(const std::string& name, const glm::mat3& value, const std::string& shaderName) const;
    void setMat4(const std::string& name, const glm::mat4& value, const std::string& shaderName) const;

    // ------ Textures ------ //

    bool createTextureManager();
    [[nodiscard]] TextureManager* getTextureManager() const { return m_textureManager; }

    // texture manager methods
    void addTexture(const std::string& name, const char* path) const;
    [[nodiscard]] Texture* getTexture(const std::string& name) const;
    void activateTexture(const std::string& name, int slot) const;
    [[nodiscard]] bool textureExists(const std::string& name) const;

    void drawTexture(const std::string& name, const FRect& destination) const;
    void drawTexture(unsigned int texID, const FRect& destination) const;

    // ------ Shapes ------ //

    bool createShapeManager();
    [[nodiscard]] ShapeManager* getShapeManager() const { return m_shapeManager; }

    void drawRect(const FRect& rect, const Color& color) const;
    void drawScreenRect(const FRect& rect, const Color& color) const;

    // color functions
    //
    // lerpColor() lerps the two colors rgb only, use lerpColorAlpha for rgba
    [[nodiscard]] Color lerpColor(const Color& a, const Color& b, float amount) const;
    // lerpColorAlpha() lerps the two colors rgba, use lerpColor for rgb only
    [[nodiscard]] Color lerpColorAlpha(const Color& a, const Color& b, float amount) const;

    // ------ Camera ------ //

    bool createCamera();
    [[nodiscard]] Camera* getCamera() const { return m_camera; }
    [[nodiscard]] Bounds::Frustum getCameraFrustum() const;
    [[nodiscard]] Bounds::AABB getCameraFrustumBV() const;

    // view & perspective matrices getters
    [[nodiscard]] glm::mat4 getViewMatrix() const;
    [[nodiscard]] glm::mat4 getProjectionMatrix() const;
    [[nodiscard]] glm::vec3 getCameraPosition() const;

    // get normal mat from model mat
    [[nodiscard]] glm::mat4 getNormalMatrix(const glm::mat4& model) const;

    // enable or disable window mouse capture
    void setCameraEnabled(bool value);
    [[nodiscard]] bool getCameraEnabled() const { return m_cameraEnabled; };

    void setCameraControlsEnabled(const bool val) { m_cameraControls = val; }
    [[nodiscard]] bool getCameraControlsEnabled() const { return m_cameraControls; }

    void setScreenShake(const float val) { m_screenShake = std::max(m_screenShake, val); }
    [[nodiscard]] float getScreenShake() const { return m_screenShake; }

    // ------ Models ------ //

    bool createModelManager();
    [[nodiscard]] ModelManager* getModelManager() const { return m_modelManager; }

    void addModel(const std::string& name, const std::string& path) const;
    [[nodiscard]] Model* getModel(const std::string& name) const;
    void renderModel(const std::string& name, const Shader* shader) const;
    [[nodiscard]] bool modelExists(const std::string& name) const;
    [[nodiscard]] Model* getModelByPath(const std::string& path) const;

    void renderModelForward(const std::string& modelName, const std::string& shaderName,
                            const glm::mat4& modelTransform, const IBLGenerator* ibl = nullptr);
    void renderModelForward(const std::string& modelName, const Shader* shader, const glm::mat4& modelTransform,
                            const IBLGenerator* ibl = nullptr);
    void renderModelForward(Model* model, const Shader* shader, const glm::mat4& modelTransform,
                            const IBLGenerator* ibl = nullptr);

    // void renderModelDeferred(const std::string& modelName, const glm::mat4& modelTransform) const;
    // void renderModelDeferred(const Model* model, const glm::mat4& modelTransform) const;

    // ------ Post Processor ------ //

    bool createPostProcessor();
    [[nodiscard]] PostProcessor* getPostProcessor() const { return m_postProcessor; }

    // enable and disable rendering to post processor
    void enablePostProcessing() const;
    void disablePostProcessing() const;

    // render post processor with screen shader
    void renderPostProcessing() const;

    // update framebuffer
    void updatePostProcessor(int width, int height);

    // ------ Deferred Renderer ------ //
    bool createDeferredRenderer();
    void updateDeferredRenderer(int width, int height);
    void renderGBuffer(const IBLGenerator* ibl, const std::vector<Lights::PointLight*>& pointLights = {});

    [[nodiscard]] DeferredRenderer* getDeferredRenderer() const { return m_deferredRenderer; }

    // ------ SSAO ------ //
    bool createSSAOGenerator();
    void updateSSAOGenerator(int width, int height);
    void renderSSAO();

    [[nodiscard]] SSAOGenerator* getSSAOGenerator() const { return m_ssaoGenerator; }
    [[nodiscard]] bool getSSAOEnabled() const { return m_ssaoEnabled; }
    void setSSAOEnabled(const bool val) { m_ssaoEnabled = val; }

    // ------ Shadows ------ //
    bool createCSMGenerator();
    [[nodiscard]] CSMGenerator* getCSMGenerator() const { return m_csmGenerator; }

    [[nodiscard]] bool getShadowsEnabled() const { return m_shadowsEnabled; }
    void setShadowsEnabled(const bool val) { m_shadowsEnabled = val; }

    void updateLS_UBO();

    // ------ Jolt Physics ------ //
    bool createJoltInstance();

    [[nodiscard]] JoltInstance* getJoltInstance() const { return m_joltInstance; }

    // ------ Font Renderer ------ //
    bool createFontRenderer();

    void initFontRenderer(const char* fontPath, int height);
    [[nodiscard]] FontManager* getFontRenderer() const { return m_fontRenderer; }

    // ------ UI Render ------ //
    bool createUIRenderer();
    [[nodiscard]] UIRenderer* getUIRenderer() const { return m_uiRenderer; }

    // ------ Audio Handler ------ //
    bool createAudioHandler();
    [[nodiscard]] AudioHandler* getAudioHandler() const { return m_audioHandler; }

    // ------ Arena ------ //

    // Arena operations
    // add object to arena and update object ID
    void addObject(EngineObject*& object) const;
    // remove object from arena
    void removeObject(EngineObject*& object) const;
    // remove object at index from arena
    void removeObjectID(unsigned int id) const;

    // window callbacks
    void mouse_callback(double xPosIn, double yPosIn);
    void scroll_callback(double yOffset) const;

    [[nodiscard]] const glm::vec3& getLightDirection() const { return m_lightDirection; }
    void setLightDirection(const glm::vec3& lightDir) { m_lightDirection = glm::normalize(lightDir); }

private:
    // memory manager
    Arena* m_arena{nullptr};

    // ----- Engine components ----- //

    // core components
    Window* m_window{nullptr};
    void (*m_customFramebufferCallback)(void* handler, int width, int height){nullptr};
    void* m_cFBO_Handler{nullptr};

    IOHandler* m_iohandler{nullptr};
    Clock* m_clock{nullptr};

    // managers
    ShaderManager* m_shaderManager{nullptr};
    TextureManager* m_textureManager{nullptr};
    ShapeManager* m_shapeManager{nullptr};
    ModelManager* m_modelManager{nullptr};

    // other components
    PostProcessor* m_postProcessor{nullptr};
    DeferredRenderer* m_deferredRenderer{nullptr};
    SSAOGenerator* m_ssaoGenerator{nullptr};
    bool m_ssaoEnabled{true};

    CSMGenerator* m_csmGenerator{nullptr};
    glm::vec3 m_lightDirection{0.0f};
    bool m_shadowsEnabled{false};
    unsigned int m_lsMatricesUBO{0};

    FontManager* m_fontRenderer{nullptr};
    UIRenderer* m_uiRenderer{nullptr};
    AudioHandler* m_audioHandler{nullptr};

    // Physics
    JoltInstance* m_joltInstance{nullptr};

    // camera stuff
    Camera* m_camera{nullptr};
    float m_camLastX{};
    float m_camLastY{};
    bool m_cameraControls{true};
    float m_screenShake{0.0f};

    // flags
    bool m_checkedShaders{false}; // shaders.json checked
    bool m_loadedShaders{false}; // shaders loaded
    bool m_camFirstMouse{true}; // first mouse movement
    bool m_cameraEnabled{false}; // camera enabled

    // miscellaneous stuff
    std::vector<float> m_deltaTimes{};

    glm::ivec2 m_tempRes{0, 0};
    float m_renderScale{1.0f};
    bool m_resize{false};
    void resizePPChain();
};

#endif
