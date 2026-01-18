//
// Created by jenskromdijk on 23/11/2025.
//

#ifndef MAIN_RENDERER_H
#define MAIN_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "engine_types.hpp"
#include "shader.hpp"
#include "model.hpp"
#include "postprocessing.hpp"
#include "ibl.hpp"
#include "scene.hpp"
#include "lights.hpp"

class DeferredRenderer final : public EngineObject
{
public:
    explicit DeferredRenderer(EngineObject* parent);
    ~DeferredRenderer() override;

    // setup framebuffers
    void init(int scrWidth, int scrHeight);
    void free();

    // render gbuffer
    void renderQuad() const;

    // setup for geometry pass
    void setupGeometryPass(const Shader* gpShader, const glm::mat4& projection, const glm::mat4& view) const;
    // unbind framebuffer
    void closeGeometryPass() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    // -------- getters -------- //
    [[nodiscard]] unsigned int getGBuffer() const { return m_gBuffer; }
    // different gBuffer components
    [[nodiscard]] unsigned int getPositionEBuffer() const { return m_positionEBuffer; }
    [[nodiscard]] unsigned int getColorBuffer() const { return m_colorBuffer; }
    [[nodiscard]] unsigned int getNormalEBuffer() const { return m_normalEBuffer; }
    [[nodiscard]] unsigned int getARMEBuffer() const { return m_ARMEBuffer; }

    [[nodiscard]] unsigned int getRenderbuffer() const { return m_RBO; }
    [[nodiscard]] int getWidth() const { return m_scrWidth; }
    [[nodiscard]] int getHeight() const { return m_scrHeight; }

    [[nodiscard]] bool getInitFlag() const { return m_init; }

private:
    // graphics buffer
    unsigned int m_gBuffer{};
    // positionEBuffer.xyz = Position
    // positionEBuffer.w = Emissive r
    unsigned int m_positionEBuffer{};
    // colorBuffer.xyzw = rgba
    unsigned int m_colorBuffer{};
    // normalEBuffer.xyz = Normal
    // normalEBuffer.w = Emissive.g
    unsigned int m_normalEBuffer{};
    // x = AO
    // y = Roughness
    // z = Metallic
    // a = Emissive.b
    unsigned int m_ARMEBuffer{};
    // render buffer
    unsigned int m_RBO{};

    // view port dimensions
    int m_scrWidth{};
    int m_scrHeight{};

    // init flag
    bool m_init{false};

    // quad vertex array object
    unsigned int m_quadVAO{};
    unsigned int m_quadVBO{};

    void initQuad();
    void freeQuad() const;
};

class RenderQueue final : public EngineObject
{
public:
    explicit RenderQueue(EngineObject* parent);
    ~RenderQueue() override;

    // clear queue
    void update();

    // hybrid renderer
    void renderFrame(const Shader* dfShader, const DeferredRenderer* dfRenderer, const Shader* fdShader,
                     const PostProcessor* postProcessor, void* engine, IBLGenerator* ibl, const glm::vec3& cameraPos,
                     const std::vector<Lights::PointLight*>& pointLights = {}) const;

    // add static model
    void addStaticModel(const Model* model, const glm::mat4& modelTransform);
    // add dynamic model (updated every frame)
    void addDynamicModel(Model* model, const glm::mat4& modelTransform);

    void addChunk(const SceneChunk* chunk, const Bounds::Frustum& camFrustum);

    // meshes to be rendered deferred
    [[nodiscard]] const std::vector<std::pair<Mesh*, glm::mat4>>& getStaticOpaqueMeshes() const
    {
        return m_staticOpaqueMeshes;
    }

    // meshes to be rendered forwardly
    [[nodiscard]] const std::vector<std::pair<Mesh*, glm::mat4>>& getStaticBlendMeshes() const
    {
        return m_staticBlendMeshes;
    }

    // dynamic models (updated every frame)
    [[nodiscard]] const std::vector<std::pair<Model*, glm::mat4>>& getDynamicModels() const { return m_dynamicModels; }

private:
    std::vector<std::pair<Mesh*, glm::mat4>> m_staticOpaqueMeshes{};
    std::vector<std::pair<Mesh*, glm::mat4>> m_staticBlendMeshes{};
    std::vector<std::pair<Model*, glm::mat4>> m_dynamicModels{};

    /*NOTE: Add previous capacity thing for dynamic vector*/

    // for static meshes
    void renderOpaqueMeshes(const Shader* dfShader) const;
    void renderBlendMeshes(const Shader* fdShader) const;
};

#endif // MAIN_RENDERER_H
