#ifndef SCENE_H
#define SCENE_H

#include "engine_types.hpp"
#include "entity.hpp"

inline constexpr float CHUNK_SIZE{24.f};

class SceneChunk
{
public:
    explicit SceneChunk(const glm::vec3& pos);
    SceneChunk(const glm::vec3& pos, const std::vector<Entity*>& entities);

    // update entities in chunk and return entities that have exited chunk (non-static)
    void updateEntities(float deltaTime, std::vector<Entity*>& discardEntities);

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

    // free all entities
    void free();

    void addEntity(const char* modelPath, const Bounds::Transform& transform, bool animated = false);
    void addEntity(Entity* entity);

    [[nodiscard]] const std::vector<Entity*>& getEntities() const { return m_entities; }

private:
    void* m_engine{nullptr};
    std::vector<Entity*> m_entities{};
};

#endif // SCENE_H
