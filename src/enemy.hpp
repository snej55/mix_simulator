// Created by Jens Kromdijk 07/03/2026

#include "core/entity.hpp"

#include "pathfinding.hpp"
#include "player.hpp"

struct EnemyArgs
{
    Player* m_player{nullptr};
    FlowFieldGenerator* m_flowField{nullptr};
    JPH::BodyInterface* m_bodyInterface{nullptr};
};

struct Enemy : public EntityController
{
    EnemyArgs m_args{};

    Enemy(void* entity);
    Enemy(void* entity, EnemyArgs args);
    void update();
};
