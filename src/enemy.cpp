// Created by Jens Kromdijk 07/03/2026

#include "enemy.hpp"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "shards.hpp"

Enemy::Enemy(Entity* entity, const char* name) : m_entity{entity}, m_name{name} {}

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

void Enemy::setupShards(const char* shardsFolder, void* engine)
{
    m_shardBody = std::make_unique<ShardBody>(m_name.c_str(), shardsFolder, m_entity, m_bodyInterface);
    m_shardBody->init(engine);
}

void Enemy::explode(Scene* scene, const float force)
{
    if (m_shardBody == nullptr || m_shardBody->getBroken())
    {
        return;
    }

    std::vector<Entity*> shards;
    m_shardBody->explode(force, shards);
    for (std::size_t s{0}; s < shards.size(); ++s)
    {
        scene->addEntity(shards[s]);
    }
}

EnemyManager::EnemyManager(Player* player, Scene* scene, FlowFieldGenerator* flowField,
                           JPH::BodyInterface* bodyInterface, void* engine) :
    m_player{player}, m_scene{scene}, m_flowField{flowField}, m_bodyInterface{bodyInterface}, m_engine{engine}
{
    getEnemies();
}

void EnemyManager::update()
{
    for (std::size_t i{0}; i < m_enemies.size(); ++i)
    {
        Enemy& enemy{m_enemies[i]};
        if (enemy.m_entity->getKill())
        {
            std::swap(m_enemies[m_enemies.size() - 1], m_enemies[i]);
            m_enemies.pop_back();
            --i;
            continue;
        }
        enemy.update();
    }

    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(), [](const Enemy& e) { return e.m_entity->getKill(); }),
        m_enemies.end());
}

void EnemyManager::getEnemies()
{
    m_enemies.clear();
    m_enemies.reserve(64);
    std::vector<Entity*> dynamicEntities{};
    m_scene->getDynamicEntities(dynamicEntities);

    for (std::size_t i{0}; i < dynamicEntities.size(); ++i)
    {
        if (dynamicEntities[i] != m_player->getEntity())
        {
            Enemy enemy{dynamicEntities[i], getEntityName(dynamicEntities[i]).c_str()};
            enemy.m_player = m_player;
            enemy.m_flowField = m_flowField;
            enemy.m_bodyInterface = m_bodyInterface;
            setupShards(enemy);

            m_enemies.emplace_back(std::move(enemy));
        }
    }
}

void EnemyManager::setupShards(Enemy& enemy)
{
    std::string_view name{enemy.m_entity->getModel()->getName()};
    if (name == "MODEL mug")
    {
        enemy.setupShards("data/models/mug_shards/", m_engine);
    }
}

std::string EnemyManager::getEntityName(Entity* entity) const
{
    std::vector<std::string> words{};
    words.reserve(2);
    Util::splitStr(entity->getModel()->getName(), words, ' ');
    return words[1];
}
