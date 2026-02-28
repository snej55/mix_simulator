// Created by Jens Kromdijk 27/02/2026

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include "pathfinding.hpp"
#include "constants.hpp"
#include "core/physics.hpp"

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

    // create box collider
    JPH::BoxShapeSettings boxSettings{JPH::Vec3{1.0f, 1.0f, 1.0f}};
    m_boxCollider = boxSettings.Create().Get();

    m_tileGrid = new TileNode*[width * height];
    for (std::size_t y{0}; y < height; ++y)
    {
        for (std::size_t x{0}; x < width; ++x)
        {
            TileNode* node{new TileNode{
                {static_cast<float>(m_center.x - m_extents.x) + static_cast<float>(x) * CST::FLOW_FIELD_TILE_SIZE,
                 static_cast<float>(m_center.y - m_extents.y) + static_cast<float>(y) * CST::FLOW_FIELD_TILE_SIZE},
                {CST::FLOW_FIELD_TILE_SIZE, CST::FLOW_FIELD_TILE_SIZE}}};
            generateQuadTree(node, jolt, 0);
            m_tileGrid[y * width + x] = node;
        }
    }
}

void FlowFieldGenerator::generateQuadTree(TileNode* node, JoltInstance* jolt, const int depth)
{
    const bool intersect{checkCollision(node, jolt)};
    if (!intersect)
    {
        return;
    }

    if (depth >= CST::FLOW_FIELD_DEPTH_LIMIT)
    {
        node->m_solid = true;
        return;
    }

    node->divide();

    for (std::size_t i{0}; i < 4; ++i)
    {
        if (node->m_children[i] != nullptr)
        {
            generateQuadTree(node->m_children[i], jolt, depth + 1);
        }
    }
}

bool FlowFieldGenerator::checkCollision(TileNode* node, JoltInstance* jolt)
{
    glm::vec2 center{node->getCenter()};
    JPH::Vec3 position{center.x, m_height, center.y};
    JPH::Quat rotation{JPH::Quat::sIdentity()};
    // make sure collider doesn't touch the floor
    JPH::Vec3 scale{node->m_dimensions.x * 0.5f, m_height * 0.49f, node->m_dimensions.y * 0.5f};

    JPH::AnyHitCollisionCollector<JPH::CollideShapeCollector> collector;
    JPH::SpecifiedObjectLayerFilter staticFilter{ObjectLayers::NON_MOVING};
    JPH::SpecifiedBroadPhaseLayerFilter broadPhaseFilter{BroadPhaseLayers::NON_MOVING};

    jolt->getPhysicsSystem()->GetNarrowPhaseQuery().CollideShape(
        m_boxCollider, scale, JPH::Mat44::sRotationTranslation(rotation, position), JPH::CollideShapeSettings{},
        JPH::Vec3::sZero(), collector, broadPhaseFilter, staticFilter);

    return collector.HadHit();
}
