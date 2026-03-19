// Created by Jens Kromdijk 16/06/2026

#include <glad/glad.h>

#include <cassert>

#include "particles.hpp"

ParticleManager::ParticleManager(Shader* particlesShader) : m_shader{particlesShader} { init(); }

ParticleManager::~ParticleManager()
{
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_instanceVBO);
    glDeleteVertexArrays(1, &m_VAO);
}

void ParticleManager::update(const float dt)
{
    constexpr float friction{0.08f};
    constexpr float decay{0.01f};

    std::size_t i{0};
    while (i < m_end)
    {
        Particle& particle{m_particles[i]};

        particle.pos += particle.vel * dt;
        particle.vel *= 1.0f - friction * dt;
        particle.vel.y -= 0.01f * dt;
        particle.size -= decay * dt;

        if (particle.size <= 0.0f)
        {
            remove(i);
        }
        else
        {
            ++i;
        }
    }
}

void ParticleManager::render()
{
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_end * sizeof(Particle), m_particles.data());

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_end);
}

void ParticleManager::addParticle(const glm::vec3& position, const glm::vec3& vel, const float size)
{
    if (m_end >= MAX_PARTICLES)
    {
        return;
    }

    m_particles[m_end] = Particle{position, vel * size, size};
    ++m_end;
}

void ParticleManager::remove(const std::size_t index)
{
    assert(index < MAX_PARTICLES);

    if (m_end == 0 || index >= m_end)
        return;

    std::swap(m_particles[index], m_particles[m_end - 1]);
    --m_end;
}

// setup instanced VAO & VBO
void ParticleManager::init()
{
    constexpr float quadVertices[]{
        -0.5f, 0.5f, // TL
        -0.5f, -0.5f, // BL
        0.5f,  0.5f, // TR
        0.5f,  -0.5f // BR
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_instanceVBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * MAX_PARTICLES, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<void*>(offsetof(Particle, pos)));
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<void*>(offsetof(Particle, vel)));
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          reinterpret_cast<void*>(offsetof(Particle, size)));
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}
