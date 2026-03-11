// Created by Jens Kromdijk 07/03/2026

#include "core/entity.hpp"

#include "pathfinding.hpp"
#include "player.hpp"
#include "shards.hpp"

#include <memory>

struct Enemy
{
    Entity* m_entity;
    std::string m_name;
    std::unique_ptr<ShardBody> m_shardBody{nullptr};
    Player* m_player{nullptr};
    FlowFieldGenerator* m_flowField{nullptr};
    JPH::BodyInterface* m_bodyInterface{nullptr};

    Enemy(Entity* entity, const char* name);

    void update();
    void setupShards(const char* shardsFolder, void* engine);
};
