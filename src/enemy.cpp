// Created by Jens Kromdijk 07/03/2026

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/Shape/SubShapeIDPair.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "particles.hpp"
#include "shards.hpp"
#include "enemy.hpp"
#include "core/engine.hpp"

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

void Enemy::explode(Scene* scene, const float force, void* manager)
{
    if (m_shardBody == nullptr || m_shardBody->getBroken())
    {
        return;
    }

    std::vector<Entity*> shards;
    m_shardBody->explode(force, shards);
    for (std::size_t s{0}; s < shards.size(); ++s)
    {
        if (manager != nullptr)
        {
            static_cast<EnemyManager*>(manager)->addShard(shards[s]);
        }
        scene->addEntity(shards[s]);
    }
    std::cout << std::endl;
}

EnemyManager::EnemyManager(Player* player, Scene* scene, FlowFieldGenerator* flowField,
                           JPH::BodyInterface* bodyInterface, void* engine) :
    m_player{player}, m_scene{scene}, m_flowField{flowField}, m_bodyInterface{bodyInterface}, m_engine{engine}
{
    m_listener.setManager(this);
    getEnemies();
}

void EnemyManager::update(const float dt, ParticleManager* particles)
{
    for (std::size_t s{0}; s < m_shards.size(); ++s)
    {
        m_shards[s].first += dt;
        if (m_shards[s].first > 60.0)
        {
            m_shards[s].second->setKill(true);
        }

        if (m_shards[s].second->getKill())
        {
            m_shards[s].second->setKill(true);
            std::swap(m_shards[0], m_shards[s]);
            m_shards.pop_back();
            --s;
        }
    }
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

        if (enemy.m_shouldExplode)
        {
            enemy.explode(m_scene, 10000.f, this);
            enemy.m_shouldExplode = false;
            static_cast<Engine*>(m_engine)->setScreenShake(16.f);

            const std::size_t amount{static_cast<std::size_t>(Util::random() * 30.f + 50.f)};
            for (std::size_t i{0}; i < amount; ++i)
            {
                glm::vec3 direction{Util::random() * 2.f - 1.f, Util::random() * 2.f - 1.f, Util::random() * 2.f - 1.f};
                direction = glm::normalize(direction);
                const float speed{1.f + Util::random() * 0.5f - 0.25f};
                particles->addParticle(enemy.m_entity->getGlobalMidpoint(), direction * speed, Util::random());
            }
        }
        enemy.update();
    }

    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(), [](const Enemy& e) { return e.m_entity->getKill(); }),
        m_enemies.end());
}

void EnemyManager::handleCollision(const JPH::BodyID& body1, const JPH::BodyID& body2)
{
    for (std::size_t i{0}; i < m_enemies.size(); ++i)
    {
        Enemy& enemy{m_enemies[i]};
        const JPH::BodyID& enemyID{enemy.m_entity->getPhysicsBody()->getBodyID()};
        const JPH::BodyID& playerID{m_player->getEntity()->getPhysicsBody()->getBodyID()};
        // if (enemyID == body1 || enemyID == body2)
        if ((enemyID == body1 && playerID == body2) || (enemyID == body2 && playerID == body1))
        {
            enemy.m_shouldExplode = true;
        }
    }
}

void EnemyManager::addShard(Entity* entity) { m_shards.emplace_back(std::pair{0.0, entity}); }

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
    else if (name == "MODEL bud_vase")
    {
        enemy.setupShards("data/models/bud_vase_shards/", m_engine);
    }
    else if (name == "MODEL cider_jug")
    {
        enemy.setupShards("data/models/cider_shards/", m_engine);
    }
}

std::string EnemyManager::getEntityName(Entity* entity) const
{
    std::vector<std::string> words{};
    words.reserve(2);
    Util::splitStr(entity->getModel()->getName(), words, ' ');
    return words[1];
}

// Jolt contact listener
JPH::ValidateResult EnemyListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                                     JPH::RVec3Arg inBaseOffset,
                                                     const JPH::CollideShapeResult& inCollisionResult)
{
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void EnemyListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                   const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
    JPH::RVec3 pos{inManifold.GetWorldSpaceContactPointOn1(0)};
    JPH::Vec3 vel1{inBody1.IsStatic() ? JPH::Vec3::sZero() : inBody1.GetPointVelocity(pos)};
    JPH::Vec3 vel2{inBody2.IsStatic() ? JPH::Vec3::sZero() : inBody2.GetPointVelocity(pos)};

    JPH::Vec3 relVel{vel1 - vel2};
    const float impactSpeed{std::abs(relVel.Dot(inManifold.mWorldSpaceNormal))};

    if (impactSpeed > 20.0f)
    {
        static_cast<EnemyManager*>(m_manager)->handleCollision(inBody1.GetID(), inBody2.GetID());
    }
}

void EnemyListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                       const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
}

void EnemyListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) {}
