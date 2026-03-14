// Created by Jens Kromdijk 14/03/2026

#ifndef PARTICLES_H
#define PARTICLES_H

#define MAX_PARTICLES 1024

#include <array>

#include <glm/glm.hpp>

struct Particle
{
    glm::vec3 pos;
    glm::vec3 vel;
    float size{1.0f};
};

class ParticleManager
{
public:
    ParticleManager() = default;
    ~ParticleManager();

    void update(float dt);
    void addParticle(const glm::vec3& position, const glm::vec3& vel);

    [[nodiscard]] const std::array<Particle*, MAX_PARTICLES>& getParticles() const { return m_particles; }
    [[nodiscard]] std::size_t getEnd() const { return m_end; }

private:
    std::array<Particle*, MAX_PARTICLES> m_particles{};
    std::size_t m_end{0};

    void remove(std::size_t index);
};

#endif
