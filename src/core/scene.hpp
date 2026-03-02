#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <ostream>
#include <unordered_map>

#include "bounds.hpp"
#include "engine_types.hpp"
#include "entity.hpp"
#include "spatial_hashing.hpp"
#include "physics.hpp"
#include "lights.hpp"

class SceneChunk
{
public:
    explicit SceneChunk(const glm::vec3& pos);
    SceneChunk(const glm::vec3& pos, const std::vector<Entity*>& entities);

    ~SceneChunk() = default;

    // update entities in chunk and return entities that have exited chunk (non-static)
    void updateEntities(float deltaTime, std::vector<Entity*>& discardEntities, JoltInstance* jolt = nullptr);

    void removeEntity(std::size_t index);
    void eraseEntity(std::size_t index);
    void clearErasedEntities();

    void addEntity(Entity* entity);

    void getVisible(const Bounds::Frustum& camFrustum, bool& visible) const;

    // getters
    [[nodiscard]] const glm::vec3& getPos() const { return m_pos; }
    [[nodiscard]] const std::vector<Entity*>& getEntities() const { return m_entities; }
    [[nodiscard]] const Bounds::AABB* getAABB() const { return m_aabb.get(); }
    [[nodiscard]] std::size_t getNumEntities() const { return m_entities.size(); }
    [[nodiscard]] bool isEmpty() const { return m_entities.empty(); }

private:
    glm::vec3 m_pos;
    std::vector<Entity*> m_entities{};
    std::unique_ptr<Bounds::AABB> m_aabb{nullptr};

    void init();
};

inline std::ostream& operator<<(std::ostream& os, const SceneChunk& chunk)
{
    os << "SceneChunk(Position: (" << chunk.getPos().x << ", " << chunk.getPos().y << ", " << chunk.getPos().z
       << "), EntityCount: " << chunk.getEntities().size() << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const SceneChunk* chunk)
{
    os << "SceneChunk(Position: (" << chunk->getPos().x << ", " << chunk->getPos().y << ", " << chunk->getPos().z
       << "), EntityCount: " << chunk->getEntities().size() << ")";
    return os;
}

class Scene final : public EngineObject
{
public:
    explicit Scene(void* engine);
    ~Scene() override;

    // load entities from json
    bool init(const char* scenePath);
    void initPhysicsBodies(JoltInstance* jolt);

    void resetEntityFlags(); // call this before updating entities
    void updateEntities(float deltaTime, JoltInstance* jolt = nullptr);

    void getVisibleChunks(const Bounds::Frustum& camFrustum, const Bounds::AABB& frustumBV,
                          std::vector<SceneChunk*>& chunks);

    void cleanupEmptyChunks();

    // free all entities
    void free();

    void addEntity(const char* modelPath, const Bounds::Transform& transform, const BodyType& bodyType,
                   bool animated = false);
    void addEntity(Entity* entity);

    [[nodiscard]] const auto& getChunks() const { return m_chunks; }
    void getShadowModels(std::vector<std::pair<Model*, glm::mat4>>& models) const;

    void addPointLight(const glm::vec3& position, const glm::vec3& color, float radius);
    void getVisiblePointLights(const Bounds::AABB& frustumBV, std::vector<Lights::PointLight*>& pointLights);
    [[nodiscard]] const std::vector<std::unique_ptr<Lights::PointLight>>& getPointLights() const
    {
        return m_pointLights;
    }

    void getPointLights(std::vector<Lights::PointLight*>& pointLights) const;

    [[nodiscard]] const glm::vec3& getLevelExtents() const { return m_levelExtents; }
    [[nodiscard]] const glm::vec3& getLevelCenter() const { return m_levelCenter; }

    void getStaticEntities(std::vector<Entity*>& entities);
    void getStaticRects(std::vector<Bounds::Rect2D>& rects);

private:
    void* m_engine{nullptr};

    std::unordered_map<SpatialHashing::ChunkKey, std::unique_ptr<SceneChunk>, SpatialHashing::ChunkKeyHasher>
        m_chunks{};
    std::vector<std::unique_ptr<Lights::PointLight>> m_pointLights{};

    glm::vec3 m_levelExtents{0.0f};
    glm::vec3 m_levelCenter{0.0f};
    void calculateLevelDimensions();

    static SpatialHashing::ChunkKey getChunkKey(const glm::vec3& pos);
};

#endif // SCENE_H
