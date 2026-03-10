// Created by Jens Kromdijk 07/03/2026

#include "enemy.hpp"
#include "shards.hpp"

Enemy::Enemy(Entity* entity, const char* name) : m_entity{entity}, m_name{name} {}

Enemy::Enemy(const Enemy& other) :
    m_entity{other.m_entity}, m_name{other.m_name}, m_shardBody{other.m_shardBody}, m_player{other.m_player},
    m_flowField{other.m_flowField}, m_bodyInterface{other.m_bodyInterface}
{
}

void Enemy::update()
{
    assert(m_player != nullptr && m_flowField != nullptr && m_bodyInterface != nullptr);
    const JPH::BodyID& bodyID{m_entity->getPhysicsBody()->getBodyID()};
    const glm::vec3 midPoint{m_entity->getGlobalMidpoint()};

    std::size_t node;
    bool success{false};
    m_flowField->getNode({midPoint.x, midPoint.z}, &node, &success, true);

    const TileNode& tile{m_flowField->getTile(node)};
    glm::vec2 direction{0.0f, 0.0f};
    if (tile.m_solid || tile.m_direction.length() < 0.01f)
    {
        direction = glm::vec2{m_player->getEntity()->getGlobalMidpoint().x - midPoint.x,
                              m_player->getEntity()->getGlobalMidpoint().z - midPoint.z};
    }
    else
    {
        direction = tile.m_direction;
    }

    direction.x = std::clamp(direction.x, -1.f, 1.f);
    direction.y = std::clamp(direction.y, -1.f, 1.f);
    direction = -direction; // dunno why it needs to be negative, just works like that

    constexpr float speed{50000.0f};
    constexpr float torque{1000.f};
    m_bodyInterface->AddForceAndTorque(bodyID, {direction.x * speed, 0.f, direction.y * speed},
                                       {direction.x * torque, 0.f, direction.y * torque});
}

void Enemy::setupShards(const char* shardsFolder)
{
    m_shardBody = new ShardBody{m_name.c_str(), shardsFolder, m_entity, m_bodyInterface};
}

void Enemy::free()
{
    delete m_shardBody;
    m_shardBody = nullptr;
}
