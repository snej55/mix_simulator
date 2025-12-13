#ifndef SCENE_H
#define SCENE_H

#include <unordered_map>
#include "bounds.hpp"
#include "engine_types.hpp"
#include "entity.hpp"
#include "spatial_hashing.hpp"

class SceneChunk
{
public:
    explicit SceneChunk(const glm::vec3& pos);
    SceneChunk(const glm::vec3& pos, const std::vector<Entity*>& entities);

    ~SceneChunk();

    // update entities in chunk and return entities that have exited chunk (non-static)
    void updateEntities(float deltaTime, std::vector<Entity*>& discardEntities);

    void removeEntity(std::size_t index);

    void addEntity(Entity* entity);

    void getVisible(const Bounds::Frustum& camFrustum, bool& visible) const;

    // getters
    [[nodiscard]] const glm::vec3& getPos() const { return m_pos; }
    [[nodiscard]] const std::vector<Entity*>& getEntities() const { return m_entities; }

private:
    glm::vec3 m_pos;
    std::vector<Entity*> m_entities{};
    std::unique_ptr<Bounds::AABB> m_aabb{nullptr};

    void init();
};

class Scene final : public EngineObject
{
public:
    explicit Scene(void* engine);
    ~Scene() override;

    // load entities from json
    bool init(const char* scenePath);

    void updateEntities(float deltaTime);

    void getVisibleChunks(const Bounds::Frustum& camFrustum, const Bounds::AABB& frustumBV,
                          std::vector<SceneChunk*>& chunks);

    void cleanupEmptyChunks();

    // free all entities
    void free();

    void addEntity(const char* modelPath, const Bounds::Transform& transform, bool animated = false);
    void addEntity(Entity* entity);

private:
    void* m_engine{nullptr};

    std::unordered_map<SpatialHashing::ChunkKey, std::unique_ptr<SceneChunk>, SpatialHashing::ChunkKeyHasher>
        m_chunks{};

    void generateChunks();
    SpatialHashing::ChunkKey getChunkKey(const glm::vec3& pos) const;
};

#endif // SCENE_H
