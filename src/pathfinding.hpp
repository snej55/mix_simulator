// Created by Jens Kromdijk 27/02/2026

#ifndef PATHFINDING_HPP
#define PATHFINDING_HPP

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#include "core/physics.hpp"

#include <array>
#include <vector>

struct TileNode
{
    TileNode(glm::vec2 position, glm::vec2 dimensions) : m_position{position}, m_dimensions{dimensions}
    {
        for (std::size_t i{0}; i < 4; ++i)
        {
            m_children[i] = 0;
        }
    }

    TileNode(glm::vec2 position, glm::vec2 dimensions, std::array<std::size_t, 4> children) :
        m_position{position}, m_dimensions{dimensions}, m_children{children}
    {
    }

    ~TileNode() = default;

    [[nodiscard]] glm::vec2 getCenter() const { return m_position + m_dimensions * 0.5f; }

    glm::vec2 m_position;
    glm::vec2 m_dimensions;

    std::array<std::size_t, 4> m_children;
    bool m_hasChildren{false};
    bool m_solid{false};
    int m_direction{0};
};

class FlowFieldGenerator
{
public:
    /*
     * Extents: extents around center in tilesize
     * Center: center position in tilesize
     * Height: height above ground in world space to check for static objects
     */
    FlowFieldGenerator(const glm::ivec2& extents, const glm::ivec2& center, float height);
    ~FlowFieldGenerator();

    void init(JoltInstance* jolt);

    void setExtents(const glm::ivec2& extents) { m_extents = extents; }
    // extents in tile size around the center
    [[nodiscard]] const glm::ivec2& getExtents() const { return m_extents; }
    void setCenter(const glm::ivec2& center) { m_center = center; }
    // center in tile size
    [[nodiscard]] const glm::ivec2& getCenter() const { return m_center; }
    void setHeight(const float height) { m_height = height; }
    // height above the ground to check for static objects
    [[nodiscard]] float getHeight() const { return m_height; }

    [[nodiscard]] bool getInit() const { return m_init; }
    [[nodiscard]] const std::vector<TileNode>& getTileGrid() const { return m_tileGrid; }

    // pos: vec3.x, vec3.z
    [[nodiscard]] std::size_t getNode(const glm::vec2& pos);

    void printQuadTree();

private:
    glm::ivec2 m_extents;
    glm::ivec2 m_center;
    float m_height;

    bool m_init{false};
    std::vector<TileNode> m_tileGrid{};
    std::size_t m_numTiles{0};

    JPH::ShapeRefC m_boxCollider;

    void generateQuadTree(std::size_t node, JoltInstance* jolt, int depth);
    [[nodiscard]] bool checkCollision(std::size_t node, JoltInstance* jolt);

    // Divides the children (1: top left, 2: top right, 3: bottom left, 4: bottom right)
    void divideNode(std::size_t node);
};

#endif
