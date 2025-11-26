//
// Created by Jens Kromdijk on 23/11/2025.
//

#include <glad/glad.h>

#include "renderer.hpp"
#include "util.hpp"

DeferredRenderer::DeferredRenderer(EngineObject* parent) : EngineObject{"DeferredRenderer", parent} { initQuad(); }

DeferredRenderer::~DeferredRenderer()
{
    freeQuad();
    free();
}

void DeferredRenderer::init(const int scrWidth, const int scrHeight)
{
    if (m_init)
        free();

    // create graphics buffer framebuffer
    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    // position color buffer
    glGenTextures(1, &m_positionEBuffer);
    glBindTexture(GL_TEXTURE_2D, m_positionEBuffer);
    // NOTE: upgrade to GL_RGBA32F in case of precision artifacts
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, scrWidth, scrHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_positionEBuffer, 0);
    // albedo color buffer
    glGenTextures(1, &m_colorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, scrWidth, scrHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_colorBuffer, 0);
    // normal color buffer
    glGenTextures(1, &m_normalEBuffer);
    glBindTexture(GL_TEXTURE_2D, m_normalEBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, scrWidth, scrHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_normalEBuffer, 0);
    // AMRE color buffer (AO, metallic, roughness, emissive)
    glGenTextures(1, &m_ARMEBuffer);
    glBindTexture(GL_TEXTURE_2D, m_ARMEBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, scrWidth, scrHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_ARMEBuffer, 0);

    // add color attachments to framebuffer (let OpenGL know which ones to use for rendering)
    unsigned int attachments[4]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, attachments);

    glGenRenderbuffers(1, &m_RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, scrWidth, scrHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

    // check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Util::beginError();
        std::cout << "DEFERRED_RENDERER::INIT::ERROR: Framebuffer is not complete!" << std::endl;
        Util::endError();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_scrWidth = scrWidth;
    m_scrHeight = scrHeight;
    m_init = true;
}

void DeferredRenderer::free()
{
    if (m_init)
    {
        glDeleteTextures(1, &m_ARMEBuffer);
        glDeleteTextures(1, &m_colorBuffer);
        glDeleteTextures(1, &m_normalEBuffer);
        glDeleteTextures(1, &m_positionEBuffer);
        glDeleteRenderbuffers(1, &m_RBO);
        glDeleteFramebuffers(1, &m_gBuffer);
        m_scrWidth = 0;
        m_scrHeight = 0;
        m_init = false;
    }
}

void DeferredRenderer::renderQuad() const
{
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void DeferredRenderer::setupGeometryPass(const Shader* gpShader, const glm::mat4& projection, const glm::mat4& view) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    gpShader->use();
    gpShader->setMat4("projection", projection);
    gpShader->setMat4("view", view);
}

// setup quad for rendering
void DeferredRenderer::initQuad()
{
    constexpr float quadVertices[]{
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
}

void DeferredRenderer::freeQuad() const
{
    glDeleteBuffers(1, &m_quadVBO);
    glDeleteVertexArrays(1, &m_quadVAO);
}

RenderQueue::RenderQueue(EngineObject* parent)
    : EngineObject{"RenderQueue", parent}
{
}

RenderQueue::~RenderQueue() = default;

void RenderQueue::update()
{
    m_dynamicOpaqueMeshes.clear();
    m_dynamicBlendMeshes.clear();
}

void RenderQueue::render(const Shader* shader, const glm::vec3& cameraPos)
{
}

void RenderQueue::addStaticModel(Model* model, const glm::mat4& modelTransform)
{
}

void RenderQueue::addDynamicModel(Model* model, const glm::mat4& modelTransform)
{
}

void RenderQueue::renderOpaqueMeshes()
{
}

void RenderQueue::renderBlendMeshes()
{
}
