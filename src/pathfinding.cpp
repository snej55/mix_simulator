// Created by Jens Kromdijk 27/02/2026

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>

#include "pathfinding.hpp"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "constants.hpp"

FlowFieldGenerator::FlowFieldGenerator(const glm::ivec2& extents, const glm::ivec2& center, const float height) :
    m_extents{extents}, m_center{center}, m_height{height}
{
}

FlowFieldGenerator::~FlowFieldGenerator()
{
    for (std::size_t i{0}; i < m_numTiles; ++i)
    {
        delete m_tileGrid[i];
    }
    delete[] m_tileGrid;
}

void FlowFieldGenerator::init(JoltInstance* jolt)
{
    // initialize tile grid
    const int width{static_cast<int>(static_cast<float>(m_extents.x * 2) / CST::FLOW_FIELD_TILE_SIZE)};
    const int height{static_cast<int>(static_cast<float>(m_extents.y * 2) / CST::FLOW_FIELD_TILE_SIZE)};
    m_numTiles = width * height;

    m_tileGrid = new TileNode*[width * height];
    for (std::size_t y{0}; y < height; ++y)
    {
        for (std::size_t x{0}; x < width; ++x)
        {
            m_tileGrid[y * width + x] = new TileNode{
                {static_cast<float>(m_center.x - m_extents.x) + static_cast<float>(x) * CST::FLOW_FIELD_TILE_SIZE,
                 static_cast<float>(m_center.y - m_extents.y) + static_cast<float>(y) * CST::FLOW_FIELD_TILE_SIZE},
                {CST::FLOW_FIELD_TILE_SIZE, CST::FLOW_FIELD_TILE_SIZE}};
        }
    }
}

void FlowFieldGenerator::initTile(TileNode* node)
{
    // create box collider
    // make sure collider doesn't touch the floor
    JPH::BoxShapeSettings boxSettings{
        JPH::Vec3{node->m_dimensions.x * 0.5f, m_height * 0.49f, node->m_dimensions.y * 0.5f}};
    JPH::Shape::ShapeResult boxResult{boxSettings.Create()};
    JPH::ShapeRefC boxShape{boxResult.Get()};

    //
}
