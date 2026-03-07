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
    assert(m_tileGrid.empty());

    // initialize tile grid
    const int width{static_cast<int>(static_cast<float>(m_extents.x * 2))};
    const int height{static_cast<int>(static_cast<float>(m_extents.y * 2))};
    m_numTiles = width * height;

    std::cout << "FLOW_FIELD_GENERATOR::INIT: Generating quadtrees!" << std::endl;

    m_tileGrid.reserve(static_cast<std::size_t>(width * height * std::pow(2, CST::FLOW_FIELD_DEPTH_LIMIT)));
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
    std::cout << "FLOW_FIELD_GENERATOR::INIT: Initialized quadtrees!" << std::endl;

    for (std::size_t i{0}; i < m_tileGrid.size(); ++i)
    {
        if (!m_tileGrid[i].m_hasChildren && !m_tileGrid[i].m_solid)
        {
            calculateNeighbours(i, Direction::NORTH);
            calculateNeighbours(i, Direction::SOUTH);
            calculateNeighbours(i, Direction::EAST);
            calculateNeighbours(i, Direction::WEST);
        }
    }
    std::cout << "FLOW_FIELD_GENERATOR::INIT: Calculated neighbours!" << std::endl;

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
            m_tileGrid.emplace_back(parent.m_position, parent.m_dimensions * 0.5f);
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

void FlowFieldGenerator::calculateFlowField(const bool constrainEdges, const float maxDistance)
{
    std::size_t startNode;
    bool success{false};
    getNode(m_playerPos, &startNode, &success, constrainEdges);
    if (!success || startNode == m_playerNode || m_tileGrid[startNode].m_solid)
    {
        // keep same flow field as before
        return;
    }
    m_playerNode = startNode;

    clearFlowField();
    m_tileGrid[startNode].m_cost = 0.0f;

    // propagate flow field
    std::vector<std::size_t> wave{startNode};
    std::vector<std::size_t> newWave{};
    newWave.reserve(m_tileGrid.size());

    while (!wave.empty())
    {
        for (std::size_t i{0}; i < wave.size(); ++i)
        {
            const std::size_t node{wave[i]};
            if (maxDistance > 0.0f && m_tileGrid[node].m_cost >= maxDistance)
                continue;

            for (const std::pair<std::size_t, Direction>& neighbour : m_tileGrid[node].m_neighbours)
            {
                // use manhattan distance
                const float cost{
                    m_tileGrid[node].m_cost +
                    (std::abs(m_tileGrid[neighbour.first].getCenter().x - m_tileGrid[node].getCenter().x) +
                     std::abs(m_tileGrid[neighbour.first].getCenter().y - m_tileGrid[node].getCenter().y)) /
                        CST::FLOW_FIELD_TILE_SIZE};

                if (cost < m_tileGrid[neighbour.first].m_cost)
                {
                    m_tileGrid[neighbour.first].m_cost = cost;
                    newWave.push_back(neighbour.first);
                }
            }
        }

        std::swap(newWave, wave);
        newWave.clear();
    }

    for (std::size_t i{0}; i < m_tileGrid.size(); ++i)
    {
        TileNode& tile{m_tileGrid[i]};
        if (!tile.m_solid && !tile.m_hasChildren)
        {
            std::pair<float, std::size_t> minCost{tile.m_cost, i};
            for (const std::pair<std::size_t, Direction>& neighbour : tile.m_neighbours)
            {
                if (m_tileGrid[neighbour.first].m_cost < minCost.first)
                {
                    minCost.first = m_tileGrid[neighbour.first].m_cost;
                    minCost.second = neighbour.first;
                }
            }

            tile.m_target = minCost.second;
            const glm::vec2 tileCenter{tile.getCenter()};
            const glm::vec2 centerPos{m_tileGrid[minCost.second].getCenter()};
            tile.m_direction = glm::vec2{tileCenter.x - centerPos.x, tileCenter.y - centerPos.y};
        }
    }
}

void FlowFieldGenerator::setPlayerPos(const glm::vec2& val) { m_playerPos = val; }

void FlowFieldGenerator::clearFlowField()
{
    for (std::size_t i{0}; i < m_tileGrid.size(); ++i)
    {
        m_tileGrid[i].m_cost = std::numeric_limits<float>::max();
        m_tileGrid[i].m_direction = glm::vec2{0.0f, 0.0f};
    }
}

