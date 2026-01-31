// Created by Jens Kromdijk 25-01-2026

#include <glad/glad.h>

#include "ui.hpp"

#include "shapes.hpp"
#include "util.hpp"

UI::Button::Button(const FRect rect) : m_rect{rect} {}
UI::Button::Button(const float x, const float y, const float width, const float height) : m_rect{x, y, width, height} {}

void UI::Button::update(const double cposX, const double cposY)
{
    float cposXf{static_cast<float>(cposX)};
    float cposYf{static_cast<float>(cposY)};

    m_highlighted = (m_rect.x <= cposXf) && (cposXf <= m_rect.x + m_rect.w) && (m_rect.y <= cposYf) &&
        (cposYf <= m_rect.y + m_rect.h);
}

UIRenderer::UIRenderer(EngineObject* parent) : EngineObject{"UIRenderer", parent} {}

UIRenderer::~UIRenderer() { free(); }

void UIRenderer::free() const
{
    glDeleteTextures(1, &m_TEX);
    glDeleteFramebuffers(1, &m_FBO);
}

void UIRenderer::init(const int width, const int height)
{
    m_width = width;
    m_height = height;
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glGenTextures(1, &m_TEX);
    glBindTexture(GL_TEXTURE_2D, m_TEX);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TEX, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Util::beginError();
        std::cout << "UI_RENDERER::INIT::ERROR: Failed to create framebuffer!";
        Util::endError();
    }
}

void UIRenderer::generate(const int width, const int height)
{
    free();
    init(width, height);
    std::cout << "UI_RENDERER::GENERATE: Regenerated framebuffer: " << width << " * " << height << '\n';
}
