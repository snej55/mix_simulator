// Created by Jens Kromdijk 21/03/2026
// Simple class to generate lens dirt texture

#ifndef DIRT_H
#define DIRT_H

#include <glm/glm.hpp>

#include <array>

#include "core/shader.hpp"

#define DIRT_BASE_SIZE 50.f
#define MAX_DIRT 100
#define DIRT_FADE_TIME 60.f

struct Dirt
{
    glm::vec2 m_pos;
    float m_size;
    float m_time;
};

class LensDirt
{
public:
    LensDirt(int width, int height);
    ~LensDirt();

    void update(float deltaTime);
    void renderDirt(const Shader* shader, unsigned int dirtTex);
    void addDirt(const glm::vec2& pos, float size);

    void generate(int width, int height);
    void free();

    void bind() const;

    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }

    [[nodiscard]] unsigned int getFBO() const { return m_FBO; }
    [[nodiscard]] unsigned int getTEX() const { return m_TEX; }
    [[nodiscard]] bool getInit() const { return m_init; }

    [[nodiscard]] const glm::mat4& getProjection() const { return m_projection; }

private:
    int m_width;
    int m_height;

    unsigned int m_FBO{0};
    unsigned int m_TEX{0};
    unsigned int m_VAO{0};
    unsigned int m_VBO{0};
    bool m_init{false};

    std::size_t m_end{0};
    std::array<Dirt, MAX_DIRT> m_dirt{};

    glm::mat4 m_projection{};

    void removeDirt(std::size_t index);
};

#endif // DIRT_H
