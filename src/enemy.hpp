// Created by Jens Kromdijk 07/03/2026

#include "core/entity.hpp"

#include "pathfinding.hpp"
#include "player.hpp"

struct Enemy
{
    Entity* m_entity;
    Player* m_player{nullptr};
    FlowFieldGenerator* m_flowField{nullptr};
    JPH::BodyInterface* m_bodyInterface{nullptr};

    Enemy(Entity* entity);
    void update();
};
