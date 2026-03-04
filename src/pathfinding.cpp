// Created by Jens Kromdijk 27/02/2026

#include "pathfinding.hpp"
#include <cstddef>
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

    std::cout << "FLOW_FIELD_GENERATOR::INIT: Generating quadtrees!" << std::endl;

    m_tileGrid.reserve(width * height * std::pow(2, CST::FLOW_FIELD_DEPTH_LIMIT));
    m_baseNodes.reserve(width * height);

    std::vector<Bounds::Rect2D> staticRects{};
    scene->getStaticRects(staticRects);

    for (int y{0}; y < height; ++y)
    {
        for (int x{0}; x < width; ++x)
        {
            TileNode node{{static_cast<float>(m_center.x - m_extents.x + x) * CST::FLOW_FIELD_TILE_SIZE,
                           static_cast<float>(m_center.y - m_extents.y + y) * CST::FLOW_FIELD_TILE_SIZE},
                          {CST::FLOW_FIELD_TILE_SIZE, CST::FLOW_FIELD_TILE_SIZE}};

            const std::size_t root{m_tileGrid.size()};
            m_tileGrid.emplace_back(node);
            m_baseNodes.emplace_back(root);

            const Bounds::Rect2D tileRect{m_tileGrid[root].getCenter(), m_tileGrid[root].m_dimensions * 0.5f};
            std::vector<Bounds::Rect2D*> neighbourEntities{};
            for (std::size_t i{0}; i < staticRects.size(); ++i)
            {
                if (staticRects[i].collideRect(tileRect))
                {
                    neighbourEntities.emplace_back(&staticRects[i]);
                }
            }

            if (!neighbourEntities.empty())
            {
                generateQuadTree(root, neighbourEntities, 0);
            }
        }
    }

    // for (int y{0}; y < height; ++y)
    // {
    //     for (int x{0}; x < width; ++x)
    //     {
    //         const std::size_t node1{m_baseNodes[y * width + x]};
    //         if (x + 1 < width)
    //         {
    //             const std::size_t node2{m_baseNodes[y * width + x + 1]};
    //             calculateNeighbours(node1, node2, Direction::EAST);
    //         }
    //         if (y + 1 < height)
    //         {
    //             const std::size_t node2{m_baseNodes[y * width + width + x]};
    //             calculateNeighbours(node1, node2, Direction::SOUTH);
    //         }
    //     }
    // }
    std::cout << "FLOW_FIELD_GENERATOR::INIT: Initialized quadtrees!" << std::endl;
    m_init = true;
}

void FlowFieldGenerator::generateQuadTree(const std::size_t node, const std::vector<Bounds::Rect2D*>& neighbourEntities,
                                          const int depth)
{
    const std::pair<bool, float> intersect{checkCollision(node, neighbourEntities)};
    if (!intersect.first)
    {
        return;
    }

    if (intersect.second >= 1.f - static_cast<float>(depth) * 0.02f)
    {
        m_tileGrid[node].m_solid = true;
        return;
    }

    if (depth >= CST::FLOW_FIELD_DEPTH_LIMIT)
    {
        m_tileGrid[node].m_solid = true;
        return;
    }

    divideNode(node);

    const std::array<std::size_t, 4> children{m_tileGrid[node].m_children};
    for (std::size_t i{0}; i < 4; ++i)
    {
        generateQuadTree(children[i], neighbourEntities, depth + 1);
    }
}

std::pair<bool, float> FlowFieldGenerator::checkCollision(const std::size_t node,
                                                          const std::vector<Bounds::Rect2D*>& neighbourEntities) const
{
    const Bounds::Rect2D tileRect{m_tileGrid[node].getCenter(), m_tileGrid[node].m_dimensions * 0.5f};
    for (const Bounds::Rect2D* rect : neighbourEntities)
    {
        if (rect->collideRect(tileRect))
        {
            const float intersect{rect->calcOverlap(tileRect)};
            return {true, intersect / (tileRect.m_extents.x * tileRect.m_extents.y * 4.f)};
        }
    }

    return {false, 0.0f};
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
        m_tileGrid[m_tileGrid.size() - 1].m_leafPos = i;
        m_tileGrid[m_tileGrid.size() - 1].m_parent = node;
        m_tileGrid[m_tileGrid.size() - 1].m_hasParent = true;
    }
    m_tileGrid[node].m_hasChildren = true;
}

void FlowFieldGenerator::getNode(const glm::vec2& pos, std::size_t* node, bool* success) const
{
    const int gridX{static_cast<int>(std::floor(pos.x / CST::FLOW_FIELD_TILE_SIZE)) - m_center.x + m_extents.x};
    const int gridY{static_cast<int>(std::floor(pos.y / CST::FLOW_FIELD_TILE_SIZE)) - m_center.y + m_extents.y};

    if (gridX < 0 || gridY < 0 || gridX >= m_extents.x * 2 || gridY >= m_extents.y * 2)
    {
        *success = false;
        return;
    }

    const std::size_t root{m_baseNodes[gridY * m_extents.x * 2 + gridX]};
    *node = getClosestChild(pos, root);
    *success = true;
}

