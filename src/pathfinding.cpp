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

FlowFieldGenerator::~FlowFieldGenerator() = default;

void FlowFieldGenerator::init(JoltInstance* jolt)
{
    assert(m_tileGrid.size() == 0);

    // initialize tile grid
    const int width{static_cast<int>(static_cast<float>(m_extents.x * 2) / CST::FLOW_FIELD_TILE_SIZE)};
    const int height{static_cast<int>(static_cast<float>(m_extents.y * 2) / CST::FLOW_FIELD_TILE_SIZE)};
    m_numTiles = width * height;

    // create box collider
    JPH::BoxShapeSettings boxSettings{JPH::Vec3{1.0f, 1.0f, 1.0f}};
    m_boxCollider = boxSettings.Create().Get();

    std::cout << "FLOW_FIELD_GENERATOR::INIT: Generating quadtrees!" << std::endl;
    m_tileGrid.reserve(width * height);
    for (std::size_t y{0}; y < height; ++y)
    {
        for (std::size_t x{0}; x < width; ++x)
        {
            m_tileGrid.emplace_back(TileNode{
                {static_cast<float>(m_center.x - m_extents.x) + static_cast<float>(x) * CST::FLOW_FIELD_TILE_SIZE,
                 static_cast<float>(m_center.y - m_extents.y) + static_cast<float>(y) * CST::FLOW_FIELD_TILE_SIZE},
                {CST::FLOW_FIELD_TILE_SIZE, CST::FLOW_FIELD_TILE_SIZE}});
            generateQuadTree(y * width + x, jolt, 0);
        }
    }
    std::cout << "FLOW_FIELD_GENERATOR::INIT: Initialized quadtrees!" << std::endl;
}

void FlowFieldGenerator::generateQuadTree(const std::size_t node, JoltInstance* jolt, const int depth)
{
    const bool intersect{checkCollision(node, jolt)};
    if (!intersect)
    {
        return;
    }

    if (depth >= CST::FLOW_FIELD_DEPTH_LIMIT)
    {
        m_tileGrid[node].m_solid = true;
        return;
    }

    divideNode(node);

    std::array<std::size_t, 4> children{m_tileGrid[node].m_children};
    for (std::size_t i{0}; i < 4; ++i)
    {
        generateQuadTree(children[i], jolt, depth + 1);
    }
}

bool FlowFieldGenerator::checkCollision(const std::size_t node, JoltInstance* jolt)
{
    glm::vec2 center{m_tileGrid[node].getCenter()};
    JPH::Vec3 position{center.x, m_height, center.y};
    JPH::Quat rotation{JPH::Quat::sIdentity()};
    // make sure collider doesn't touch the floor
    JPH::Vec3 scale{m_tileGrid[node].m_dimensions.x * 0.5f, m_height * 0.49f, m_tileGrid[node].m_dimensions.y * 0.5f};

    JPH::AnyHitCollisionCollector<JPH::CollideShapeCollector> collector;
    JPH::SpecifiedObjectLayerFilter staticFilter{ObjectLayers::NON_MOVING};
    JPH::SpecifiedBroadPhaseLayerFilter broadPhaseFilter{BroadPhaseLayers::NON_MOVING};

    jolt->getPhysicsSystem()->GetNarrowPhaseQuery().CollideShape(
        m_boxCollider, scale, JPH::Mat44::sRotationTranslation(rotation, position), JPH::CollideShapeSettings{},
        JPH::Vec3::sZero(), collector, broadPhaseFilter, staticFilter);

    return collector.HadHit();
}

void FlowFieldGenerator::divideNode(const std::size_t node)
{
    TileNode parent{m_tileGrid[node]};
    if (parent.m_hasChildren)
        return;

    for (std::size_t i{0}; i < 4; ++i)
    {
        switch (i)
        {
        case 0:
            m_tileGrid.emplace_back(TileNode{parent.m_position, parent.m_dimensions * 0.5f});
            break;
        case 1:
            m_tileGrid.emplace_back(TileNode{{parent.m_position.x + parent.m_dimensions.x * 0.5f, parent.m_position.y},
                                             parent.m_dimensions * 0.5f});
            break;
        case 2:
            m_tileGrid.emplace_back(TileNode{{parent.m_position.x, parent.m_position.y + parent.m_dimensions.y * 0.5f},
                                             parent.m_dimensions * 0.5f});
            break;
        default:
            m_tileGrid.emplace_back(TileNode{{parent.m_position.x + parent.m_dimensions.x * 0.5f,
                                              parent.m_position.y + parent.m_dimensions.y * 0.5f},
                                             parent.m_dimensions * 0.5f});
            break;
        }
        m_tileGrid[node].m_children[i] = m_tileGrid.size() - 1;
    }
    m_tileGrid[node].m_hasChildren = true;
}
