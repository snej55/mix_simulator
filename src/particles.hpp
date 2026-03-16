// Created by Jens Kromdijk 14/03/2026

#ifndef PARTICLES_H
#define PARTICLES_H

#define MAX_PARTICLES 10000

#include <array>

#include <glm/glm.hpp>

#include "core/shader.hpp"

struct Particle
{
    glm::vec3 pos;
    glm::vec3 vel;
    float size;
};

class ParticleManager
{
public:
    explicit ParticleManager(Shader* particleShader);
    ~ParticleManager();

    void update(float dt);
    // NOTE: Use shader first, this only renders VAO
    void render();
    void addParticle(const glm::vec3& position, const glm::vec3& vel, float size = 1.f);

    [[nodiscard]] const Shader* getShader() const { return m_shader; }
    [[nodiscard]] const std::array<Particle, MAX_PARTICLES>& getParticles() const { return m_particles; }
    [[nodiscard]] std::size_t getEnd() const { return m_end; }

    [[nodiscard]] unsigned int getVAO() const { return m_VAO; }
    [[nodiscard]] unsigned int getVBO() const { return m_VBO; }
    [[nodiscard]] unsigned int getInstanceVBO() const { return m_instanceVBO; }

private:
    Shader* m_shader;

    std::array<Particle, MAX_PARTICLES> m_particles{};
    std::size_t m_end{0};

    unsigned int m_VAO{0};
    unsigned int m_VBO{0};
    unsigned int m_instanceVBO{0};

    void remove(std::size_t index);
    void init();
};

#endif
