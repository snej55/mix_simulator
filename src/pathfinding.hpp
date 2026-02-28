// Created by Jens Kromdijk 27/02/2026

#ifndef PATHFINDING_HPP
#define PATHFINDING_HPP

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#include "core/physics.hpp"

#include <array>

struct TileNode
{
    TileNode(glm::vec2 position, glm::vec2 dimensions) : m_position{position}, m_dimensions{dimensions}
    {
        for (std::size_t i{0}; i < 4; ++i)
        {
            m_children[i] = nullptr;
        }
    }

    TileNode(glm::vec2 position, glm::vec2 dimensions, std::array<TileNode*, 4> children) :
        m_position{position}, m_dimensions{dimensions}, m_children{children}
    {
    }

    ~TileNode()
    {
        for (std::size_t i{0}; i < 4; ++i)
        {
            delete m_children[i];
        }
    }

    /* Divides the children
     * ___________
     * | 1  |  2 |
     * |---------|
     * | 3  |  4 |
     * -----------
     */
    void divide()
    {
        if (m_children[0] == nullptr)
            return;

        for (std::size_t i{0}; i < 4; ++i)
        {
            TileNode* node;
            switch (i)
            {
            case 0:
                node = new TileNode{m_position, m_dimensions * 0.5f};
                break;
            case 1:
                node = new TileNode{{m_position.x + m_dimensions.x * 0.5f, m_position.y}, m_dimensions * 0.5f};
                break;
            case 2:
                node = new TileNode{{m_position.x, m_position.y + m_dimensions.y * 0.5f}, m_dimensions * 0.5f};
                break;
            default:
                node = new TileNode{{m_position.x + m_dimensions.y * 0.5f, m_position.y + m_dimensions.y * 0.5f},
                                    m_dimensions * 0.5f};
                break;
            }
            m_children[i] = node;
        }
    }

    [[nodiscard]] bool getDivided() const { return m_children[0] != nullptr; }
    [[nodiscard]] glm::vec2 getCenter() const { return m_position + m_dimensions * 0.5f; }

    glm::vec2 m_position;
    glm::vec2 m_dimensions;

    std::array<TileNode*, 4> m_children;
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
    [[nodiscard]] TileNode** getTileGrid() const { return m_tileGrid; }

    void initTile(TileNode* node);

private:
    glm::ivec2 m_extents;
    glm::ivec2 m_center;
    float m_height;

    bool m_init{false};
    TileNode** m_tileGrid{nullptr};
    std::size_t m_numTiles{0};
};

#endif