void FlowFieldGenerator::getNode(const glm::vec2& pos, std::size_t* node, bool* success,
                                 const bool constrainEdges) const
{
    assert(success != nullptr || constrainEdges);
    int gridX{static_cast<int>(std::floor(pos.x / CST::FLOW_FIELD_TILE_SIZE)) - m_center.x + m_extents.x};
    int gridY{static_cast<int>(std::floor(pos.y / CST::FLOW_FIELD_TILE_SIZE)) - m_center.y + m_extents.y};

    if (!constrainEdges)
    {
        if (gridX < 0 || gridY < 0 || gridX >= m_extents.x * 2 || gridY >= m_extents.y * 2)
        {
            *success = false;
            return;
        }
    }
    else
    {
        gridX = std::clamp(gridX, 0, m_extents.x * 2 - 1);
        gridY = std::clamp(gridY, 0, m_extents.y * 2 - 1);
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

void FlowFieldGenerator::getEdgeChildren(const std::size_t node, std::pair<std::size_t, std::size_t>& children,
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
    }
}

void FlowFieldGenerator::getSubEdges(const std::size_t node, std::vector<std::size_t>& edges,
                                     const Direction direction) const
{
    if (!m_tileGrid[node].m_hasChildren)
    {
        edges.push_back(node);
        return;
    }
    std::pair<std::size_t, std::size_t> children;
    getEdgeChildren(node, children, direction);

    getSubEdges(children.first, edges, direction);
    getSubEdges(children.second, edges, direction);
}

void FlowFieldGenerator::calculateNeighbours(const std::size_t node, const Direction direction)
{
    const TileNode& tile{m_tileGrid[node]};

    // ok to search for neighbour in this direction
    bool onEdge{false};
    const std::size_t neighbour{findNeighbour(node, direction, onEdge)};
    if (onEdge)
    {
        return;
    }

    std::vector<std::size_t> edges{};
    getSubEdges(neighbour, edges, getOpposite(direction));

    const Bounds::Rect2D tileRect{tile.getCenter(), tile.m_dimensions * 0.5f};
    const bool x{direction == Direction::NORTH || direction == Direction::SOUTH};
    for (std::size_t i{0}; i < edges.size(); ++i)
    {
        const TileNode& edge{m_tileGrid[edges[i]]};
        if (edge.m_solid)
            continue;

        const Bounds::Rect2D edgeRect{edge.getCenter(), edge.m_dimensions * 0.49f};
        if (tileRect.collideAxis(edgeRect, x))
        {
            m_tileGrid[node].m_neighbours.emplace_back(edges[i], direction);
        }
    }
}

std::size_t FlowFieldGenerator::findNeighbour(const std::size_t node, const Direction direction, bool& edge) const
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
            edge = true;
            return 0;
        }

        edge = false;
        const std::size_t root{m_baseNodes[gridY * m_extents.x * 2 + gridX]};
        return root;
    }

    edge = false;
    if (direction == Direction::NORTH)
    {
        if (tile.m_leafPos < 2)
        {
            return findNeighbour(tile.m_parent, direction, edge);
        }
        return (tile.m_leafPos == 2) ? m_tileGrid[tile.m_parent].m_children[0]
                                     : m_tileGrid[tile.m_parent].m_children[1];
    }
    if (direction == Direction::SOUTH)
    {
        if (tile.m_leafPos >= 2)
        {
            return findNeighbour(tile.m_parent, direction, edge);
        }
        return tile.m_leafPos == 0 ? m_tileGrid[tile.m_parent].m_children[2] : m_tileGrid[tile.m_parent].m_children[3];
    }
    if (direction == Direction::EAST)
    {
        if (tile.m_leafPos == 1 || tile.m_leafPos == 3)
        {
            return findNeighbour(tile.m_parent, direction, edge);
        }
        return m_tileGrid[tile.m_parent].m_children[tile.m_leafPos + 1];
    }
    // must be west
    assert(direction == Direction::WEST);
    if (tile.m_leafPos == 0 || tile.m_leafPos == 2)
    {
        return findNeighbour(tile.m_parent, direction, edge);
    }

    return m_tileGrid[tile.m_parent].m_children[tile.m_leafPos - 1];
}

void FlowFieldGenerator::printQuadTree() const
{
    for (std::size_t i{0}; i < m_tileGrid.size(); ++i)
    {
        printNode(i);
    }
    std::cout << std::endl;
}

void FlowFieldGenerator::printNode(const std::size_t node) const
{
    const TileNode& tile{m_tileGrid[node]};
    std::cout << std::boolalpha << "{'pos': [" << tile.m_position.x << ", " << tile.m_position.y << "], 'dimensions': ["
              << tile.m_dimensions.x << ", " << tile.m_dimensions.y << "], 'solid': " << tile.m_solid
              << ", 'hasChildren': " << tile.m_hasChildren << ", 'neighbours': [";
    for (std::size_t i{0}; i < tile.m_neighbours.size(); ++i)
    {
        std::cout << tile.m_neighbours[i].first;
        if (i != tile.m_neighbours.size() - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "], 'cost': " << tile.m_cost << ", 'direction': [" << tile.m_direction.x << ", " << tile.m_direction.y
              << "], 'target': " << tile.m_target << "},\n";
}
