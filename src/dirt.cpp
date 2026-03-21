// Created by Jens Kromdijk 21/03/2026

#include <glad/glad.h>

#include "dirt.hpp"
#include "core/util.hpp"

#include <iostream>

LensDirt::LensDirt(const int width, const int height) { generate(width, height); }

LensDirt::~LensDirt()
{
    free();

    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
}

void LensDirt::update(const float deltaTime)
{
    std::size_t i{0};
    while (i < m_end)
    {
        Dirt& dirt{m_dirt[i]};
        dirt.m_time += deltaTime;
        if (dirt.m_time >= DIRT_FADE_TIME)
        {
            removeDirt(i);
        }
        else
        {
            ++i;
        }
    }
}

void LensDirt::renderDirt(const Shader* shader, const unsigned int dirtTex)
{
    bind();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shader->use();
    shader->setMat4("projection", m_projection);
    shader->setInt("tex", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dirtTex);

    glBindVertexArray(m_VAO);
    for (std::size_t i{0}; i < m_end; ++i)
    {
        const Dirt& d{m_dirt[i]};
        shader->setFloat("size", d.m_size);
        shader->setFloat("fade", 1.f - (d.m_time / DIRT_FADE_TIME));

        const float w{d.m_size * DIRT_BASE_SIZE};
        const float h{d.m_size * DIRT_BASE_SIZE};
        const float x{d.m_pos.x};
        const float y{d.m_pos.y - h};

        const float vertices[6][4]{// 1st triangle
                                   {x, y + h, 0.0f, 0.0f},
                                   {x, y, 0.0f, 1.0f},
                                   {x + w, y, 1.0f, 1.0f},
                                   // second triangle
                                   {x, y + h, 0.0f, 0.0f},
                                   {x + w, y, 1.0f, 1.0f},
                                   {x + w, y + h, 1.0f, 0.0f}};

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void LensDirt::addDirt(const glm::vec2& pos, const float size)
{
    if (m_end >= MAX_DIRT)
    {
        return;
    }

    m_dirt[m_end] = Dirt{pos, size, 0.0f};
    ++m_end;
}

void LensDirt::removeDirt(const std::size_t index)
{
    assert(index < MAX_DIRT);

    if (m_end == 0 || index >= m_end)
    {
        return;
    }

    std::swap(m_dirt[index], m_dirt[m_end - 1]);
    --m_end;
}

void LensDirt::generate(const int width, const int height)
{
    m_width = width;
    m_height = height;

    free();

    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glGenTextures(1, &m_TEX);
    glBindTexture(GL_TEXTURE_2D, m_TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

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

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_init = true;

    m_projection = glm::ortho(0.0f, static_cast<float>(m_width), 0.0f, static_cast<float>(m_height));

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

void LensDirt::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glViewport(0, 0, m_width, m_height);
    glDisable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
