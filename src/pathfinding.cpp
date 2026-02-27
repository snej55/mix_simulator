// Created by Jens Kromdijk 27/02/2026

#include "pathfinding.hpp"

FlowFieldGenerator::FlowFieldGenerator(const glm::ivec2& extents, const glm::ivec2& center, const float height) :
    m_extents{extents}, m_center{center}, m_height{height}
{
}

FlowFieldGenerator::~FlowFieldGenerator() {}

void FlowFieldGenerator::init(JoltInstance* jolt)
{
    // initialize tile grid
    const unsigned int width{static_cast<unsigned int>(m_extents.x) * 2};
    const unsigned int height{static_cast<unsigned int>(m_extents.y) * 2};
}
