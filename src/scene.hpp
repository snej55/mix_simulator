#ifndef SCENE_H
#define SCENE_H

#include "engine_types.hpp"
#include "entity.hpp"

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
