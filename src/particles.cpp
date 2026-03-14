// Created by Jens Kromdijk 16/06/2026

#include "particles.hpp"

#include <cassert>

void ParticleManager::update(const float dt)
{
    constexpr float friction{0.98f};
    constexpr float decay{0.01f};

    for (std::size_t i{0}; i < m_particles.size(); ++i)
    {
        Particle* particle{m_particles[i]};

        particle->pos.x += particle->vel.x * dt;
        particle->vel.x += (particle->vel.x * friction - particle->vel.x) * dt;
        particle->pos.y += particle->vel.y * dt;
        particle->vel.y += (particle->vel.y * friction - particle->vel.y) * dt;

        particle->size -= decay * dt;
        if (particle->size <= 0.0f)
        {
            remove(i);
            --i;
        }
    }
}

void ParticleManager::addParticle(const glm::vec3& position, const glm::vec3& vel)
{
    if (m_end >= MAX_PARTICLES)
        return;

    m_particles[m_end] = new Particle{position, vel};
    ++m_end;
}

void ParticleManager::remove(const std::size_t index)
{
    assert(index < MAX_PARTICLES);

    std::swap(m_particles[index], m_particles[m_end - 1]);
    delete m_particles[m_end - 1];
    m_particles[m_end - 1] = nullptr;
    if (m_end > 0)
        --m_end;
}
