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
    void explode(Scene* scene, float force);
};

class EnemyManager
{
public:
    EnemyManager(Player* player, Scene* scene, FlowFieldGenerator* flowField, JPH::BodyInterface* bodyInterface,
                 void* engine);

    void update();

private:
    Player* m_player;
    Scene* m_scene;
    FlowFieldGenerator* m_flowField;
    JPH::BodyInterface* m_bodyInterface;
    void* m_engine;

    std::vector<Enemy> m_enemies{};

    void getEnemies();
    void setupShards(Enemy& enemy);
    [[nodiscard]] std::string getEntityName(Entity* entity) const;
};
