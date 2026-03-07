// Created by Jens Kromdijk 07/03/2026

#include "enemy.hpp"

Enemy::Enemy(void* entity) : EntityController(entity) {}
Enemy::Enemy(void* entity, EnemyArgs args) : EntityController(entity), m_args{args} {}

void Enemy::update()
{
    assert(m_args.m_player != nullptr && m_args.m_flowField != nullptr && m_args.m_bodyInterface != nullptr);
    Entity* entity{static_cast<Entity*>(m_entity)};
    const JPH::BodyID& bodyID{entity->getPhysicsBody()->getBodyID()};
    const glm::vec3 midPoint{entity->getGlobalMidpoint()};

    std::size_t node;
    m_args.m_flowField->getNode({midPoint.x, midPoint.z}, &node, nullptr, true);

    const TileNode& tile{m_args.m_flowField->getTile(node)};
    glm::vec2 direction{0.0f, 0.0f};
    if (tile.m_solid || tile.m_direction.length() < 0.01f)
    {
        direction = glm::vec2{m_args.m_player->getEntity()->getGlobalMidpoint().x - midPoint.x,
                              m_args.m_player->getEntity()->getGlobalMidpoint().z - midPoint.z};
    }
    else
    {
        direction = tile.m_direction;
    }

    direction.x = std::clamp(direction.x, -1.f, 1.f);
    direction.y = std::clamp(direction.y, -1.f, 1.f);
    direction = -direction; // dunno why it needs to be negative, just works like that

    constexpr float speed{100000.0f};
    constexpr float torque{50000.f};
    m_args.m_bodyInterface->AddForceAndTorque(bodyID, {direction.x * speed, 0.f, direction.y * speed},
                                              {direction.x * torque, 0.f, direction.y * torque});
}
