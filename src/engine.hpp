#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>

#include "arena.hpp"
#include "camera.hpp"
#include "clock.hpp"
#include "engine_types.hpp"
#include "iohandler.hpp"
#include "model.hpp"
#include "postprocessing.hpp"
#include "shader.hpp"
#include "shapes.hpp"
#include "texture.hpp"
#include "window.hpp"

class Engine final : public EngineObject
{
public:
    Engine();
    // free components
    ~Engine() override;

    // initialize components
    bool init(int width, int height, const char* title);
    // update components
    void update();

    // ------ Window ------ //

    // create window object
    bool createWindow(int width, int height, const char* title);
    // window getters
    [[nodiscard]] Window* getWindow() const { return m_window; }
    [[nodiscard]] int getWidth() const { return m_window->getWidth(); }
    [[nodiscard]] int getHeight() const { return m_window->getHeight(); }

    // clear screen
    void clear() const;

    // enable wireframe rendering
    void enableWireframe() const;

    void disableWireframe() const;

    void displayFrameTime();

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
    // get time since start from clock in milliseconds
    [[nodiscard]] float getTime() const;

    // ------ Shaders ------ //

    // create shader manager
    bool createShaderManager();
    [[nodiscard]] ShaderManager* getShaderManager() const { return m_shaderManager; }

    // shader manager methods
    void addShader(const std::string& name, const char* fragPath, const char* vertPath) const;
    [[nodiscard]] Shader* getShader(const std::string& name) const;
    void useShader(const std::string& name) const;
    [[nodiscard]] bool shaderExists(const std::string& name) const;

    // verify shaders exist
    [[nodiscard]] bool checkShaders();
    // load shaders from shaders.json
    void loadShaders();

    [[nodiscard]] bool getShadersChecked() const { return m_checkedShaders; }
    [[nodiscard]] bool getShadersLoaded() const { return m_loadedShaders; }

    // shader uniform setters
    // shader uniforms
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

    // ------ Shapes ------ //

    bool createShapeManager();
    [[nodiscard]] ShapeManager* getShapeManager() const { return m_shapeManager; }

    void drawRect(const FRect& rect, const Color& color) const;

    // color functions
    //
    // lerpColor() lerps the two colors rgb only, use lerpColorAlpha for rgba
    [[nodiscard]] Color lerpColor(const Color& a, const Color& b, float amount) const;
    // lerpColorAlpha() lerps the two colors rgba, use lerpColor for rgb only
    [[nodiscard]] Color lerpColorAlpha(const Color& a, const Color& b, float amount) const;

    // ------ Camera ------ //

    bool createCamera();
    [[nodiscard]] Camera* getCamera() const { return m_camera; }

    // view & perspective matrices getters
    [[nodiscard]] glm::mat4 getViewMatrix() const;
    [[nodiscard]] glm::mat4 getProjectionMatrix() const;
    [[nodiscard]] glm::vec3 getCameraPosition() const;

    // get normal mat from model mat
    [[nodiscard]] glm::mat4 getNormalMatrix(const glm::mat4& model) const;

    // enable or disable window mouse capture
    void setCameraEnabled(bool value);
    [[nodiscard]] bool getCameraEnabled() const { return m_cameraEnabled; };

    // ------ Models ------ //

    bool createModelManager();
    [[nodiscard]] ModelManager* getModelManager() const { return m_modelManager; }

    void addModel(const std::string& name, const std::string& path) const;
    [[nodiscard]] Model* getModel(const std::string& name) const;
    void renderModel(const std::string& name, const Shader* shader) const;
    [[nodiscard]] bool modelExists(const std::string& name) const;

    // ------ Post Processor ------ //

    bool createPostProcessor();
    [[nodiscard]] PostProcessor* getPostProcessor() const { return m_postProcessor; }

    // enable and disable rendering to post processor
    void enablePostProcessing() const;
    void disablePostProcessing() const;

    // render post processor with screen shader
    void renderPostProcessing() const;

    // update framebuffer
    void updatePostProcessor(int width, int height) const;

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

private:
    // memory manager
    Arena* m_arena{nullptr};

    // ----- Engine components ----- //

    // core components
    Window* m_window{nullptr};
    IOHandler* m_iohandler{nullptr};
    Clock* m_clock{nullptr};

    // managers
    ShaderManager* m_shaderManager{nullptr};
    TextureManager* m_textureManager{nullptr};
    ShapeManager* m_shapeManager{nullptr};
    ModelManager* m_modelManager{nullptr};

    // other components
    PostProcessor* m_postProcessor{nullptr};

    // camera stuff
    Camera* m_camera{nullptr};
    float m_camLastX{};
    float m_camLastY{};

    // flags
    bool m_checkedShaders{false}; // shaders.json checked
    bool m_loadedShaders{false}; // shaders loaded
    bool m_camFirstMouse{true}; // first mouse movement
    bool m_cameraEnabled{false}; // camera enabled

    // miscallaneous stuff
    std::vector<float> m_deltaTimes{};
};

#endif
