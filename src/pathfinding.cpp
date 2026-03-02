// Created by Jens Kromdijk 27/02/2026

#include "pathfinding.hpp"
#include "constants.hpp"
#include "core/bounds.hpp"

FlowFieldGenerator::FlowFieldGenerator(const glm::ivec2& extents, const glm::ivec2& center, const float height) :
    m_extents{extents}, m_center{center}, m_height{height}
{
}

FlowFieldGenerator::~FlowFieldGenerator() = default;

void FlowFieldGenerator::init(Scene* scene)
{
    assert(m_tileGrid.size() == 0);

    // initialize tile grid
    const int width{static_cast<int>(static_cast<float>(m_extents.x * 2))};
    const int height{static_cast<int>(static_cast<float>(m_extents.y * 2))};
    m_numTiles = width * height;

    // create box collider
    JPH::BoxShapeSettings boxSettings{JPH::Vec3{1.0f, 1.0f, 1.0f}};
    m_boxCollider = boxSettings.Create().Get();

    std::cout << "FLOW_FIELD_GENERATOR::INIT: Generating quadtrees!" << std::endl;

    m_tileGrid.reserve(width * height);
    std::vector<Bounds::Rect2D> staticRects{};
    scene->getStaticRects(staticRects);

    std::cout << m_center.x << " " << m_center.y << std::endl;
    std::cout << m_extents.x << " " << m_extents.y << std::endl;
    for (std::size_t y{0}; y < height; ++y)
    {
        for (std::size_t x{0}; x < width; ++x)
        {
            m_tileGrid.emplace_back(
                TileNode{{static_cast<float>(m_center.x - m_extents.x + x) * CST::FLOW_FIELD_TILE_SIZE,
                          static_cast<float>(m_center.y - m_extents.y + y) * CST::FLOW_FIELD_TILE_SIZE},
                         {CST::FLOW_FIELD_TILE_SIZE, CST::FLOW_FIELD_TILE_SIZE}});

            const Bounds::Rect2D tileRect{
                {static_cast<float>(m_center.x - m_extents.x + x) * CST::FLOW_FIELD_TILE_SIZE +
                     CST::FLOW_FIELD_TILE_SIZE * 0.5f,
                 static_cast<float>(m_center.y - m_extents.y + y) * CST::FLOW_FIELD_TILE_SIZE +
                     CST::FLOW_FIELD_TILE_SIZE * 0.5f},
                {CST::FLOW_FIELD_TILE_SIZE * 0.5f, CST::FLOW_FIELD_TILE_SIZE * 0.5f}};
            std::vector<Bounds::Rect2D*> neighbourEntities{};
            for (std::size_t i{0}; i < staticRects.size(); ++i)
            {
                if (staticRects[i].colliderect(tileRect))
                {
                    neighbourEntities.emplace_back(&staticRects[i]);
                }
            }

            if (neighbourEntities.size() > 0)
            {
                generateQuadTree(y * width + x, neighbourEntities, 0);
            }
        }
    }
    for (const Bounds::Rect2D rect : staticRects)
    {
        std::cout << "[" << rect.m_center.x - rect.m_extents.x << ", " << rect.m_center.y - rect.m_extents.y << ", "
                  << rect.m_extents.x * 2 << ", " << rect.m_extents.y * 2 << "],\n";
    }
    std::cout << "FLOW_FIELD_GENERATOR::INIT: Initialized quadtrees!" << std::endl;
}

void FlowFieldGenerator::generateQuadTree(const std::size_t node, const std::vector<Bounds::Rect2D*>& neighbourEntities,
                                          const int depth)
{
    const bool intersect{checkCollision(node, neighbourEntities)};
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
        generateQuadTree(children[i], neighbourEntities, depth + 1);
    }
}

bool FlowFieldGenerator::checkCollision(const std::size_t node, const std::vector<Bounds::Rect2D*>& neighbourEntities)
{
    // glm::vec2 center{m_tileGrid[node].getCenter()};
    // JPH::Vec3 position{center.x, m_height, center.y};
    // JPH::Quat rotation{JPH::Quat::sIdentity()};
    // // make sure collider doesn't touch the floor
    // JPH::Vec3 scale{m_tileGrid[node].m_dimensions.x * 0.5f, m_height * 0.49f, m_tileGrid[node].m_dimensions.y *
    // 0.5f};

    // JPH::AnyHitCollisionCollector<JPH::CollideShapeCollector> collector;
    // JPH::SpecifiedObjectLayerFilter staticFilter{ObjectLayers::NON_MOVING};
    // JPH::SpecifiedBroadPhaseLayerFilter broadPhaseFilter{BroadPhaseLayers::NON_MOVING};

    // jolt->getPhysicsSystem()->GetNarrowPhaseQuery().CollideShape(
    //     m_boxCollider, scale, JPH::Mat44::sRotationTranslation(rotation, position), JPH::CollideShapeSettings{},
    //     JPH::Vec3::sZero(), collector, broadPhaseFilter, staticFilter);

    Bounds::Rect2D tileRect{m_tileGrid[node].getCenter(), m_tileGrid[node].m_dimensions * 0.5f};
    for (const Bounds::Rect2D* rect : neighbourEntities)
    {
        if (rect->colliderect(tileRect))
        {
            return true;
        }
    }

    return false;
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

std::size_t FlowFieldGenerator::getNode(const glm::vec2& pos)
{
    int gridX{static_cast<int>(pos.x / CST::FLOW_FIELD_TILE_SIZE) - m_extents.x};
    int gridY{static_cast<int>(pos.y / CST::FLOW_FIELD_TILE_SIZE) - m_extents.y};

    return 0;
}

void FlowFieldGenerator::printQuadTree()
{
    for (std::size_t i{0}; i < m_tileGrid.size(); ++i)
    {
        TileNode* node{&m_tileGrid[i]};
        std::cout << std::boolalpha << "{'pos': [" << node->m_position.x << ", " << node->m_position.y
                  << "], 'dimensions': [" << node->m_dimensions.x << ", " << node->m_dimensions.y
                  << "], 'solid': " << node->m_solid << "},\n";
    }
    std::cout << std::endl;
}
