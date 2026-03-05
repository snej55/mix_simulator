// Created by Jens Kromdijk 27/02/2026

#ifndef PATHFINDING_HPP
#define PATHFINDING_HPP

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#include "core/scene.hpp"
#include "core/bounds.hpp"

#include <array>
#include <vector>

enum class Direction
{
    NORTH,
    SOUTH,
    EAST,
    WEST
};

inline Direction getOpposite(const Direction dir)
{
    switch (dir)
    {
    case Direction::NORTH:
        return Direction::SOUTH;
    case Direction::SOUTH:
        return Direction::NORTH;
    case Direction::EAST:
        return Direction::WEST;
    case Direction::WEST:
        return Direction::EAST;
    }
    return Direction::NORTH;
}

struct TileNode
{
    TileNode(const glm::vec2 position, const glm::vec2 dimensions) : m_position{position}, m_dimensions{dimensions}
    {
        for (std::size_t i{0}; i < 4; ++i)
        {
            m_children[i] = 0;
        }
    }

    TileNode(const glm::vec2 position, const glm::vec2 dimensions, const std::array<std::size_t, 4>& children) :
        m_position{position}, m_dimensions{dimensions}, m_children{children}
    {
    }

    ~TileNode() = default;

    [[nodiscard]] glm::vec2 getCenter() const { return m_position + m_dimensions * 0.5f; }
    [[nodiscard]] float getCost() const { return m_cost + m_bonus; }

    glm::vec2 m_position;
    glm::vec2 m_dimensions;

    std::array<std::size_t, 4> m_children{};
    std::size_t m_parent{0};
    std::size_t m_leafPos{0};
    std::vector<std::pair<std::size_t, Direction>> m_neighbours{};

    bool m_hasParent{false};
    bool m_hasChildren{false};
    bool m_solid{false};

    float m_bonus{0.0f}; // added to cost
    float m_cost{0.0f};
};

class FlowFieldGenerator
{
public:
    /*
     * Extents: extents around center in tile size
     * Center: center position in tile size
     * Height: height above ground in world space to check for static objects
     */
    FlowFieldGenerator(const glm::ivec2& extents, const glm::ivec2& center, float height);
    ~FlowFieldGenerator();

    void init(Scene* scene);

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
    [[nodiscard]] const std::vector<std::size_t>& getBaseNodes() const { return m_baseNodes; }

    // pos: vec3.x, vec3.
    void getNode(const glm::vec2& pos, std::size_t* node, bool* success, bool constrainEdges = false) const;
    void calculateFlowField(const glm::vec2& pos, bool* success, bool constrainEdges = false);
    void clearFlowField();

    void printQuadTree() const;
    void printNode(std::size_t node) const;

private:
    glm::ivec2 m_extents;
    glm::ivec2 m_center;
    float m_height;

    bool m_init{false};
    std::vector<TileNode> m_tileGrid{};
    std::size_t m_numTiles{0};

    std::vector<std::size_t> m_baseNodes{};

    JPH::ShapeRefC m_boxCollider;

    void generateQuadTree(std::size_t node, const std::vector<Bounds::Rect2D*>& neighbourEntities, int depth);
    [[nodiscard]] std::pair<bool, float> checkCollision(std::size_t node,
                                                        const std::vector<Bounds::Rect2D*>& neighbourEntities) const;

    [[nodiscard]] std::size_t getClosestChild(const glm::vec2& pos, std::size_t node) const;
    void getEdgeChildren(std::size_t node, std::pair<std::size_t, std::size_t>& children, Direction direction) const;
    // calls getEdgeChildren recursively to get all edge nodes on boundary
    void getSubEdges(std::size_t node, std::vector<std::size_t>& edges, Direction direction) const;

    void calculateNeighbours(std::size_t node, Direction direction);
    [[nodiscard]] std::size_t findNeighbour(std::size_t node, Direction direction, bool& edge) const;

    // Divides the children (1: top left, 2: top right, 3: bottom left, 4: bottom right)
    void divideNode(std::size_t node);
};

#endif
