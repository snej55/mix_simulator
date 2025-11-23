//
// Created by jenskromdijk on 23/11/2025.
//

#include <glad/glad.h>

#include "renderer.hpp"
#include "util.hpp"

DeferredRenderer::DeferredRenderer(EngineObject* parent) : EngineObject{"DeferredRenderer", parent} {}

DeferredRenderer::~DeferredRenderer() = default;

void DeferredRenderer::init(const unsigned int scrWidth, const unsigned int scrHeight)
{
    // create graphics buffer framebuffer
    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    // position color buffer
    glGenTextures(1, &m_positionEBuffer);
    glBindTexture(GL_TEXTURE_2D, m_positionEBuffer);
    // NOTE: upgrade to GL_RGBA32F incase of precision artifacts
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, scrWidth, scrHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_positionEBuffer, 0);
    // albedo color buffer
    glGenTextures(1, &m_colorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, scrWidth, scrHeight, 0, GL_RGBA8, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_colorBuffer, 0);
    // normal color buffer
    glGenTextures(1, &m_normalEBuffer);
    glBindTexture(GL_TEXTURE_2D, m_normalEBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, scrWidth, scrHeight, 0, GL_RGBA16F, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_normalEBuffer, 0);
    // AMRE color buffer (AO, metallic, roughness, emissive)
    glGenTextures(1, &m_ARMEBuffer);
    glBindTexture(GL_TEXTURE_2D, m_ARMEBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, scrWidth, scrHeight, 0, GL_RGBA8, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_ARMEBuffer, 0);

    // add color attachments to framebuffer (let OpenGL know which ones to use for rendering)
    unsigned int attachments[4]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(4, attachments);

    glGenRenderbuffers(1, &m_RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scrWidth, scrHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

    // check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Util::beginError();
        std::cout << "DEFERRED_RENDERER::INIT::ERROR: Framebuffer is not complete!" << std::endl;
        Util::endError();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
