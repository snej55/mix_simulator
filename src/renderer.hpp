//
// Created by jenskromdijk on 23/11/2025.
//

#ifndef MAIN_RENDERER_H
#define MAIN_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "engine_types.hpp"
#include "shader.hpp"

class DeferredRenderer final : public EngineObject
{
public:
    explicit DeferredRenderer(EngineObject* parent);
    ~DeferredRenderer() override;

    // setup framebuffers
    void init(int scrWidth, int scrHeight);
    void free();

    // render gbuffer
    void renderQuad();

    // setup for geometry pass
    void setupGeometryPass(const Shader* gpShader, const glm::mat4& projection, const glm::mat4& view);
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
    void freeQuad();
};


#endif // MAIN_RENDERER_H