std::size_t FlowFieldGenerator::getClosestChild(const glm::vec2& pos, const std::size_t node) const
{
    if (!m_tileGrid[node].m_hasChildren)
    {
        return node;
    }

    const TileNode& tile{m_tileGrid[node]};
    if (pos.x > tile.getCenter().x)
    {
        if (pos.y > tile.getCenter().y)
        {
            return getClosestChild(pos, tile.m_children[3]);
        }
        return getClosestChild(pos, tile.m_children[1]);
    }
    if (pos.y > tile.getCenter().y)
    {
        return getClosestChild(pos, tile.m_children[2]);
    }
    return getClosestChild(pos, tile.m_children[0]);
}

void FlowFieldGenerator::getEdgeChildren(std::size_t node, std::pair<std::size_t, std::size_t>& children,
                                         const Direction direction) const
{
    if (!m_tileGrid[node].m_hasChildren)
    {
        children = {0, 0};
        return;
    }

    const TileNode& tile{m_tileGrid[node]};
    switch (direction)
    {
    case Direction::NORTH:
        children = {tile.m_children[0], tile.m_children[1]};
        return;
    case Direction::SOUTH:
        children = {tile.m_children[2], tile.m_children[3]};
        return;
    case Direction::EAST:
        children = {tile.m_children[1], tile.m_children[3]};
        return;
    case Direction::WEST:
        children = {tile.m_children[0], tile.m_children[2]};
        return;
    }
}


void FlowFieldGenerator::calculateFlowField(const glm::vec2& pos, bool* success)
{
    const int gridX{static_cast<int>(std::floor(pos.x / CST::FLOW_FIELD_TILE_SIZE)) - m_center.x + m_extents.x};
    const int gridY{static_cast<int>(std::floor(pos.y / CST::FLOW_FIELD_TILE_SIZE)) - m_center.y + m_extents.y};

    if (gridX < 0 || gridY < 0 || gridX >= m_extents.x * 2 || gridY >= m_extents.y * 2)
    {
        *success = false;
        return;
    }

    const std::size_t root{m_baseNodes[gridY * m_extents.x * 2 + gridX]};
    *success = true;
}

// NOTE: Don't call for edge leaves
std::size_t FlowFieldGenerator::findNeighbour(const std::size_t node, const Direction direction) const
{
    // we reached a base node
    const TileNode& tile{m_tileGrid[node]};
    if (!tile.m_hasParent)
    {
        int gridX{static_cast<int>(std::floor(tile.getCenter().x / CST::FLOW_FIELD_TILE_SIZE)) - m_center.x +
                  m_extents.x};
        int gridY{static_cast<int>(std::floor(tile.getCenter().y / CST::FLOW_FIELD_TILE_SIZE)) - m_center.y +
                  m_extents.y};

        switch (direction)
        {
        case Direction::NORTH:
            gridY -= 1;
            break;
        case Direction::SOUTH:
            gridY += 1;
            break;
        case Direction::EAST:
            gridX += 1;
            break;
        case Direction::WEST:
            gridX -= 1;
            break;
        }

        if (gridX < 0 || gridY < 0 || gridX >= m_extents.x * 2 || gridY >= m_extents.y * 2)
        {
            Util::beginError();
            std::cout << "FLOW_FIELD_GENERATOR::FIND_NEIGHBOUR::ERROR: Base node doesn't exist!" << std::endl;
            Util::endError();
            return 0;
        }

        const std::size_t root{m_baseNodes[gridY * m_extents.x * 2 + gridX]};
        return root;
    }

    if (direction == Direction::NORTH)
    {
        if (tile.m_leafPos < 2)
        {
            return findNeighbour(tile.m_parent, direction);
        }
        return (tile.m_leafPos == 2) ? m_tileGrid[tile.m_parent].m_children[0]
                                     : m_tileGrid[tile.m_parent].m_children[1];
    }
    else if (direction == Direction::SOUTH)
    {
        if (tile.m_leafPos >= 2)
        {
            return findNeighbour(tile.m_parent, direction);
        }
        return (tile.m_leafPos == 0) ? m_tileGrid[tile.m_parent].m_children[2]
                                     : m_tileGrid[tile.m_parent].m_children[3];
    }
    else if (direction == Direction::EAST)
    {
        if (tile.m_leafPos == 1 || tile.m_leafPos == 3)
        {
            return findNeighbour(tile.m_parent, direction);
        }
        return m_tileGrid[tile.m_parent].m_children[tile.m_leafPos + 1];
    }
    // must be west
    assert(direction == Direction::WEST);
    if (tile.m_leafPos == 0 || tile.m_leafPos == 2)
    {
        return findNeighbour(tile.m_parent, direction);
    }
    return m_tileGrid[tile.m_parent].m_children[tile.m_leafPos - 1];
}

void FlowFieldGenerator::printQuadTree() const
{
    for (std::size_t i{0}; i < m_tileGrid.size(); ++i)
    {
        const TileNode* node{&m_tileGrid[i]};
        std::cout << std::boolalpha << "{'pos': [" << node->m_position.x << ", " << node->m_position.y
                  << "], 'dimensions': [" << node->m_dimensions.x << ", " << node->m_dimensions.y
                  << "], 'solid': " << node->m_solid << "},\n";
    }
    std::cout << std::endl;
}
