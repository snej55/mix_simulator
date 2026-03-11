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
    bool m_shouldExplode{false};

    Enemy(Entity* entity, const char* name);

    void update();
    void setupShards(const char* shardsFolder, void* engine);
    void explode(Scene* scene, float force);
};

class EnemyListener : public JPH::ContactListener
{
public:
    virtual JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                                  JPH::RVec3Arg inBaseOffset,
                                                  const JPH::CollideShapeResult& inCollisionResult) override;

    virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

    virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                    const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

    virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

    void setManager(void* manager) { m_manager = manager; }
    [[nodiscard]] void* getManager() { return m_manager; }

private:
    void* m_manager{nullptr};
};

class EnemyManager
{
public:
    EnemyManager(Player* player, Scene* scene, FlowFieldGenerator* flowField, JPH::BodyInterface* bodyInterface,
                 void* engine);

    void update();

    void handleCollision(const JPH::BodyID& body1, const JPH::BodyID& body2);

    [[nodiscard]] const std::vector<Enemy>& getEnemiesVec() const { return m_enemies; }
    [[nodiscard]] EnemyListener* getListener() { return &m_listener; }

private:
    Player* m_player;
    Scene* m_scene;
    FlowFieldGenerator* m_flowField;
    JPH::BodyInterface* m_bodyInterface;
    void* m_engine;

    std::vector<Enemy> m_enemies{};
    EnemyListener m_listener{};

    void getEnemies();
    void setupShards(Enemy& enemy);
    [[nodiscard]] std::string getEntityName(Entity* entity) const;
};
