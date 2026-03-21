// Created by Jens Kromdijk 21/03/2026

#include <glad/glad.h>

#include "dirt.hpp"
#include "core/util.hpp"

#include <iostream>

LensDirt::LensDirt(const int width, const int height) { generate(width, height); }

LensDirt::~LensDirt() { free(); }

void LensDirt::generate(const int width, const int height)
{
    m_width = width;
    m_height = height;

    free();

    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glGenTextures(1, &m_TEX);
    glBindTexture(GL_TEXTURE_2D, m_TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TEX, 0);

    constexpr unsigned int attachments[1]{GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Util::beginError();
        std::cout << "LENS_DIRT::INIT::ERROR: Framebuffer is incomplete!" << std::endl;
        Util::endError();
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_init = true;

    std::cout << "Generated lens dirt FBO (" << m_width << " * " << m_height << ")\n";
}

void LensDirt::free()
{
    if (!m_init)
        return;

    glDeleteTextures(1, &m_TEX);
    glDeleteFramebuffers(1, &m_FBO);
    m_TEX = 0;
    m_FBO = 0;
    m_width = 0;
    m_height = 0;
    m_init = false;
}

void LensDirt::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_FBO); }
