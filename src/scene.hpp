#ifndef SCENE_H
#define SCENE_H

#include "engine_types.hpp"
#include "entity.hpp"

class Scene final : public EngineObject
{
public:
    explicit Scene(EngineObject* parent);
    ~Scene() override;

    // load entities from json
    bool init(const char* scenePath);

    // free all entities
    void free();

    void addEntity(Entity* entity);

private:
    std::vector<Entity*> m_entities{};
};

#endif // SCENE_H
