// Created by Jens Kromdijk 27/02/2026

#ifndef PATHFINDING_HPP
#define PATHFINDING_HPP

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

class FlowFieldGenerator
{
public:
    /*
     * Extents: extents around center in tilesize
     * Center: center position in tilesize
     * Height: height above ground in world space to check for static objects
     */
    FlowFieldGenerator(const glm::ivec2& extents, const glm::ivec2& center, float height);

    void setExtents(const glm::ivec2& extents) { m_extents = extents; }
    // extents in tile size around the center
    [[nodiscard]] const glm::ivec2& getExtents() const { return m_extents; }
    void setCenter(const glm::ivec2& center) { m_center = center; }
    // center in tile size
    [[nodiscard]] const glm::ivec2& getCenter() const { return m_center; }
    void setHeight(const float height) { m_height = height; }
    // height above the ground to check for static objects
    [[nodiscard]] float getHeight() const { return m_height; }

private:
    glm::ivec2 m_extents;
    glm::ivec2 m_center;
    float m_height;
};

#endif
